#include <coordinates/coordinates.hpp>
#include <doctest/doctest.h>

using namespace coordinates;

TEST_CASE("GPS week to frame") {
    CHECK(frame_from_gps_week(2200) == FrameId::WGS84_G2139);
    CHECK(frame_from_gps_week(1800) == FrameId::WGS84_G1762);
    CHECK(frame_from_gps_week(1700) == FrameId::WGS84_G1674);
    CHECK(frame_from_gps_week(1200) == FrameId::WGS84_G1150);
}

TEST_CASE("Frame names") {
    CHECK(std::string(frame_name(FrameId::WGS84_G1762)) == "wgs84(g1762)");
    CHECK(std::string(frame_name(FrameId::PZ90_11)) == "pz-90.11");
    CHECK(std::string(frame_name(FrameId::CGCS2000)) == "cgcs2000");
    CHECK(std::string(frame_name(FrameId::ITRF2014)) == "itrf2014");
}

TEST_CASE("Identity transformation") {
    TransformGraph graph;
    Vector3d       pos(4000000.0, 3000000.0, 5000000.0);
    auto result = graph.transform(FrameId::ITRF2008, FrameId::ITRF2008, 2010.0, 2010.0, pos,
                                  Vector3d::Zero());

    CHECK(result.final_position.x() == doctest::Approx(pos.x()).epsilon(1e-12));
    CHECK(result.final_position.y() == doctest::Approx(pos.y()).epsilon(1e-12));
    CHECK(result.final_position.z() == doctest::Approx(pos.z()).epsilon(1e-12));
}

TEST_CASE("ITRF2008 to ITRF2005 transformation") {
    TransformGraph graph;
    Vector3d       pos = Vector3d(4000000.0, 3000000.0, 5000000.0);
    auto result        = graph.transform(FrameId::ITRF2008, FrameId::ITRF2005, 2000.0, 2000.0, pos,
                                         Vector3d::Zero());

    CHECK(result.final_position.x() != pos.x());
    CHECK(result.final_position.y() != pos.y());
    CHECK(result.final_position.z() != pos.z());
    CHECK((result.final_position - pos).norm() < 10.0);
}

TEST_CASE("ITRF path finding") {
    TransformGraph graph;
    auto           path = graph.find_path(FrameId::ITRF2014, 2010.0, FrameId::ITRF2005, 2010.0);

    CHECK(path.size() >= 2);
    CHECK(path.front().frame == FrameId::ITRF2014);
    CHECK(path.back().frame == FrameId::ITRF2005);
}

TEST_CASE("ETRF89 to ITRF2020 transformation") {
    TransformGraph graph;
    Vector3d       input(4027894.006, 307045.600, 4919474.910);
    Vector3d       velocity(0.01, 0.2, 0.03);
    Vector3d       expected_pos(4027893.7905, 307045.7371, 4919475.0977);
    Vector3d       expected_vel(-0.0052, 0.2167, 0.0435);

    Vector3d expected_itrf89_pos(4027893.8448, 307045.7814, 4919475.0306);
    Vector3d expected_itrf89_vel(-0.0047, 0.2165, 0.0410);

    auto path = graph.find_path(FrameId::ETRF89, 2000.0, FrameId::ITRF2020, 2000.0);
    REQUIRE(!path.empty());

    auto itrf89_result =
        graph.transform(FrameId::ETRF89, FrameId::ITRF89, 2000.0, 2000.0, input, velocity);

    CHECK(itrf89_result.final_position.x() == doctest::Approx(expected_itrf89_pos.x()));
    CHECK(itrf89_result.final_position.y() == doctest::Approx(expected_itrf89_pos.y()));
    CHECK(itrf89_result.final_position.z() == doctest::Approx(expected_itrf89_pos.z()));

    CHECK(itrf89_result.final_velocity.x() ==
          doctest::Approx(expected_itrf89_vel.x()).epsilon(0.001));
    CHECK(itrf89_result.final_velocity.y() ==
          doctest::Approx(expected_itrf89_vel.y()).epsilon(0.001));
    CHECK(itrf89_result.final_velocity.z() ==
          doctest::Approx(expected_itrf89_vel.z()).epsilon(0.001));

    auto result =
        graph.transform(FrameId::ETRF89, FrameId::ITRF2020, 2000.0, 2000.0, input, velocity);

    CHECK(result.final_position.x() == doctest::Approx(expected_pos.x()));
    CHECK(result.final_position.y() == doctest::Approx(expected_pos.y()));
    CHECK(result.final_position.z() == doctest::Approx(expected_pos.z()));

    CHECK(result.final_velocity.x() == doctest::Approx(expected_vel.x()).epsilon(0.001));
    CHECK(result.final_velocity.y() == doctest::Approx(expected_vel.y()).epsilon(0.001));
    CHECK(result.final_velocity.z() == doctest::Approx(expected_vel.z()).epsilon(0.001));
}

TEST_CASE("ETRF2020 to ITRF94 transformation with epoch change") {
    TransformGraph graph;
    Vector3d       input(4027894.006, 307045.600, 4919474.910);
    Vector3d       velocity(0.01, 0.2, 0.03);

    auto result_itrf2020 =
        graph.transform(FrameId::ETRF2020, FrameId::ITRF2020, 2000.0, 2000.0, input, velocity);
    CHECK(result_itrf2020.final_position.x() == doctest::Approx(4027893.8575).epsilon(0.001));
    CHECK(result_itrf2020.final_position.y() == doctest::Approx(307045.7843).epsilon(0.001));
    CHECK(result_itrf2020.final_position.z() == doctest::Approx(4919475.0201).epsilon(0.001));
    CHECK(result_itrf2020.final_velocity.x() == doctest::Approx(-0.0035).epsilon(0.001));
    CHECK(result_itrf2020.final_velocity.y() == doctest::Approx(0.2168).epsilon(0.001));
    CHECK(result_itrf2020.final_velocity.z() == doctest::Approx(0.0400).epsilon(0.001));

    auto result_itrf94 =
        graph.transform(FrameId::ITRF2020, FrameId::ITRF94, 2000.0, 2000.0,
                        result_itrf2020.final_position, result_itrf2020.final_velocity);
    CHECK(result_itrf94.final_position.x() == doctest::Approx(4027893.8712).epsilon(0.001));
    CHECK(result_itrf94.final_position.y() == doctest::Approx(307045.7913).epsilon(0.001));
    CHECK(result_itrf94.final_position.z() == doctest::Approx(4919474.9994).epsilon(0.001));
    CHECK(result_itrf94.final_velocity.x() == doctest::Approx(-0.0029).epsilon(0.001));
    CHECK(result_itrf94.final_velocity.y() == doctest::Approx(0.2166).epsilon(0.001));
    CHECK(result_itrf94.final_velocity.z() == doctest::Approx(0.0375).epsilon(0.001));

    double   time_diff = 2025.0 - 2000.0;
    Vector3d pos_itrf94_2025 =
        result_itrf94.final_position + result_itrf94.final_velocity * time_diff;
    CHECK(pos_itrf94_2025.x() == doctest::Approx(4027893.7976).epsilon(0.001));
    CHECK(pos_itrf94_2025.y() == doctest::Approx(307051.2058).epsilon(0.001));
    CHECK(pos_itrf94_2025.z() == doctest::Approx(4919475.9368).epsilon(0.001));
}
