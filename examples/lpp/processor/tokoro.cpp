#include "tokoro.hpp"
#include "ssr_example.h"
#include "ubx.hpp"

#ifdef INCLUDE_GENERATOR_TOKORO
#include <math.h>

#include <ephemeris/bds.hpp>
#include <ephemeris/gal.hpp>
#include <ephemeris/gps.hpp>
#include <format/nav/d1.hpp>
#include <format/nav/gal/inav.hpp>
#include <format/nav/gps/lnav.hpp>
#include <format/ubx/messages/rxm_sfrbx.hpp>

#include <generator/rtcm/generator.hpp>
#include <generator/tokoro/coordinate.hpp>
#include <generator/tokoro/generator.hpp>
#include <loglet/loglet.hpp>
#include <time/gps.hpp>
#include <time/tai.hpp>
#include <time/utc.hpp>

#define LOGLET_CURRENT_MODULE "p/tokoro"

using namespace streamline;
using namespace generator::tokoro;
using namespace format::ubx;

class EphemerisExtractor : public Inspector<UbxMessage> {
public:
    EphemerisExtractor(Generator& generator) : mGenerator(generator) {}

    void handle_gps_lnav(RxmSfrbx* sfrbx);
    void handle_gps(RxmSfrbx* sfrbx);

    void handle_gal_inav(RxmSfrbx* sfrbx);
    void handle_gal(RxmSfrbx* sfrbx);

    void handle_bds_d1(RxmSfrbx* sfrbx);
    void handle_bds(RxmSfrbx* sfrbx);

    void inspect(System&, DataType const& message) override;

private:
    Generator&                                 mGenerator;
    format::nav::gps::lnav::EphemerisCollector mGpsCollector;
    format::nav::gal::InavEphemerisCollector   mGalCollector;
    format::nav::D1Collector                   mBdsCollector;
};

void EphemerisExtractor::handle_gps_lnav(RxmSfrbx* sfrbx) {
    auto words = format::nav::Words::from_sfrbx_l1ca(sfrbx->words());

    format::nav::gps::lnav::Subframe subframe{};
    if (!format::nav::gps::lnav::Subframe::decode(words, subframe)) return;

    ephemeris::GpsEphemeris ephemeris{};
    if (!mGpsCollector.process(sfrbx->sv_id(), subframe, ephemeris)) return;

    mGenerator.process_ephemeris(ephemeris);
}

void EphemerisExtractor::handle_gps(RxmSfrbx* sfrbx) {
    if (sfrbx->sig_id() == 0) {
        handle_gps_lnav(sfrbx);
    }
}

void EphemerisExtractor::handle_gal_inav(format::ubx::RxmSfrbx* sfrbx) {
    auto words = format::nav::Words::from_sfrbx_e5b(sfrbx->words());

    format::nav::gal::InavWord word{};
    if (!format::nav::gal::InavWord::decode(words, word)) return;

    ephemeris::GalEphemeris ephemeris{};
    if (!mGalCollector.process(sfrbx->sv_id(), word, ephemeris)) return;

    mGenerator.process_ephemeris(ephemeris);
}

void EphemerisExtractor::handle_gal(format::ubx::RxmSfrbx* sfrbx) {
    if (sfrbx->sig_id() == 5) {
        handle_gal_inav(sfrbx);
    }
}

void EphemerisExtractor::handle_bds_d1(format::ubx::RxmSfrbx* sfrbx) {
    auto words = format::nav::Words::from_sfrbx_bds_d1(sfrbx->words());

    format::nav::D1Subframe subframe{};
    if (!format::nav::D1Subframe::decode(words, sfrbx->sv_id(), subframe)) return;

    ephemeris::BdsEphemeris ephemeris{};
    if (!mBdsCollector.process(sfrbx->sv_id(), subframe, ephemeris)) return;

    mGenerator.process_ephemeris(ephemeris);
}

void EphemerisExtractor::handle_bds(format::ubx::RxmSfrbx* sfrbx) {
    if (sfrbx->sig_id() == 0) {
        handle_bds_d1(sfrbx);
    } else if (sfrbx->sig_id() == 1) {
        // handle_bds_d2(sfrbx);
    }
}

