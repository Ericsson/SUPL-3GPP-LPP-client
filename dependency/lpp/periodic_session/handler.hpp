#pragma once
#include <lpp/periodic_session.hpp>
#include <lpp/transaction.hpp>
#include <scheduler/periodic.hpp>

#include <chrono>
#include <unordered_map>
#include <vector>

namespace lpp {

class Client;
class Session;
class PeriodicSession {
public:
    EXPLICIT PeriodicSession(Client* client, Session* session, PeriodicSessionHandle handle);
    virtual ~PeriodicSession();

    NODISCARD PeriodicSessionHandle handle() const { return mHandle; }

    NODISCARD virtual bool is_valid() const { return mHandle.is_valid(); }

    void message(TransactionHandle const& transaction, Message message);
    void end(TransactionHandle const& transaction);

    void try_destroy();

protected:
    // Response to a request transaction
    virtual void request_response(TransactionHandle const& transaction, Message message) = 0;
    // Periodic session transaction started
    virtual void periodic_begin(TransactionHandle const& transaction);
    // Periodic session transaction ended
    virtual void periodic_ended(TransactionHandle const& transaction);
    // Message received for the periodic session
    virtual void periodic_message(TransactionHandle const& transaction, Message message) = 0;
    // An out-going request hasn't been answered for 5 seconds
    virtual void stale_request(TransactionHandle const& transaction) = 0;

    bool send_new_request(Message message);
    void check_active_requests();
    void destroy();

    void handle_request_response(TransactionHandle const& transaction, Message message);
    void handle_unsolicited_message(TransactionHandle const& transaction, Message message);
    void handle_periodic_begin(TransactionHandle const& transaction, Message message);
    void handle_periodic_ended(TransactionHandle const& transaction);

private:
    void register_request(TransactionHandle const& transaction);
    void unregister_request(TransactionHandle const& transaction);

protected:
    Client*                        mClient;
    Session*                       mSession;
    PeriodicSessionHandle          mHandle;
    std::vector<TransactionHandle> mPeriodicTransactions;

    std::unordered_map<TransactionHandle, std::chrono::steady_clock::time_point>
                            mRequestTransactions;
    scheduler::PeriodicTask mPeriodicTask;
};

}  // namespace lpp
