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

#include "ContactList.hpp"

#include "opentxs/api/ContactManager.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/network/zeromq/Message.hpp"
#include "opentxs/network/zeromq/SubscribeSocket.hpp"

#define OT_METHOD "opentxs::ui::implementation::ContactList::"

namespace opentxs::ui::implementation
{
ContactList::ContactList(
    const network::zeromq::Context& zmq,
    const api::ContactManager& contact,
    const Identifier& nymID)
    : ContactListType(zmq, contact, contact.ContactID(nymID), nymID, nullptr)
    , owner_contact_id_(last_id_)
    , owner_(*this, zmq, contact, owner_contact_id_, "Owner")
    , contact_subscriber_callback_(network::zeromq::ListenCallback::Factory(
          [this](const network::zeromq::Message& message) -> void {
              this->process_contact(message);
          }))
    , contact_subscriber_(
          zmq_.SubscribeSocket(contact_subscriber_callback_.get()))
{
    // WARNING do not attempt to use blank_ in this class
    init();
    const auto& endpoint = network::zeromq::Socket::ContactUpdateEndpoint;
    otWarn << OT_METHOD << __FUNCTION__ << ": Connecting to " << endpoint
           << std::endl;
    const auto listening = contact_subscriber_->Start(endpoint);

    OT_ASSERT(listening)

    startup_.reset(new std::thread(&ContactList::startup, this));

    OT_ASSERT(startup_)
}

void ContactList::add_item(
    const ContactListID& id,
    const ContactListSortKey& index)
{
    if (owner_contact_id_ == id) {
        Lock lock(owner_.lock_);
        owner_.name_ = index;

        return;
    }

    insert_outer(id, index);
}

void ContactList::construct_item(
    const ContactListID& id,
    const ContactListSortKey& index) const
{
    names_.emplace(id, index);
    items_[index].emplace(
        id, new ContactListItem(*this, zmq_, contact_manager_, id, index));
}

/** Returns owner contact. Sets up iterators for next row */
const opentxs::ui::ContactListItem& ContactList::first(const Lock& lock) const
{
    OT_ASSERT(verify_lock(lock))

    have_items_->Set(first_valid_item(lock));
    start_->Set(!have_items_.get());
    last_id_ = owner_contact_id_;

    return owner_;
}

const Identifier& ContactList::ID() const { return owner_contact_id_; }

ContactListOuter::const_iterator ContactList::outer_first() const
{
    return items_.begin();
}

ContactListOuter::const_iterator ContactList::outer_end() const
{
    return items_.end();
}

void ContactList::process_contact(const network::zeromq::Message& message)
{
    wait_for_startup();
    const std::string id(message);
    const Identifier contactID(id);

    OT_ASSERT(false == contactID.empty())

    const auto name = contact_manager_.ContactName(contactID);
    add_item(contactID, name);
}

void ContactList::startup()
{
    const auto contacts = contact_manager_.ContactList();
    otWarn << OT_METHOD << __FUNCTION__ << ": Loading " << contacts.size()
           << " contacts." << std::endl;

    for (const auto & [ id, alias ] : contacts) {
        add_item(Identifier(id), alias);
    }

    startup_complete_->On();
}
}  // namespace opentxs::ui::implementation
