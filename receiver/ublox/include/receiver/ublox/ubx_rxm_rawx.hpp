#pragma once
#include <memory>
#include <receiver/ublox/message.hpp>
#include <string>
#include <vector>

namespace receiver {
namespace ublox {

namespace raw {
struct RxmRawx {
    /* 0x00 */ double   rcv_tow;
    /* 0x08 */ uint16_t week;
    /* 0x0A */ int8_t   leap_s;
    /* 0x0B */ uint8_t  num_meas;
    /* 0x0C */ struct {
        /* bit 0 */ uint8_t leap_sec;
        /* bit 1 */ uint8_t clk_reset;
    } rec_stat;
    /* 0x0D */ uint8_t  version;
    /* 0x0E */ uint16_t reserved0;
};

struct RxmRawxMeasurement {
    /* 0x00 */ double   pr_mes;
    /* 0x08 */ double   cp_mes;
    /* 0x10 */ float    do_mes;
    /* 0x14 */ uint8_t  gnss_id;
    /* 0x15 */ uint8_t  sv_id;
    /* 0x16 */ uint8_t  sig_id;
    /* 0x17 */ uint8_t  freq_id;
    /* 0x18 */ uint16_t locktime;
    /* 0x1A */ uint8_t  cno;
    /* 0x1B */ uint8_t  pr_stdev;
    /* 0x1C */ uint8_t  cp_stdev;
    /* 0x1D */ uint8_t  do_stdev;
    /* 0x1E */ struct {
        /* bit 0 */ uint8_t pr_valid;
        /* bit 1 */ uint8_t cp_valid;
        /* bit 2 */ uint8_t half_cyc;
        /* bit 3 */ uint8_t sub_half_cyc;
    } trk_stat;
    /* 0x1F */ uint8_t reserved0;
};
}  // namespace raw

class Decoder;
class Encoder;
class UbxRxmRawx : public Message {
public:
    UBLOX_CONSTEXPR static uint8_t CLASS_ID   = 0x02;
    UBLOX_CONSTEXPR static uint8_t MESSAGE_ID = 0x15;

    UBLOX_EXPLICIT UbxRxmRawx(raw::RxmRawx                         payload,
                              std::vector<raw::RxmRawxMeasurement> measurements,
                              std::vector<uint8_t>&&               data) UBLOX_NOEXCEPT;
    ~UbxRxmRawx() override = default;

    UbxRxmRawx(UbxRxmRawx const& other) : Message(other), mPayload(other.mPayload) {}
    UbxRxmRawx(UbxRxmRawx&&)                 = delete;
    UbxRxmRawx& operator=(UbxRxmRawx const&) = delete;
    UbxRxmRawx& operator=(UbxRxmRawx&&)      = delete;

    UBLOX_NODISCARD const raw::RxmRawx& payload() const UBLOX_NOEXCEPT { return mPayload; }

    UBLOX_NODISCARD double   rcv_tow() const UBLOX_NOEXCEPT { return mPayload.rcv_tow; }
    UBLOX_NODISCARD uint16_t week() const UBLOX_NOEXCEPT { return mPayload.week; }
    UBLOX_NODISCARD int8_t   leap_s() const UBLOX_NOEXCEPT { return mPayload.leap_s; }
    UBLOX_NODISCARD uint8_t  num_meas() const UBLOX_NOEXCEPT { return mPayload.num_meas; }
    UBLOX_NODISCARD bool     rec_stat_leap_sec() const UBLOX_NOEXCEPT {
        return mPayload.rec_stat.leap_sec;
    }
    UBLOX_NODISCARD bool rec_stat_clk_reset() const UBLOX_NOEXCEPT {
        return mPayload.rec_stat.clk_reset;
    }
    UBLOX_NODISCARD uint8_t version() const UBLOX_NOEXCEPT { return mPayload.version; }

    UBLOX_NODISCARD std::vector<raw::RxmRawxMeasurement> const&
                    measurements() const UBLOX_NOEXCEPT {
        return mMeasurements;
    }

    void print() const UBLOX_NOEXCEPT override;

    UBLOX_NODISCARD static std::unique_ptr<Message> parse(Decoder&             decoder,
                                                          std::vector<uint8_t> data) UBLOX_NOEXCEPT;

private:
    raw::RxmRawx                         mPayload;
    std::vector<raw::RxmRawxMeasurement> mMeasurements;
};

}  // namespace ublox
}  // namespace receiver
