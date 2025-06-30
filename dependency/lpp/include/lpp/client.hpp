#pragma once
#include <lpp/message.hpp>
#include <lpp/periodic_session.hpp>
#include <lpp/session.hpp>
#include <lpp/transaction.hpp>
#include <supl/cell.hpp>

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>

namespace lpp {

namespace messages {
struct ProvideLocationInformation;
}

struct RequestAssistanceData;
struct ProvideCapabilities;
class PeriodicSession;
class LocationInformationDelivery;
struct PeriodicLocationInformationDeliveryDescription;
class Client {
public:
    explicit Client(supl::Identity identity, supl::Cell supl_cell, std::string const& host,
                    uint16_t port);
    ~Client();

    // Called once the client is connected and ready to send and receive messages
    std::function<void(Client&)> on_connected;
    // Called when the client is disconnected from the server, may be called without a prior call to
    // on_connected
    std::function<void(Client&)> on_disconnected;

    // A request capabilities message has been received from the server. By default, the client will
    // respond with a "expected" capabilities message, to override this behavior, return true to
    // indicate the you have handled the message yourself.
    std::function<bool(Client&, TransactionHandle const&, Message const&)> on_capabilities;

    // A provide capabilities message has been send from the client. This callback is useful for a
    // quick way to know when it's safe to start sending request assistance data messages.
    std::function<void(Client&)> on_provide_capabilities;

    // Called when the client receives a request location information from the server. If this
    // callback is set and you return true, the client will assume that you have handled the message
    // otherwise it will setup the periodic location information delivery for you. To reject request
    // location information, simply return true and do nothing.
    std::function<bool(Client&, TransactionHandle const&, Message const&)>
        on_request_location_information;
    // Called every time to get the latest location information to send to the server.
    std::function<bool(Client&, LocationInformationDelivery const&,
                       messages::ProvideLocationInformation&)>
        on_provide_location_information;
    // More advanced callback for providing location information to the server. If you return true,
    // you're responsible for sending the provide location information message yourself. This will
    // disable the default behavior of the client - which is to call
    // 'on_provide_location_information' and send the message automatically.
    std::function<bool(Client&, LocationInformationDelivery const&)>
        on_provide_location_information_advanced;

    // Request assistance data from the server
    PeriodicSessionHandle
    request_assistance_data(RequestAssistanceData const& request_assistance_data);
    // Update assistance data with a new cell for the given periodic session
    bool update_assistance_data(PeriodicSessionHandle const& session, supl::Cell cell);
    // Cancel assistance data for the given periodic session
    void cancel_assistance_data(PeriodicSessionHandle const& session);

    bool is_periodic_session_valid(PeriodicSessionHandle const& session) const;

    bool start_periodic_location_information(
        TransactionHandle const&                              transaction,
        PeriodicLocationInformationDeliveryDescription const& description);
    TransactionHandle start_periodic_location_information(
        PeriodicLocationInformationDeliveryDescription const& description);
    void stop_periodic_location_information(TransactionHandle const& transaction);

    // Schedule the client to run on the given scheduler. Without calling this method, the client
    // will never connect to the server.
    void schedule(scheduler::Scheduler* scheduler);

    // Cancel the client, this will disconnect the client from the server and stop all periodic
    // sessions
    void cancel();

    void set_interface(std::string const& interface) { mInterface = interface; }

    void set_capabilities(ProvideCapabilities const& capabilities);

    void set_hack_bad_transaction_initiator(bool value) { mHackBadTransactionInitiator = value; }
    void set_hack_never_send_abort(bool value) { mHackNeverSendAbort = value; }

protected:
    using Pah = std::shared_ptr<PeriodicSession>;
    using Lid = std::shared_ptr<LocationInformationDelivery>;

    void process_message(TransactionHandle const& transaction, Message message);
    void process_request_capabilities(TransactionHandle const& transaction, Message message);
    void process_request_assistance_data(TransactionHandle const& transaction, Message message);
    void process_request_location_information(TransactionHandle const& transaction,
                                              Message                  message);
    void process_provide_capabilities(TransactionHandle const& transaction, Message message);
    void process_provide_assistance_data(TransactionHandle const& transaction, Message message);
    void process_provide_location_information(TransactionHandle const& transaction,
                                              Message                  message);

    void process_abort(TransactionHandle const& transaction, Message message);
    void process_error(TransactionHandle const& transaction, Message message);

    void process_begin_transaction(TransactionHandle const& transaction);
    void process_end_transaction(TransactionHandle const& transaction);

    bool allocate_periodic_session_handle(PeriodicSessionHandle& handle);
    bool deallocate_periodic_session_handle(PeriodicSessionHandle const& handle);
    void want_to_be_destroyed(PeriodicSessionHandle const& handle);

    PeriodicSession* find_by_periodic_session_handle(PeriodicSessionHandle const& session);
    PeriodicSession* find_by_request_transaction_handle(TransactionHandle const& transaction);
    PeriodicSession* find_by_periodic_transaction_handle(TransactionHandle const& transaction);

private:
    std::string                          mHost;
    uint16_t                             mPort;
    std::string                          mInterface;
    Session                              mSession;
    scheduler::Scheduler*                mScheduler;
    std::unique_ptr<ProvideCapabilities> mCapabilities;

    // HACK:
    bool mHackBadTransactionInitiator;
    bool mHackNeverSendAbort;

    std::unordered_map<TransactionHandle, PeriodicSessionHandle> mRequestTransactions;
    std::unordered_map<TransactionHandle, PeriodicSessionHandle> mPeriodicTransactions;
    std::unordered_map<PeriodicSessionHandle, Pah>               mSessions;
    std::unordered_map<TransactionHandle, Lid>                   mLocationInformationDeliveries;
    long                                                         mNextSessionId;
    std::vector<PeriodicSessionHandle>                           mSessionsToDestroy;

    friend class PeriodicSession;
    friend class LocationInformationDelivery;
};

}  // namespace lpp