void EphemerisExtractor::inspect(streamline::System&, DataType const& message) {
    auto ptr = message.get();
    if (!ptr) return;

    auto sfrbx = dynamic_cast<RxmSfrbx*>(ptr);
    if (!sfrbx) return;

#if 1
    if (sfrbx->gnss_id() == 0) {
        handle_gps(sfrbx);
    } else if (sfrbx->gnss_id() == 2) {
        handle_gal(sfrbx);
    } else if (sfrbx->gnss_id() == 3) {
        handle_bds(sfrbx);
    }
#endif
}

#define TRIMBLE_TEST 0
#define SEARCH_TEST 0

struct SearchCodeRange {
    double g04;
    double g05;
    double g07;
};

static SearchCodeRange evaluate_at(ts::Tai const& t, Float3 const& p, Generator& generator) {
    auto rs = generator.define_reference_station(ReferenceStationConfig{
        p,
        p,
        true,
        false,
        false,
        false,
    });

    rs->set_shaprio_correction(true);
    rs->set_earth_solid_tides_correction(true);
    rs->set_phase_windup_correction(true);
    rs->set_antenna_phase_variation_correction(false);
    rs->set_tropospheric_height_correction(true);
    rs->include_satellite(SatelliteId::from_gps_prn(4));
    rs->include_satellite(SatelliteId::from_gps_prn(5));
    rs->include_satellite(SatelliteId::from_gps_prn(7));
    rs->include_signal(SignalId::GPS_L1_CA);

    rs->generate(t);
    return {rs->g04_l1_ca(), rs->g05_l1_ca(), rs->g07_l1_ca()};
}

static double evaluate_error_at(ts::Tai const& t, Float3 const& p, Generator& generator, double g04,
                                double g05, double g07) {
    auto code_range = evaluate_at(t, p, generator);
    auto g04_error  = code_range.g04 - g04;
    auto g05_error  = code_range.g05 - g05;
    auto g07_error  = code_range.g07 - g07;
    return std::sqrt(g04_error * g04_error + g05_error * g05_error + g07_error * g07_error);
}

static Float3 search(ts::Tai const& t, Float3 const& input, Generator& generator, double g04,
                     double g05, double g07, int depth) {
    auto p         = input;
    auto dx        = 1.0;
    auto dy        = 1.0;
    auto dz        = 1.0;
    auto step_size = 1000.0;

    for (;;) {
        // calculate the gradient to find the direction of the search
        auto base = evaluate_error_at(t, p, generator, g04, g05, g07);
        printf("search: %g | %.4f, %.4f, %.4f\n", base, p.x, p.y, p.z);
        if (base < 0.00001) return p;

        auto x = evaluate_error_at(t, p + Float3{dx, 0.0, 0.0}, generator, g04, g05, g07);
        auto y = evaluate_error_at(t, p + Float3{0.0, dy, 0.0}, generator, g04, g05, g07);
        auto z = evaluate_error_at(t, p + Float3{0.0, 0.0, dz}, generator, g04, g05, g07);

        auto gx = (base - x);
        auto gy = (base - y);
        auto gz = (base - z);

        p             = p + Float3{gx, gy, gz} * step_size;
        auto new_base = evaluate_error_at(t, p, generator, g04, g05, g07);
        if (new_base > base) {
            printf("step: %f\n", step_size);
            step_size /= 2;
            dx /= 2;
            dy /= 2;
            dz /= 2;
            continue;
        }
    }
}

struct SearchLocation {
    int64_t x;
    int64_t y;
    int64_t z;
};

inline bool operator==(SearchLocation const& a, SearchLocation const& b) {
    return a.x == b.x && a.y == b.y && a.z == b.z;
}

namespace std {
template <>
struct hash<SearchLocation> {
    std::size_t operator()(SearchLocation const& location) const {
        return std::hash<int64_t>{}(location.x) ^ std::hash<int64_t>{}(location.y) ^
               std::hash<int64_t>{}(location.z);
    }
};
}  // namespace std

