#pragma once
#include <supl/types.hpp>

namespace supl {

struct Cell {
    enum class Type {
        UNKNOWN = 0,
        GSM,
        LTE,
        NR,
    };

    Type type;
    union {
        struct {
            int64_t mcc;
            int64_t mnc;
            int64_t lac;
            int64_t ci;
        } gsm;

        struct {
            int64_t mcc;
            int64_t mnc;
            int64_t ci;
            int64_t tac;
            int64_t phys_id;
        } lte;
    } data;

    static Cell gsm(int64_t mcc, int64_t mnc, int64_t lac, int64_t ci) {
        return {.type = Type::GSM, .data{.gsm = {mcc, mnc, lac, ci}}};
    }

    static Cell lte(int64_t mcc, int64_t mnc, int64_t tac, int64_t ci, int64_t phys_id = 0) {
        return {.type = Type::LTE, .data{.lte = {mcc, mnc, ci, tac, phys_id}}};
    }
};

}  // namespace supl
