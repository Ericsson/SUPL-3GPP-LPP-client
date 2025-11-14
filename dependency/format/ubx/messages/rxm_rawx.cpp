#include "messages/rxm_rawx.hpp"
#include "decoder.hpp"
#include "encoder.hpp"
#include "parser.hpp"

#include <cstdio>
#include <loglet/loglet.hpp>

LOGLET_MODULE2(ubx, rawx);
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF2(ubx, rawx)

namespace format {
namespace ubx {

UbxRxmRawx::UbxRxmRawx(raw::RxmRawx payload, std::vector<raw::RxmRawxMeasurement> measurements,
                       std::vector<uint8_t>&& data) NOEXCEPT
    : Message(CLASS_ID, MESSAGE_ID, std::move(data)),
      mPayload(std::move(payload)),
      mMeasurements(std::move(measurements)) {}

void UbxRxmRawx::print() const NOEXCEPT {
    printf("[%02X %02X] UBX-RXM-RAWX: tow=%f, week=%u, num_meas=%u\n", message_class(),
           message_id(), mPayload.rcv_tow, mPayload.week, mPayload.num_meas);
}

std::unique_ptr<Message> UbxRxmRawx::clone() const NOEXCEPT {
    return std::unique_ptr<Message>{new UbxRxmRawx{*this}};
}

std::unique_ptr<Message> UbxRxmRawx::parse(Decoder& decoder, std::vector<uint8_t> data) NOEXCEPT {
    FUNCTION_SCOPE();
    if (decoder.remaining() < 16) {
        VERBOSEF("not enough data for payload");
        return nullptr;
    }

    auto rcv_tow   = decoder.r8();
    auto week      = decoder.u2();
    auto leap_s    = decoder.i1();
    auto num_meas  = decoder.u1();
    auto rec_stat  = decoder.x1();
    auto version   = decoder.u1();
    auto reserved0 = decoder.u2();
    if (decoder.error()) {
        VERBOSEF("failed to decode payload");
        return nullptr;
    }

    raw::RxmRawx payload;
    payload.rcv_tow            = rcv_tow;
    payload.week               = week;
    payload.leap_s             = leap_s;
    payload.num_meas           = num_meas;
    payload.rec_stat.leap_sec  = (rec_stat >> 0) & 0x01;
    payload.rec_stat.clk_reset = (rec_stat >> 1) & 0x01;
    payload.version            = version;
    payload.reserved0          = reserved0;

    DEBUGF("rcv_tow=%f", rcv_tow);
    DEBUGF("week=%u", week);
    DEBUGF("leap_s=%d", leap_s);
    DEBUGF("num_meas=%u", num_meas);
    DEBUGF("rec_stat=0x%02X", rec_stat);
    DEBUGF("version=%u", version);

    std::vector<raw::RxmRawxMeasurement> measurements;
    for (uint8_t i = 0; i < num_meas; i++) {
        auto pr_mes         = decoder.r8();
        auto cp_mes         = decoder.r8();
        auto do_mes         = decoder.r4();
        auto gnss_id        = decoder.u1();
        auto sv_id          = decoder.u1();
        auto sig_id         = decoder.u1();
        auto freq_id        = decoder.u1();
        auto locktime       = decoder.u2();
        auto cno            = decoder.u1();
        auto pr_stdev       = decoder.x1();
        auto cp_stdev       = decoder.x1();
        auto do_stdev       = decoder.x1();
        auto trk_stat       = decoder.x1();
        auto meas_reserved0 = decoder.u1();
        if (decoder.error()) {
            VERBOSEF("failed to decode measurement %u", i);
            return nullptr;
        }

        raw::RxmRawxMeasurement measurement;
        measurement.pr_mes                = pr_mes;
        measurement.cp_mes                = cp_mes;
        measurement.do_mes                = do_mes;
        measurement.gnss_id               = gnss_id;
        measurement.sv_id                 = sv_id;
        measurement.sig_id                = sig_id;
        measurement.freq_id               = freq_id;
        measurement.locktime              = locktime;
        measurement.cno                   = cno;
        measurement.pr_stdev              = pr_stdev;
        measurement.cp_stdev              = cp_stdev;
        measurement.do_stdev              = do_stdev;
        measurement.trk_stat.pr_valid     = (trk_stat >> 0) & 0x01;
        measurement.trk_stat.cp_valid     = (trk_stat >> 1) & 0x01;
        measurement.trk_stat.half_cyc     = (trk_stat >> 2) & 0x01;
        measurement.trk_stat.sub_half_cyc = (trk_stat >> 3) & 0x01;
        measurement.reserved0             = meas_reserved0;
        measurements.push_back(measurement);
    }

    if (decoder.error()) {
        VERBOSEF("failed to decode measurements");
        return nullptr;
    } else {
        VERBOSEF("decoded %zu measurements", measurements.size());
        return std::unique_ptr<Message>{
            new UbxRxmRawx(std::move(payload), std::move(measurements), std::move(data))};
    }
}

}  // namespace ubx
}  // namespace format
