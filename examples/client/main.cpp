#include <cxx11_compat.hpp>
#include <version.hpp>

#include <cmath>

#include <external_warnings.hpp>
EXTERNAL_WARNINGS_PUSH
#include <args.hpp>
EXTERNAL_WARNINGS_POP

#include <format/ctrl/cid.hpp>
#include <format/ctrl/identity.hpp>

#include <loglet/loglet.hpp>
#include <lpp/assistance_data.hpp>
#include <lpp/messages/provide_location_information.hpp>
#include <lpp/provide_capabilities.hpp>
#include <scheduler/timeout.hpp>

#include <arpa/inet.h>
#include <unistd.h>

#ifdef DATA_TRACING
#include <datatrace/datatrace.hpp>
#endif

#include "processor/control.hpp"
#include "processor/location_information.hpp"
#include "processor/lpp.hpp"
#include "processor/nmea.hpp"
#include "processor/rtcm.hpp"
#include "processor/test.hpp"
#include "processor/ubx.hpp"
#include "processor/ubx_options.hpp"

#if defined(INCLUDE_GENERATOR_RTCM)
#include "processor/lpp2eph.hpp"
#include "processor/lpp2frame_rtcm.hpp"
#include "processor/lpp2rtcm.hpp"
#include "processor/rtcm2eph.hpp"
#include "processor/ubx2eph.hpp"
#endif

#if defined(INCLUDE_GENERATOR_SPARTN)
#include "processor/lpp2spartn.hpp"
#endif

#if defined(INCLUDE_GENERATOR_TOKORO)
#include "processor/agnss.hpp"
#include "processor/tokoro.hpp"
#endif

#if defined(INCLUDE_GENERATOR_IDOKEIDO)
#include "processor/idokeido.hpp"
#endif

#ifdef DATA_TRACING
#include "processor/possib_logger.hpp"
#endif

#include "client.hpp"
#include "processor/tlf.hpp"

LOGLET_MODULE(client);
LOGLET_MODULE(output);
LOGLET_MODULE(p);
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF(client)

static void client_request(Program& program, lpp::Client& client) {
    if (!program.config.assistance_data.enabled) {
        DEBUGF("assistance data is disabled");
        return;
    } else if (!program.cell) {
        ERRORF("internal error: no cell information");
        return;
    }

    auto cell = *program.cell.get();
    if (program.assistance_data_request_count == 0 && program.initial_cell &&
        !program.config.assistance_data.use_latest_cell_on_reconnect) {
        cell = *program.initial_cell;
        DEBUGF("using initial cell for first assistance data request");

        // To ensure that a cell change event is triggered after the first
        // assistance data request, we need to invalidate the current cell
        // information by resetting the cell to the initial cell.
        program.cell.reset(new supl::Cell(cell));
    }

    DEBUGF("request assistance data (GNSS)");

    program.assistance_data_request_count++;
    program.assistance_data_session = client.request_assistance_data({
        program.config.assistance_data.type,
        cell,
        {
            program.config.assistance_data.gps,
            program.config.assistance_data.glonass,
            program.config.assistance_data.galileo,
            program.config.assistance_data.beidou,
        },
        {
            program.config.assistance_data.delivery_amount,
            program.config.assistance_data.antenna_height,
        },
        [&program](lpp::Client&, lpp::Message message) {
            INFOF("provide assistance data (non-periodic)");
            program.stream.push(std::move(message));
        },
        [&program](lpp::Client&, lpp::PeriodicSessionHandle, lpp::Message message) {
            INFOF("provide assistance data (periodic)");
            program.stream.push(std::move(message));
        },
        [](lpp::Client&, lpp::PeriodicSessionHandle) {
            INFOF("request assistance data (started)");
        },
        [](lpp::Client&, lpp::PeriodicSessionHandle) {
            INFOF("request assistance data (ended)");
        },
        [&](lpp::Client&) {
            ERRORF("request assistance data failed");
        },
    });
}

static void client_request_assisted_gnss(Program& program, lpp::Client& client) {
    if (!program.cell) {
        ERRORF("internal error: no cell information");
        return;
    }

    DEBUGF("request assistance data (AGNSS)");

    auto cell = *program.cell.get();
    client.request_assistance_data({
        lpp::SingleRequestAssistanceData::Type::AGNSS,
        cell,
        {
            program.config.assistance_data.gps,
            program.config.assistance_data.glonass,
            program.config.assistance_data.galileo,
            program.config.assistance_data.beidou,
        },
        [&program](lpp::Client&, lpp::Message message) {
            INFOF("[AGNSS] provide assistance data");
            program.stream.push(std::move(message));
        },
        [&](lpp::Client&) {
            ERRORF("[AGNSS] request assistance data failed");
        },
    });
}

static void client_location_information(Program& program, lpp::Client& client) {
    if (!program.config.location_information.unsolicited) {
        return;
    } else if (!program.config.location_information.enable) {
        DEBUGF("location information is disabled");
        return;
    }

    lpp::PeriodicLocationInformationDeliveryDescription description{};
    description.ha_gnss_metrics            = true;
    description.reporting_amount_unlimited = true;
    description.reporting_interval         = std::chrono::seconds(1);
    description.coordinate_type.ha_ellipsoid_point_with_altitude_and_uncertainty_ellipsoid = true;
    description.coordinate_type.ha_ellipsoid_point_with_scalable_altitude_and_uncertainty_ellipse =
        true;
    description.coordinate_type.ha_ellipsoid_point_with_scalable_uncertainty_ellipse = true;
    description.coordinate_type.ha_ellipsoid_point_with_uncertainty_ellipse          = true;

    client.start_periodic_location_information(description);
    DEBUGF("started periodic location information");
}

