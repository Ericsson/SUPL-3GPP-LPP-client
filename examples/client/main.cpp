#include <args.hpp>
#include <format/ctrl/cid.hpp>
#include <format/ctrl/identity.hpp>
#include <format/ctrl/parser.hpp>
#include <format/lpp/uper_parser.hpp>
#include <format/nmea/message.hpp>
#include <format/nmea/parser.hpp>
#include <format/ubx/message.hpp>
#include <format/ubx/parser.hpp>
#include <loglet/loglet.hpp>
#include <lpp/assistance_data.hpp>
#include <lpp/client.hpp>
#include <lpp/session.hpp>
#include <scheduler/scheduler.hpp>
#include <streamline/system.hpp>

#include <thread>
#include <unistd.h>

#ifdef DATA_TRACING
#include <datatrace/datatrace.hpp>
#endif

#include "processor/control.hpp"
#include "processor/lpp.hpp"

#include "client.hpp"

#define LOGLET_CURRENT_MODULE "client"

struct Client {
    Config                                                config;
    scheduler::Scheduler                                  scheduler;
    streamline::System                                    stream;
    std::vector<std::unique_ptr<format::nmea::Parser>>    nmea_parsers;
    std::vector<std::unique_ptr<format::ubx::Parser>>     ubx_parsers;
    std::vector<std::unique_ptr<format::ctrl::Parser>>    ctrl_parsers;
    std::vector<std::unique_ptr<format::lpp::UperParser>> lpp_uper_parsers;
};

lpp::PeriodicSessionHandle gAssistanceDataSession{};

static void client_connected(Client& program, lpp::Client& client) {
    gAssistanceDataSession = client.request_assistance_data({
        program.config.assistance_data.type,
        program.config.assistance_data.cell,
        [](lpp::Client& client, lpp::Message message) {
            INFOF("received message (non-periodic)");
        },
        [](lpp::Client& client, lpp::PeriodicSessionHandle session, lpp::Message message) {
            INFOF("received message (periodic)");
        },
        [](lpp::Client& client, lpp::PeriodicSessionHandle session) {
            INFOF("start of assistance data");
        },
        [](lpp::Client& client, lpp::PeriodicSessionHandle session) {
            INFOF("end of assistance data");
        },
    });
}

static void client_initialize(Client& program, lpp::Client& client) {
    client.on_connected = [&program](lpp::Client& client) {
        INFOF("connected to LPP server");
        client_connected(program, client);
    };

    client.on_disconnected = [](lpp::Client& client) {
        INFOF("disconnected from LPP server");
        // TODO: reconnect
    };

    client.on_request_location_information = [](lpp::Client&                  client,
                                                lpp::TransactionHandle const& transaction,
                                                lpp::Message const&           message) {
        INFOF("received request location information");
        return false;
    };

    client.on_provide_location_information = [](lpp::Client&                               client,
                                                lpp::LocationInformationDelivery const&    delivery,
                                                lpp::messages::ProvideLocationInformation& data) {
        INFOF("providing location information");
        return false;
    };
}

static void initialize_inputs(Client& client, InputConfig const& config) {
    for (auto const& input : config.inputs) {
        format::nmea::Parser*    nmea{};
        format::ubx::Parser*     ubx{};
        format::ctrl::Parser*    ctrl{};
        format::lpp::UperParser* lpp_uper{};

        if ((input.format & INPUT_FORMAT_NMEA) != 0) nmea = new format::nmea::Parser{};
        if ((input.format & INPUT_FORMAT_UBX) != 0) ubx = new format::ubx::Parser{};
        if ((input.format & INPUT_FORMAT_CTRL) != 0) ctrl = new format::ctrl::Parser{};
        if ((input.format & INPUT_FORMAT_LPP) != 0) lpp_uper = new format::lpp::UperParser{};

        DEBUGF("input  %p: %s%s%s%s", input.interface.get(),
               (input.format & INPUT_FORMAT_UBX) ? "ubx " : "",
               (input.format & INPUT_FORMAT_NMEA) ? "nmea " : "",
               (input.format & INPUT_FORMAT_CTRL) ? "ctrl " : "",
               (input.format & INPUT_FORMAT_LPP) ? "lpp " : "");

        if (!nmea && !ubx && !ctrl && !lpp_uper) {
            WARNF("-- skipping input %p, no format specified", input.interface.get());
            continue;
        }

        if (nmea) client.nmea_parsers.push_back(std::unique_ptr<format::nmea::Parser>(nmea));
        if (ubx) client.ubx_parsers.push_back(std::unique_ptr<format::ubx::Parser>(ubx));
        if (ctrl) client.ctrl_parsers.push_back(std::unique_ptr<format::ctrl::Parser>(ctrl));
        if (lpp_uper)
            client.lpp_uper_parsers.push_back(std::unique_ptr<format::lpp::UperParser>(lpp_uper));

        input.interface->schedule(client.scheduler);
        input.interface->callback = [&client, nmea, ubx, ctrl,
                                     lpp_uper](io::Input&, uint8_t* buffer, size_t count) {
            if (nmea) {
                nmea->append(buffer, count);
                for (;;) {
                    auto message = nmea->try_parse();
                    if (!message) break;
                    client.stream.push(std::move(message));
                }
            }

            if (ubx) {
                ubx->append(buffer, count);
                for (;;) {
                    auto message = ubx->try_parse();
                    if (!message) break;
                    client.stream.push(std::move(message));
                }
            }

            if (ctrl) {
                ctrl->append(buffer, count);
                for (;;) {
                    auto message = ctrl->try_parse();
                    if (!message) break;
                    client.stream.push(std::move(message));
                }
            }

            if (lpp_uper) {
                lpp_uper->append(buffer, count);
                for (;;) {
                    auto message = lpp_uper->try_parse();
                    if (!message) break;
                    XDEBUGF("lpp/msg", "create %p", message);
                    auto lpp_message = lpp::Message{message};
                    client.stream.push(std::move(lpp_message));
                }
            }
        };
    }
}