#define SEARCH_SCALE (5.0)

static void search_area(ts::Tai const& t, int64_t x, int64_t y, int64_t z, int64_t sx, int64_t sy,
                        int64_t sz, int64_t ex, int64_t ey, int64_t ez,
                        SearchCodeRange const& expected_code_range, Generator& generator,
                        std::unordered_map<SearchLocation, double>* search_results) {
    auto points = (ex - sx + 1) * (ey - sy + 1) * (ez - sz + 1);
    printf("search area: %li, %li, %li (%li)\n", x, y, z, points);
    for (auto i = sx; i <= ex; i++) {
        for (auto j = sy; j <= ey; j++) {
            for (auto k = sz; k <= ez; k++) {
                auto p = Float3{
                    static_cast<double>(x + i) / SEARCH_SCALE,
                    static_cast<double>(y + j) / SEARCH_SCALE,
                    static_cast<double>(z + k) / SEARCH_SCALE,
                };

                auto code_range = evaluate_at(t, p, generator);
                auto g04_error  = std::abs(code_range.g04 - expected_code_range.g04);
                auto g05_error  = std::abs(code_range.g05 - expected_code_range.g05);
                auto error      = 0.0;
                if (expected_code_range.g04 > 1.0) error += g04_error;  // * g04_error;
                if (expected_code_range.g05 > 1.0) error += g05_error;  // * g05_error;

                SearchLocation location = {x + i, y + j, z + k};
                auto           it       = search_results->find(location);
                if (it == search_results->end()) {
                    search_results->insert({location, error});
                } else {
                    it->second += error;
                }
            }
        }
    }
}

class SsrEvaluator : public Inspector<LppMessage> {
public:
    SsrEvaluator(OutputOptions const& options) : mGenerator(), mOptions(options) {}

    Generator& generator() { return mGenerator; }

    std::unordered_map<SearchLocation, double> search_results;
    double                                     search_count = 0.0;

    bool    first = true;
    ts::Tai last_time;

