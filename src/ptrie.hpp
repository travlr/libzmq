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

#ifndef __ZMQ_PTRIE_HPP_INCLUDED__
#define __ZMQ_PTRIE_HPP_INCLUDED__

#include <stddef.h>
#include <set>

#include "stdint.hpp"

namespace zmq
{

    class ptrie_t
    {
    public:

        typedef std::set <class writer_t*> pipes_t;

        ptrie_t ();
        ~ptrie_t ();

        //  Add key to the trie.
        bool add (unsigned char *prefix_, size_t size_, class writer_t *pipe_);

        //  Remove all subscriptions for a specific peer from the trie.
        void rm (class writer_t *pipe_);

        //  Remove specific subscription from the trie.
        bool rm (unsigned char *prefix_, size_t size_, class writer_t *pipe_);

        //  Get all matching pipes.
        void match (unsigned char *data_, size_t size_, pipes_t &pipes_);

    private:

        pipes_t pipes;
        unsigned char min;
        unsigned short count;
        union {
            class ptrie_t *node;
            class ptrie_t **table;
        } next;

        ptrie_t (const ptrie_t&);
        const ptrie_t &operator = (const ptrie_t&);
    };

}

#endif

