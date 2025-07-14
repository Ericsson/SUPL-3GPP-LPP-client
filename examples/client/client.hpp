#pragma once
#include <memory>
#include <vector>

#include <scheduler/scheduler.hpp>
#include <streamline/system.hpp>

#include <format/ctrl/parser.hpp>
#include <format/lpp/uper_parser.hpp>
#include <format/nmea/message.hpp>
#include <format/nmea/parser.hpp>
#include <format/rtcm/message.hpp>
#include <format/rtcm/parser.hpp>
#include <format/ubx/message.hpp>
#include <format/ubx/parser.hpp>

#include <lpp/client.hpp>
#include <lpp/location_information.hpp>
#include <lpp/session.hpp>
#include <scheduler/periodic.hpp>

#include "config.hpp"
#include "stage.hpp"

struct InputContext {
    std::string                              name;
    InputInterface const*                    input{};
    std::unique_ptr<format::nmea::Parser>    nmea{};
    std::unique_ptr<format::rtcm::Parser>    rtcm{};
    std::unique_ptr<format::ubx::Parser>     ubx{};
    std::unique_ptr<format::ctrl::Parser>    ctrl{};
    std::unique_ptr<format::lpp::UperParser> lpp_uper{};
    std::unique_ptr<format::lpp::UperParser> lpp_uper_pad{};
};

struct Program {
    Config               config;
    scheduler::Scheduler scheduler;
    streamline::System   stream;
    bool                 is_disconnected;

    lpp::PeriodicSessionHandle assistance_data_session{};
    size_t                     assistance_data_request_count;

    lpp::Optional<lpp::LocationInformation> latest_location_information;
    lpp::Optional<lpp::HaGnssMetrics>       latest_gnss_metrics;
    bool                                    latest_location_information_submitted{false};

    std::unique_ptr<supl::Cell>     initial_cell;
    std::unique_ptr<supl::Cell>     cell;
    std::unique_ptr<supl::Identity> identity;
    std::unique_ptr<lpp::Client>    client;

    std::vector<std::unique_ptr<InputContext>> input_contexts;
    std::vector<std::unique_ptr<InputStage>>   input_stages;

    std::unique_ptr<scheduler::PeriodicTask> fake_location_task;

    void update_location_information(lpp::LocationInformation const& location) {
        latest_location_information           = location;
        latest_location_information_submitted = false;
    }

    void update_gnss_metrics(lpp::HaGnssMetrics const& metrics) { latest_gnss_metrics = metrics; }

    bool has_ctrl_parsers() const {
        for (auto const& input : input_contexts) {
            if (input->ctrl) return true;
        }
        return false;
    }

    bool has_nmea_parsers() const {
        for (auto const& input : input_contexts) {
            if (input->nmea) return true;
        }
        return false;
    }

    bool has_ubx_parsers() const {
        for (auto const& input : input_contexts) {
            if (input->ubx) return true;
        }
        return false;
    }
};
