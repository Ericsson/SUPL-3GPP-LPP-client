#include <args.hpp>

#include <loglet/loglet.hpp>
#include <lpp/session.hpp>
#include <scheduler/periodic_task.hpp>
#include <scheduler/scheduler.hpp>
#include <scheduler/task.hpp>
#include <sys/eventfd.h>
#include <thread>
#include <unistd.h>

#define LOGLET_CURRENT_MODULE "client"

std::unique_ptr<lpp::Transaction> transaction{};

int main(int argc, char** argv) {
    Scheduler scheduler{};

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
        transaction->send();
    };

    session.on_begin_transaction = [](lpp::Session&                 session,
                                      const lpp::TransactionHandle& transaction) {
        INFOF("begin transaction");
    };

    session.on_end_transaction = [](lpp::Session&                 session,
                                    const lpp::TransactionHandle& transaction) {
        INFOF("end transaction");
    };

    if (!session.connect("129.192.83.118", 5431)) {
        ERRORF("failed to connect to LPP server");
        return 1;
    }

    session.schedule(&scheduler);

    scheduler.execute_forever();

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
