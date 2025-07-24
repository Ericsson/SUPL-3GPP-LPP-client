#pragma once
#include <lpp/periodic_session.hpp>
#include <lpp/transaction.hpp>
#include <scheduler/periodic.hpp>
#include <lpp/assistance_data.hpp>

#include <chrono>
#include <unordered_map>
#include <vector>

namespace lpp {

class Client;
class Session;
class SingleSession {
public:
    EXPLICIT SingleSession(Client* client, Session* session, TransactionHandle handle);
    virtual ~SingleSession();

    void schedule(scheduler::Scheduler& scheduler);
    void cancel();

    void message(TransactionHandle const& transaction, Message message);

    void try_destroy();

    void set_hack_never_send_abort(bool value) { mHackNeverSendAbort = value; }

    NODISCARD TransactionHandle const& transaction() const { return mTransaction; }

protected:
    // Response to a request transaction
    virtual void request_response(TransactionHandle const& transaction, Message message) = 0;
    // An out-going request hasn't been answered for 5 seconds
    virtual void stale_request(TransactionHandle const& transaction) = 0;

    bool send_new_request(Message message);
    void check_active_requests();
    void destroy();

    void handle_request_response(TransactionHandle const& transaction, Message message);

private:
    void register_request(TransactionHandle const& transaction);
    void unregister_request(TransactionHandle const& transaction);

protected:
    Client*                               mClient;
    Session*                              mSession;
    TransactionHandle                     mTransaction;
    std::chrono::steady_clock::time_point mLastRequestTime;

    // HACK:
    bool mHackNeverSendAbort;

    scheduler::PeriodicTask mPeriodicTask;
};

class SingleRequestAssistanceSession : public SingleSession {
public:
    SingleRequestAssistanceSession(Client* client, Session* session, TransactionHandle handle,
                                   SingleRequestAssistanceData data);
    ~SingleRequestAssistanceSession() override;

    void request_response(TransactionHandle const& transaction, Message message) override;
    void stale_request(TransactionHandle const& transaction) override;

private:
    SingleRequestAssistanceData mData;
};

}  // namespace lpp
