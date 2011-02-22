/*
    Copyright (c) 2007-2010 iMatix Corporation

    This file is part of 0MQ.

    0MQ is free software; you can redistribute it and/or modify it under
    the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    0MQ is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdlib.h>

#include <new>
#include <algorithm>

#include "platform.hpp"
#if defined ZMQ_HAVE_WINDOWS
#include "windows.hpp"
#endif

#include "err.hpp"
#include "pipe.hpp"
#include "ptrie.hpp"

zmq::ptrie_t::ptrie_t () :
    min (0),
    count (0)
{
}

zmq::ptrie_t::~ptrie_t ()
{
    if (count == 1)
        delete next.node;
    else if (count > 1) {
        for (unsigned short i = 0; i != count; ++i)
            if (next.table [i])
                delete next.table [i];
        free (next.table);
    }
}

bool zmq::ptrie_t::add (unsigned char *prefix_, size_t size_, writer_t *pipe_)
{
    //  We are at the node corresponding to the prefix. We are done.
    if (!size_) {
        pipes.insert (pipe_);
        return pipes.size () == 1;
    }

    unsigned char c = *prefix_;
    if (c < min || c >= min + count) {

        //  The character is out of range of currently handled
        //  charcters. We have to extend the table.
        if (!count) {
            min = c;
            count = 1;
            next.node = NULL;
        }
        else if (count == 1) {
            unsigned char oldc = min;
            ptrie_t *oldp = next.node;
            count = (min < c ? c - min : min - c) + 1;
            next.table = (ptrie_t**)
                malloc (sizeof (ptrie_t*) * count);
            zmq_assert (next.table);
            for (unsigned short i = 0; i != count; ++i)
                next.table [i] = 0;
            min = std::min (min, c);
            next.table [oldc - min] = oldp;
        }
        else if (min < c) {

            //  The new character is above the current character range.
            unsigned short old_count = count;
            count = c - min + 1;
            next.table = (ptrie_t**) realloc ((void*) next.table,
                sizeof (ptrie_t*) * count);
            zmq_assert (next.table);
            for (unsigned short i = old_count; i != count; i++)
                next.table [i] = NULL;
        }
        else {

            //  The new character is below the current character range.
            unsigned short old_count = count;
            count = (min + old_count) - c;
            next.table = (ptrie_t**) realloc ((void*) next.table,
                sizeof (ptrie_t*) * count);
            zmq_assert (next.table);
            memmove (next.table + min - c, next.table,
                old_count * sizeof (ptrie_t*));
            for (unsigned short i = 0; i != min - c; i++)
                next.table [i] = NULL;
            min = c;
        }
    }

    //  If next node does not exist, create one.
    if (count == 1) {
        if (!next.node) {
            next.node = new (std::nothrow) ptrie_t;
            zmq_assert (next.node);
        }
        return next.node->add (prefix_ + 1, size_ - 1, pipe_);
    }
    else {
        if (!next.table [c - min]) {
            next.table [c - min] = new (std::nothrow) ptrie_t;
            zmq_assert (next.table [c - min]);
        }
        return next.table [c - min]->add (prefix_ + 1, size_ - 1, pipe_);
    }
}

void zmq::ptrie_t::rm (writer_t *pipe_)
{
    //  Remove the subscriptions from this node.
    pipes.erase (pipe_);

    //  If there are no subnodes in the trie, return.
    if (count == 0)
        return;

    //  If there's one subnode (optimisation).
    if (count == 1) {
        next.node->rm (pipe_);
        return;
    }

    //  If there are multiple subnodes.
    for (unsigned char c = 0; c != count; c++) {
        if (next.table [c])
            next.table [c]->rm (pipe_);
    }  
}

bool zmq::ptrie_t::rm (unsigned char *prefix_, size_t size_, writer_t *pipe_)
{
     if (!size_) {
         pipes_t::size_type erased = pipes.erase (pipe_);
         zmq_assert (erased == 1);
         return pipes.empty ();
     }

     unsigned char c = *prefix_;
     if (!count || c < min || c >= min + count)
         return false;

     ptrie_t *next_node =
         count == 1 ? next.node : next.table [c - min];

     if (!next_node)
         return false;

     return next_node->rm (prefix_ + 1, size_ - 1, pipe_);
}

void zmq::ptrie_t::match (unsigned char *data_, size_t size_, pipes_t &pipes_)
{
    //  Merge the subscriptions from this node to the resultset.
    pipes_.insert (pipes.begin (), pipes.end ());

    //  If there are no subnodes in the trie, return.
    if (count == 0)
        return;

    //  If there's one subnode (optimisation).
    if (count == 1) {
        next.node->match (data_ + 1, size_ - 1, pipes_);
        return;
    }

    //  If there are multiple subnodes.
    for (unsigned char c = 0; c != count; c++) {
        if (next.table [c])
            next.table [c]->match (data_ + 1, size_ - 1, pipes_);
    }   
}

