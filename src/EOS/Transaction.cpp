#include "Transaction.h"

#include <stdexcept>
#include <ctime>

#include <TrezorCrypto/base58.h>
#include <TrezorCrypto/ripemd160.h>

#include "../Hash.h"
#include "../HexCoding.h"

using namespace TW::EOS;
using json = nlohmann::json;

Signature::Signature(Data sig, Type type) : data(sig), type(type) {
    if (sig.size() != DataSize) {
        throw std::invalid_argument("Invalid signature size!");
    }

    if (type == Type::Legacy) {
        throw std::invalid_argument("Legacy signatures are not supported.");
    }
}

void Signature::serialize(Data& os) const noexcept {
    // type should never be Legacy
    uint32_t typeId = type == Type::ModernK1 ? 0 : 1;
    Bravo::encodeVarInt32(typeId, os);
    os.insert(os.end(), data.begin(), data.end());
}

std::string Signature::string() const noexcept {
    const auto& prefix = sigPrefixForType(type);
    const auto& subPrefix = type == Type::ModernR1 ? Modern::R1::prefix : Modern::K1::prefix;

    Data buffer(data);

    // append the subPrefix to the buffer data hash the buffer
    for (const char& c : subPrefix) {
        buffer.push_back(static_cast<uint8_t>(c));
    }

    Data hash;
    hash.resize(RIPEMD160_DIGEST_LENGTH);

    ripemd160(buffer.data(), DataSize + subPrefix.size(), hash.data());

    // drop the subPrefix and append the checksum to the bufer
    buffer.resize(DataSize);

    for(size_t i = 0; i < ChecksumSize; i++) {
        buffer.push_back(hash[i]);
    }

    // create a base58 representation of the buffer
    size_t b58Size = (DataSize + ChecksumSize) * 138 / 100 + 2;;
    char b58[b58Size];
    b58enc(b58, &b58Size, buffer.data(), DataSize + ChecksumSize);

    return prefix + std::string(b58);
}

void Extension::serialize(Data& os) const noexcept {
    encode16LE(type, os);
    Bravo::encodeVarInt32(buffer.size(), os);
    append(os, buffer);
}

json Extension::serialize() const noexcept {
    return json::array({type, hex(buffer)});
}

Transaction::Transaction(const std::string& referenceBlockId, uint32_t referenceBlockTime) {
    setReferenceBlock(parse_hex(referenceBlockId));
    expiration = referenceBlockTime + Transaction::ExpirySeconds;
}

void Transaction::setReferenceBlock(const Data& refBlockId) {
    if (refBlockId.size() != Hash::sha256Size) {
        throw std::invalid_argument("Invalid Reference Block Id!");
    }

    refBlockNumber = decode16BE(refBlockId.data() + 2);
    refBlockPrefix = decode32LE(refBlockId.data() + 8);
}

void Transaction::serialize(Data& os) const noexcept{
    using namespace Bravo;

    encode32LE(expiration, os);
    encode16LE(refBlockNumber, os);
    encode32LE(refBlockPrefix, os);
    encodeVarInt32(maxNetUsageWords, os);
    os.push_back(maxCPUUsageInMS);
    encodeVarInt32(delaySeconds, os);

    encodeCollection(contextFreeActions, os);
    encodeCollection(actions, os);
    encodeCollection(transactionExtensions, os);
}

json Transaction::serialize() const noexcept {
    using namespace Bravo;

    // get a formatted date
    char formattedDate[20];
    time_t time = expiration;
    if (strftime(formattedDate, 19, "%FT%T", std::gmtime(&time)) != 19) {
        std::runtime_error("Error creating a formatted string!");
    }

    // create a json array of signatures
    json sigs = json::array();
    for (const auto& sig : signatures) {
        sigs.push_back(sig.string());
    }

    // create a json array of context-free data
    json cfdJSON = json::array();
    for (const auto& d : contextFreeData) {
        cfdJSON.push_back(hex(d));
    }

    // add everything to the json object
    json obj;
    obj["ref_block_num"] = refBlockNumber;
    obj["ref_block_prefix"] = refBlockPrefix;
    obj["expiration"] = std::string(formattedDate, 19);
    obj["max_net_usage_words"] = maxNetUsageWords;
    obj["max_cpu_usage_ms"] = maxCPUUsageInMS;
    obj["delay_sec"] = delaySeconds;
    obj["context_free_actions"] = encodeCollection(contextFreeActions);
    obj["actions"] = encodeCollection(actions);
    obj["transaction_extensions"] = encodeCollection(transactionExtensions);
    obj["signatures"] = sigs;
    obj["context_free_data"] = cfdJSON;

    return obj;
}
