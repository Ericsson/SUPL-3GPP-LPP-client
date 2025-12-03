#include "coordinates/transform_graph.hpp"
#include <algorithm>
#include <cmath>
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include "coordinates/frames.hpp"
#include "coordinates/transform.hpp"
#include "coordinates/transform/itrf.hpp"
#include "coordinates/transform/itrf_etrf.hpp"
#include "coordinates/transform/itrf_etrf2000.hpp"
#include "coordinates/transform/itrf_etrf2014.hpp"
#include "coordinates/transform/itrf_etrf2020.hpp"
#include "coordinates/transform/sweref99.hpp"
#include "coordinates/transform/wgs84.hpp"

#include <loglet/loglet.hpp>

LOGLET_MODULE2(coord, graph);
#undef LOGLET_CURRENT_MODULE
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF2(coord, graph)

namespace coordinates {

template <typename FromFrame, typename ToFrame>
void add_edge_impl(TransformGraph& graph) {
    using Tf = Transform<FromFrame, ToFrame>;
    graph.add_edge(
        {FrameTrait<FromFrame>::id, FrameTrait<ToFrame>::id, &Tf::apply_epoch, &Tf::apply});
}

template <typename FromFrame, typename ToFrame>
void add_bidirectional_edge_impl(TransformGraph& graph) {
    add_edge_impl<FromFrame, ToFrame>(graph);
    add_edge_impl<ToFrame, FromFrame>(graph);
}

TransformGraph::TransformGraph() {
    add_bidirectional_edge_impl<ITRF2020, ITRF2014>(*this);
    add_bidirectional_edge_impl<ITRF2020, ITRF2008>(*this);
    add_bidirectional_edge_impl<ITRF2020, ITRF2005>(*this);
    add_bidirectional_edge_impl<ITRF2020, ITRF2000>(*this);
    add_bidirectional_edge_impl<ITRF2020, ITRF97>(*this);
    add_bidirectional_edge_impl<ITRF2020, ITRF96>(*this);
    add_bidirectional_edge_impl<ITRF2020, ITRF94>(*this);
    add_bidirectional_edge_impl<ITRF2020, ITRF93>(*this);
    add_bidirectional_edge_impl<ITRF2020, ITRF92>(*this);
    add_bidirectional_edge_impl<ITRF2020, ITRF91>(*this);
    add_bidirectional_edge_impl<ITRF2020, ITRF90>(*this);
    add_bidirectional_edge_impl<ITRF2020, ITRF89>(*this);
    add_bidirectional_edge_impl<ITRF2020, ITRF88>(*this);

    add_bidirectional_edge_impl<WGS84_G2296, ITRF2020>(*this);

    add_bidirectional_edge_impl<ITRF2014, ETRF2014>(*this);
    add_bidirectional_edge_impl<ITRF2005, ETRF2005>(*this);
    add_bidirectional_edge_impl<ITRF2008, ETRF2014>(*this);
    add_bidirectional_edge_impl<ITRF2005, ETRF2014>(*this);
    add_bidirectional_edge_impl<ITRF2020, ETRF2014>(*this);
    add_bidirectional_edge_impl<ITRF2000, ETRF2014>(*this);
    add_bidirectional_edge_impl<ITRF97, ETRF2014>(*this);
    add_bidirectional_edge_impl<ITRF96, ETRF2014>(*this);
    add_bidirectional_edge_impl<ITRF94, ETRF2014>(*this);
    add_bidirectional_edge_impl<ITRF93, ETRF2014>(*this);
    add_bidirectional_edge_impl<ITRF92, ETRF2014>(*this);
    add_bidirectional_edge_impl<ITRF91, ETRF2014>(*this);
    add_bidirectional_edge_impl<ITRF90, ETRF2014>(*this);
    add_bidirectional_edge_impl<ITRF89, ETRF2014>(*this);
    add_bidirectional_edge_impl<ITRF2014, ETRF2000>(*this);
    add_bidirectional_edge_impl<ITRF2008, ETRF2000>(*this);
    add_bidirectional_edge_impl<ITRF2005, ETRF2000>(*this);
    add_bidirectional_edge_impl<ITRF2020, ETRF2000>(*this);
    add_bidirectional_edge_impl<ITRF2000, ETRF2000>(*this);
    add_bidirectional_edge_impl<ITRF97, ETRF2000>(*this);
    add_bidirectional_edge_impl<ITRF96, ETRF2000>(*this);
    add_bidirectional_edge_impl<ITRF94, ETRF2000>(*this);
    add_bidirectional_edge_impl<ITRF93, ETRF2000>(*this);
    add_bidirectional_edge_impl<ITRF92, ETRF2000>(*this);
    add_bidirectional_edge_impl<ITRF91, ETRF2000>(*this);
    add_bidirectional_edge_impl<ITRF90, ETRF2000>(*this);
    add_bidirectional_edge_impl<ITRF89, ETRF2000>(*this);
    add_bidirectional_edge_impl<ITRF2020, ETRF2020>(*this);
    add_bidirectional_edge_impl<ITRF89, ETRF89>(*this);
    add_bidirectional_edge_impl<ITRF94, ETRF94>(*this);
    add_bidirectional_edge_impl<ITRF89, ETRF2020>(*this);
    add_bidirectional_edge_impl<ITRF2014, ETRF2020>(*this);
    add_bidirectional_edge_impl<ITRF2008, ETRF2020>(*this);
    add_bidirectional_edge_impl<ITRF2005, ETRF2020>(*this);
    add_bidirectional_edge_impl<ITRF2000, ETRF2020>(*this);
    add_bidirectional_edge_impl<ITRF97, ETRF2020>(*this);
    add_bidirectional_edge_impl<ITRF96, ETRF2020>(*this);
    add_bidirectional_edge_impl<ITRF94, ETRF2020>(*this);
    add_bidirectional_edge_impl<ITRF93, ETRF2020>(*this);
    add_bidirectional_edge_impl<ITRF92, ETRF2020>(*this);
    add_bidirectional_edge_impl<ITRF91, ETRF2020>(*this);
    add_bidirectional_edge_impl<ITRF90, ETRF2020>(*this);

    add_bidirectional_edge_impl<WGS84_G2296, ITRF2020>(*this);

    add_edge_impl<ETRF2014, Sweref99>(*this);
    add_edge_impl<Sweref99, ETRF2014>(*this);
}

void TransformGraph::add_edge(TransformEdge const& edge) {
    mEdges.push_back(edge);
}

std::vector<PathNode> TransformGraph::find_path(FrameId from, double from_epoch, FrameId to,
                                                double to_epoch) const {
    VSCOPE_FUNCTIONF("%s@%.1f -> %s@%.1f", frame_name(from), from_epoch, frame_name(to), to_epoch);

    struct NodeState {
        FrameId frame;
        double  epoch;
        double  cost;

        bool operator>(NodeState const& other) const { return cost > other.cost; }
    };

    struct StateHash {
        std::size_t operator()(std::pair<FrameId, double> const& p) const {
            return std::hash<int>()(static_cast<int>(p.first)) ^ std::hash<double>()(p.second);
        }
    };

    std::priority_queue<NodeState, std::vector<NodeState>, std::greater<NodeState>>       queue;
    std::unordered_map<std::pair<FrameId, double>, double, StateHash>                     best_cost;
    std::unordered_map<std::pair<FrameId, double>, std::pair<FrameId, double>, StateHash> parent;

    queue.push({from, from_epoch, 0.0});
    best_cost[{from, from_epoch}] = 0.0;

    while (!queue.empty()) {
        auto current = queue.top();
        TRACEF("visiting %s@%.1f (%zu)", frame_name(current.frame), current.epoch, queue.size());
        queue.pop();

        if (current.frame == to && std::abs(current.epoch - to_epoch) < 0.001) {
            std::vector<PathNode> path;
            FrameId               f = to;
            double                e = to_epoch;

            while (f != from || std::abs(e - from_epoch) >= 0.001) {
                path.push_back({f, e});
                auto p = parent[{f, e}];
                f      = p.first;
                e      = p.second;
            }
            path.push_back({from, from_epoch});
            std::reverse(path.begin(), path.end());

            VERBOSEF("path found: %zu steps, cost: %.3f", path.size(), current.cost);
            for (auto const& node : path) {
                VERBOSEF("  %s@%.1f", frame_name(node.frame), node.epoch);
            }
            return path;
        }

        if (current.cost > best_cost[{current.frame, current.epoch}]) {
            TRACEF("skipping %s@%.1f (cost: %.3f)", frame_name(current.frame), current.epoch,
                   current.cost);
            continue;
        }

        // First check transformation that don't change epoch
        for (auto const& edge : mEdges) {
            if (edge.from != current.frame) {
                continue;
            }

            auto new_epoch = (*edge.epoch_func)(current.epoch, current.epoch);
            auto new_cost  = current.cost + 1.0;
            auto new_key   = std::make_pair(edge.to, new_epoch);
            if (!best_cost.count(new_key) || new_cost < best_cost[new_key]) {
                TRACEF("[pe] adding %s@%.1f (cost: %.3f)", frame_name(edge.to), new_epoch,
                       new_cost);
                best_cost[new_key] = new_cost;
                parent[new_key]    = {current.frame, current.epoch};
                queue.push({edge.to, new_epoch, new_cost});
            } else {
                TRACEF("[pe] skipping %s@%.1f (cost: %.3f)", frame_name(edge.to), new_epoch,
                       new_cost);
            }
        }

        // Then check transformations that change epoch
        for (auto const& edge : mEdges) {
            if (edge.from != current.frame) {
                continue;
            }

            auto new_epoch = (*edge.epoch_func)(current.epoch, to_epoch);
            auto new_cost  = current.cost + 1.0 + std::abs(new_epoch - current.epoch);
            auto new_key   = std::make_pair(edge.to, new_epoch);
            if (!best_cost.count(new_key) || new_cost < best_cost[new_key]) {
                TRACEF("[me] adding %s@%.1f (cost: %.3f)", frame_name(edge.to), new_epoch,
                       new_cost);
                best_cost[new_key] = new_cost;
                parent[new_key]    = {current.frame, current.epoch};
                queue.push({edge.to, new_epoch, new_cost});
            } else {
                TRACEF("[me] skipping %s@%.1f (cost: %.3f)", frame_name(edge.to), new_epoch,
                       new_cost);
            }
        }
    }

    VERBOSEF("no path found");
    return {};
}

bool TransformGraph::has_direct_edge(FrameId from, FrameId to) const {
    for (auto const& edge : mEdges) {
        if (edge.from == from && edge.to == to) {
            return true;
        }
    }
    return false;
}

TransformResult TransformGraph::transform(FrameId from, FrameId to, double from_epoch,
                                          double to_epoch, Vector3d const& position,
                                          Vector3d const& velocity) const {
    VSCOPE_FUNCTIONF("%s@%.1f -> %s@%.1f", frame_name(from), from_epoch, frame_name(to), to_epoch);

    auto path = find_path(from, from_epoch, to, to_epoch);
    if (path.empty()) {
        return {position, velocity, from_epoch, {}};
    }

    State                      current_state = {from, from_epoch, position, velocity};
    std::vector<TransformStep> steps;

    for (size_t i = 1; i < path.size(); ++i) {
        auto const& prev = path[i - 1];
        auto const& next = path[i];

        for (auto const& edge : mEdges) {
            if (edge.from == prev.frame && edge.to == next.frame) {
                TRACEF("transforming %s@%.1f -> %s@%.1f", frame_name(prev.frame), prev.epoch,
                       frame_name(next.frame), next.epoch);

                State next_state = (*edge.state_func)(current_state, next.epoch);
                steps.push_back({current_state, next_state});
                current_state = next_state;
                break;
            }
        }
    }

    return {current_state.position, current_state.velocity, current_state.epoch, steps};
}

}  // namespace coordinates