    void inspect(System&, DataType const& message) override {
        if (!message) return;

#if 1
        mGenerator.process_lpp(*message.get());

        // auto r_x = 3226.697e3;
        // auto r_y = 902.44e3;
        // auto r_z = 5409.136e3;

        auto t = mGenerator.last_correction_data_time();
        if (!first && t.timestamp().seconds() <= last_time.timestamp().seconds()) {
            return;
        }
        first     = false;
        last_time = t;

        printf("time: %s %24li %24f\n", ts::Utc(t).rtklib_time_string().c_str(),
               t.timestamp().seconds(), ts::Gps{t}.time_of_week().full_seconds());
#if 0
        auto p   = EcefPosition{r_x, r_y, r_z};
        auto wgs = generator::tokoro::ecef_to_wgs84(p);

        printf("ground position: %.3f, %.3f, %.3f\n", p.x, p.y, p.z);
        printf("wgs84  position: %.12f, %.12f, %.12f\n", wgs.x, wgs.y, wgs.z);
#endif

#if TRIMBLE_TEST
        printf("%li: %s %f\n", t.timestamp().seconds(), t.rtklib_time_string().c_str(),
               ts::Gps{t}.time_of_week().full_seconds());

        // 2024/09/09 12:12:46.000000000000 1725883929
        // 2024/10/01 12:10:42.000000000000 1727784605
        // 2024/12/05 07:24:27.000000000000 1733383430
        // 2024/12/05 07:25:20.000000000000 1733383483
        if (t.timestamp().seconds() != 1733383483) {
            printf("----------- SKIPING ----------------\n");
            return;
        }

#endif

#if SEARCH_TEST
        printf("%li: %s %li\n", t.timestamp().seconds(), t.rtklib_time_string().c_str(),
               ts::Gps{t}.time_of_week().seconds());

        auto i = Float3{
            // 3228520.2226,
            // 898382.7298,
            // 5408690.7539,

            3227560.2384,
            898383.8764,
            5409177.4438,
        };

        auto epoch_time = ts::Gps{t}.time_of_week().seconds();
        if (epoch_time != 372279) {
            return;
        }

        auto found = search(t, i, mGenerator, 23470062.8889767, 23017268.5300473459064960,
                            21072533.1741807349026203, 10);

        printf("found: %.4f, %.4f, %.4f\n", found.x, found.y, found.z);

        for (;;) {
            sleep(1);
        }

#if 0
#if 0
        auto i = Float3{
            3228520.2226,
            898382.7298,
            5408690.7539,
        };
#elif 1
        auto l = 0.0;
        auto k = Float3{-94, 83, 93.8};
        k.normalize();
        auto i = Float3{3228334.0000, 898546.8000, 5408876.0000};
        i.x += l * k.x;
        i.y += l * k.y;
        i.z += l * k.z;
#else
        auto i = Float3{
            3227721,
            898456,
            5408993,
        };
#endif

        auto i_x = static_cast<int64_t>(i.x * SEARCH_SCALE);
        auto i_y = static_cast<int64_t>(i.y * SEARCH_SCALE);
        auto i_z = static_cast<int64_t>(i.z * SEARCH_SCALE);

        auto sx = -10;
        auto ex = 10;
        auto sy = -10;
        auto ey = 10;
        auto sz = -10;
        auto ez = 10;

        auto epoch_time              = ts::Gps{t}.time_of_week().seconds();
        auto expected_code_range_g04 = 0.0;
        auto expected_code_range_g05 = 0.0;
        if (epoch_time == 372279) {
            expected_code_range_g04 = 23470062.8889767;
            expected_code_range_g05 = 23017268.5300473459064960;
        } else if (epoch_time == 372281) {
            expected_code_range_g04 = 23471334.2520917;
            expected_code_range_g05 = 23016041.2676810584962368;
        } else if (epoch_time == 372282) {
            expected_code_range_g04 = 23471969.9961908;
            expected_code_range_g05 = 23015427.7347775399684906;
        } else if (epoch_time == 372283) {
            expected_code_range_g04 = 23472378.6071389;
            expected_code_range_g05 = 23014814.2733501121401787;
        } else if (epoch_time == 372284) {
            expected_code_range_g04 = 23473014.4227141;
            expected_code_range_g05 = 23014200.8655297532677650;
        } else if (epoch_time == 372285) {
            expected_code_range_g04 = 23473650.2740273;
            expected_code_range_g05 = 23013587.5291854888200760;
        } else if (epoch_time == 372286) {
            expected_code_range_g04 = 23474286.2325546;
            expected_code_range_g05 = 23012974.2464482933282852;
        } else if (epoch_time == 372287) {
            expected_code_range_g04 = 23474922.1374749;
            expected_code_range_g05 = 23012361.0351871885359287;
        } else if (epoch_time == 372288) {
            expected_code_range_g04 = 23475558.0960022;
            expected_code_range_g05 = 23011747.8775331526994705;
        } else if (epoch_time == 372289) {
            expected_code_range_g04 = 23476194.0545296;
            expected_code_range_g05 = 23011134.7913552075624466;
        } else if (epoch_time == 372291) {
            expected_code_range_g04 = 23477466.1145364;
            expected_code_range_g05 = 23009908.7976895533502102;
        } else if (epoch_time == 372293) {
            expected_code_range_g04 = 23478738.2638884;
            expected_code_range_g05 = 23008683.0541902109980583;
        } else if (epoch_time >= 372294) {
            printf("----------- SEARCH ----------------\n");

            auto min_area = Float3{
                static_cast<double>(i_x + sx) / SEARCH_SCALE,
                static_cast<double>(i_y + sy) / SEARCH_SCALE,
                static_cast<double>(i_z + sz) / SEARCH_SCALE,
            };
            auto max_area = Float3{
                static_cast<double>(i_x + ex) / SEARCH_SCALE,
                static_cast<double>(i_y + sy) / SEARCH_SCALE,
                static_cast<double>(i_z + sz) / SEARCH_SCALE,
            };
            printf("area: %.4f, %.4f, %.4f\n", min_area.x, min_area.y, min_area.z);
            printf("    : %.4f, %.4f, %.4f\n", max_area.x, max_area.y, max_area.z);
            printf("step: %.4f\n", 1.0 / SEARCH_SCALE);

            struct SearchOrder {
                SearchLocation location;
                double         error;
            };

            std::vector<SearchOrder> search_order;
            for (auto const& [location, error] : search_results) {
                search_order.push_back({location, error});
            }

            std::sort(search_order.begin(), search_order.end(),
                      [](SearchOrder const& a, SearchOrder const& b) {
                          return a.error < b.error;
                      });

            // print the top 10
            Float3 m{};
            for (auto i = 0; i < 20; i++) {
                auto const& [location, error] = search_order[i];
                auto p                        = Float3{
                    static_cast<double>(location.x) / SEARCH_SCALE,
                    static_cast<double>(location.y) / SEARCH_SCALE,
                    static_cast<double>(location.z) / SEARCH_SCALE,
                };
                m.x += p.x;
                m.y += p.y;
                m.z += p.z;
                printf("search: %.4f, %.4f, %.4f (%.4f)\n", p.x, p.y, p.z, error / search_count);
            }

            m.x /= 20.0;
            m.y /= 20.0;
            m.z /= 20.0;

            printf("mean: %.4f, %.4f, %.4f\n", m.x, m.y, m.z);

            for (;;) {
                sleep(1);
            }
        } else {
            printf("----------- SKIPING ----------------\n");
            return;
        }

        auto time_now           = ts::Tai::now();
        auto search_code_ranges = SearchCodeRange{
            expected_code_range_g04,
            expected_code_range_g05,
        };
        search_area(t, i_x, i_y, i_z, sx, sy, sz, ex, ey, ez, search_code_ranges, mGenerator,
                    &search_results);
        search_count += 1.0;

        auto time_end = ts::Tai::now();
        auto duration = time_end.timestamp().full_seconds() - time_now.timestamp().full_seconds();
        printf("searched: %.3f seconds\n", duration);
        return;
#endif
#endif

        mReferenceStation->generate(t);
        auto messages = mReferenceStation->produce();

        // auto messages = mGenerator.generate(t, p);
        for (auto& submessage : messages) {
            printf("message: %4d: %zu bytes\n", submessage.id(), submessage.data().size());
        }

        for (auto& submessage : messages) {
            auto buffer = submessage.data().data();
            auto size   = submessage.data().size();

            // TODO(ewasjon): These message should be passed back into the system
            for (auto const& output : mOptions.outputs) {
                if ((output.format & OUTPUT_FORMAT_RTCM) != 0) {
                    output.interface->write(buffer, size);
                }
            }
        }

#if TRIMBLE_TEST
        for (;;) {
            sleep(1);
        }
#endif
#endif
    }

