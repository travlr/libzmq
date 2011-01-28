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

#include "matcher.hpp"
#include "pipe.hpp"
#include "err.hpp"

zmq::matcher_t::matcher_t ()
{
}

zmq::matcher_t::~matcher_t ()
{
}

void zmq::matcher_t::add (const blob_t &topic_, writer_t *writer_)
{
    //  Create a new topic node, if not already present.
    subscriptions_t::iterator it = subscriptions.insert (
        subscriptions_t::value_type (topic_, writers_t ())).first;

    //  Register the callback with the topic.
    it->second.push_back (writer_);
}

void zmq::matcher_t::rm (const blob_t &topic_, writer_t *writer_)
{
    bool found = false;
    subscriptions_t::iterator its;
    for (its = subscriptions.begin (); its != subscriptions.end (); ++its) {
        for (writers_t::iterator itw = its->second.begin ();
              itw != its->second.end (); ++itw) {
            if (*itw == writer_) {
                found = true;
                its->second.erase (itw);
                break;
            }
        }
        if (found == true)
            break;
    }
    zmq_assert (found);
    if (its->second.empty ())
        subscriptions.erase (its);
}

void zmq::matcher_t::rm (writer_t *writer_)
{
    for (subscriptions_t::iterator its = subscriptions.begin ();
          its != subscriptions.end ();) {
        for (writers_t::iterator itw = its->second.begin ();
              itw != its->second.end ();) {
            if (*itw == writer_) {
                writers_t::iterator to_erase = itw;
                ++itw;
                its->second.erase (to_erase);
            }
            else
                ++itw;
        }
        if (its->second.empty ()) {
            subscriptions_t::iterator to_erase = its;
            ++its;
            subscriptions.erase (to_erase);
        }
        else
            ++its;
    }
}

void zmq::matcher_t::send (zmq_msg_t *msg_)
{
    for (subscriptions_t::iterator its = subscriptions.begin ();
          its != subscriptions.end (); ++its) {
        for (writers_t::iterator itw = its->second.begin ();
              itw != its->second.end (); ++itw) {
            zmq_msg_t msg;
            zmq_msg_copy (&msg, msg_);
            if ((*itw)->write (&msg))
                (*itw)->flush ();
            zmq_msg_close (&msg);
        }
    }
}
