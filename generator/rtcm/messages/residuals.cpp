#include "residuals.hpp"
#include "encoder.hpp"
#include "time/gps_time.hpp"
#include "time/glo_time.hpp"

using namespace generator::rtcm;

extern generator::rtcm::Message generate_1030(const Residuals& residuals) {
    auto message_id = 1030U;
    auto time       = ts::GPS_Time(residuals.time).time_of_week().seconds();

    auto encoder = Encoder();
    encoder.u16(12, message_id);
    encoder.u32(20, static_cast<uint32_t>(time));
    encoder.u16(12, residuals.reference_station_id);
    encoder.u8(7, residuals.n_refs);

    auto satellite_count = 0;
    for (auto& satellite : residuals.satellites) {
        if (satellite.id.gnss() != SatelliteId::Gnss::GPS) continue;
        if (!satellite.id.as_df009().valid) continue;
        satellite_count++;
    }

    encoder.u8(5, static_cast<uint8_t>(satellite_count));

    for (auto& satellite : residuals.satellites) {
        if (satellite.id.gnss() != SatelliteId::Gnss::GPS) continue;
        if (!satellite.id.as_df009().valid) continue;

        encoder.u8(6, satellite.id.as_df009().value);
        encoder.u32(8, static_cast<uint32_t>(satellite.s_oc / 0.5));
        encoder.u32(9, static_cast<uint32_t>(satellite.s_od / 0.01));
        encoder.u32(6, static_cast<uint32_t>(satellite.s_oh / 0.1));
        encoder.u32(10, static_cast<uint32_t>(satellite.s_lc / 0.5));
        encoder.u32(10, static_cast<uint32_t>(satellite.s_ld / 0.01));
    }

    auto frame_encoder = Encoder();
    frame_encoder.u8(8, 0xD3);
    frame_encoder.u8(6, 0);
    frame_encoder.u16(10, encoder.byte_count());
    frame_encoder.copy(encoder.buffer());
    frame_encoder.checksum();

    return generator::rtcm::Message(message_id, frame_encoder.buffer());
}

extern generator::rtcm::Message generate_1031(const Residuals& residuals) {
    auto message_id = 1031U;
    auto time       = ts::GLO_Time(residuals.time).time_of_day().seconds();

    auto encoder = Encoder();
    encoder.u16(12, message_id);
    encoder.u32(17, static_cast<uint32_t>(time));
    encoder.u16(12, residuals.reference_station_id);
    encoder.u8(7, residuals.n_refs);

    auto satellite_count = 0;
    for (auto& satellite : residuals.satellites) {
        if (satellite.id.gnss() != SatelliteId::Gnss::GLONASS) continue;
        if (!satellite.id.as_df038().valid) continue;
        satellite_count++;
    }

    encoder.u8(5, static_cast<uint8_t>(satellite_count));

    for (auto& satellite : residuals.satellites) {
        if (satellite.id.gnss() != SatelliteId::Gnss::GLONASS) continue;
        if (!satellite.id.as_df038().valid) continue;

        encoder.u8(6, satellite.id.as_df038().value);
        encoder.u32(8, static_cast<uint32_t>(satellite.s_oc / 0.5));
        encoder.u32(9, static_cast<uint32_t>(satellite.s_od / 0.01));
        encoder.u32(6, static_cast<uint32_t>(satellite.s_oh / 0.1));
        encoder.u32(10, static_cast<uint32_t>(satellite.s_lc / 0.5));
        encoder.u32(10, static_cast<uint32_t>(satellite.s_ld / 0.01));
    }

    auto frame_encoder = Encoder();
    frame_encoder.u8(8, 0xD3);
    frame_encoder.u8(6, 0);
    frame_encoder.u16(10, encoder.byte_count());
    frame_encoder.copy(encoder.buffer());
    frame_encoder.checksum();

    return generator::rtcm::Message(message_id, frame_encoder.buffer());
}