static void client_initialize(Program& program, lpp::Client&) {
    program.client->on_connected = [](lpp::Client&) {
        INFOF("connected to location server");
    };

    program.client->on_disconnected = [&program](lpp::Client&) {
        INFOF("disconnected from location server");
        program.is_disconnected               = true;
        program.assistance_data_request_count = 0;

        if (program.config.location_server.shutdown_on_disconnect) {
            INFOF("shutting down location server");
            exit(1);
        }
    };

    program.client->on_provide_capabilities = [&program](lpp::Client& client) {
        INFOF("capabilities handshake completed");
        client_request(program, client);
        client_location_information(program, client);
    };

    program.client->on_request_location_information = [&program](lpp::Client&,
                                                                 lpp::TransactionHandle const&,
                                                                 lpp::Message const&) {
        INFOF("request location information");

        if (program.config.location_information.unsolicited) {
            WARNF("unsolicited location information already requested");
            WARNF("the request location information will be ignored");
            return true;
        }

        if (!program.config.location_information.enable) {
            WARNF("ignoring request location information because location information is disabled");
            return true;
        }

        return false;
    };

    program.client->on_provide_location_information =
        [&program](lpp::Client&, lpp::LocationInformationDelivery const&,
                   lpp::messages::ProvideLocationInformation& data) {
            INFOF("provide location information");

            if (program.latest_location_information.has_value()) {
                if (program.latest_location_information_submitted) {
                    DEBUGF("location information already submitted");
                    return false;
                }

                data.location_information = program.latest_location_information.const_value();
                data.gnss_metrics         = program.latest_gnss_metrics;
                program.latest_location_information_submitted = true;
                return true;
            }

            DEBUGF("no location information available");
            return false;
        };

    if (program.config.assistance_data.request_assisted_gnss) {
        client_request_assisted_gnss(program, *program.client);
    }

    // Configure Capaiblities
    lpp::ProvideCapabilities capabilities{};
    capabilities.gnss.gps     = program.config.assistance_data.gps;
    capabilities.gnss.glonass = program.config.assistance_data.glonass;
    capabilities.gnss.galileo = program.config.assistance_data.galileo;
    capabilities.gnss.beidou  = program.config.assistance_data.beidou;

    if (program.config.assistance_data.type == lpp::PeriodicRequestAssistanceData::Type::OSR) {
        capabilities.assistance_data.osr = true;
    } else if (program.config.assistance_data.type ==
               lpp::PeriodicRequestAssistanceData::Type::SSR) {
        capabilities.assistance_data.ssr = true;
    }

    program.client->set_capabilities(capabilities);

    program.client->set_hack_bad_transaction_initiator(
        program.config.location_server.hack_bad_transaction_initiator);
    program.client->set_hack_never_send_abort(program.config.location_server.hack_never_send_abort);
}

static void process_input(Program& program, InputContext& p, InputFormat formats,
                          uint8_t const* buffer, size_t count, uint64_t tag) {
    VERBOSEF("input %s: %zu bytes", p.name.c_str(), count);
#if !defined(DISABLE_VERBOSE)
    if (loglet::is_module_level_enabled(LOGLET_CURRENT_MODULE, loglet::Level::Verbose)) {
        char print_buffer[512];
        for (size_t i = 0; i < count;) {
            int print_count = 0;
            for (size_t j = 0; j < 16; j++) {
                if (i + j < count) {
                    print_count +=
                        snprintf(print_buffer + print_count, sizeof(print_buffer) - print_count,
                                 "%02X ", buffer[i + j]);
                } else {
                    print_count += snprintf(print_buffer + print_count,
                                            sizeof(print_buffer) - print_count, "   ");
                }
            }
            for (size_t j = 0; j < 16; j++) {
                if (i + j < count) {
                    print_count +=
                        snprintf(print_buffer + print_count, sizeof(print_buffer) - print_count,
                                 "%c", isprint(buffer[i + j]) ? buffer[i + j] : '.');
                }
            }

            TRACEF("%s", print_buffer);
            i += 16;
        }
    }
#endif

    if (p.nmea && (formats & INPUT_FORMAT_NMEA) != 0) {
        p.nmea->append(buffer, count);
        for (;;) {
            auto message = p.nmea->try_parse();
            if (!message) break;
            if (p.input->discard_errors && dynamic_cast<format::nmea::ErrorMessage*>(message.get()))
                continue;
            if (p.input->discard_unknowns &&
                dynamic_cast<format::nmea::UnsupportedMessage*>(message.get()))
                continue;
            if (p.input->print) message->print();
            program.stream.push(std::move(message), tag);
        }
    }

    if (p.rtcm && (formats & INPUT_FORMAT_RTCM) != 0) {
        p.rtcm->append(buffer, count);
        for (;;) {
            auto message = p.rtcm->try_parse();
            if (!message) break;
            if (p.input->discard_errors && dynamic_cast<format::rtcm::ErrorMessage*>(message.get()))
                continue;
            if (p.input->discard_unknowns &&
                dynamic_cast<format::rtcm::UnsupportedMessage*>(message.get()))
                continue;
            if (p.input->print) message->print();
            program.stream.push(std::move(message));
        }
    }

    if (p.ubx && (formats & INPUT_FORMAT_UBX) != 0) {
        p.ubx->append(buffer, count);
        for (;;) {
            auto message = p.ubx->try_parse();
            if (!message) break;
            if (p.input->discard_unknowns &&
                dynamic_cast<format::ubx::UnsupportedMessage*>(message.get()))
                continue;
            if (p.input->print) message->print();
            program.stream.push(std::move(message), tag);
        }
    }

    if (p.ctrl && (formats & INPUT_FORMAT_CTRL) != 0) {
        p.ctrl->append(buffer, count);
        for (;;) {
            auto message = p.ctrl->try_parse();
            if (!message) break;
            if (p.input->print) message->print();
            program.stream.push(std::move(message), tag);
        }
    }

    if (p.lpp_uper && (formats & INPUT_FORMAT_LPP_UPER) != 0) {
        p.lpp_uper->append(buffer, count);
        for (;;) {
            auto message = p.lpp_uper->try_parse();
            if (!message) break;

            auto lpp_message = lpp::Message{message};
            if (p.input->print) {
                lpp::print(lpp_message);
            }
            program.stream.push(std::move(lpp_message), tag);
        }
    }

    if (p.lpp_uper_pad && (formats & INPUT_FORMAT_LPP_UPER_PAD) != 0) {
        p.lpp_uper_pad->append(buffer, count);
        for (;;) {
            auto message = p.lpp_uper_pad->try_parse_provide_assistance_data();
            if (!message) break;

            if (p.input->print) {
                lpp::print(message);
            }

            lpp::destroy(message);
        }
    }
}

