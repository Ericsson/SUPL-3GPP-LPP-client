#pragma once
#include <core/core.hpp>
#include <loglet/loglet.hpp>

LOGLET_MODULE_FORWARD_REF(supl);
#undef LOGLET_CURRENT_MODULE
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF(supl)

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
            uint64_t mcc;
            uint64_t mnc;
            uint64_t lac;
            uint64_t ci;
        } gsm;

        struct {
            uint64_t mcc;
            uint64_t mnc;
            uint64_t ci;
            uint64_t tac;
            uint64_t phys_id;
        } lte;

        struct {
            uint64_t mcc;
            uint64_t mnc;
            uint64_t ci;
            uint64_t tac;
            uint64_t phys_id;
        } nr;
    } data;

    static Cell gsm(uint64_t mcc, uint64_t mnc, uint64_t lac, uint64_t ci) {
        Cell cell{};
        cell.type         = Type::GSM;
        cell.data.gsm.mcc = mcc;
        cell.data.gsm.mnc = mnc;
        cell.data.gsm.lac = lac;
        cell.data.gsm.ci  = ci;
        return cell;
    }

    static Cell lte(uint64_t mcc, uint64_t mnc, uint64_t tac, uint64_t ci, uint64_t phys_id = 0) {
        Cell cell{};
        cell.type             = Type::LTE;
        cell.data.lte.mcc     = mcc;
        cell.data.lte.mnc     = mnc;
        cell.data.lte.ci      = ci;
        cell.data.lte.tac     = tac;
        cell.data.lte.phys_id = phys_id;
        return cell;
    }

    static Cell nr(uint64_t mcc, uint64_t mnc, uint64_t tac, uint64_t ci, uint64_t phys_id = 0) {
        Cell cell{};
        cell.type            = Type::NR;
        cell.data.nr.mcc     = mcc;
        cell.data.nr.mnc     = mnc;
        cell.data.nr.ci      = ci;
        cell.data.nr.tac     = tac;
        cell.data.nr.phys_id = phys_id;
        return cell;
    }
};

inline bool operator==(Cell const& lhs, Cell const& rhs) {
    if (lhs.type != rhs.type) return false;

    switch (lhs.type) {
    case Cell::Type::GSM:
        return lhs.data.gsm.mcc == rhs.data.gsm.mcc && lhs.data.gsm.mnc == rhs.data.gsm.mnc &&
               lhs.data.gsm.lac == rhs.data.gsm.lac && lhs.data.gsm.ci == rhs.data.gsm.ci;
    case Cell::Type::LTE:
        return lhs.data.lte.mcc == rhs.data.lte.mcc && lhs.data.lte.mnc == rhs.data.lte.mnc &&
               lhs.data.lte.tac == rhs.data.lte.tac && lhs.data.lte.ci == rhs.data.lte.ci;
    case Cell::Type::NR:
        return lhs.data.nr.mcc == rhs.data.nr.mcc && lhs.data.nr.mnc == rhs.data.nr.mnc &&
               lhs.data.nr.tac == rhs.data.nr.tac && lhs.data.nr.ci == rhs.data.nr.ci;
    case Cell::Type::UNKNOWN: break;
    }

    UNREACHABLE();
    return false;
}

inline bool operator!=(Cell const& lhs, Cell const& rhs) {
    return !(lhs == rhs);
}

}  // namespace supl

#undef LOGLET_CURRENT_MODULE
