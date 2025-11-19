#include <cstring>
#include <dirent.h>
#include <doctest/doctest.h>
#include <ephemeris/gps.hpp>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <time/utc.hpp>
#include <vector>

static std::vector<std::string> find_gps_files() {
    std::vector<std::string> files;
    char const*              paths[] = {"../../tests/data/gps", "../tests/data/gps"};

    for (auto path : paths) {
        DIR* dir = opendir(path);
        if (!dir) continue;

        struct dirent* entry;
        while ((entry = readdir(dir)) != nullptr) {
            if (strstr(entry->d_name, ".txt") != nullptr) {
                files.push_back(std::string(path) + "/" + entry->d_name);
            }
        }
        closedir(dir);

        if (!files.empty()) break;
    }

    return files;
}

TEST_CASE("GPS ephemeris computation") {
    auto files = find_gps_files();
    REQUIRE(!files.empty());

    for (auto const& filename : files) {
        std::ifstream f(filename);
        REQUIRE(f.is_open());
        CAPTURE(filename);

        std::string             line;
        int                     count = 0;
        ephemeris::GpsEphemeris current_eph{};
        bool                    has_eph = false;

        while (std::getline(f, line)) {
            if (line.empty() || line[0] == '#') continue;

            std::istringstream iss(line);
            std::string        type;
            iss >> type;

            if (type == "EPH") {
                int prn, week, iode, iodc, sv_health, ura_index, fit_flag, l2p_flag, ca_or_p_on_l2;
                double toe, toc, tgd, af2, af1, af0;
                double crc, crs, cuc, cus, cic, cis;
                double e, m0, delta_n, a, i0, omega0, omega, omega_dot, idot;
                double expected_x, expected_y, expected_z;

                iss >> prn >> week >> toe >> toc >> iode >> iodc >> tgd >> af2 >> af1 >> af0 >>
                    crc >> crs >> cuc >> cus >> cic >> cis >> e >> m0 >> delta_n >> a >> i0 >>
                    omega0 >> omega >> omega_dot >> idot >> sv_health >> ura_index >> fit_flag >>
                    l2p_flag >> ca_or_p_on_l2 >> expected_x >> expected_y >> expected_z;

                current_eph.prn               = prn;
                current_eph.week_number       = week;
                current_eph.toe               = toe;
                current_eph.toc               = toc;
                current_eph.iode              = iode;
                current_eph.iodc              = iodc;
                current_eph.tgd               = tgd;
                current_eph.af2               = af2;
                current_eph.af1               = af1;
                current_eph.af0               = af0;
                current_eph.crc               = crc;
                current_eph.crs               = crs;
                current_eph.cuc               = cuc;
                current_eph.cus               = cus;
                current_eph.cic               = cic;
                current_eph.cis               = cis;
                current_eph.e                 = e;
                current_eph.m0                = m0;
                current_eph.delta_n           = delta_n;
                current_eph.a                 = a;
                current_eph.i0                = i0;
                current_eph.omega0            = omega0;
                current_eph.omega             = omega;
                current_eph.omega_dot         = omega_dot;
                current_eph.idot              = idot;
                current_eph.sv_health         = sv_health;
                current_eph.ura_index         = ura_index;
                current_eph.fit_interval_flag = fit_flag;
                current_eph.l2_p_data_flag    = l2p_flag;
                current_eph.ca_or_p_on_l2     = ca_or_p_on_l2;
                current_eph.lpp_iod           = 0;
                current_eph.aodo              = 0;
                has_eph                       = true;
            } else if (type == "TEST" && has_eph) {
                long        test_id;
                long        gps_sec;
                long        offset;
                std::string time_str;
                double      ref_x, ref_y, ref_z;
                double      ref_vx, ref_vy, ref_vz;
                double      ref_clock_bias, ref_clock_drift;

                iss >> test_id >> gps_sec >> offset >> std::quoted(time_str) >> ref_x >> ref_y >>
                    ref_z >> ref_vx >> ref_vy >> ref_vz >> ref_clock_bias >> ref_clock_drift;

                int  week     = gps_sec / 604800;
                long tow      = gps_sec % 604800;
                auto time     = ts::Gps::from_week_tow(week, tow, 0.0);
                auto gps_time = ts::Gps{ts::Timestamp{gps_sec}};

                CAPTURE(current_eph.prn);
                CAPTURE(current_eph.toe);
                CAPTURE(test_id);
                CAPTURE(week);
                CAPTURE(tow);
                CAPTURE(offset);

                CHECK(ts::Utc{time}.rtklib_time_string(3) == time_str);
                CHECK(ts::Utc{gps_time}.rtklib_time_string(3) == time_str);

                auto result = current_eph.compute(time);

                CHECK(doctest::Approx(result.position.x) == ref_x);
                CHECK(doctest::Approx(result.position.y) == ref_y);
                CHECK(doctest::Approx(result.position.z) == ref_z);

                CHECK(doctest::Approx(result.velocity.x).epsilon(0.001) == ref_vx);
                CHECK(doctest::Approx(result.velocity.y).epsilon(0.001) == ref_vy);
                CHECK(doctest::Approx(result.velocity.z).epsilon(0.001) == ref_vz);

                auto clock_bias = result.clock + result.relativistic_correction_brdc;
                CHECK(doctest::Approx(clock_bias) == ref_clock_bias);

                count++;
            }
        }

        CHECK(count > 0);
    }
}
