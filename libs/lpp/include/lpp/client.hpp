#pragma once
#include <lpp/message.hpp>
#include <lpp/periodic_session.hpp>
#include <lpp/session.hpp>
#include <lpp/transaction.hpp>
#include <lpp/types.hpp>
#include <supl/cell.hpp>

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>

namespace lpp {

struct RequestAssistanceData;
class PeriodicSession;
class Client {
public:
    explicit Client(supl::Identity identity, const std::string& host, uint16_t port);

    // Called once the client is connected and ready to send and receive messages
    std::function<void(Client&)> on_connected;
    // Called when the client is disconnected from the server, may be called without a prior call to
    // on_connected
    std::function<void(Client&)> on_disconnected;

    // A request capabilities message has been received from the server. By default, the client will
    // respond with a "expected" capabilities message, to override this behavior, return true to
    // indicate the you have handled the message yourself.
    std::function<bool(Client&, const TransactionHandle&, const Message&)> on_capabilities;

    // Request assistance data from the server
    PeriodicSessionHandle
    request_assistance_data(const RequestAssistanceData& request_assistance_data);
    // Update assistance data with a new cell for the given periodic session
    bool update_assistance_data(const PeriodicSessionHandle& session, supl::Cell cell);
    // Cancel assistance data for the given periodic session
    void cancel_assistance_data(const PeriodicSessionHandle& session);

    bool is_periodic_session_valid(const PeriodicSessionHandle& session) const;

    // Schedule the client to run on the given scheduler. Without calling this method, the client
    // will never connect to the server.
    void schedule(Scheduler* scheduler);

protected:
    using Pah = std::shared_ptr<PeriodicSession>;

    void process_message(const TransactionHandle& transaction, Message message);
    void process_request_capabilities(const TransactionHandle& transaction, Message message);
    void process_request_assistance_data(const TransactionHandle& transaction, Message message);
    void process_request_location_information(const TransactionHandle& transaction,
                                              Message                  message);
    void process_provide_capabilities(const TransactionHandle& transaction, Message message);
    void process_provide_assistance_data(const TransactionHandle& transaction, Message message);
    void process_provide_location_information(const TransactionHandle& transaction,
                                              Message                  message);

    void process_abort(const TransactionHandle& transaction, Message message);
    void process_error(const TransactionHandle& transaction, Message message);

    void process_begin_transaction(const TransactionHandle& transaction);
    void process_end_transaction(const TransactionHandle& transaction);

    bool allocate_periodic_session_handle(PeriodicSessionHandle& handle);
    bool deallocate_periodic_session_handle(const PeriodicSessionHandle& handle);

    PeriodicSession* find_by_periodic_session_handle(const PeriodicSessionHandle& session);
    PeriodicSession* find_by_request_transaction_handle(const TransactionHandle& transaction);
    PeriodicSession* find_by_periodic_transaction_handle(const TransactionHandle& transaction);

private:
    std::string mHost;
    uint16_t    mPort;
    Session     mSession;
    Scheduler*  mScheduler;

    std::unordered_map<TransactionHandle, PeriodicSessionHandle> mRequestTransactions;
    std::unordered_map<TransactionHandle, PeriodicSessionHandle> mPeriodicTransactions;
    std::unordered_map<PeriodicSessionHandle, Pah>               mSessions;
    long                                                         mNextSessionId;

    friend class PeriodicSession;
};

}  // namespace lpp
