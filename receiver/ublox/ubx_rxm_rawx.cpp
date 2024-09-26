#include "ubx_rxm_rawx.hpp"

#include <stdio.h>

#include "decoder.hpp"
#include "encoder.hpp"
#include "parser.hpp"

namespace receiver {
namespace ublox {

UbxRxmRawx::UbxRxmRawx(raw::RxmRawx payload, std::vector<raw::RxmRawxMeasurement> measurements,
                       std::vector<uint8_t>&& data) UBLOX_NOEXCEPT
    : Message(CLASS_ID, MESSAGE_ID, std::move(data)),
      mPayload(std::move(payload)),
      mMeasurements(std::move(measurements)) {}

void UbxRxmRawx::print() const UBLOX_NOEXCEPT {
    printf("[%02X %02X] UBX-RXM-RAWX: tow=%f, week=%u, num_meas=%u\n", message_class(),
           message_id(), mPayload.rcv_tow, mPayload.week, mPayload.num_meas);
}

std::unique_ptr<Message> UbxRxmRawx::parse(Decoder&             decoder,
                                           std::vector<uint8_t> data) UBLOX_NOEXCEPT {
    if (decoder.remaining() < 16) {
        return nullptr;
    }

    auto rcv_tow   = decoder.R8();
    auto week      = decoder.U2();
    auto leap_s    = decoder.I1();
    auto num_meas  = decoder.U1();
    auto rec_stat  = decoder.X1();
    auto version   = decoder.U1();
    auto reserved0 = decoder.U2();
    if (decoder.error()) {
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

    std::vector<raw::RxmRawxMeasurement> measurements;
    for (uint8_t i = 0; i < num_meas; i++) {
        auto pr_mes    = decoder.R8();
        auto cp_mes    = decoder.R8();
        auto do_mes    = decoder.R4();
        auto gnss_id   = decoder.U1();
        auto sv_id     = decoder.U1();
        auto sig_id    = decoder.U1();
        auto freq_id   = decoder.U1();
        auto locktime  = decoder.U2();
        auto cno       = decoder.U1();
        auto pr_stdev  = decoder.X1();
        auto cp_stdev  = decoder.X1();
        auto do_stdev  = decoder.X1();
        auto trk_stat  = decoder.X1();
        auto reserved0 = decoder.U1();
        if (decoder.error()) {
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
        measurement.reserved0             = reserved0;
        measurements.push_back(measurement);
    }

    if (decoder.error()) {
        return nullptr;
    } else {
        return std::unique_ptr<Message>{
            new UbxRxmRawx(std::move(payload), std::move(measurements), std::move(data))};
    }
}

}  // namespace ublox
}  // namespace receiver
