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

#ifndef OPENTXS_CLIENT_NYMDATA_HPP
#define OPENTXS_CLIENT_NYMDATA_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/Proto.hpp"

#include <cstdint>
#include <memory>
#include <string>

#ifdef SWIG
// clang-format off
%rename($ignore, regextarget=1, fullname=1) "opentxs::NymData::Type.*";
// clang-format on
#endif  // SWIG

namespace opentxs
{
namespace api
{
namespace client
{
namespace implementation
{
class Wallet;
}  // implementation
}  // client
}  // api

class NymData
{
public:
    NymData(const NymData&) = default;
    NymData(NymData&&) = default;

    std::uint32_t GetType() const;
#ifndef SWIG
    bool HaveContract(
        const Identifier& id,
        const proto::ContactItemType currency,
        const bool primary,
        const bool active) const;
#endif
    bool HaveContract(
        const std::string& id,
        const std::uint32_t currency,
        const bool primary,
        const bool active) const;
    std::string Name() const;
#ifndef SWIG
    std::string PaymentCode(const proto::ContactItemType currency) const;
#endif
    std::string PaymentCode(const std::uint32_t currency) const;
    std::string PreferredOTServer() const;
    std::string PrintContactData() const;
    proto::ContactItemType Type() const;
    bool Valid() const;

#ifndef SWIG
    bool AddContract(
        const std::string& instrumentDefinitionID,
        const proto::ContactItemType currency,
        const bool primary,
        const bool active);
#endif
    bool AddContract(
        const std::string& instrumentDefinitionID,
        const std::uint32_t currency,
        const bool primary,
        const bool active);
#ifndef SWIG
    bool AddPaymentCode(
        const std::string& code,
        const proto::ContactItemType currency,
        const bool primary,
        const bool active);
#endif
    bool AddPaymentCode(
        const std::string& code,
        const std::uint32_t currency,
        const bool primary,
        const bool active);
    bool AddPreferredOTServer(const std::string& id, const bool primary);
#ifndef SWIG
    bool SetType(const proto::ContactItemType type, const std::string& name);
#endif
    bool SetType(const std::uint32_t type, const std::string& name);

    ~NymData() = default;

private:
    friend class api::client::implementation::Wallet;

    std::shared_ptr<Nym> nym_;

    const ContactData& data() const;

    Nym& nym();

    NymData(const std::shared_ptr<Nym>& nym);
    NymData() = delete;
    NymData& operator=(const NymData&) = delete;
    NymData& operator=(NymData&&) = delete;
};
}  // namespace opentxs
#endif  // OPENTXS_CLIENT_NYMDATA_HPP
