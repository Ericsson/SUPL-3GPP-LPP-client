#pragma once
#include <memory>
#include <vector>

#include <scheduler/scheduler.hpp>
#include <streamline/system.hpp>

#include <format/ctrl/parser.hpp>
#include <format/lpp/uper_parser.hpp>
#include <format/nmea/message.hpp>
#include <format/nmea/parser.hpp>
#include <format/ubx/message.hpp>
#include <format/ubx/parser.hpp>

#include <lpp/client.hpp>
#include <lpp/session.hpp>

#include "config.hpp"

struct Program {
    Config               config;
    scheduler::Scheduler scheduler;
    streamline::System   stream;
    bool                 is_disconnected;

    lpp::PeriodicSessionHandle assistance_data_session{};

    lpp::Optional<lpp::LocationInformation> latest_location_information;
    lpp::Optional<lpp::HaGnssMetrics>       latest_gnss_metrics;
    bool                                    latest_location_information_submitted{false};

    std::unique_ptr<supl::Cell>     cell;
    std::unique_ptr<supl::Identity> identity;
    std::unique_ptr<lpp::Client>    client;

    std::vector<std::unique_ptr<format::nmea::Parser>>    nmea_parsers;
    std::vector<std::unique_ptr<format::ubx::Parser>>     ubx_parsers;
    std::vector<std::unique_ptr<format::ctrl::Parser>>    ctrl_parsers;
    std::vector<std::unique_ptr<format::lpp::UperParser>> lpp_uper_parsers;

    void update_location_information(lpp::LocationInformation const& location) {
        latest_location_information           = location;
        latest_location_information_submitted = false;
    }

    void update_gnss_metrics(lpp::HaGnssMetrics const& metrics) { latest_gnss_metrics = metrics; }
};
