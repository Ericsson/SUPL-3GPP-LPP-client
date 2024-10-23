#include <args.hpp>
#include <loglet/loglet.hpp>
#include <lpp/assistance_data.hpp>
#include <lpp/client.hpp>
#include <lpp/session.hpp>
#include <scheduler/scheduler.hpp>
#include <sys/eventfd.h>
#include <thread>
#include <unistd.h>

#include "client.hpp"

#define LOGLET_CURRENT_MODULE "client"

lpp::PeriodicSessionHandle gAssistanceDataSession{};
Config                     gConfig;

static void client_connected(lpp::Client& client) {
    gAssistanceDataSession = client.request_assistance_data({
        gConfig.assistance_data_type,
        gConfig.cell,
        [](lpp::Client& client, lpp::Message message) {
            INFOF("received message (non-periodic)");
            process_assistance_data(gConfig, std::move(message));
        },
        [](lpp::Client& client, lpp::PeriodicSessionHandle session, lpp::Message message) {
            INFOF("received message (periodic)");
            process_assistance_data(gConfig, std::move(message));
        },
        [](lpp::Client& client, lpp::PeriodicSessionHandle session) {
            INFOF("start of assistance data");
        },
        [](lpp::Client& client, lpp::PeriodicSessionHandle session) {
            INFOF("end of assistance data");
        },
    });
}

static void client_initialize(lpp::Client& client) {
    client.on_connected = [](lpp::Client& client) {
        INFOF("connected to LPP server");
        client_connected(client);
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

int main(int argc, char** argv) {
    loglet::set_level(loglet::Level::Verbose);
    loglet::disable_module("sched");
    loglet::disable_module("supl");
    loglet::disable_module("lpp/s");
    // loglet::disable_module("lpp/c");
    // loglet::disable_module("lpp/ad");
    // loglet::disable_module("lpp/ps");

    gConfig.cell          = supl::Cell::lte(240, 1, 1, 3);
    gConfig.output_format = OutputFormat::RTCM;

    lpp::Client client{
        supl::Identity::msisdn(919825098250),
        "129.192.83.118",
        5431,
    };

    client_initialize(client);

    scheduler::Scheduler scheduler{};
    client.schedule(&scheduler);

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

    scheduler.execute();
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