static void initialize_inputs(Program& program, InputConfig& config) {
    for (auto& input : config.inputs) {
        format::nmea::Parser*    nmea{};
        format::rtcm::Parser*    rtcm{};
        format::ubx::Parser*     ubx{};
        format::ctrl::Parser*    ctrl{};
        format::lpp::UperParser* lpp_uper{};
        format::lpp::UperParser* lpp_uper_pad{};

        if ((input.format & INPUT_FORMAT_NMEA) != 0)
            nmea = new format::nmea::Parser{input.nmea_lf_only};
        if ((input.format & INPUT_FORMAT_RTCM) != 0) rtcm = new format::rtcm::Parser{};
        if ((input.format & INPUT_FORMAT_UBX) != 0) ubx = new format::ubx::Parser{};
        if ((input.format & INPUT_FORMAT_CTRL) != 0) ctrl = new format::ctrl::Parser{};
        if ((input.format & INPUT_FORMAT_LPP_UPER) != 0) lpp_uper = new format::lpp::UperParser{};
        if ((input.format & INPUT_FORMAT_LPP_UPER_PAD) != 0)
            lpp_uper_pad = new format::lpp::UperParser{};

        std::stringstream tag_stream;
        for (size_t i = 0; i < input.tags.size(); i++) {
            if (i > 0) tag_stream << ",";
            tag_stream << input.tags[i];
        }

        auto tag_str = tag_stream.str();
        auto tag     = program.config.get_tag(input.tags) | program.config.get_tag("input");

        std::stringstream stage_stream;
        for (size_t i = 0; i < input.stages.size(); i++) {
            if (i > 0) stage_stream << "->";
            stage_stream << input.stages[i];
        }

        auto stage_str = stage_stream.str();

        DEBUGF("input  %p: %s%s%s%s%s%s %s[%" PRIu64 "] | stages=%s", input.interface.get(),
               (input.format & INPUT_FORMAT_UBX) ? "ubx " : "",
               (input.format & INPUT_FORMAT_NMEA) ? "nmea " : "",
               (input.format & INPUT_FORMAT_RTCM) ? "rtcm " : "",
               (input.format & INPUT_FORMAT_CTRL) ? "ctrl " : "",
               (input.format & INPUT_FORMAT_LPP_UPER) ? "lpp-uper " : "",
               (input.format & INPUT_FORMAT_LPP_UPER_PAD) ? "lpp-uper-pad " : "", tag_str.c_str(),
               tag, stage_str.c_str());

        if (!nmea && !rtcm && !ubx && !ctrl && !lpp_uper && !lpp_uper_pad) {
            WARNF("-- skipping input %p, no format specified", input.interface.get());
            continue;
        }

        std::string event_name = input.interface->event_name();
        if (nmea) event_name += "+nmea";
        if (rtcm) event_name += "+rtcm";
        if (ubx) event_name += "+ubx";
        if (ctrl) event_name += "+ctrl";
        if (lpp_uper) event_name += "+lpp-uper";
        if (lpp_uper_pad) event_name += "+lpp-uper-pad";

        input.interface->set_event_name(event_name);

        if (program.config.input.shutdown_on_complete && !input.exclude_from_shutdown) {
            program.active_inputs++;
            DEBUGF("registered input for shutdown tracking (active_inputs=%d)",
                   program.active_inputs.load());
            input.interface->on_complete = [&program]() {
                auto remaining = --program.active_inputs;
                DEBUGF("input completed (active_inputs=%d)", remaining);
                if (remaining == 0 && !program.shutdown_scheduled) {
                    program.shutdown_scheduled = true;
                    INFOF("all inputs completed, scheduling shutdown in %ld ms",
                          program.config.input.shutdown_delay.count());
                    program.shutdown_task.reset(
                        new scheduler::TimeoutTask(program.config.input.shutdown_delay));
                    program.shutdown_task->callback = [&program]() {
                        INFOF("shutdown timeout expired, interrupting scheduler");
                        program.scheduler.interrupt();
                    };
                    if (!program.shutdown_task->schedule(program.scheduler)) {
                        ERRORF("failed to schedule shutdown task");
                        program.shutdown_task.reset();
                    }
                }
            };
        }

        auto context   = std::make_unique<InputContext>();
        context->name  = std::move(event_name);
        context->input = &input;
        if (nmea) context->nmea = std::unique_ptr<format::nmea::Parser>(nmea);
        if (rtcm) context->rtcm = std::unique_ptr<format::rtcm::Parser>(rtcm);
        if (ubx) context->ubx = std::unique_ptr<format::ubx::Parser>(ubx);
        if (ctrl) context->ctrl = std::unique_ptr<format::ctrl::Parser>(ctrl);
        if (lpp_uper) context->lpp_uper = std::unique_ptr<format::lpp::UperParser>(lpp_uper);
        if (lpp_uper_pad)
            context->lpp_uper_pad = std::unique_ptr<format::lpp::UperParser>(lpp_uper_pad);

        auto context_ptr = context.get();
        program.input_contexts.push_back(std::move(context));

        auto stage = std::unique_ptr<InputStage>(
            new InterfaceInputStage(std::move(input.interface), input.format));
        stage->callback = [context_ptr, &program, tag](InputFormat format, uint8_t const* buffer,
                                                       size_t length) {
            process_input(program, *context_ptr, format, buffer, length, tag);
        };

        for (auto const& stage_name : input.stages) {
            if (stage_name == "tlf") {
                stage = std::unique_ptr<InputStage>(new TlfInputStage(std::move(stage)));
            } else {
                ERRORF("unsupported stage: %s", stage_name.c_str());
            }
        }

        (void)stage->schedule(program.scheduler);

        program.input_stages.push_back(std::move(stage));
    }
}

