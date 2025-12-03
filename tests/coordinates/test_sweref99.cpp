#include <coordinates/coordinates.hpp>
#include <doctest/doctest.h>
#include "../test_helpers.hpp"

using namespace coordinates;

TEST_CASE("SWEREF99 transformation paths") {
    TransformGraph graph;

    auto forward = graph.find_path(FrameId::ITRF2014, 2020.25, FrameId::Sweref99, 1999.5);
    CHECK(!forward.empty());

    auto reverse = graph.find_path(FrameId::Sweref99, 1999.5, FrameId::ITRF2014, 2020.25);
    CHECK(!reverse.empty());
}

TEST_CASE("ITRF2014 to SWEREF99 transformation") {
    TransformGraph graph;

    SUBCASE("Point A") {
        Vector3d pos(2251700.0000, 819600.0000, 5891200.0000);
        auto result = graph.transform(FrameId::ITRF2014, FrameId::Sweref99, 2020.25, 1999.5, pos,
                                      Vector3d::Zero());

        CHECK(result.final_position.x() == Tolerance(2251700.5587).abs(2e-3));
        CHECK(result.final_position.y() == Tolerance(819599.6862).abs(2e-3));
        CHECK(result.final_position.z() == Tolerance(5891199.6467).abs(2e-3));
    }

    SUBCASE("Point B") {
        Vector3d pos(2885900.0000, 827500.0000, 5608600.0000);
        auto result = graph.transform(FrameId::ITRF2014, FrameId::Sweref99, 2020.25, 1999.5, pos,
                                      Vector3d::Zero());

        CHECK(result.final_position.x() == Tolerance(2885900.4905).abs(1e-3));
        CHECK(result.final_position.y() == Tolerance(827499.6116).abs(1e-3));
        CHECK(result.final_position.z() == Tolerance(5608599.5602).abs(1e-3));
    }

    SUBCASE("Point C") {
        Vector3d pos(3468700.0000, 864800.0000, 5264500.0000);
        auto result = graph.transform(FrameId::ITRF2014, FrameId::Sweref99, 2020.25, 1999.5, pos,
                                      Vector3d::Zero());

        CHECK(result.final_position.x() == Tolerance(3468700.5350).abs(1e-3));
        CHECK(result.final_position.y() == Tolerance(864799.5674).abs(1e-3));
        CHECK(result.final_position.z() == Tolerance(5264499.6517).abs(1e-3));
    }
}

TEST_CASE("SWEREF99 to ITRF2014 transformation") {
    TransformGraph graph;

    SUBCASE("Point A") {
        Vector3d pos(2251700.5587, 819599.6862, 5891199.6467);
        auto result = graph.transform(FrameId::Sweref99, FrameId::ITRF2014, 1999.5, 2020.25, pos,
                                      Vector3d::Zero());

        CHECK(result.final_position.x() == Tolerance(2251700.0000).abs(0.01));
        CHECK(result.final_position.y() == Tolerance(819600.0000).abs(0.01));
        CHECK(result.final_position.z() == Tolerance(5891200.0000).abs(0.01));
    }

    SUBCASE("Point B") {
        Vector3d pos(2885900.4905, 827499.6116, 5608599.5602);
        auto result = graph.transform(FrameId::Sweref99, FrameId::ITRF2014, 1999.5, 2020.25, pos,
                                      Vector3d::Zero());

        CHECK(result.final_position.x() == Tolerance(2885900.0000).abs(0.01));
        CHECK(result.final_position.y() == Tolerance(827500.0000).abs(0.01));
        CHECK(result.final_position.z() == Tolerance(5608600.0000).abs(0.01));
    }

    SUBCASE("Point C") {
        Vector3d pos(3468700.5350, 864799.5674, 5264499.6517);
        auto result = graph.transform(FrameId::Sweref99, FrameId::ITRF2014, 1999.5, 2020.25, pos,
                                      Vector3d::Zero());

        CHECK(result.final_position.x() == Tolerance(3468700.0000).abs(0.01));
        CHECK(result.final_position.y() == Tolerance(864800.0000).abs(0.01));
        CHECK(result.final_position.z() == Tolerance(5264500.0000).abs(0.01));
    }
}
