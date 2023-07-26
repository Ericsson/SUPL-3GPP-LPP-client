#pragma once
#include <osr/osr.h>

struct MessageFilter {
    // Tell RTCM-Generator which MSM messages you're interested in. The output
    // will be the highest MSM message possible with the data available.
    struct {
        bool msm4 = true;  // if possible generate MSM4 messages
        bool msm5 = true;  // if possible generate MSM5 messages
        bool msm7 = true;  // if possible generate MSM7 messages
    } msm;

    struct {
        bool mt1006 = true;
        bool mt1032 = true;
    } reference_station;

    bool mt1030 = true;
    bool mt1031 = true;

    bool mt1230 = true;
};

struct GNSSSystems {
    bool gps;
    bool glonass;
    bool galileo;
    bool beidou;
};

struct Generated {
    int  msm;
    bool mt1074;
    bool mt1075;
    bool mt1076;
    bool mt1077;
    bool mt1084;
    bool mt1085;
    bool mt1086;
    bool mt1087;
    bool mt1094;
    bool mt1095;
    bool mt1096;
    bool mt1097;
    bool mt1124;
    bool mt1125;
    bool mt1126;
    bool mt1127;
    bool mt1006;
    bool mt1032;
    bool mt1030;
    bool mt1031;
    bool mt1230;
};

struct Message {
    
};

struct LPP_Message;
class RTCMGenerator {
public:
    RTCMGenerator() = default;
    RTCMGenerator(GNSSSystems, MessageFilter);

    void update_filter(MessageFilter);

    // Process LPP messages with support for segementation.
    // Returns true if this the last message of an epoch and the convert function can be call.
    bool process(LPP_Message* message);

    // Generate RTCM messages from LPP message.
    // Returns bytes written to 'buffer' and 'buffer_size' is updated to
    // required size.
    size_t convert(unsigned char* buffer, size_t* buffer_size, Generated* = NULL);

    size_t generate(OSR* osr, bool got_reference_station, unsigned char* buffer,
                    size_t* buffer_size, Generated* generated = NULL);

private:
    bool verify_msm4(OSR_GNSS* gnss);
    bool verify_msm5(OSR_GNSS* gnss);
    bool verify_msm7(OSR_GNSS* gnss);

private:
    GNSSSystems   gnss_systems;
    MessageFilter filter;
    OSR           osr;

    bool mExpectSegmentedMessage;
    bool mGotReferenceStation;
    time_t ref_station_last_sent;
};