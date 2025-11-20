#include <doctest/doctest.h>
#include <msgpack/msgpack.hpp>
#include <msgpack/vector.hpp>

TEST_CASE("msgpack - nil") {
    std::vector<uint8_t> buffer;
    msgpack::Packer      packer(buffer);

    packer.pack_nil();

    CHECK(buffer.size() == 1);
    CHECK(buffer[0] == 0xc0);

    msgpack::Unpacker unpacker(buffer.data(), buffer.size());
    CHECK(unpacker.unpack_nil());
}

TEST_CASE("msgpack - bool") {
    std::vector<uint8_t> buffer;
    msgpack::Packer      packer(buffer);

    packer.pack_bool(true);
    packer.pack_bool(false);

    msgpack::Unpacker unpacker(buffer.data(), buffer.size());
    bool              value;
    CHECK(unpacker.unpack_bool(value));
    CHECK(value == true);
    CHECK(unpacker.unpack_bool(value));
    CHECK(value == false);
}

TEST_CASE("msgpack - positive fixint") {
    std::vector<uint8_t> buffer;
    msgpack::Packer      packer(buffer);

    packer.pack_uint(42);

    CHECK(buffer.size() == 1);
    CHECK(buffer[0] == 42);

    msgpack::Unpacker unpacker(buffer.data(), buffer.size());
    uint64_t          value;
    CHECK(unpacker.unpack_uint(value));
    CHECK(value == 42);
}

TEST_CASE("msgpack - negative fixint") {
    std::vector<uint8_t> buffer;
    msgpack::Packer      packer(buffer);

    packer.pack_int(-5);

    CHECK(buffer.size() == 1);

    msgpack::Unpacker unpacker(buffer.data(), buffer.size());
    int64_t           value;
    CHECK(unpacker.unpack_int(value));
    CHECK(value == -5);
}

TEST_CASE("msgpack - uint8") {
    std::vector<uint8_t> buffer;
    msgpack::Packer      packer(buffer);

    packer.pack_uint(200);

    msgpack::Unpacker unpacker(buffer.data(), buffer.size());
    uint64_t          value;
    CHECK(unpacker.unpack_uint(value));
    CHECK(value == 200);
}

TEST_CASE("msgpack - int8") {
    std::vector<uint8_t> buffer;
    msgpack::Packer      packer(buffer);

    packer.pack_int(-100);

    msgpack::Unpacker unpacker(buffer.data(), buffer.size());
    int64_t           value;
    CHECK(unpacker.unpack_int(value));
    CHECK(value == -100);
}

TEST_CASE("msgpack - uint16") {
    std::vector<uint8_t> buffer;
    msgpack::Packer      packer(buffer);

    packer.pack_uint(50000);

    msgpack::Unpacker unpacker(buffer.data(), buffer.size());
    uint64_t          value;
    CHECK(unpacker.unpack_uint(value));
    CHECK(value == 50000);
}

TEST_CASE("msgpack - int16") {
    std::vector<uint8_t> buffer;
    msgpack::Packer      packer(buffer);

    packer.pack_int(-30000);

    msgpack::Unpacker unpacker(buffer.data(), buffer.size());
    int64_t           value;
    CHECK(unpacker.unpack_int(value));
    CHECK(value == -30000);
}

TEST_CASE("msgpack - uint32") {
    std::vector<uint8_t> buffer;
    msgpack::Packer      packer(buffer);

    packer.pack_uint(3000000000);

    msgpack::Unpacker unpacker(buffer.data(), buffer.size());
    uint64_t          value;
    CHECK(unpacker.unpack_uint(value));
    CHECK(value == 3000000000);
}

TEST_CASE("msgpack - int32") {
    std::vector<uint8_t> buffer;
    msgpack::Packer      packer(buffer);

    packer.pack_int(-2000000000);

    msgpack::Unpacker unpacker(buffer.data(), buffer.size());
    int64_t           value;
    CHECK(unpacker.unpack_int(value));
    CHECK(value == -2000000000);
}

TEST_CASE("msgpack - uint64") {
    std::vector<uint8_t> buffer;
    msgpack::Packer      packer(buffer);

    packer.pack_uint(10000000000ULL);

    msgpack::Unpacker unpacker(buffer.data(), buffer.size());
    uint64_t          value;
    CHECK(unpacker.unpack_uint(value));
    CHECK(value == 10000000000ULL);
}

TEST_CASE("msgpack - int64") {
    std::vector<uint8_t> buffer;
    msgpack::Packer      packer(buffer);

    packer.pack_int(-5000000000LL);

    msgpack::Unpacker unpacker(buffer.data(), buffer.size());
    int64_t           value;
    CHECK(unpacker.unpack_int(value));
    CHECK(value == -5000000000LL);
}

