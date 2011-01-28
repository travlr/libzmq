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

#ifndef __ZMQ_MATCHER_HPP_INCLUDED__
#define __ZMQ_MATCHER_HPP_INCLUDED__

#include <vector>
#include <map>

#include "../include/zmq.h"

#include "blob.hpp"

namespace zmq
{

    class matcher_t
    {
    public:

        matcher_t ();
        ~matcher_t ();

        void add (const blob_t &topic_, class writer_t *writer_);
        void rm (const blob_t &topic_, class writer_t *writer_);

        //  Remove all the subscriptions associated with this peer.
        void rm (class writer_t *writer_);

        void send (zmq_msg_t *msg_);

    private:

        typedef std::vector <class writer_t*> writers_t;
        typedef std::map <blob_t, writers_t> subscriptions_t;
        subscriptions_t subscriptions;

        matcher_t (const matcher_t&);
        const matcher_t &operator = (const matcher_t&);
    };

}

#endif
