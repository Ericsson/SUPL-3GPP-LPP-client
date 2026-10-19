#pragma once
#include <cstdint>
#include <string>

#include <client-io/types.hpp>

/// Sourcetable "STR;" entry fields, see NTRIP 1.0/2.0 specification.
struct MountpointOptions {
    std::string mountpoint = "STREAM";
    std::string identifier = "SUPL-3GPP-LPP-client";
    std::string format     = "RTCM 3.3";
    std::string format_details;
    std::string carrier     = "2";
    std::string nav_system  = "GPS+GLO+GAL+BDS";
    std::string network     = "SUPL-3GPP-LPP-client";
    std::string country     = "SWE";
    double      latitude    = 0.0;
    double      longitude   = 0.0;
    bool        nmea        = false;
    bool        solution    = false;
    std::string generator   = "SUPL-3GPP-LPP-client";
    std::string compr_encrp = "none";
    std::string bitrate     = "0";
    std::string misc;

    bool username_required = false;
};

struct CasterOptions {
    std::string address = "0.0.0.0";
    uint16_t    port    = 2101;

    MountpointOptions mountpoint;

    std::string username;
    std::string password;
};

struct CasterConfig {
    CasterOptions caster;
    InputsConfig  inputs;
};

extern CasterConfig parse_configuration(int argc, char** argv);
