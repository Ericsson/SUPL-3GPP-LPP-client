#pragma once
#include <lpp/message.hpp>
#include <lpp/periodic_session.hpp>
#include <supl/cell.hpp>

#include <functional>

namespace lpp {

class Client;
struct RequestAssistanceData {
    enum class Type {
        /// @brief Request OSR specific assistance data.
        OSR,
        /// @brief Request SSR specific assistance data.
        SSR,
        /// @brief Request A-GNSS specific assistance data.
        AGNSS,
    };

    /// @brief The type of assistance data to request.
    Type type;

    /// @brief The cell information to use for the request.
    supl::Cell cell;

    /// @brief The GNSS systems to request assistance data for.
    struct {
        bool gps;
        bool glonass;
        bool galileo;
        bool beidou;
    } gnss;

    /// @brief Callback for non-periodic assistance data.
    /// @note This will be called _before_ the `on_started` callback.
    std::function<void(Client&, Message)> on_non_periodic;

    /// @brief Callback for periodic assistance data.
    std::function<void(Client&, PeriodicSessionHandle, Message)> on_periodic;

    /// @brief Callback for when a periodic session is started. Called after `on_non_periodic`.
    std::function<void(Client&, PeriodicSessionHandle)> on_started;

    /// @brief Callback for when a periodic session is ended.
    std::function<void(Client&, PeriodicSessionHandle)> on_ended;
};

}  // namespace lpp