static void initialize_outputs(Program& program, OutputConfig& config) {
    VSCOPE_FUNCTION();

    bool lpp_xer_output  = false;
    bool lpp_uper_output = false;
    bool nmea_output     = false;
    bool ubx_output      = false;
    bool ctrl_output     = false;
    // TODO(ewasjon): bool spartn_output   = false;
    bool rtcm_output = false;
#ifdef DATA_TRACING
    bool possib_output = false;
#endif
    bool location_output = false;
    bool test_output     = false;
    for (auto& output : config.outputs) {
        if (!output.initial_interface) continue;

        std::stringstream itag_stream;
        for (size_t i = 0; i < output.include_tags.size(); i++) {
            if (i > 0) itag_stream << ",";
            itag_stream << output.include_tags[i];
        }

        auto itag_str = itag_stream.str();

        std::stringstream otag_stream;
        for (size_t i = 0; i < output.exclude_tags.size(); i++) {
            if (i > 0) otag_stream << ",";
            otag_stream << output.exclude_tags[i];
        }

        auto otag_str = otag_stream.str();

        output.include_tag_mask = program.config.get_tag(output.include_tags);
        output.exclude_tag_mask = program.config.get_tag(output.exclude_tags);

        std::stringstream stage_stream;
        for (size_t i = 0; i < output.stages.size(); i++) {
            if (i > 0) stage_stream << "->";
            stage_stream << output.stages[i];
        }

        auto stage_str = stage_stream.str();

        DEBUGF("output: %-14s %s%s%s%s%s%s%s%s%s%s%s | include=%s[%" PRIu64
               "] | exclude=%s[%" PRIu64 "] | stages=%s",
               output.initial_interface.get()->name(),
               (output.format & OUTPUT_FORMAT_UBX) ? "ubx " : "",
               (output.format & OUTPUT_FORMAT_NMEA) ? "nmea " : "",
               (output.format & OUTPUT_FORMAT_SPARTN) ? "spartn " : "",
               (output.format & OUTPUT_FORMAT_RTCM) ? "rtcm " : "",
               (output.format & OUTPUT_FORMAT_CTRL) ? "ctrl " : "",
               (output.format & OUTPUT_FORMAT_LPP_XER) ? "lpp-xer " : "",
               (output.format & OUTPUT_FORMAT_LPP_UPER) ? "lpp-uper " : "",
               (output.format & OUTPUT_FORMAT_LFR) ? "lfr " : "",
               (output.format & OUTPUT_FORMAT_POSSIB) ? "possib " : "",
               (output.format & OUTPUT_FORMAT_LOCATION) ? "location " : "",
               (output.format & OUTPUT_FORMAT_TEST) ? "test " : "", itag_str.c_str(),
               output.include_tag_mask, otag_str.c_str(), output.exclude_tag_mask,
               stage_str.c_str());

        if (output.lpp_xer_support()) lpp_xer_output = true;
        if (output.lpp_uper_support()) lpp_uper_output = true;
        if (output.nmea_support()) nmea_output = true;
        if (output.ubx_support()) ubx_output = true;
        if (output.ctrl_support()) ctrl_output = true;
        // TODO(ewasjon): if (output.spartn_support()) spartn_output = true;
        if (output.rtcm_support()) rtcm_output = true;
#ifdef DATA_TRACING
        if (output.possib_support()) possib_output = true;
#endif
        if (output.location_support()) location_output = true;
        if (output.test_support()) test_output = true;

        auto last_stage = std::unique_ptr<OutputStage>(
            new InterfaceOutputStage(std::move(output.initial_interface)));
        if (!output.stages.empty()) {
            for (int i = output.stages.size() - 1; i >= 0; i--) {
                auto& stage_name = output.stages[i];
                if (stage_name == "tlf") {
                    last_stage = std::make_unique<TlfOutputStage>(std::move(last_stage));
                } else {
                    ERRORF("unsupported stage: %s", stage_name.c_str());
                }
            }
        }
        output.stage = std::move(last_stage);

        ASSERT(output.stage, "stage is null");
        if (!output.stage->schedule(program.scheduler)) {
            ERRORF("failed to schedule output");
        }
    }

    if (lpp_xer_output) program.stream.add_inspector<LppXerOutput>(config);
    if (lpp_uper_output) program.stream.add_inspector<LppUperOutput>(config);
    if (nmea_output) program.stream.add_inspector<NmeaOutput>(config);
    if (ubx_output) program.stream.add_inspector<UbxOutput>(config);
    if (rtcm_output) program.stream.add_inspector<RtcmOutput>(config);
    if (ctrl_output) program.stream.add_inspector<CtrlOutput>(config);
#ifdef DATA_TRACING
    if (possib_output) program.stream.add_inspector<PossibOutput>(config);
#endif
    if (location_output) program.stream.add_inspector<LocationOutput>(config);
    if (test_output) test_outputer(program.scheduler, config, program.config.get_tag("test"));
}