static void initialize_outputs(Client& client, OutputConfig const& config) {
    VSCOPE_FUNCTION();

    bool lpp_xer_output  = false;
    bool lpp_uper_output = false;
    bool nmea_output     = false;
    bool ubx_output      = false;
    bool ctrl_output     = false;
    bool spartn_output   = false;
    bool lpp_rf_output   = false;
    for (auto& output : config.outputs) {
        DEBUGF("output %p: %s%s%s%s%s%s%s", output.interface.get(),
               (output.format & OUTPUT_FORMAT_UBX) ? "ubx " : "",
               (output.format & OUTPUT_FORMAT_NMEA) ? "nmea " : "",
               (output.format & OUTPUT_FORMAT_SPARTN) ? "spartn " : "",
               (output.format & OUTPUT_FORMAT_CTRL) ? "ctrl " : "",
               (output.format & OUTPUT_FORMAT_LPP_XER) ? "lpp-xer " : "",
               (output.format & OUTPUT_FORMAT_LPP_UPER) ? "lpp-uper " : "",
               (output.format & OUTPUT_FORMAT_LPP_RTCM_FRAME) ? "lpp-rf " : "");

        if ((output.format & OUTPUT_FORMAT_LPP_XER) != 0) lpp_xer_output = true;
        if ((output.format & OUTPUT_FORMAT_LPP_UPER) != 0) lpp_uper_output = true;
        if ((output.format & OUTPUT_FORMAT_NMEA) != 0) nmea_output = true;
        if ((output.format & OUTPUT_FORMAT_UBX) != 0) ubx_output = true;
        if ((output.format & OUTPUT_FORMAT_CTRL) != 0) ctrl_output = true;
        if ((output.format & OUTPUT_FORMAT_SPARTN) != 0) spartn_output = true;
        if ((output.format & OUTPUT_FORMAT_LPP_RTCM_FRAME) != 0) lpp_rf_output = true;
    }

    if (lpp_xer_output) client.stream.add_inspector<LppXerOutput>(config);
    if (lpp_uper_output) client.stream.add_inspector<LppUperOutput>(config);
    // if (nmea_output) client.stream.add_inspector<NmeaOutput>(config);
    // if (ubx_output) client.stream.add_inspector<UbxOutput>(config);
    if (ctrl_output) client.stream.add_inspector<CtrlOutput>(config);
}

