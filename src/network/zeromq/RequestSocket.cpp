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

#include "RequestSocket.hpp"

#include "opentxs/core/Log.hpp"
#include "opentxs/network/zeromq/Message.hpp"

#include <zmq.h>

#define OT_METHOD "opentxs::network::zeromq::implementation::RequestSocket::"

namespace opentxs::network::zeromq
{
OTZMQRequestSocket RequestSocket::Factory(const class Context& context)
{
    return OTZMQRequestSocket(new implementation::RequestSocket(context));
}
}  // namespace opentxs::network::zeromq

namespace opentxs::network::zeromq::implementation
{
RequestSocket::RequestSocket(const zeromq::Context& context)
    : ot_super(context, SocketType::Request)
    , CurveClient(lock_, socket_)
{
}

RequestSocket* RequestSocket::clone() const
{
    return new RequestSocket(context_);
}

Socket::MessageSendResult RequestSocket::SendRequest(opentxs::Data& input) const
{
    return SendRequest(Message::Factory(input));
}

Socket::MessageSendResult RequestSocket::SendRequest(
    const std::string& input) const
{
    auto copy = input;

    return SendRequest(Message::Factory(copy));
}

Socket::MessageSendResult RequestSocket::SendRequest(
    zeromq::Message& request) const
{
    OT_ASSERT(nullptr != socket_);

    Lock lock(lock_);
    MessageSendResult output{SendResult::ERROR, Message::Factory()};
    auto& status = output.first;
    auto& reply = output.second;
    Message& message = reply;
    const bool sent = (-1 != zmq_msg_send(request, socket_, 0));

    if (false == sent) {
        otErr << OT_METHOD << __FUNCTION__ << ": Send error:\n"
              << zmq_strerror(zmq_errno()) << std::endl;

        return output;
    }

    const bool received = (-1 != zmq_msg_recv(message, socket_, 0));

    if (false == received) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Receive error: " << zmq_strerror(zmq_errno()) << std::endl;
        status = SendResult::TIMEOUT;

        return output;
    }

    status = SendResult::VALID_REPLY;

    return output;
}

bool RequestSocket::SetCurve(const ServerContract& contract) const
{
    return set_curve(contract);
}

bool RequestSocket::SetSocksProxy(const std::string& proxy) const
{
    return set_socks_proxy(proxy);
}

bool RequestSocket::Start(const std::string& endpoint) const
{
    return start_client(endpoint);
}
}  // namespace opentxs::network::zeromq::implementation
