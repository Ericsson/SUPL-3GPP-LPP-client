#pragma once
#include "ecef.hpp"
#include "frame.hpp"
#include "llh.hpp"
#include "transform_params.hpp"

namespace coordinates {

struct State {
    FrameId  frame;
    double   epoch;
    Vector3d position;
    Vector3d velocity;
};

template <typename FromFrame, typename ToFrame, typename Enable = void>
struct Transform {
    static double apply_epoch(double current_epoch, double target_epoch);
    static State  apply(State const& state, double target_epoch);
};

template <typename FromFrame, typename ToFrame>
struct TimeDependentHelmertTransform {
    static constexpr bool is_defined = false;
};

template <typename FromFrame, typename ToFrame>
struct Transform<FromFrame, ToFrame,
                 std::enable_if_t<TimeDependentHelmertTransform<FromFrame, ToFrame>::is_defined>> {
    static double apply_epoch(double current_epoch, double) { return current_epoch; }

    static State apply(State const& state, double) {
        auto const& params = TimeDependentHelmertTransform<FromFrame, ToFrame>::params();
        auto        dt     = state.epoch - params.reference_epoch;

        auto epoch_params = Helmert7Params::at_epoch(params.base, params.rate, dt);
        auto position     = epoch_params.apply_position(state.position);
        auto velocity = Helmert7Params::apply_velocity(epoch_params, params.rate, state.position,
                                                       state.velocity);
        return {FrameTrait<ToFrame>::id, state.epoch, position, velocity};
    }
};

}  // namespace coordinates