static void setup_print_inspectors(Program& program) {
    VSCOPE_FUNCTION();

    bool nmea_print  = false;
    bool ubx_print   = false;
    bool rtcm_print  = false;
    bool ctrl_print  = false;
    bool agnss_print = false;

    for (auto& print : program.config.print.prints) {
        print.include_tag_mask = program.config.get_tag(print.include_tags);
        print.exclude_tag_mask = program.config.get_tag(print.exclude_tags);

        if (print.nmea_support()) nmea_print = true;
        if (print.ubx_support()) ubx_print = true;
        if (print.rtcm_support()) rtcm_print = true;
        if (print.ctrl_support()) ctrl_print = true;
        if (print.agnss_support()) agnss_print = true;
    }

    if (nmea_print) program.stream.add_inspector<NmeaPrint>(program.config.print);
    if (ubx_print) program.stream.add_inspector<UbxPrint>(program.config.print);
    if (rtcm_print) program.stream.add_inspector<RtcmPrint>(program.config.print);
    if (ctrl_print) program.stream.add_inspector<CtrlPrint>(program.config.print);
#if defined(INCLUDE_GENERATOR_TOKORO)
    if (agnss_print) program.stream.add_inspector<MissingEphemerisPrint>(program.config.print);
#endif
}

static void setup_location_stream(Program& program) {
    if (program.config.location_information.use_ubx_location) {
        if (!program.has_ubx_parsers()) {
            WARNF("location from UBX enabled but no UBX input is configured");
        }

        DEBUGF("location from UBX enabled");
        program.stream.add_inspector<UbxLocation>(program.config.location_information);
    }

    if (program.config.location_information.use_nmea_location) {
        if (!program.has_nmea_parsers()) {
            WARNF("location from NMEA enabled but no NMEA input is configured");
        }

        DEBUGF("location from NMEA enabled");
        program.stream.add_consumer<NmeaLocation>(program.config.location_information);
    }

    program.stream.add_inspector<LocationCollector>(program);
    program.stream.add_inspector<MetricsCollector>(program);
}

static void setup_control_stream(Program& program) {
    if (!program.has_ctrl_parsers()) {
        DEBUGF("no input with control messages, control messages will be ignored");
        return;
    }

    auto ctrl_events = program.stream.add_inspector<CtrlEvents>();
    if (!ctrl_events) {
        WARNF("failed to create control events inspector, control messages will be ignored");
        return;
    }

    ctrl_events->on_cell_id = [&program](format::ctrl::CellId const& cell) {
        supl::Cell new_cell{};
        if (cell.is_nr()) {
            new_cell = supl::Cell::nr(cell.mcc(), cell.mnc(), cell.tac(), cell.cell());
        } else {
            new_cell = supl::Cell::lte(cell.mcc(), cell.mnc(), cell.tac(), cell.cell());
        }

        auto cell_changed = !program.cell || (*program.cell.get() != new_cell);
        program.cell.reset(new supl::Cell(new_cell));

        if (new_cell.type == supl::Cell::Type::GSM) {
            WARNF("GSM is not supported");
        } else if (new_cell.type == supl::Cell::Type::LTE) {
            INFOF("cell: LTE %" PRIi64 ":%" PRIi64 ":%" PRIi64 ":%" PRIu64 " %s",
                  new_cell.data.lte.mcc, new_cell.data.lte.mnc, new_cell.data.lte.tac,
                  new_cell.data.lte.ci, cell_changed ? "(changed)" : "");
        } else if (new_cell.type == supl::Cell::Type::NR) {
            INFOF("cell: NR %" PRIi64 ":%" PRIi64 ":%" PRIi64 ":%" PRIu64 " %s",
                  new_cell.data.nr.mcc, new_cell.data.nr.mnc, new_cell.data.nr.tac,
                  new_cell.data.nr.ci, cell_changed ? "(changed)" : "");
        } else {
            WARNF("unsupported cell type");
        }

        if (!program.initial_cell && program.config.assistance_data.wait_for_cell) {
            INFOF("found initial cell");
            program.initial_cell.reset(new supl::Cell(new_cell));
        }

        if (cell_changed && program.cell) {
            if (!program.assistance_data_session.is_valid()) {
                WARNF("cell id received, but no assistance data session is active");
                return;
            } else if (!program.client) {
                WARNF("cell id received, but no client is active (--ls-disable is set)");
                return;
            }

            if (!program.client->update_assistance_data(program.assistance_data_session,
                                                        *program.cell.get())) {
                WARNF("failed to update assistance data with new cell");
            }
        }
    };

    ctrl_events->on_identity_imsi = [&program](format::ctrl::IdentityImsi const& identity) {
        if (program.identity) {
            WARNF("identity already set, ignoring new identity");
            return;
        }

        program.identity = std::unique_ptr<supl::Identity>(
            new supl::Identity(supl::Identity::imsi(identity.imsi())));
    };
}

