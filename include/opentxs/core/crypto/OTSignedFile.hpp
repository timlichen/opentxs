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

#ifndef OPENTXS_CORE_CRYPTO_OTSIGNEDFILE_HPP
#define OPENTXS_CORE_CRYPTO_OTSIGNEDFILE_HPP

#include "opentxs/core/Contract.hpp"

namespace opentxs
{

class OTSignedFile : public Contract
{
private: // Private prevents erroneous use by other classes.
    typedef Contract ot_super;

protected:
    String m_strSignedFilePayload; // This class exists to wrap another and
                                   // save it in signed form.
    // The "payload" (the wrapped contents) are stored in this member.

    String m_strLocalDir; // The local subdirectory where the file is, such as
                          // "nyms" or "certs"
    String m_strSignedFilename; // The file stores its own name. Later, when
                                // loading it back up, you can
    // see that the name matches internally, and that the signature matches,
    // therefore, no one has switched the file or meddled with its contents.

    String m_strPurportedLocalDir; // This is the subdirectory according to
                                   // the file.
    String m_strPurportedFilename; // This is the filename according to the
                                   // file.

    String m_strSignerNymID; // Optional. Here in case you ever
                             // want to use it.

    // THOUGHT: What if someone switched the file for an older version of
    // itself? Seems to me that he could
    // make the server accept the file, in that case. Like maybe an account file
    // with a higher balance?
    // Similarly, what if someone erased a spent token file? Then the software
    // would accept it as a new
    // token once again. Also, the cash account would be deducted twice for the
    // same token, which means it
    // would no longer contain enough to cover all the tokens...
    // Therefore it seems to me that, even with the files signed, there are
    // still attacks possible when
    // the attacker has write/erase access to the filesystem. I'd like to make
    // it impervious even to that.

    virtual int32_t ProcessXMLNode(irr::io::IrrXMLReader*& xml);

public:
    // These assume SetFilename() was already called,
    // or at least one of the constructors that uses it.
    //
    EXPORT OTSignedFile();
    OTSignedFile(const String& LOCAL_SUBDIR, const String& FILE_NAME);
    OTSignedFile(const char* LOCAL_SUBDIR, const String& FILE_NAME);
    EXPORT OTSignedFile(const char* LOCAL_SUBDIR, const char* FILE_NAME);
    EXPORT bool LoadFile();
    EXPORT bool SaveFile();
    bool VerifyFile(); // Returns true or false, whether actual subdir/file
                       // matches purported subdir/file.
    // (You should still verify the signature on it as well, if you are doing
    // this.)
    void SetFilename(const String& LOCAL_SUBDIR, const String& FILE_NAME);
    EXPORT String& GetFilePayload();
    EXPORT void SetFilePayload(const String& strArg);
    EXPORT String& GetSignerNymID();
    EXPORT void SetSignerNymID(const String& strArg);
    EXPORT virtual ~OTSignedFile();
    virtual void Release();
    void Release_SignedFile();
    virtual void UpdateContents();
};

} // namespace opentxs

#endif // OPENTXS_CORE_CRYPTO_OTSIGNEDFILE_HPP
