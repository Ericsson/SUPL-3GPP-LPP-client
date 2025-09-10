#include "cid.hpp"

#include <stdio.h>

namespace format {
namespace ctrl {

CellId::CellId(std::string message, uint32_t mcc, uint32_t mnc, uint32_t tac, uint64_t cell,
               bool is_nr) NOEXCEPT : Message(std::move(message)),
                                      mMcc(mcc),
                                      mMnc(mnc),
                                      mTac(tac),
                                      mCell(cell),
                                      mIsNr(is_nr) {}

CellId::~CellId() = default;

void CellId::print() const NOEXCEPT {
    printf("[  CID]: mcc=%u, mnc=%u, tac=%u, cell=%" PRIu64 ", a=%s", mMcc, mMnc, mTac, mCell,
           mIsNr ? "NR" : "LTE");
}

std::unique_ptr<Message> CellId::clone() const NOEXCEPT {
    return std::unique_ptr<Message>(new CellId(*this));
}

}  // namespace ctrl
}  // namespace format
