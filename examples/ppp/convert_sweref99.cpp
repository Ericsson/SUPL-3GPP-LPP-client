#include <coordinates/coordinates.hpp>
#include <cstdio>

int main() {
    // LIND.1 APPROX POSITION from RINEX (SWEREF99 ECEF)
    coordinates::Vector3d pos(3064016.0808, 793861.8843, 5519122.0636);

    // Observation epoch: 2026-03-17 ≈ 2026 + 76/365
    double epoch = 2026.0 + 76.0 / 365.0;

    coordinates::TransformGraph graph;
    auto result = graph.transform(coordinates::FrameId::Sweref99, coordinates::FrameId::ITRF2020,
                                  1999.5, epoch, pos, coordinates::Vector3d::Zero());

    printf("SWEREF99 input:  %.4f  %.4f  %.4f\n", pos.x(), pos.y(), pos.z());
    printf("ITRF2020 output: %.4f  %.4f  %.4f\n", result.final_position.x(),
           result.final_position.y(), result.final_position.z());
    return 0;
}