int main(int argc, char** argv) {
#if defined(FORCE_LOG_LEVEL_VERBOSE)
    loglet::set_level(loglet::Level::Verbose);
#elif defined(FORCE_LOG_LEVEL_DEBUG)
    loglet::set_level(loglet::Level::Debug);
#elif defined(FORCE_LOG_LEVEL_INFO)
    loglet::set_level(loglet::Level::Info);
#elif defined(FORCE_LOG_LEVEL_WARNING)
    loglet::set_level(loglet::Level::Warning);
#elif defined(FORCE_LOG_LEVEL_ERROR)
    loglet::set_level(loglet::Level::Error);
#endif

    INFOF("S3LP Client (" CLIENT_VERSION ")");

    Client program{};
    if (!config::parse(argc, argv, &program.config)) {
        return 1;
    }

    loglet::set_level(program.config.logging.log_level);
    for (auto const& [module, level] : program.config.logging.module_levels) {
        loglet::set_module_level(module.c_str(), level);
    }

#ifdef DATA_TRACING
    if (program.config.data_tracing.enabled) {
        datatrace::initialize(program.config.data_tracing.device,
                              program.config.data_tracing.server, program.config.data_tracing.port,
                              program.config.data_tracing.username,
                              program.config.data_tracing.password);
    }
#endif

    initialize_inputs(program, program.config.input);
    initialize_outputs(program, program.config.output);

    program.scheduler.execute();

    return 0;

    loglet::disable_module("sched");
    loglet::disable_module("supl");
    loglet::disable_module("lpp/s");
    // loglet::disable_module("lpp/c");
    // loglet::disable_module("lpp/ad");
    // loglet::disable_module("lpp/ps");

    // gConfig.cell          = supl::Cell::lte(240, 1, 1, 3);
    // gConfig.output_format = OutputFormat::RTCM;

    lpp::Client client{
        supl::Identity::msisdn(919825098250),
        "129.192.83.118",
        5431,
    };

    client_initialize(program, client);

    client.schedule(&program.scheduler);

#if 0

    lpp::Session session{
        lpp::VERSION_16_4_0,
        supl::Identity::msisdn(919825098250),
    };

    session.on_connected = [](lpp::Session& session) {
        INFOF("connected to LPP server");
    };

    session.on_disconnected = [](lpp::Session& session) {
        INFOF("disconnected from LPP server");
    };

    session.on_established = [](lpp::Session& session) {
        INFOF("established with LPP server");

        transaction = session.create_transaction();

        auto request_assistance_data =
            lpp::create_request_assistance_data(lpp::RequestAssistanceData{
                .cell                       = supl::Cell::lte(240, 1, 1, 3),
                .periodic_session_id        = 1,
                .periodic_session_initiator = false,
                .gps                        = true,
                .glonass                    = true,
                .galileo                    = true,
                .bds                        = false,
                .rtk_observations           = 1,
                .rtk_residuals              = 1,
                .rtk_bias_information       = 1,
                .rtk_reference_station_info = 1,
            });
        transaction->send(request_assistance_data);
    };

    session.on_begin_transaction = [](lpp::Session&                 session,
                                      const lpp::TransactionHandle& transaction) {
        INFOF("(%s%ld) transaction started", transaction.is_client() ? "C" : "S", transaction.id());
    };

    session.on_end_transaction = [](lpp::Session&                 session,
                                    const lpp::TransactionHandle& transaction) {
        INFOF("(%s%ld) transaction ended", transaction.is_client() ? "C" : "S", transaction.id());
    };

    session.on_message = [](lpp::Session& session, const lpp::TransactionHandle& transaction,
                            lpp::Message message) {
        INFOF("(%s%ld) received message", transaction.is_client() ? "C" : "S", transaction.id());
    };

    if (!session.connect("129.192.83.118", 5431)) {
        ERRORF("failed to connect to LPP server");
        return 1;
    }

    session.schedule(&scheduler);
#endif

    program.scheduler.execute();
#if 0
    double test = 0;

    // create eventfd for testing
    int eventfd = ::eventfd(0, EFD_NONBLOCK);
    if (eventfd == -1) {
        std::cerr << "Failed to create eventfd" << std::endl;
        return 1;
    }

    // create thread for testing
    std::thread thread([eventfd]() {
        uint64_t value = 1;
        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(2));
            if (::write(eventfd, &value, sizeof(value)) == -1) {
                std::cerr << "Failed to write to eventfd" << std::endl;
            }
            value += 1;
        }
    });

    Scheduler scheduler{};

    auto io_task     = new IoTask(eventfd);
    io_task->on_read = [&test](int fd) {
        uint64_t value;
        if (::read(fd, &value, sizeof(value)) == -1) {
            std::cerr << "Failed to read from eventfd" << std::endl;
        }
        test += value;
        std::cout << "test: " << test << std::endl;
    };
    scheduler.schedule(io_task);

    scheduler.schedule(new PeriodicCallbackTask(
        std::chrono::seconds(1), [&test](std::chrono::steady_clock::duration difference) {
            test += 1;
            std::cout << "test: " << test << ", difference: "
                      << std::chrono::duration_cast<std::chrono::microseconds>(difference).count()
                      << "us" << std::endl;
        }));

    scheduler.execute_forever();
#endif
    return 0;
}
