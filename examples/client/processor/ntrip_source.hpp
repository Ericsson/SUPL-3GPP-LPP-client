#pragma once
#include <lpp/location_information.hpp>
#include <scheduler/scheduler.hpp>
#include <scheduler/timeout.hpp>
#include "../config.hpp"

#include <functional>
#include <memory>
#include <random>

class NtripSource {
public:
    using LocationProvider = std::function<lpp::Optional<lpp::LocationInformation>()>;
    using DataCallback     = std::function<void(uint8_t const*, size_t)>;

    NtripSource(NtripConfig config, DataCallback on_data,
                LocationProvider location_provider = nullptr);
    ~NtripSource();

    bool schedule(scheduler::Scheduler& scheduler);

private:
    void connect();
    void disconnect();
    void send_nmea();
    void poll();

    std::string build_gga() const;
    void        apply_bias(double& lat, double& lon) const;
    void        schedule_reconnect();

    NtripConfig      mConfig;
    DataCallback     mOnData;
    LocationProvider mLocationProvider;

    int  mSocket     = -1;
    bool mHeaderDone = false;

    scheduler::RepeatableTimeoutTask mPollTask;
    scheduler::RepeatableTimeoutTask mReconnectTask;
    scheduler::RepeatableTimeoutTask mPositionTask;

    std::mt19937                           mRng{std::random_device{}()};
    std::uniform_real_distribution<double> mOffsetAngle{0.0, 2.0 * M_PI};
    std::uniform_real_distribution<double> mOffsetDist{0.0, 1.0};
    double                                 mOffsetLatM = 0.0;
    double                                 mOffsetLonM = 0.0;
};
