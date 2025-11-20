#include <doctest/doctest.h>
#include <msgpack/msgpack.hpp>
#include <msgpack/vector.hpp>

struct Person {
    std::string         name;
    int32_t             age;
    std::vector<double> scores;

    MSGPACK_DEFINE(name, age, scores)
};

TEST_CASE("msgpack - nested custom types") {
    std::vector<uint8_t> buffer;
    msgpack::Packer      packer(buffer);

    Person p{"Alice", 30, {95.5, 87.3, 92.1}};
    p.msgpack_pack(packer);

    msgpack::Unpacker unpacker(buffer.data(), buffer.size());
    Person            result;
    CHECK(result.msgpack_unpack(unpacker));
    CHECK(result.name == "Alice");
    CHECK(result.age == 30);
    CHECK(result.scores.size() == 3);
    CHECK(result.scores[0] == doctest::Approx(95.5));
}

TEST_CASE("msgpack - array of custom types") {
    std::vector<uint8_t> buffer;
    msgpack::Packer      packer(buffer);

    packer.pack_array_header(2);
    Person p1{"Alice", 30, {95.5}};
    Person p2{"Bob", 25, {88.0}};
    p1.msgpack_pack(packer);
    p2.msgpack_pack(packer);

    msgpack::Unpacker unpacker(buffer.data(), buffer.size());
    uint32_t          size;
    CHECK(unpacker.unpack_array_header(size));
    CHECK(size == 2);

    Person result1, result2;
    CHECK(result1.msgpack_unpack(unpacker));
    CHECK(result2.msgpack_unpack(unpacker));
    CHECK(result1.name == "Alice");
    CHECK(result2.name == "Bob");
}

TEST_CASE("msgpack - manual map packing") {
    std::vector<uint8_t> buffer;
    msgpack::Packer      packer(buffer);

    packer.pack_map_header(2);
    msgpack::pack(packer, std::string("name"));
    msgpack::pack(packer, std::string("Alice"));
    msgpack::pack(packer, std::string("age"));
    msgpack::pack(packer, 30);

    msgpack::Unpacker unpacker(buffer.data(), buffer.size());
    uint32_t          size;
    CHECK(unpacker.unpack_map_header(size));
    CHECK(size == 2);

    std::string key1, value1;
    CHECK(msgpack::unpack(unpacker, key1));
    CHECK(msgpack::unpack(unpacker, value1));
    CHECK(key1 == "name");
    CHECK(value1 == "Alice");

    std::string key2;
    int32_t     value2;
    CHECK(msgpack::unpack(unpacker, key2));
    CHECK(msgpack::unpack(unpacker, value2));
    CHECK(key2 == "age");
    CHECK(value2 == 30);
}

TEST_CASE("msgpack - empty containers") {
    std::vector<uint8_t> buffer;
    msgpack::Packer      packer(buffer);

    std::vector<int32_t> empty_vec;
    msgpack::pack(packer, empty_vec);

    msgpack::Unpacker    unpacker(buffer.data(), buffer.size());
    std::vector<int32_t> result;
    CHECK(msgpack::unpack(unpacker, result));
    CHECK(result.empty());
}

TEST_CASE("msgpack - large array") {
    std::vector<uint8_t> buffer;
    msgpack::Packer      packer(buffer);

    std::vector<int32_t> large_vec(1000);
    for (size_t i = 0; i < large_vec.size(); ++i) {
        large_vec[i] = static_cast<int32_t>(i);
    }
    msgpack::pack(packer, large_vec);

    msgpack::Unpacker    unpacker(buffer.data(), buffer.size());
    std::vector<int32_t> result;
    CHECK(msgpack::unpack(unpacker, result));
    CHECK(result.size() == 1000);
    CHECK(result[0] == 0);
    CHECK(result[999] == 999);
}
