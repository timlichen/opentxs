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

#include <opentxs/core/crypto/Libsecp256k1.hpp>

#include <opentxs/core/FormattedKey.hpp>
#include <opentxs/core/Log.hpp>
#include <opentxs/core/Nym.hpp>
#include <opentxs/core/crypto/Crypto.hpp>
#include <opentxs/core/crypto/CryptoEngine.hpp>
#include <opentxs/core/crypto/CryptoUtil.hpp>
#include <opentxs/core/crypto/Letter.hpp>
#include <opentxs/core/crypto/NymParameters.hpp>
#include <opentxs/core/crypto/OTASCIIArmor.hpp>
#include <opentxs/core/crypto/OTAsymmetricKey.hpp>
#include <opentxs/core/crypto/OTEnvelope.hpp>
#include <opentxs/core/crypto/OTKeypair.hpp>
#include <opentxs/core/crypto/OTPassword.hpp>
#include <opentxs/core/crypto/OTPasswordData.hpp>
#include <opentxs/core/crypto/OTSymmetricKey.hpp>
#include <opentxs/core/crypto/OTSignature.hpp>
#include <opentxs/core/crypto/AsymmetricKeySecp256k1.hpp>
#include <opentxs/core/crypto/Letter.hpp>

#include <vector>

namespace opentxs
{

Libsecp256k1::Libsecp256k1(CryptoUtil& ssl)
    : Crypto(),
    context_(secp256k1_context_create(SECP256K1_CONTEXT_SIGN | SECP256K1_CONTEXT_VERIFY)),
    ssl_(ssl)
{
    OT_ASSERT_MSG(nullptr != context_, "Libsecp256k1::Libsecp256k1: secp256k1_context_create failed.");
}


bool Libsecp256k1::SignContract(
    const String& strContractUnsigned,
    const OTAsymmetricKey& theKey,
    OTSignature& theSignature, // output
    CryptoHash::HashType hashType,
    const OTPasswordData* pPWData)
{
    OTData hash;
    bool haveDigest = CryptoEngine::Instance().Hash().Digest(hashType, strContractUnsigned, hash);

    if (haveDigest) {
        OTPassword privKey;
        bool havePrivateKey = AsymmetricKeyToECDSAPrivkey(theKey, *pPWData, privKey);

        if (havePrivateKey) {
            secp256k1_ecdsa_signature_t ecdsaSignature;

            bool signatureCreated = secp256k1_ecdsa_sign(
                context_,
                &ecdsaSignature,
                reinterpret_cast<const unsigned char*>(hash.GetPointer()),
                reinterpret_cast<const unsigned char*>(privKey.getMemory()),
                nullptr,
                nullptr);

            if (signatureCreated) {
                bool signatureSet = ECDSASignatureToOTSignature(ecdsaSignature, theSignature);
                return signatureSet;
            } else {
                    otErr << __FUNCTION__ << ": "
                    << "Call to secp256k1_ecdsa_sign() failed.\n";

                    return false;
            }
        } else {
                otErr << __FUNCTION__ << ": "
                << "Can not extract ecdsa private key from OTAsymmetricKey.\n";

                return false;
        }
    } else {
        otErr << __FUNCTION__ << ": "
        << "Failed to obtain the contract hash.\n";

        return false;
    }
}

bool Libsecp256k1::VerifySignature(
    const String& strContractToVerify,
    const OTAsymmetricKey& theKey,
    const OTSignature& theSignature,
    const CryptoHash::HashType hashType,
    __attribute__((unused)) const OTPasswordData* pPWData
    ) const
{
    OTData hash;
    bool haveDigest = CryptoEngine::Instance().Hash().Digest(hashType, strContractToVerify, hash);

    if (haveDigest) {
        secp256k1_pubkey_t ecdsaPubkey;
        bool havePublicKey = AsymmetricKeyToECDSAPubkey(theKey, ecdsaPubkey);

        if (havePublicKey) {
            secp256k1_ecdsa_signature_t ecdsaSignature;

            bool haveSignature = OTSignatureToECDSASignature(theSignature, ecdsaSignature);

            if (haveSignature) {
                bool signatureVerified = secp256k1_ecdsa_verify(
                    context_,
                    &ecdsaSignature,
                    reinterpret_cast<const unsigned char*>(hash.GetPointer()),
                    &ecdsaPubkey);

                return signatureVerified;
            }
        }
    }
    return false;
}

bool Libsecp256k1::OTSignatureToECDSASignature(
    const OTSignature& inSignature,
    secp256k1_ecdsa_signature_t& outSignature) const
{
    OTData signature;

    bool hasSignature = inSignature.GetData(signature);

    if (hasSignature) {
        const uint8_t* sigStart = static_cast<const uint8_t*>(signature.GetPointer());

        if (nullptr != sigStart) {

            if (sizeof(secp256k1_ecdsa_signature_t) == signature.GetSize()) {
                secp256k1_ecdsa_signature_t ecdsaSignature;

                for(uint32_t i=0; i < signature.GetSize(); i++) {
                    ecdsaSignature.data[i] = *(sigStart + i);
                }

                outSignature = ecdsaSignature;

                return true;
            }
        }
    }
    return false;
}

bool Libsecp256k1::ECDSASignatureToOTSignature(
    const secp256k1_ecdsa_signature_t& inSignature,
    OTSignature& outSignature) const
{
    OTData signature;

    signature.Assign(inSignature.data, sizeof(secp256k1_ecdsa_signature_t));
    bool signatureSet = outSignature.SetData(signature);

    return signatureSet;
}

bool Libsecp256k1::AsymmetricKeyToECDSAPubkey(
        const OTAsymmetricKey& asymmetricKey,
        secp256k1_pubkey_t& pubkey) const
{
    String encodedPubkey;
    bool havePublicKey = asymmetricKey.GetPublicKey(encodedPubkey);

    if (havePublicKey) {
        OTData serializedPubkey;
        bool pubkeydecoded = CryptoUtil::Base58CheckDecode(encodedPubkey.Get(), serializedPubkey);

        if (pubkeydecoded) {
            secp256k1_pubkey_t parsedPubkey;

            bool pubkeyParsed = secp256k1_ec_pubkey_parse(
                context_,
                &parsedPubkey,
                reinterpret_cast<const unsigned char*>(serializedPubkey.GetPointer()),
                serializedPubkey.GetSize());

            if (pubkeyParsed) {
                pubkey = parsedPubkey;
                return true;
            }
        }
    }
    return false;
}

bool Libsecp256k1::ECDSAPubkeyToAsymmetricKey(
        const secp256k1_pubkey_t& pubkey,
        OTAsymmetricKey& asymmetricKey) const
{
    OTData serializedPubkey;

    bool keySerialized = secp256k1_pubkey_serialize(serializedPubkey, pubkey);

    if (keySerialized) {
        FormattedKey encodedPublicKey(CryptoUtil::Base58CheckEncode(serializedPubkey).Get());

        return asymmetricKey.SetPublicKey(encodedPublicKey);
    }
    return false;
}

bool Libsecp256k1::AsymmetricKeyToECDSAPrivkey(
    const OTAsymmetricKey& asymmetricKey,
    const OTPasswordData& passwordData,
    OTPassword& privkey,
    bool ephemeral) const
{
    FormattedKey encodedPrivkey;
    bool havePrivateKey = asymmetricKey.GetPrivateKey(encodedPrivkey);

    if (havePrivateKey) {
        return AsymmetricKeyToECDSAPrivkey(encodedPrivkey, passwordData, privkey, ephemeral);
    } else {
        return false;
    }
}

bool Libsecp256k1::AsymmetricKeyToECDSAPrivkey(
    const FormattedKey& asymmetricKey,
    const OTPasswordData& passwordData,
    OTPassword& privkey,
    bool ephemeral) const
{

    BinarySecret masterPassword = std::make_shared<OTPassword>();

    if (ephemeral) {
        masterPassword->setPassword("test");
    } else {
        masterPassword = CryptoSymmetric::GetMasterKey(passwordData, true);
    }

    OTPassword keyPassword;
    CryptoEngine::Instance().Hash().Digest(CryptoHash::SHA256, *masterPassword, keyPassword);

    OTData encryptedPrivkey;
    bool privkeydecoded = CryptoUtil::Base58CheckDecode(asymmetricKey.Get(), encryptedPrivkey);

    if (!privkeydecoded) {
        otErr << "Libsecp256k1::" << __FUNCTION__
              << ": Could not decode base58 encrypted private key.\n";
        return false;
    }

    OTData decryptedKey;
    CryptoEngine::Instance().AES().Decrypt(
        CryptoSymmetric::AES_256_ECB,
        keyPassword,
        static_cast<const char*>(encryptedPrivkey.GetPointer()),
        encryptedPrivkey.GetSize(),
        decryptedKey);

    return privkey.setMemory(decryptedKey);
}

bool Libsecp256k1::ECDSAPrivkeyToAsymmetricKey(
        const OTPassword& privkey,
        const OTPasswordData& passwordData,
        OTAsymmetricKey& asymmetricKey,
        bool ephemeral) const
{
    BinarySecret masterPassword = std::make_shared<OTPassword>();

    if (ephemeral) {
        masterPassword->setPassword("test");
    } else {
        masterPassword = CryptoSymmetric::GetMasterKey(passwordData, true);
    }

    OTPassword keyPassword;
    CryptoEngine::Instance().Hash().Digest(CryptoHash::SHA256, *masterPassword, keyPassword);

    OTData encryptedKey;
    CryptoEngine::Instance().AES().Encrypt(
        CryptoSymmetric::AES_256_ECB,
        keyPassword,
        static_cast<const char*>(privkey.getMemory()),
        privkey.getMemorySize(),
        encryptedKey);

    FormattedKey formattedKey(CryptoUtil::Base58CheckEncode(encryptedKey).Get());

    return asymmetricKey.SetPrivateKey(formattedKey);
}

bool Libsecp256k1::ECDH(
    const OTAsymmetricKey& publicKey,
    const OTAsymmetricKey& privateKey,
    const OTPasswordData passwordData,
    OTPassword& secret,
    bool ephemeral) const
{
    OTPassword scalar;
    secp256k1_pubkey_t point;

    bool havePrivateKey = AsymmetricKeyToECDSAPrivkey(privateKey, passwordData, scalar, ephemeral);

    if (havePrivateKey) {
        bool havePublicKey = AsymmetricKeyToECDSAPubkey(publicKey, point);

        if (havePublicKey) {
            secret.SetSize(PrivateKeySize);

            return secp256k1_ecdh(
                context_,
                reinterpret_cast<unsigned char*>(secret.getMemoryWritable()),
                &point,
                static_cast<const unsigned char*>(scalar.getMemory()));
        } else {
            otErr << "Libsecp256k1::" << __FUNCTION__ << " could not obtain public key.\n.";
            return false;
        }
    } else {
        otErr << "Libsecp256k1::" << __FUNCTION__ << " could not obtain private key.\n.";
        return false;
    }
}

bool Libsecp256k1::EncryptSessionKeyECDH(
        const OTPassword& sessionKey,
        const OTAsymmetricKey& privateKey,
        const OTAsymmetricKey& publicKey,
        const OTPasswordData& passwordData,
        symmetricEnvelope& encryptedSessionKey,
        bool ephemeral) const
{
    CryptoSymmetric::Mode algo = CryptoSymmetric::StringToMode(std::get<0>(encryptedSessionKey));
    CryptoHash::HashType hmac = CryptoHash::StringToHashType(std::get<1>(encryptedSessionKey));

    if (CryptoSymmetric::ERROR_MODE == algo) {
        otErr << "Libsecp256k1::" << __FUNCTION__ << ": Unsupported encryption algorithm.\n";
        return false;
    }

    if (CryptoHash::ERROR == hmac) {
        otErr << "Libsecp256k1::" << __FUNCTION__ << ": Unsupported hmac algorithm.\n";
        return false;
    }

    OTData nonce;
    String nonceReadable = CryptoEngine::Instance().Util().Nonce(CryptoSymmetric::KeySize(algo), nonce);

    // Calculate ECDH shared secret
    BinarySecret ECDHSecret(CryptoEngine::Instance().AES().InstantiateBinarySecretSP());
    bool haveECDH = ECDH(publicKey, privateKey, passwordData, *ECDHSecret, ephemeral);

    if (haveECDH) {
        // In order to make sure the session key is always encrypted to a different key for every Seal() action,
        // even if the sender and recipient are the same, don't use the ECDH secret directly. Instead, calculate
        // an HMAC of the shared secret and a nonce and use that as the AES encryption key.
        OTPassword sharedSecret;
        CryptoEngine::Instance().Hash().HMAC(hmac, *ECDHSecret, nonce, sharedSecret);

        // The values calculated above might not be the correct size for the default symmetric encryption
        // function.
        if ((sharedSecret.getMemorySize() >= CryptoSymmetric::KeySize(algo)) &&
            (nonce.GetSize() >= CryptoSymmetric::IVSize(algo))) {

            OTPassword truncatedSharedSecret(sharedSecret.getMemory(), CryptoSymmetric::KeySize(algo));
            OTData truncatedNonce(nonce.GetPointer(), CryptoSymmetric::IVSize(algo));

                OTData ciphertext, tag;
                bool encrypted = CryptoEngine::Instance().AES().Encrypt(
                    algo,
                    truncatedSharedSecret,
                    truncatedNonce,
                    static_cast<const char*>(sessionKey.getMemory()),
                    sessionKey.getMemorySize(),
                    ciphertext,
                    tag);

                    if (encrypted) {
                        OTASCIIArmor encodedCiphertext(ciphertext);
                        String tagReadable(CryptoUtil::Base58CheckEncode(tag));

                        std::get<2>(encryptedSessionKey) = nonceReadable;
                        std::get<3>(encryptedSessionKey) = tagReadable;
                        std::get<4>(encryptedSessionKey) = std::make_shared<OTEnvelope>(encodedCiphertext);

                        return true;
                    } else {
                        otErr << "Libsecp256k1::" << __FUNCTION__ << ": Session key encryption failed.\n";
                        return false;
                    }
        } else {
            otErr << "Libsecp256k1::" << __FUNCTION__ << ": Insufficient nonce or key size.\n";
            return false;
        }

    } else {
        otErr << "Libsecp256k1::" << __FUNCTION__ << ": ECDH shared secret negotiation failed.\n";
        return false;
    }
}

bool Libsecp256k1::DecryptSessionKeyECDH(
    const symmetricEnvelope& encryptedSessionKey,
    const OTAsymmetricKey& privateKey,
    const OTAsymmetricKey& publicKey,
    const OTPasswordData passwordData,
    OTPassword& sessionKey) const
{
    CryptoSymmetric::Mode algo = CryptoSymmetric::StringToMode(std::get<0>(encryptedSessionKey));
    CryptoHash::HashType hmac = CryptoHash::StringToHashType(std::get<1>(encryptedSessionKey));

    if (CryptoSymmetric::ERROR_MODE == algo) {
        otErr << "Libsecp256k1::" << __FUNCTION__ << ": Unsupported encryption algorithm.\n";
        return false;
    }

    if (CryptoHash::ERROR == hmac) {
        otErr << "Libsecp256k1::" << __FUNCTION__ << ": Unsupported hmac algorithm.\n";
        return false;
    }

    // Extract and decode the nonce
    OTData nonce;
    bool nonceDecoded = CryptoUtil::Base58CheckDecode(std::get<2>(encryptedSessionKey).Get(), nonce);

    if (nonceDecoded) {
        // Calculate ECDH shared secret
        BinarySecret ECDHSecret(CryptoEngine::Instance().AES().InstantiateBinarySecretSP());
        bool haveECDH = ECDH(publicKey, privateKey, passwordData, *ECDHSecret);

        if (haveECDH) {
            // In order to make sure the session key is always encrypted to a different key for every Seal() action
            // even if the sender and recipient are the same, don't use the ECDH secret directly. Instead, calculate
            // an HMAC of the shared secret and a nonce and use that as the AES encryption key.
            OTPassword sharedSecret;
            CryptoEngine::Instance().Hash().HMAC(hmac, *ECDHSecret, nonce, sharedSecret);

            // The values calculated above might not be the correct size for the default symmetric encryption
            // function.
            if (
                (sharedSecret.getMemorySize() >= CryptoConfig::SymmetricKeySize()) &&
                (nonce.GetSize() >= CryptoConfig::SymmetricIvSize())) {

                    OTPassword truncatedSharedSecret(sharedSecret.getMemory(), CryptoSymmetric::KeySize(algo));
                    OTData truncatedNonce(nonce.GetPointer(), CryptoSymmetric::IVSize(algo));

                    // Extract and decode the tag from the envelope
                    OTData tag;
                    CryptoUtil::Base58CheckDecode(std::get<3>(encryptedSessionKey).Get(), tag);

                    // Extract and decode the ciphertext from the envelope
                    OTData ciphertext;
                    OTASCIIArmor encodedCiphertext;
                    std::get<4>(encryptedSessionKey)->GetAsciiArmoredData(encodedCiphertext);
                    encodedCiphertext.GetData(ciphertext);

                    return CryptoEngine::Instance().AES().Decrypt(
                                                            algo,
                                                            truncatedSharedSecret,
                                                            truncatedNonce,
                                                            tag,
                                                            static_cast<const char*>(ciphertext.GetPointer()),
                                                            ciphertext.GetSize(),
                                                            sessionKey);
            } else {
                otErr << "Libsecp256k1::" << __FUNCTION__ << ": Insufficient nonce or key size.\n";
                return false;
            }

        } else {
            otErr << "Libsecp256k1::" << __FUNCTION__ << ": ECDH shared secret negotiation failed.\n";
            return false;
        }
    } else {
        otErr << "Libsecp256k1::" << __FUNCTION__ << ": Can not decode nonce.\n";
        return false;
    }
}

bool Libsecp256k1::secp256k1_privkey_tweak_add(
    uint8_t key [PrivateKeySize],
    const uint8_t tweak [PrivateKeySize]) const
{
    if (nullptr != context_) {
        return secp256k1_ec_privkey_tweak_add(context_, key, tweak);
    } else {
        return false;
    }
}

bool Libsecp256k1::secp256k1_pubkey_create(
    secp256k1_pubkey_t& pubkey,
    const OTPassword& privkey) const
{
    if (nullptr != context_) {
        return secp256k1_ec_pubkey_create(context_, &pubkey, static_cast<const unsigned char*>(privkey.getMemory()));
    }

    return false;
}

bool Libsecp256k1::secp256k1_pubkey_serialize(
        OTData& serializedPubkey,
        const secp256k1_pubkey_t& pubkey) const
{
    if (nullptr != context_) {
        uint8_t serializedOutput [65] {};
        int serializedSize = 0;

        bool serialized = secp256k1_ec_pubkey_serialize(context_, serializedOutput, &serializedSize, &pubkey, false);

        if (serialized) {
            serializedPubkey.Assign(serializedOutput, serializedSize);
            return serialized;
        }
    }

    return false;
}

bool Libsecp256k1::secp256k1_pubkey_parse(
        secp256k1_pubkey_t& pubkey,
        const OTPassword& serializedPubkey) const
{
    if (nullptr != context_) {

        const uint8_t* inputStart = static_cast<const uint8_t*>(serializedPubkey.getMemory());

        bool parsed = secp256k1_ec_pubkey_parse(context_, &pubkey, inputStart, serializedPubkey.getMemorySize());

        return parsed;
    }

    return false;
}

Libsecp256k1::~Libsecp256k1()
{
    OT_ASSERT_MSG(nullptr != context_, "Libsecp256k1::~Libsecp256k1: context_ should never be nullptr, yet it was.")
    secp256k1_context_destroy(context_);
    context_ = nullptr;
}

void Libsecp256k1::Init_Override() const
{
    static bool bNotAlreadyInitialized = true;
    OT_ASSERT_MSG(bNotAlreadyInitialized, "Libsecp256k1::Init_Override: Tried to initialize twice.");
    bNotAlreadyInitialized = false;
    // --------------------------------
    uint8_t randomSeed [32]{};
    ssl_.RandomizeMemory(randomSeed, 32);

    OT_ASSERT_MSG(nullptr != context_, "Libsecp256k1::Libsecp256k1: secp256k1_context_create failed.");

    int __attribute__((unused)) randomize = secp256k1_context_randomize(context_,
                                                                        randomSeed);
}

void Libsecp256k1::Cleanup_Override() const
{
}

} // namespace opentxs
