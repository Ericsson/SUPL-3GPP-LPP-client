#pragma once
#include <format/ctrl/message.hpp>

namespace format {
namespace ctrl {

class CellId : public Message {
public:
    CellId(std::string message, uint32_t mcc, uint32_t mnc, uint32_t tac, uint64_t cell,
           bool is_nr) NOEXCEPT;
    ~CellId() override;

    CellId(CellId const& other)
        : Message(other), mMcc(other.mMcc), mMnc(other.mMnc), mTac(other.mTac), mCell(other.mCell),
          mIsNr(other.mIsNr) {}
    CellId(CellId&&)                 = delete;
    CellId& operator=(CellId const&) = delete;
    CellId& operator=(CellId&&)      = delete;

    NODISCARD uint32_t mcc() const NOEXCEPT { return mMcc; }
    NODISCARD uint32_t mnc() const NOEXCEPT { return mMnc; }
    NODISCARD uint32_t tac() const NOEXCEPT { return mTac; }
    NODISCARD uint64_t cell() const NOEXCEPT { return mCell; }
    NODISCARD bool     is_nr() const NOEXCEPT { return mIsNr; }

    void      print() const NOEXCEPT override;
    NODISCARD std::unique_ptr<Message> clone() const NOEXCEPT override;

private:
    uint32_t mMcc;
    uint32_t mMnc;
    uint32_t mTac;
    uint64_t mCell;
    bool     mIsNr;
};

}  // namespace ctrl
}  // namespace format
