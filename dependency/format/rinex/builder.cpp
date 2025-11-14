#include "builder.hpp"

#include <algorithm>
#include <iomanip>
#include <unordered_set>

#include <loglet/loglet.hpp>
#include <time/gps.hpp>
#include <time/utc.hpp>
#include <version.hpp>

LOGLET_MODULE2(format, rinex);
#undef LOGLET_CURRENT_MODULE
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF2(format, rinex)

namespace format {
namespace rinex {

static char const* observation_data_type(bool gps, bool glo, bool gal, bool bds) {
    auto count = 0;
    count += gps ? 1 : 0;
    count += glo ? 1 : 0;
    count += gal ? 1 : 0;
    count += bds ? 1 : 0;
    if (count > 1) {
        return "M: MIXED";
    }

    if (gps) {
        return "G: GPS";
    } else if (glo) {
        return "R: GLONASS";
    } else if (gal) {
        return "E: Galileo";
    } else if (bds) {
        return "C: BDS";
    } else {
        return "M: MIXED";
    }
}

static SignalId const gGpsSignals[4] = {
    SignalId::GPS_L1_CA,
    // SignalId::GPS_L1_P,
    // SignalId::GPS_L1_Z_TRACKING,
    // SignalId::GPS_L2_C_A,
    // SignalId::GPS_L2_P,
    SignalId::GPS_L2_Z_TRACKING,
    // SignalId::GPS_L2_L2C_M,
    // SignalId::GPS_L2_L2C_L,
    SignalId::GPS_L2_L2C_M_L,
    // SignalId::GPS_L5_I,
    // SignalId::GPS_L5_Q,
    SignalId::GPS_L5_I_Q,
    // SignalId::GPS_L1_L1C_D,
    // SignalId::GPS_L1_L1C_P,
    // SignalId::GPS_L1_L1C_D_P,
};

static SignalId const gGloSignals[2] = {
    SignalId::GLONASS_G1_CA, SignalId::GLONASS_G2_CA,
    // SignalId::GLONASS_G1_P,
    // SignalId::GLONASS_G2_P,
    // SignalId::GLONASS_G1A_D,
    // SignalId::GLONASS_G1A_P,
    // SignalId::GLONASS_G1A_D_P,
    // SignalId::GLONASS_G2A_I,
    // SignalId::GLONASS_G2A_P,
    // SignalId::GLONASS_G2A_I_P,
    // SignalId::GLONASS_G3_I,
    // SignalId::GLONASS_G3_Q,
    // SignalId::GLONASS_G3_I_Q,
};

static SignalId const gGalSignals[4] = {
    // SignalId::GALILEO_E1_C_NO_DATA,
    // SignalId::GALILEO_E1_A,
    // SignalId::GALILEO_E1_B_I_NAV_OS_CS_SOL,
    SignalId::GALILEO_E1_B_C,
    // SignalId::GALILEO_E1_A_B_C,
    // SignalId::GALILEO_E6_C,
    // SignalId::GALILEO_E6_A,
    // SignalId::GALILEO_E6_B,
    // SignalId::GALILEO_E6_B_C,
    // SignalId::GALILEO_E6_A_B_C,
    // SignalId::GALILEO_E5B_I,
    // SignalId::GALILEO_E5B_Q,
    SignalId::GALILEO_E5B_I_Q,
    // SignalId::GALILEO_E5_A_B_I,
    // SignalId::GALILEO_E5_A_B_Q,
    SignalId::GALILEO_E5_A_B_I_Q,
    // SignalId::GALILEO_E5A_I,
    // SignalId::GALILEO_E5A_Q,
    SignalId::GALILEO_E5A_I_Q,
};

static SignalId const gBdsSignals[4] = {
    SignalId::BEIDOU_B1_I,
    // SignalId::BEIDOU_B1_Q,
    // SignalId::BEIDOU_B1_I_Q,
    SignalId::BEIDOU_B3_I,
    // SignalId::BEIDOU_B3_Q,
    // SignalId::BEIDOU_B3_I_Q,
    // SignalId::BEIDOU_B2_I,
    // SignalId::BEIDOU_B2_Q,
    // SignalId::BEIDOU_B2_I_Q,
    // SignalId::BEIDOU_B1C_D,
    // SignalId::BEIDOU_B1C_P,
    SignalId::BEIDOU_B1C_D_P,
    // SignalId::BEIDOU_B2A_D,
    // SignalId::BEIDOU_B2A_P,
    SignalId::BEIDOU_B2A_D_P,
};

Builder::Builder(std::string path, double version) NOEXCEPT : mVersion(version),
                                                              mPath(std::move(path)) {
    VSCOPE_FUNCTION();
    mProgram = "Tokoro";
    mRunBy   = "Tokoro";

    mMarkerName = "Tokoro";
    mMarkerType = "GEODETIC";

    mObserver = "Tokoro";
    mAgency   = "Tokoro";

    mReceiverNumber  = "NA";
    mReceiverType    = "NA";
    mReceiverVersion = "NA";

    mAntennaSerial = "0";
    mAntennaNumber = "GPPNULLANTENNA";
    mAntennaType   = "NONE";

    mApproxPosition = {0.0, 0.0, 0.0};
    mAntennaDelta   = {0.0, 0.0, 0.0};

    mInitialized = false;
}

void Builder::generate_observation_order() {
    mGpsTypeOrder.clear();
    mGloTypeOrder.clear();
    mGalTypeOrder.clear();
    mBdsTypeOrder.clear();

    if (mGpsSupport) {
        for (auto const& signal : gGpsSignals) {
            mGpsTypeOrder.push_back({ObservationKind::Code, signal});
            mGpsTypeOrder.push_back({ObservationKind::Phase, signal});
            // mGpsTypeOrder.push_back({ObservationKind::Doppler, signal});
            mGpsTypeOrder.push_back({ObservationKind::SignalStrength, signal});
        }
    }

    if (mGloSupport) {
        for (auto const& signal : gGloSignals) {
            mGloTypeOrder.push_back({ObservationKind::Code, signal});
            mGloTypeOrder.push_back({ObservationKind::Phase, signal});
            // mGloTypeOrder.push_back({ObservationKind::Doppler, signal});
            mGloTypeOrder.push_back({ObservationKind::SignalStrength, signal});
        }
    }

    if (mGalSupport) {
        for (auto const& signal : gGalSignals) {
            mGalTypeOrder.push_back({ObservationKind::Code, signal});
            mGalTypeOrder.push_back({ObservationKind::Phase, signal});
            // mGalTypeOrder.push_back({ObservationKind::Doppler, signal});
            mGalTypeOrder.push_back({ObservationKind::SignalStrength, signal});
        }
    }

    if (mBdsSupport) {
        for (auto const& signal : gBdsSignals) {
            mBdsTypeOrder.push_back({ObservationKind::Code, signal});
            mBdsTypeOrder.push_back({ObservationKind::Phase, signal});
            // mBdsTypeOrder.push_back({ObservationKind::Doppler, signal});
            mBdsTypeOrder.push_back({ObservationKind::SignalStrength, signal});
        }
    }
}

void Builder::header_begin() {
    VSCOPE_FUNCTION();

    if (mVersion < 3.00) {
        WARNF("unsupported RINEX version %d", mVersion);
        return;
    }

    auto date = ts::Utc::now();

    mStream << std::setw(9) << std::right << std::fixed << std::setprecision(2) << mVersion  //
            << std::setw(11) << std::left << ""                                              //
            << std::setw(20) << std::left << "OBSERVATION DATA"                              //
            << std::setw(20) << std::left
            << observation_data_type(mGpsSupport, mGloSupport, mGalSupport, mBdsSupport)  //
            << std::setw(20) << std::left << "RINEX VERSION / TYPE" << std::endl;

    mStream << std::setw(20) << std::left << mProgram             //
            << std::setw(20) << std::left << mRunBy               //
            << std::setw(20) << std::left << date.rinex_string()  //
            << std::setw(20) << std::left << "PGM / RUN BY / DATE" << std::endl;

    header_comment("************************************************************");
    header_comment("This file was generated by Tokoro (" CLIENT_VERSION ")");
    header_comment("************************************************************");

    // MARKER NAME
    mStream << std::setw(60) << std::left << mMarkerName  //
            << std::setw(20) << std::left << "MARKER NAME" << std::endl;
    // MARKER TYPE
    mStream << std::setw(20) << std::left << mMarkerType  //
            << std::setw(40) << std::left << ""           //
            << std::setw(20) << std::left << "MARKER TYPE" << std::endl;

    // OBSERVER / AGENCY
    mStream << std::setw(20) << std::left << mObserver  //
            << std::setw(40) << std::left << mAgency    //
            << std::setw(20) << std::left << "OBSERVER / AGENCY" << std::endl;

    // REC # / TYPE / VERS
    mStream << std::setw(20) << std::left << mReceiverNumber   //
            << std::setw(20) << std::left << mReceiverType     //
            << std::setw(20) << std::left << mReceiverVersion  //
            << std::setw(20) << std::left << "REC # / TYPE / VERS" << std::endl;

    // ANT # / TYPE
    mStream << std::setw(20) << std::left << mAntennaSerial  //
            << std::setw(20) << std::left << mAntennaNumber  //
            << std::setw(20) << std::left << mAntennaType    //
            << std::setw(20) << std::left << "ANT # / TYPE" << std::endl;

    // APPROX POSITION XYZ
    mStream << std::setw(14) << std::right << std::fixed << std::setprecision(4)
            << mApproxPosition.x  //
            << std::setw(14) << std::right << std::fixed << std::setprecision(4)
            << mApproxPosition.y  //
            << std::setw(14) << std::right << std::fixed << std::setprecision(4)
            << mApproxPosition.z                 //
            << std::setw(18) << std::left << ""  //
            << std::setw(20) << std::left << "APPROX POSITION XYZ" << std::endl;

    // ANTENNA: DELTA H/E/N
    mStream << std::setw(14) << std::right << std::fixed << std::setprecision(4) << mAntennaDelta.x
            << std::setw(14) << std::right << std::fixed << std::setprecision(4) << mAntennaDelta.y
            << std::setw(14) << std::right << std::fixed << std::setprecision(4) << mAntennaDelta.z
            << std::setw(18) << std::left << ""  //
            << std::setw(20) << std::left << "ANTENNA: DELTA H/E/N" << std::endl;
}

void Builder::header_time_of_first_observation(ts::Tai const& time) {
    VSCOPE_FUNCTION();

    auto gps_time = ts::Gps{time};
    auto tp       = gps_time.to_timepoint();
    mStream                                                                                 //
        << std::setw(6) << std::right << tp.year                                            //
        << std::setw(6) << std::right << tp.month                                           //
        << std::setw(6) << std::right << tp.day                                             //
        << std::setw(6) << std::right << tp.hour                                            //
        << std::setw(6) << std::right << tp.minutes                                         //
        << std::setw(13) << std::right << std::fixed << std::setprecision(7) << tp.seconds  //
        << std::setw(5) << ""                                                               //
        << std::setw(3) << std::right << "GPS"                                              //
        << std::setw(9) << ""                                                               //
        << std::setw(20) << std::left << "TIME OF FIRST OBS" << std::endl;
}

void Builder::header_end() {
    VSCOPE_FUNCTION();
    mStream << std::setw(60) << std::left << ""  //
            << std::setw(20) << std::left << "END OF HEADER" << std::endl;
}

void Builder::header_comment(std::string const& comment) {
    VSCOPE_FUNCTION();
    mStream << std::setw(60) << std::left << comment  //
            << std::setw(20) << std::left << "COMMENT" << std::endl;
}

void Builder::header_observation_types() {
    VSCOPE_FUNCTION();

    std::sort(mGpsTypeOrder.begin(), mGpsTypeOrder.end(),
              [](ObservationType const& a, ObservationType const& b) {
                  return a < b;
              });
    std::sort(mGloTypeOrder.begin(), mGloTypeOrder.end(),
              [](ObservationType const& a, ObservationType const& b) {
                  return a < b;
              });
    std::sort(mGalTypeOrder.begin(), mGalTypeOrder.end(),
              [](ObservationType const& a, ObservationType const& b) {
                  return a < b;
              });
    std::sort(mBdsTypeOrder.begin(), mBdsTypeOrder.end(),
              [](ObservationType const& a, ObservationType const& b) {
                  return a < b;
              });

    header_observation_types_gnss(mGpsTypeOrder, 'G');
    header_observation_types_gnss(mGloTypeOrder, 'R');
    header_observation_types_gnss(mGalTypeOrder, 'E');
    header_observation_types_gnss(mBdsTypeOrder, 'C');
}

void Builder::header_observation_types_gnss(std::vector<ObservationType> const& types,
                                            char                                gnss_ch) {
    VSCOPE_FUNCTION();
    if (types.empty()) {
        return;
    }

    mStream << gnss_ch << "  " << std::setw(3) << std::right << types.size();
    size_t i = 0;
    for (; i < types.size(); i++) {
        if (i > 0 && i % 13 == 0) {
            mStream << "      ";
        }
        mStream << " " << types[i].kind_char() << types[i].signal_id.to_rinex();
        if ((i % 13 == 12)) {
            mStream << "  " << std::setw(20) << std::left << "SYS / # / OBS TYPES" << std::endl;
        }
    }
    if ((i % 13) > 0) {
        mStream << std::setw(4 * (13 - (i % 13))) << "" << "  " << std::setw(20) << std::left
                << "SYS / # / OBS TYPES" << std::endl;
    }
}

void Builder::header_phase_shift_type(ObservationType const& type, char gnss_ch) {
    VSCOPE_FUNCTION();

    if (type.kind != ObservationKind::Phase) {
        return;
    }

    mStream << gnss_ch << " " << type.kind_char() << type.signal_id.to_rinex() << " "
            << std::setw(60 - 6) << "" << std::setw(20) << std::left << "SYS / PHASE SHIFT TYPE"
            << std::endl;
}

void Builder::header_phase_shift() {
    VSCOPE_FUNCTION();

    for (auto const& type : mGpsTypeOrder)
        header_phase_shift_type(type, 'G');
    for (auto const& type : mGloTypeOrder)
        header_phase_shift_type(type, 'R');
    for (auto const& type : mGalTypeOrder)
        header_phase_shift_type(type, 'E');
    for (auto const& type : mBdsTypeOrder)
        header_phase_shift_type(type, 'C');
}

void Builder::epoch(ts::Tai const& time, std::vector<SatelliteId>& satellites) {
    VSCOPE_FUNCTION();

    if (satellites.empty()) {
        VERBOSEF("no satellites to write");
        return;
    }

    if (!mInitialized) {
        // create the file and truncate it
        mStream.open(mPath, std::ios::out | std::ios::trunc);
        if (!mStream.is_open()) {
            ERRORF("failed to open file %s", mPath.c_str());
            return;
        }

        generate_observation_order();

        header_begin();
        header_observation_types();
        header_time_of_first_observation(time);
        header_phase_shift();
        header_end();

        mInitialized = true;
    }

    std::sort(satellites.begin(), satellites.end(), [](SatelliteId const& a, SatelliteId const& b) {
        return a < b;
    });

    auto flag           = 0;
    auto num_satellites = satellites.size();

    auto gps_time = ts::Gps{time};
    auto tp       = gps_time.to_timepoint();
    mStream << "> " << std::setw(4) << tp.year                                                  //
            << std::setw(3) << std::right << tp.month                                           //
            << std::setw(3) << std::right << tp.day                                             //
            << std::setw(3) << std::right << tp.hour                                            //
            << std::setw(3) << std::right << tp.minutes                                         //
            << std::setw(11) << std::fixed << std::right << std::setprecision(7) << tp.seconds  //
            << std::setw(3) << std::right << flag                                               //
            << std::setw(3) << std::right << num_satellites;

    if (mVersion <= 2.99) {
        size_t i = 0;
        for (; i < num_satellites; i++) {
            if (i > 0 && i % 12 == 0) {
                mStream << std::endl;
            }
            mStream << satellites[i].to_string();
        }
        if (i % 12 > 0) {
            mStream << std::endl;
        }
    } else {
        mStream << std::endl;
    }
}

std::vector<ObservationType> const& Builder::observation_types(SatelliteId const& id) const {
    VSCOPE_FUNCTION();

    if (id.is_gps()) {
        return mGpsTypeOrder;
    } else if (id.is_glonass()) {
        return mGloTypeOrder;
    } else if (id.is_galileo()) {
        return mGalTypeOrder;
    } else if (id.is_beidou()) {
        return mBdsTypeOrder;
    } else {
        UNREACHABLE();
    }
}

void Builder::observations(SatelliteId                                        satellite_id,
                           std::unordered_map<ObservationType, double> const& observations,
                           std::unordered_set<SignalId> const&                loss_signals) {
    VSCOPE_FUNCTION();

    if (mVersion >= 3.00) {
        mStream << satellite_id.to_string();
    }

    auto& order = observation_types(satellite_id);
    for (auto const& type : order) {
        auto it = observations.find(type);
        if (it == observations.end()) {
            mStream << std::setw(14) << std::right << "" << "  ";
        } else if (type.kind == ObservationKind::Phase) {
            mStream << std::setw(15) << std::right << std::fixed << std::setprecision(3)
                    << it->second;
            if (loss_signals.count(type.signal_id) > 0) {
                mStream << "0";
            }
            mStream << " ";
        } else {
            mStream << std::setw(14) << std::right << std::fixed << std::setprecision(3)
                    << it->second << "  ";
        }
    }

    mStream << std::endl;
}

// G32  23052113.554                        1259.391          37.900    23052119.850 931.909 32.300
// E12  25450665.795   133744043.105          47.000

}  // namespace rinex
}  // namespace format
