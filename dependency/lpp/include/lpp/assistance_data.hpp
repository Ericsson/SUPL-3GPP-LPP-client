#pragma once
#include <lpp/message.hpp>
#include <lpp/periodic_session.hpp>
#include <supl/cell.hpp>

#include <functional>

namespace lpp {

class Client;
struct PeriodicRequestAssistanceData {
    enum class Type {
        /// @brief Request OSR specific assistance data.
        OSR,
        /// @brief Request SSR specific assistance data.
        SSR,
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

    /// @brief The assistance data request configuration
    struct {
        /// @brief The amount of deliveries to request. Amount = 2^x, if x=32 then amount=unlimited
        long delivery_amount;
        /// @brief If the reference station request should request the antenna height
        bool antenna_height;
    } config;

    /// @brief Callback for non-periodic assistance data.
    /// @note This will be called _before_ the `on_started` callback.
    std::function<void(Client&, Message)> on_non_periodic;

    /// @brief Callback for periodic assistance data.
    std::function<void(Client&, PeriodicSessionHandle, Message)> on_periodic;

    /// @brief Callback for when a periodic session is started. Called after `on_non_periodic`.
    std::function<void(Client&, PeriodicSessionHandle)> on_started;

    /// @brief Callback for when a periodic session is ended.
    std::function<void(Client&, PeriodicSessionHandle)> on_ended;

    /// @brief Called when a request assistance data failed
    std::function<void(Client&)> on_error;
};

struct SingleRequestAssistanceData {
    enum class Type {
        /// @brief Request AGNSS specific assistance data.
        AGNSS
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

    /// @brief Callback for providing assistance data
    std::function<void(Client&, Message)> on_message;

    /// @brief Called when a request assistance data failed
    std::function<void(Client&)> on_error;
};

}  // namespace lpp