static void setup_fake_location(Program& program) {
    program.fake_location_task = std::unique_ptr<scheduler::PeriodicTask>(
        new scheduler::PeriodicTask{std::chrono::seconds(1)});
    program.fake_location_task->callback = [&program]() {
        DEBUGF("generate fake location information");
        lpp::LocationInformation info{};
        info.time     = ts::Tai::now();
        info.location = lpp::LocationShape::ha_ellipsoid_altitude_with_uncertainty(
            program.config.location_information.fake.latitude,
            program.config.location_information.fake.longitude,
            program.config.location_information.fake.altitude,
            lpp::HorizontalAccuracy::to_ellipse_68(1, 1, 0), lpp::VerticalAccuracy::from_1sigma(1));
        program.stream.push(std::move(info));
    };

    INFOF("enable fake location information");
    if (!program.fake_location_task->schedule(program.scheduler)) {
        ERRORF("failed to schedule fake location information generation");
    }
}

static void setup_lpp2osr(Program& program) {
#if defined(INCLUDE_GENERATOR_RTCM)
    if (program.config.lpp2rtcm.enabled) {
        program.stream.add_inspector<Lpp2Rtcm>(program.config.output, program.config.lpp2rtcm);
    }

    if (program.config.lpp2frame_rtcm.enabled) {
        program.stream.add_inspector<Lpp2FrameRtcm>(program.config.output,
                                                    program.config.lpp2frame_rtcm);
    }

    if (program.config.lpp2eph.enabled) {
        program.stream.add_inspector<Lpp2Eph>(program.config.lpp2eph);
    }

    if (program.config.ubx2eph.enabled) {
        program.stream.add_inspector<Ubx2Eph>(program.config.ubx2eph);
    }

    if (program.config.rtcm2eph.enabled) {
        program.stream.add_inspector<Rtcm2Eph>(program.config.rtcm2eph);
    }
#endif
}

static void setup_lpp2spartn(Program& program) {
#if defined(INCLUDE_GENERATOR_SPARTN)
    if (program.config.lpp2spartn.enabled) {
        program.stream.add_inspector<Lpp2Spartn>(program.config.output, program.config.lpp2spartn);
    }
#endif
}

static void setup_tokoro(Program& program) {
#if defined(INCLUDE_GENERATOR_TOKORO)
    if (program.config.tokoro.enabled) {
        auto tokoro = program.stream.add_inspector<Tokoro>(
            program.config.output, program.config.tokoro, program.scheduler);
        program.stream.add_inspector<TokoroEphemerisGps>(*tokoro);
        program.stream.add_inspector<TokoroEphemerisGal>(*tokoro);
        program.stream.add_inspector<TokoroEphemerisBds>(*tokoro);
        program.stream.add_inspector<TokoroLocation>(*tokoro);
    }
#endif
}

static void setup_idokeido(Program& program) {
#if defined(INCLUDE_GENERATOR_IDOKEIDO)
    if (program.config.idokeido.enabled) {
        auto idokeido_spp = program.stream.add_inspector<IdokeidoSpp>(
            program.config.output, program.config.idokeido, program.scheduler, program.stream);
        program.stream.add_inspector<IdokeidoEphemerisUbx<IdokeidoSpp>>(*idokeido_spp);
        program.stream.add_inspector<IdokeidoMeasurmentUbx<IdokeidoSpp>>(*idokeido_spp);
    }
#else
    (void)program;
#endif
}

static void apply_ubx_config(Program& program) {
    if (program.config.ubx_config.interfaces.empty()) {
        return;
    }

    INFOF("applying UBX configuration at startup");
    UbxConfigApplicator applicator(program.config.ubx_config, program.scheduler);

    if (!applicator.apply_configurations()) {
        WARNF("failed to apply some UBX configurations");
    }

    if (program.config.ubx_config.apply_and_exit) {
        INFOF("UBX configuration applied, exiting as requested");
        exit(0);
    }
}

static bool setup_agnss(Program& program) {
    if (program.config.agnss.imsi) {
        program.agnss_identity = std::unique_ptr<supl::Identity>(
            new supl::Identity(supl::Identity::imsi(*program.config.agnss.imsi)));
    } else if (program.config.agnss.msisdn) {
        program.agnss_identity = std::unique_ptr<supl::Identity>(
            new supl::Identity(supl::Identity::msisdn(*program.config.agnss.msisdn)));
    } else if (program.config.agnss.ipv4) {
        uint8_t ipv4[4];
        if (inet_pton(AF_INET, program.config.agnss.ipv4->c_str(), ipv4) != 1) {
            ERRORF("invalid A-GNSS IPv4 address: %s", program.config.agnss.ipv4->c_str());
            return false;
        }
        program.agnss_identity =
            std::unique_ptr<supl::Identity>(new supl::Identity(supl::Identity::ipv4(ipv4)));
    }

    if (!program.agnss_identity && !program.identity) {
        ERRORF("identity is required for A-GNSS client");
        return false;
    }
    if (!program.cell) {
        ERRORF("cell information is required for A-GNSS client");
        return false;
    }
    return true;
}

