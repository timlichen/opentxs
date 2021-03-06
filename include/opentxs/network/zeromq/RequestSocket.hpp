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

#ifndef OPENTXS_NETWORK_ZEROMQ_REQUESTSOCKET_HPP
#define OPENTXS_NETWORK_ZEROMQ_REQUESTSOCKET_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/network/zeromq/Socket.hpp"

#ifdef SWIG
// clang-format off
%template(ZMQMessageSendResult) std::pair<opentxs::SendResult, Pimpl<opentxs::network::zeromq::Message>>;
%ignore opentxs::Pimpl<opentxs::network::zeromq::RequestSocket>::operator+=;
%ignore opentxs::Pimpl<opentxs::network::zeromq::RequestSocket>::operator==;
%ignore opentxs::Pimpl<opentxs::network::zeromq::RequestSocket>::operator!=;
%ignore opentxs::Pimpl<opentxs::network::zeromq::RequestSocket>::operator<;
%ignore opentxs::Pimpl<opentxs::network::zeromq::RequestSocket>::operator<=;
%ignore opentxs::Pimpl<opentxs::network::zeromq::RequestSocket>::operator>;
%ignore opentxs::Pimpl<opentxs::network::zeromq::RequestSocket>::operator>=;
%template(OTZMQRequestSocket) opentxs::Pimpl<opentxs::network::zeromq::RequestSocket>;
%rename($ignore, regextarget=1, fullname=1) "opentxs::network::zeromq::RequestSocket::Factory.*";
%rename($ignore, regextarget=1, fullname=1) "opentxs::network::zeromq::RequestSocket::SetCurve.*";
%rename(ZMQRequestSocket) opentxs::network::zeromq::RequestSocket;
// clang-format on
#endif  // SWIG

namespace opentxs
{
namespace network
{
namespace zeromq
{
class RequestSocket : virtual public Socket
{
public:
    EXPORT static OTZMQRequestSocket Factory(const class Context& context);

    EXPORT virtual std::pair<
        opentxs::SendResult,
        opentxs::Pimpl<opentxs::network::zeromq::Message>>
    SendRequest(opentxs::Data& message) const = 0;
    EXPORT virtual std::pair<
        opentxs::SendResult,
        opentxs::Pimpl<opentxs::network::zeromq::Message>>
    SendRequest(const std::string& message) const = 0;
    EXPORT virtual std::pair<
        opentxs::SendResult,
        opentxs::Pimpl<opentxs::network::zeromq::Message>>
    SendRequest(opentxs::network::zeromq::Message& message) const = 0;
    EXPORT virtual bool SetCurve(const ServerContract& contract) const = 0;
    EXPORT virtual bool SetSocksProxy(const std::string& proxy) const = 0;

    EXPORT virtual ~RequestSocket() = default;

protected:
    RequestSocket() = default;

private:
    friend OTZMQRequestSocket;

    virtual RequestSocket* clone() const = 0;

    RequestSocket(const RequestSocket&) = delete;
    RequestSocket(RequestSocket&&) = default;
    RequestSocket& operator=(const RequestSocket&) = delete;
    RequestSocket& operator=(RequestSocket&&) = default;
};
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs
#endif  // OPENTXS_NETWORK_ZEROMQ_REQUESTSOCKET_HPP