    void set_reference_station(std::shared_ptr<ReferenceStation> rs) {
        mReferenceStation = std::move(rs);
    }

private:
    Generator                         mGenerator;
    OutputOptions const&              mOptions;
    std::shared_ptr<ReferenceStation> mReferenceStation;
};

void tokoro_initialize(System& system, ssr_example::SsrGlobals const& globals,
                       OutputOptions const& options) {
    auto  evaluator = system.add_inspector<SsrEvaluator>(options);
    auto& generator = evaluator->generator();

    generator.set_iod_consistency_check(globals.iod_consistency_check);
    generator.set_rtoc(globals.rtoc);
    generator.set_ocit(globals.ocit);

#if TRIMBLE_TEST
    // Trimble Test Reference
    auto location_itrf2020 = Float3{
        3233520.957,
        859415.096,
        5412047.363,
    };
    auto location_output = location_itrf2020;
    auto rs              = generator.define_reference_station(ReferenceStationConfig{
        location_itrf2020,
        location_output,
        globals.generate_gps,
        globals.generate_glonass,
        globals.generate_galileo,
        globals.generate_beidou,
    });
#elif 0
    // SE_Lin_Office ETRF89
    auto location_etrf89 = Float3{
        3227560.90670000016689,
        898383.43410000007134,
        5409177.11160000041127,
    };
    // SE_Lin_Office ITRF2020
    auto location_itrf2020_2024 = itrf_transform(Itrf::ITRF1989, Itrf::ITRF2020, 2024.0,
                                                 etrf89_to_itrf89(2024.0, location_etrf89));

    auto physical_location = Float3{
        3219441.0553999999538064,
        927415.5111000000033528,
        5409116.6926000006496906,
    };

    printf("etrf89:   %+14.4f %+14.4f %+14.4f (----.--)\n", location_etrf89.x, location_etrf89.y,
           location_etrf89.z);
    printf("itrf2020: %+14.4f %+14.4f %+14.4f (2024.00)\n", location_itrf2020_2024.x,
           location_itrf2020_2024.y, location_itrf2020_2024.z);

    auto from_epoch             = 2005.0;
    auto to_epoch               = 2024.9;
    auto location_itrf2020_1991 = itrf_transform(Itrf::ITRF1989, Itrf::ITRF2020, to_epoch,
                                                 etrf89_to_itrf89(from_epoch, location_etrf89));
    printf("itrf2020: %+14.4f %+14.4f %+14.4f (%.2f)\n", location_itrf2020_1991.x,
           location_itrf2020_1991.y, location_itrf2020_1991.z, to_epoch);
    printf("from tri: %+14.4f %+14.4f %+14.4f\n", 3227560.2384, 898383.8764, 5409177.4438);

    auto location_itrf2020 = Float3{3227560.2384, 898383.8764, 5409177.4438};
    auto location_output   = location_etrf89;

    auto rs = generator.define_reference_station(ReferenceStationConfig{
        location_itrf2020,
        location_output,
        globals.generate_gps,
        globals.generate_glonass,
        globals.generate_galileo,
        globals.generate_beidou,
    });
    rs->set_physical_ground_position(physical_location);
#elif 1
    // SE_Lin_Office (new)
    auto vrs_location = Float3{
        3228520.8885000003501773,
        898382.2873000000836328,
        5408690.4176000002771616,
    };

    auto physical_location = Float3{
        3220403.2762000001966953,
        927414.9581000000471249,
        5408630.0054000001400709,
    };

    // SE_Lin_Office ITRF2020 (from Trimble)
    auto generate_location = Float3{
        3228520.2226,
        898382.7298,
        5408690.7539,
    };

    auto rs = generator.define_reference_station(ReferenceStationConfig{
        generate_location,
        vrs_location,
        globals.generate_gps,
        globals.generate_glonass,
        globals.generate_galileo,
        globals.generate_beidou,
    });
    rs->set_physical_ground_position(physical_location);
#endif

    rs->set_shaprio_correction(globals.shapiro_correction);
    rs->set_earth_solid_tides_correction(globals.earth_solid_tides_correction);
    rs->set_phase_windup_correction(globals.phase_windup_correction);
    rs->set_antenna_phase_variation_correction(globals.antenna_phase_variation_correction);
    rs->set_tropospheric_height_correction(globals.tropospheric_height_correction);

    rs->set_negative_phase_windup(globals.negative_phase_windup);

#if TRIMBLE_TEST
#if 1
    rs->include_satellite(SatelliteId::from_gps_prn(7));
    rs->include_satellite(SatelliteId::from_gps_prn(9));
    //rs->include_satellite(SatelliteId::from_gps_prn(13));
    rs->include_signal(SignalId::GPS_L1_CA);
#endif

#if 0
    rs->include_satellite(SatelliteId::from_gal_prn(4));
    rs->include_satellite(SatelliteId::from_gal_prn(10));
    rs->include_signal(SignalId::GALILEO_E1_B_C);
#endif

#if 0
    rs->include_satellite(SatelliteId::from_bds_prn(19));
    //rs->include_satellite(SatelliteId::from_gal_prn(10));
    //rs->include_signal(SignalId::GALILEO_E1_B_C);
#endif
#endif

    evaluator->set_reference_station(rs);

    system.add_inspector<EphemerisExtractor>(generator);
}

#endif
