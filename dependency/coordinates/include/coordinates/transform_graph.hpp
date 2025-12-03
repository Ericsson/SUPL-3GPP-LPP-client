#pragma once
#include <vector>
#include "frame.hpp"
#include "transform.hpp"
#include "transform_params.hpp"

namespace coordinates {

struct PathNode {
    FrameId frame;
    double  epoch;
};

typedef double (*EpochFunction)(double current_epoch, double target_epoch);
typedef State (*StateFunction)(State const& state, double target_epoch);

struct TransformEdge {
    FrameId       from;
    FrameId       to;
    EpochFunction epoch_func;
    StateFunction state_func;
};

struct TransformStep {
    State state_in;
    State state_out;
};

struct TransformResult {
    Vector3d                   final_position;
    Vector3d                   final_velocity;
    double                     final_epoch;
    std::vector<TransformStep> steps;
};

class TransformGraph {
public:
    TransformGraph();

    void add_edge(TransformEdge const& edge);

    std::vector<PathNode> find_path(FrameId from, double from_epoch, FrameId to,
                                    double to_epoch) const;

    bool has_direct_edge(FrameId from, FrameId to) const;

    TransformResult transform(FrameId from, FrameId to, double from_epoch, double to_epoch,
                              Vector3d const& position, Vector3d const& velocity) const;

private:
    std::vector<TransformEdge> mEdges;
};

}  // namespace coordinates
