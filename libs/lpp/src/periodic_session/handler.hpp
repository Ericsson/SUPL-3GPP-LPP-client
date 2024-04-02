#pragma once
#include <lpp/periodic_session.hpp>
#include <lpp/transaction.hpp>
#include <lpp/types.hpp>
#include <scheduler/periodic_task.hpp>

#include <unordered_map>
#include <chrono>

namespace lpp {

class Client;
class Session;
class PeriodicSession {
public:
    LPP_EXPLICIT PeriodicSession(Client* client, Session* session, PeriodicSessionHandle handle);
    virtual ~PeriodicSession();

    LPP_NODISCARD PeriodicSessionHandle handle() const { return mHandle; }

    LPP_NODISCARD virtual bool is_valid() const { return mHandle.is_valid(); }

    void message(const TransactionHandle& transaction, Message message);
    void end(const TransactionHandle& transaction);

protected:
    // Response to a request transaction
    virtual void request_response(const TransactionHandle& transaction, Message message) = 0;
    // Periodic session transaction started
    virtual void periodic_begin(const TransactionHandle& transaction) = 0;
    // Periodic session transaction ended
    virtual void periodic_ended(const TransactionHandle& transaction) = 0;
    // Message received for the periodic session
    virtual void periodic_message(const TransactionHandle& transaction, Message message) = 0;

    bool send_new_request(Message message);
    void check_active_requests();

private:
    void register_request(const TransactionHandle& transaction);
    void unregister_request(const TransactionHandle& transaction);

protected:
    Client*               mClient;
    Session*              mSession;
    PeriodicSessionHandle mHandle;
    TransactionHandle     mPeriodicTransaction;

    std::unordered_map<TransactionHandle, std::chrono::steady_clock::time_point>
                         mRequestTransactions;
    PeriodicCallbackTask mPeriodicTask;
};

}  // namespace lpp
