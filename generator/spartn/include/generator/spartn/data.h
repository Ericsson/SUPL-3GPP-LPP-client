#ifndef SPARTN_DATA_
#define SPARTN_DATA_

#include <generator/spartn/time.hpp>
#include <iostream>

#include <algorithm>
#include <bitset>
#include <memory>
#include <queue>

class SPARTN_Data {
public:
    enum class DataType {
        Block,
        Field,
    };

    virtual ~SPARTN_Data() = default;
    virtual enum DataType get_data_type() const = 0;
};

class SPARTN_Block : public SPARTN_Data {
public:
    virtual ~SPARTN_Block() = default;

    enum DataType get_data_type() const override { return DataType::Block; }

    std::queue<std::unique_ptr<SPARTN_Data>> data = {};
};

class SPARTN_Field : public SPARTN_Data {
public:
    SPARTN_Field(const uint8_t id_, const uint8_t bit_count_, const int64_t bits_) {
        this->id        = id_;
        this->bit_count = bit_count_;
        this->bits      = bits_ < 0 ? bits_ & ((1 << bit_count_) - 1) : bits_;
    }

    SPARTN_Field(const uint8_t id_, const uint8_t bit_count_,
                 const std::bitset<Constants::max_spartn_bit_count> bits_) {
        this->id        = id_;
        this->bit_count = bit_count_;
        this->bits      = bits_;
    }

    virtual ~SPARTN_Field() = default;

    enum DataType get_data_type() const override { return DataType::Field; }

    uint8_t                                      id;
    uint8_t                                      bit_count;
    std::bitset<Constants::max_spartn_bit_count> bits = {};
};

#endif