int main(int argc, char** argv) {
    loglet::initialize();

#if defined(FORCE_LOG_LEVEL_TRACE)
    loglet::set_level(loglet::Level::Trace);
#elif defined(FORCE_LOG_LEVEL_VERBOSE)
    loglet::set_level(loglet::Level::Verbose);
#elif defined(FORCE_LOG_LEVEL_DEBUG)
    loglet::set_level(loglet::Level::Debug);
#elif defined(FORCE_LOG_LEVEL_INFO)
    loglet::set_level(loglet::Level::Info);
#elif defined(FORCE_LOG_LEVEL_NOTICE)
    loglet::set_level(loglet::Level::Notice);
#elif defined(FORCE_LOG_LEVEL_WARNING)
    loglet::set_level(loglet::Level::Warning);
#elif defined(FORCE_LOG_LEVEL_ERROR)
    loglet::set_level(loglet::Level::Error);
#endif

    INFOF("S3LC Client %s", CLIENT_VERSION);
    INFOF("  Commit: %s%s (%s)", GIT_COMMIT_HASH, GIT_DIRTY ? "-dirty" : "", GIT_BRANCH);
    INFOF("  Built: %s [%s]", BUILD_DATE, BUILD_TYPE);
    INFOF("  Compiler: %s", BUILD_COMPILER);
    INFOF("  Platform: %s (%s)", BUILD_SYSTEM, BUILD_ARCH);

    Config config{};
    if (!config::parse(argc, argv, &config)) {
        return 1;
    }

    loglet::set_level(config.logging.log_level);
    loglet::set_color_enable(config.logging.color);
    loglet::set_always_flush(config.logging.flush);
    loglet::set_report_errors(config.logging.report_errors);
    loglet::set_use_stderr(config.logging.use_stderr);

    if (config.logging.log_file) {
        FILE* log_fp = fopen(config.logging.log_file->c_str(), "w");
        if (!log_fp) {
            ERRORF("failed to open log file: %s", config.logging.log_file->c_str());
            return 1;
        }
        loglet::set_output_file(log_fp);
        loglet::set_always_flush(true);
    }

    for (auto const& entry : config.logging.module_levels) {
        auto const& name    = entry.first;
        auto const& level   = entry.second;
        auto        modules = loglet::get_modules(name);
        for (auto module : modules) {
            loglet::set_module_level(module, level);
        }
        if (modules.empty()) {
            ERRORF("unknown module(s): %s", name.c_str());
        }
    }

    if (config.logging.tree) {
        loglet::iterate_modules(
            [](loglet::LogModule const* module, int depth, void*) {
                char buffer[64];
                snprintf(buffer, sizeof(buffer), "%*s%s", depth * 2, "", module->name);
                INFOF("%-20s %s%s%s%s%s%s%s%s", buffer,
                      loglet::is_module_level_enabled(module, loglet::Level::Trace) ? "T" : ".",
                      loglet::is_module_level_enabled(module, loglet::Level::Verbose) ? "V" : ".",
                      loglet::is_module_level_enabled(module, loglet::Level::Debug) ? "D" : ".",
                      loglet::is_module_level_enabled(module, loglet::Level::Info) ? "I" : ".",
                      loglet::is_module_level_enabled(module, loglet::Level::Notice) ? "N" : ".",
                      loglet::is_module_level_enabled(module, loglet::Level::Warning) ? "W" : ".",
                      loglet::is_module_level_enabled(module, loglet::Level::Error) ? "E" : ".",
                      loglet::is_module_level_enabled(module, loglet::Level::Disabled) ? "#" : ".");
            },
            nullptr);
    }

    config::dump(&config);

    Program program{};
    program.config = std::move(config);

#ifdef DATA_TRACING
    if (program.config.data_tracing.enabled) {
        datatrace::initialize(
            program.config.data_tracing.device, program.config.data_tracing.server,
            program.config.data_tracing.port, program.config.data_tracing.username,
            program.config.data_tracing.password, program.config.data_tracing.reliable);
        datatrace::set_ssr_data(!program.config.data_tracing.disable_ssr_data);
    }
#endif

    program.stream          = streamline::System{program.scheduler};
    program.is_disconnected = false;

    program.config.register_tag("input");

    initialize_inputs(program, program.config.input);
    initialize_outputs(program, program.config.output);
    setup_print_inspectors(program);

    apply_ubx_config(program);

    setup_location_stream(program);
    DEBUGF("setup_location_stream() completed");
    setup_control_stream(program);
    DEBUGF("setup_control_stream() completed");

    setup_lpp2osr(program);
    DEBUGF("setup_lpp2osr() completed");
    setup_lpp2spartn(program);
    DEBUGF("setup_lpp2spartn() completed");
    setup_tokoro(program);
    DEBUGF("setup_tokoro() completed");
    setup_idokeido(program);
    DEBUGF("setup_idokeido() completed");

    if (program.config.location_information.fake.enabled) {
        setup_fake_location(program);
    }

#ifdef DATA_TRACING
    if (program.config.data_tracing.possib_log) {
        program.stream.add_inspector<LppPossibBuilder>(program.config.data_tracing.possib_wrap);
    }
#endif

    scheduler::PeriodicTask reconnect_task{std::chrono::seconds(15)};
    reconnect_task.set_event_name("lpp-reconnect");
    reconnect_task.callback = [&program]() {
        if (program.is_disconnected) {
            INFOF("reconnecting to LPP server");
            program.is_disconnected = false;
            program.client->schedule(&program.scheduler);
        }
    };

    if (program.config.location_server.enabled || program.config.agnss.enabled) {
        if (program.config.identity.wait_for_identity) {
            INFOF("waiting for identity");

            if (program.config.identity.imsi || program.config.identity.msisdn ||
                program.config.identity.ipv4) {
                WARNF("identity from command line will be ignored");
            }

            if (!program.has_ctrl_parsers()) {
                ERRORF("no input with control messages, cannot wait for identity");
                return 1;
            }

            program.scheduler.execute_while([&program] {
                return program.identity == nullptr;
            });
        }

        if (!program.identity) {
            if (program.config.identity.imsi) {
                program.identity = std::unique_ptr<supl::Identity>(
                    new supl::Identity(supl::Identity::imsi(*program.config.identity.imsi)));
            } else if (program.config.identity.msisdn) {
                program.identity = std::unique_ptr<supl::Identity>(
                    new supl::Identity(supl::Identity::msisdn(*program.config.identity.msisdn)));
            } else if (program.config.identity.ipv4) {
                uint8_t ipv4[4];
                if (inet_pton(AF_INET, program.config.identity.ipv4->c_str(), ipv4) != 1) {
                    ERRORF("invalid IPv4 address: %s", program.config.identity.ipv4->c_str());
                    return 1;
                }

                program.identity =
                    std::unique_ptr<supl::Identity>(new supl::Identity(supl::Identity::ipv4(ipv4)));
            } else {
                ERRORF("you must provide an identity");
                return 1;
            }
        }
    }

    if (program.config.location_server.enabled) {
        if (program.config.assistance_data.wait_for_cell) {
            INFOF("waiting for cell information");
            if (!program.has_ctrl_parsers()) {
                ERRORF("no input with control messages, cannot wait for cell information");
                return 1;
            }

            program.scheduler.execute_while([&program] {
                return !program.initial_cell;
            });
        } else {
            program.initial_cell.reset(new supl::Cell(program.config.assistance_data.cell));
            program.cell.reset(new supl::Cell(program.config.assistance_data.cell));
        }

        if (program.config.location_server.slp_host_cell) {
            if (!program.cell) {
                ERRORF("cell information is required when using --slp-host-cell");
                return 1;
            }

            // h-slp.%03d.%03d.pub.3gppnetwork.org
            char buffer[256];
            snprintf(buffer, sizeof(buffer),
                     "h-slp.%03" PRIu64 ".%03" PRIu64 ".pub.3gppnetwork.org",
                     program.cell->data.nr.mcc, program.cell->data.nr.mnc);
            program.config.location_server.host = buffer;
            INFOF("generated host: \"%s\"", program.config.location_server.host.c_str());
        } else if (program.config.location_server.slp_host_imsi) {
            if (!program.identity) {
                ERRORF("identity information is required when using --slp-host-imsi");
                return 1;
            } else if (program.identity->type != supl::Identity::Type::IMSI) {
                ERRORF("identity must be of type IMSI when using --slp-host-imsi");
                return 1;
            }

            auto imsi   = program.identity->data.imsi;
            auto digits = std::to_string(imsi).size();
            if (digits < 6) {
                throw args::ValidationError("`imsi` must be at least 6 digits long");
            }

            uint64_t mcc = (imsi / static_cast<uint64_t>(std::pow(10, digits - 3))) % 1000;
            uint64_t mnc = (imsi / static_cast<uint64_t>(std::pow(10, digits - 6))) % 1000;

            // h-slp.%03d.%03d.pub.3gppnetwork.org
            char buffer[256];
            snprintf(buffer, sizeof(buffer),
                     "h-slp.%03" PRIu64 ".%03" PRIu64 ".pub.3gppnetwork.org", mcc, mnc);
            program.config.location_server.host = buffer;
            INFOF("generated host: \"%s\"", program.config.location_server.host.c_str());
        }

        ASSERT(program.identity, "identity must be set by now");
        ASSERT(program.cell, "cell must be set by now");
        auto client = new lpp::Client{
            *program.identity,
            *program.cell,
            program.config.location_server.host,
            program.config.location_server.port,
        };
        if (program.config.location_server.interface) {
            client->set_interface(*program.config.location_server.interface);
        }
        program.client.reset(client);

        client_initialize(program, *client);

        if (!reconnect_task.schedule(program.scheduler)) {
            ERRORF("failed to schedule reconnect task");
        }
        client->schedule(&program.scheduler);
    } else if (program.config.agnss.enabled && !program.config.assistance_data.wait_for_cell) {
        DEBUGF("[AGNSS] initializing cell for A-GNSS (location server disabled)");
        program.initial_cell.reset(new supl::Cell(program.config.assistance_data.cell));
        program.cell.reset(new supl::Cell(program.config.assistance_data.cell));
    }

    if (program.config.agnss.enabled) {
        if (!setup_agnss(program)) {
            return 1;
        }
        if (!program.cell) {
            WARNF("A-GNSS enabled but cell not set");
        } else {
            ASSERT(program.agnss_identity || program.identity, "at least one identity must be set");
            auto& identity = program.agnss_identity ? *program.agnss_identity : *program.identity;
            program.stream.add_inspector<AGnssProcessor>(
                program.config.agnss, identity, *program.cell, program.scheduler, program.stream);
        }
    }

    program.scheduler.execute();
    return 0;
}
