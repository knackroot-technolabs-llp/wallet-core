#include "Bravo/Address.h"
#include "HexCoding.h"
#include "PrivateKey.h"

#include <gtest/gtest.h>
#include <string>

using namespace TW;
using namespace TW::Bravo;

TEST(SteemAddress, Invalid) {
	ASSERT_FALSE(Address::isValid("abc"));
	ASSERT_FALSE(Address::isValid("5LZBF18uucMKVyKMQ7r6qnXBWn9StM5AJYHs1kidPeyNexuBUm"));
	ASSERT_FALSE(Address::isValid("STM5LZBF18uucMKVyKMQ7r6qnXBWn9StM5AJYHs1kidPeyNexuBU"));
	ASSERT_FALSE(Address::isValid("STM5LZBF18uucMKVyKMQ7r6qnXBWn9StM5AJYHs1kidPeyNexuBUmm"));
	ASSERT_FALSE(Address::isValid("MTS115LZBF18uucMKVyKMQ7r6qnXBWn9StM5AJYHs1kidPeyNexuBUm"));
}

TEST(SteemAddress, Base58) {
	const std::vector<std::string> prefixes = { "STM", "TST" };

	ASSERT_EQ(
		Address("STM6LLegbAgLAy28EHrffBVuANFWcFgmqRMW13wBmTExqFE9SCkg4", prefixes).string(),
		"STM6LLegbAgLAy28EHrffBVuANFWcFgmqRMW13wBmTExqFE9SCkg4"
	);
	ASSERT_EQ(
		Address("STM52K5kMmwiRyNQYAf7ymCMz6hieE8siyrqNt1t57ac9hvBrRdaa", prefixes).string(),
		"STM52K5kMmwiRyNQYAf7ymCMz6hieE8siyrqNt1t57ac9hvBrRdaa"
	);
	ASSERT_EQ(
		Address("STM6EFA9Ge5KQaCS2jGZVf7xHZ6hzcH7uvikf5oR7YnYKemkucxB4", prefixes).string(),
		"STM6EFA9Ge5KQaCS2jGZVf7xHZ6hzcH7uvikf5oR7YnYKemkucxB4"
	);
}

TEST(SteemAddress, FromPrivateKey) {
	const std::vector<std::string> prefixes = { "STM", "TST" };

	std::string privArray[3]{ "559aead08264d5795d3909718cdd05abd49572e84fe55590eef31a88a08fdffd",
								"df7e70e5021544f4834bbee64a9e3789febc4be81470df629cad6ddb03320a5c",
								"6b23c0d5f35d1b11f9b683f0b0a617355deb11277d91ae091d399c655b87940d" };

	std::string pubArray[3]{ "STM52K5kMmwiRyNQYAf7ymCMz6hieE8siyrqNt1t57ac9hvBrRdaa",
								"STM6EFA9Ge5KQaCS2jGZVf7xHZ6hzcH7uvikf5oR7YnYKemkucxB4",
								"STM7pkBGaen6htFXNb4Dj2UNS631YwXkw1ugaEzzVZ7UGK54jP47H" };

	for (int i = 0; i < 3; i++) {
		const auto privateKey = PrivateKey(parse_hex(privArray[i]));
		const auto publicKey = PublicKey(privateKey.getPublicKey(PublicKeyType::secp256k1));
		const auto address = Address(publicKey, "STM");

		ASSERT_EQ(address.string(), pubArray[i]);
	}
}

TEST(SteemAddress, IsValid) {
	const std::vector<std::string> prefixes = { "STM", "TST" };

	ASSERT_TRUE(Address::isValid("STM52K5kMmwiRyNQYAf7ymCMz6hieE8siyrqNt1t57ac9hvBrRdaa", prefixes));
	ASSERT_TRUE(Address::isValid("STM6EFA9Ge5KQaCS2jGZVf7xHZ6hzcH7uvikf5oR7YnYKemkucxB4", prefixes));
	ASSERT_TRUE(Address::isValid("STM7pkBGaen6htFXNb4Dj2UNS631YwXkw1ugaEzzVZ7UGK54jP47H", prefixes));
	ASSERT_TRUE(Address::isValid("STM52K5kMmwiRyNQYAf7ymCMz6hieE8siyrqNt1t57ac9hvBrRdaa", prefixes));
}