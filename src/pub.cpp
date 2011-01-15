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

#include "pub.hpp"
#include "likely.hpp"

zmq::pub_t::pub_t (class ctx_t *parent_, uint32_t tid_) :
    xpub_t (parent_, tid_)
{
    options.type = ZMQ_PUB;
}

zmq::pub_t::~pub_t ()
{
}

int zmq::pub_t::xsend (zmq_msg_t *msg_, int flags_)
{
    //  Before sending the message, process all the pending subscriptions.
    zmq_msg_t sub;
    zmq_msg_init (&sub);
    while (true) {
        int rc = xpub_t::xrecv (&sub, ZMQ_NOBLOCK);
        if (rc != 0 && errno == EAGAIN)
            break;
        if (unlikely (rc != 0)) {
            zmq_msg_close (&sub);
            return -1;
        }

        //  At this point we have the subscription. As for now, just ignore it.
printf ("(un)subscription received!\n");
    }
    zmq_msg_close (&sub);

    return xpub_t::xsend (msg_, flags_);
}

bool zmq::pub_t::xhas_out ()
{
    return xpub_t::xhas_out ();
}

int zmq::pub_t::xrecv (zmq_msg_t *msg_, int flags_)
{
    //  Overload the XPUB's recv function.
    errno = ENOTSUP;
    return -1;
}

bool zmq::pub_t::xhas_in ()
{
    return false;
}