TEST_CASE("msgpack - float") {
    std::vector<uint8_t> buffer;
    msgpack::Packer      packer(buffer);

    packer.pack_float(3.14f);

    msgpack::Unpacker unpacker(buffer.data(), buffer.size());
    float             value;
    CHECK(unpacker.unpack_float(value));
    CHECK(value == doctest::Approx(3.14f));
}

TEST_CASE("msgpack - double") {
    std::vector<uint8_t> buffer;
    msgpack::Packer      packer(buffer);

    packer.pack_double(3.141592653589793);

    msgpack::Unpacker unpacker(buffer.data(), buffer.size());
    double            value;
    CHECK(unpacker.unpack_double(value));
    CHECK(value == doctest::Approx(3.141592653589793));
}

TEST_CASE("msgpack - fixstr") {
    std::vector<uint8_t> buffer;
    msgpack::Packer      packer(buffer);

    packer.pack_str("hello", 5);

    msgpack::Unpacker unpacker(buffer.data(), buffer.size());
    char const*       data;
    uint32_t          length;
    CHECK(unpacker.unpack_str(data, length));
    CHECK(length == 5);
    CHECK(std::string(data, length) == "hello");
}

TEST_CASE("msgpack - str8") {
    std::vector<uint8_t> buffer;
    msgpack::Packer      packer(buffer);

    std::string str(100, 'a');
    packer.pack_str(str.data(), static_cast<uint32_t>(str.size()));

    msgpack::Unpacker unpacker(buffer.data(), buffer.size());
    char const*       data;
    uint32_t          length;
    CHECK(unpacker.unpack_str(data, length));
    CHECK(length == 100);
}

TEST_CASE("msgpack - bin8") {
    std::vector<uint8_t> buffer;
    msgpack::Packer      packer(buffer);

    uint8_t bin_data[] = {0x01, 0x02, 0x03, 0x04};
    packer.pack_bin(bin_data, 4);

    msgpack::Unpacker unpacker(buffer.data(), buffer.size());
    uint8_t const*    data;
    uint32_t          length;
    CHECK(unpacker.unpack_bin(data, length));
    CHECK(length == 4);
    CHECK(data[0] == 0x01);
    CHECK(data[3] == 0x04);
}

TEST_CASE("msgpack - fixarray") {
    std::vector<uint8_t> buffer;
    msgpack::Packer      packer(buffer);

    packer.pack_array_header(3);
    packer.pack_uint(1);
    packer.pack_uint(2);
    packer.pack_uint(3);

    msgpack::Unpacker unpacker(buffer.data(), buffer.size());
    uint32_t          size;
    CHECK(unpacker.unpack_array_header(size));
    CHECK(size == 3);

    uint64_t value;
    CHECK(unpacker.unpack_uint(value));
    CHECK(value == 1);
    CHECK(unpacker.unpack_uint(value));
    CHECK(value == 2);
    CHECK(unpacker.unpack_uint(value));
    CHECK(value == 3);
}

TEST_CASE("msgpack - fixmap") {
    std::vector<uint8_t> buffer;
    msgpack::Packer      packer(buffer);

    packer.pack_map_header(2);
    packer.pack_str("key1", 4);
    packer.pack_uint(100);
    packer.pack_str("key2", 4);
    packer.pack_uint(200);

    msgpack::Unpacker unpacker(buffer.data(), buffer.size());
    uint32_t          size;
    CHECK(unpacker.unpack_map_header(size));
    CHECK(size == 2);
}

TEST_CASE("msgpack - std::string") {
    std::vector<uint8_t> buffer;
    msgpack::Packer      packer(buffer);

    std::string str = "test string";
    msgpack::pack(packer, str);

    msgpack::Unpacker unpacker(buffer.data(), buffer.size());
    std::string       result;
    CHECK(msgpack::unpack(unpacker, result));
    CHECK(result == "test string");
}

TEST_CASE("msgpack - std::vector<int>") {
    std::vector<uint8_t> buffer;
    msgpack::Packer      packer(buffer);

    std::vector<int32_t> vec = {1, 2, 3, 4, 5};
    msgpack::pack(packer, vec);

    msgpack::Unpacker    unpacker(buffer.data(), buffer.size());
    std::vector<int32_t> result;
    CHECK(msgpack::unpack(unpacker, result));
    CHECK(result.size() == 5);
    CHECK(result[0] == 1);
    CHECK(result[4] == 5);
}

struct Point {
    int32_t x;
    int32_t y;

    MSGPACK_DEFINE(x, y)
};

TEST_CASE("msgpack - custom type with MSGPACK_DEFINE") {
    std::vector<uint8_t> buffer;
    msgpack::Packer      packer(buffer);

    Point p{10, 20};
    p.msgpack_pack(packer);

    msgpack::Unpacker unpacker(buffer.data(), buffer.size());
    Point             result{0, 0};
    CHECK(result.msgpack_unpack(unpacker));
    CHECK(result.x == 10);
    CHECK(result.y == 20);
}
