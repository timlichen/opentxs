/************************************************************
 *
 *                 OPEN TRANSACTIONS
 *
 *       Financial Cryptography and Digital Cash
 *       Library, Protocol, API, Server, CLI, GUI
 *
 *       -- Anonymous Numbered Accounts.
 *       -- Untraceable Digital Cash.
 *       -- Triple-Signed Receipts.
 *       -- Cheques, Vouchers, Transfers, Inboxes.
 *       -- Basket Currencies, Markets, Payment Plans.
 *       -- Signed, XML, Ricardian-style Contracts.
 *       -- Scripted smart contracts.
 *
 *  EMAIL:
 *  fellowtraveler@opentransactions.org
 *
 *  WEBSITE:
 *  http://www.opentransactions.org/
 *
 *  -----------------------------------------------------
 *
 *   LICENSE:
 *   This Source Code Form is subject to the terms of the
 *   Mozilla Public License, v. 2.0. If a copy of the MPL
 *   was not distributed with this file, You can obtain one
 *   at http://mozilla.org/MPL/2.0/.
 *
 *   DISCLAIMER:
 *   This program is distributed in the hope that it will
 *   be useful, but WITHOUT ANY WARRANTY; without even the
 *   implied warranty of MERCHANTABILITY or FITNESS FOR A
 *   PARTICULAR PURPOSE.  See the Mozilla Public License
 *   for more details.
 *
 ************************************************************/

#include "opentxs/stdafx.hpp"

#include "PullSocket.hpp"

#include "opentxs/core/Log.hpp"
#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/network/zeromq/Message.hpp"

#include <chrono>

#include <zmq.h>

//#define OT_METHOD "opentxs::network::zeromq::implementation::PullSocket::"

namespace opentxs::network::zeromq
{
OTZMQPullSocket PullSocket::Factory(
    const class Context& context,
    const bool client,
    const ListenCallback& callback)
{
    return OTZMQPullSocket(
        new implementation::PullSocket(context, client, callback));
}

OTZMQPullSocket PullSocket::Factory(
    const class Context& context,
    const bool client)
{
    return OTZMQPullSocket(new implementation::PullSocket(context, client));
}
}  // namespace opentxs::network::zeromq

namespace opentxs::network::zeromq::implementation
{
PullSocket::PullSocket(
    const zeromq::Context& context,
    const bool client,
    const zeromq::ListenCallback& callback,
    const bool startThread)
    : ot_super(context, SocketType::Subscribe)
    , Receiver(lock_, socket_, startThread)
    , client_(client)
    , callback_(callback)
{
}

PullSocket::PullSocket(
    const zeromq::Context& context,
    const bool client,
    const zeromq::ListenCallback& callback)
    : PullSocket(context, client, callback, true)
{
}

PullSocket::PullSocket(const zeromq::Context& context, const bool client)
    : PullSocket(context, client, ListenCallback::Factory(), false)
{
}

PullSocket* PullSocket::clone() const
{
    return new PullSocket(context_, client_, callback_);
}

bool PullSocket::have_callback() const { return true; }

void PullSocket::process_incoming(const Lock& lock, Message& message)
{
    OT_ASSERT(verify_lock(lock))

    callback_.Process(message);
}

bool PullSocket::Start(const std::string& endpoint) const
{
    if (client_) {

        return start_client(endpoint);
    } else {

        return bind(endpoint);
    }
}

PullSocket::~PullSocket() {}
}  // namespace opentxs::network::zeromq::implementation
