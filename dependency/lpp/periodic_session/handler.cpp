#include "handler.hpp"
#include "../lpp.hpp"
#include "lpp/abort.hpp"
#include "lpp/client.hpp"

#include <algorithm>
#include <loglet/loglet.hpp>
#include <scheduler/scheduler.hpp>

#define LOGLET_CURRENT_MODULE "lpp/ps"

namespace lpp {

std::string PeriodicSessionHandle::to_string() const {
    return (mInitiator == Initiator::TargetDevice ? "T" : "S") + std::to_string(mId);
}

PeriodicSession::PeriodicSession(Client* client, Session* session, PeriodicSessionHandle handle)
    : mClient(client), mSession(session), mHandle(handle), mPeriodicTask{std::chrono::seconds(5)} {
    mPeriodicTask.callback = [this]() {
        check_active_requests();
    };
    if (mClient && mClient->mScheduler) {
        mPeriodicTask.schedule(*mClient->mScheduler);
    }
}

PeriodicSession::~PeriodicSession() {
    destroy();
}

void PeriodicSession::handle_request_response(TransactionHandle const& transaction,
                                              Message                  message) {
    VSCOPE_FUNCTIONF("%s", transaction.to_string().c_str());
    request_response(transaction, std::move(message));
}

void PeriodicSession::handle_periodic_begin(TransactionHandle const& transaction, Message message) {
    VSCOPE_FUNCTIONF("%s", transaction.to_string().c_str());

    mPeriodicTransactions.push_back(transaction);
    if (mClient) {
        mClient->mPeriodicTransactions[transaction] = mHandle;
    }

    periodic_begin(transaction);
    periodic_message(transaction, std::move(message));
}

void PeriodicSession::handle_periodic_ended(TransactionHandle const& transaction) {
    VSCOPE_FUNCTIONF("%s", transaction.to_string().c_str());

    auto it = std::find(mPeriodicTransactions.begin(), mPeriodicTransactions.end(), transaction);
    ASSERT(it != mPeriodicTransactions.end(), "transaction mismatch");
    mPeriodicTransactions.erase(it);

    if (mClient) {
        mClient->mPeriodicTransactions.erase(transaction);
    }

    periodic_ended(transaction);
}

void PeriodicSession::message(TransactionHandle const& transaction, Message message) {
    VSCOPE_FUNCTIONF("%s", transaction.to_string().c_str());

    // Check if the transaction is a request transaction
    auto rit = mRequestTransactions.find(transaction);
    if (rit != mRequestTransactions.end()) {
        handle_request_response(transaction, std::move(message));
        return;
    }

    // Check if the transaction is the periodic transaction
    auto pit = std::find(mPeriodicTransactions.begin(), mPeriodicTransactions.end(), transaction);
    if (pit != mPeriodicTransactions.end()) {
        periodic_message(transaction, std::move(message));
        return;
    }

    // Otherwise, this is the new transaction for the periodic session
    handle_periodic_begin(transaction, std::move(message));
}

void PeriodicSession::end(TransactionHandle const& transaction) {
    VSCOPE_FUNCTIONF("%s", transaction.to_string().c_str());

    auto rit = mRequestTransactions.find(transaction);
    if (rit != mRequestTransactions.end()) {
        unregister_request(transaction);
        return;
    }

    auto pit = std::find(mPeriodicTransactions.begin(), mPeriodicTransactions.end(), transaction);
    if (pit != mPeriodicTransactions.end()) {
        handle_periodic_ended(transaction);
        return;
    }

    WARNF("unknown transaction ending: %s", transaction.to_string().c_str());
}

void PeriodicSession::periodic_begin(TransactionHandle const&) {}

void PeriodicSession::periodic_ended(TransactionHandle const&) {}

void PeriodicSession::stale_request(TransactionHandle const&) {}

bool PeriodicSession::send_new_request(Message message) {
    VSCOPE_FUNCTION();
    if (!mSession) return false;

    auto transaction = mSession->create_transaction();
    if (!transaction.is_valid()) {
        ERRORF("failed to create transaction");
        return false;
    }

    transaction.send(message);
    register_request(transaction);
    return true;
}

void PeriodicSession::register_request(TransactionHandle const& transaction) {
    VSCOPE_FUNCTIONF("%s", transaction.to_string().c_str());
    mRequestTransactions[transaction] = std::chrono::steady_clock::now();
    if (mClient) {
        mClient->mRequestTransactions[transaction] = mHandle;
    }
}

void PeriodicSession::unregister_request(TransactionHandle const& transaction) {
    VSCOPE_FUNCTIONF("%s", transaction.to_string().c_str());
    auto it = mRequestTransactions.find(transaction);
    if (it != mRequestTransactions.end()) {
        mRequestTransactions.erase(it);
    }

    if (mClient) {
        mClient->mRequestTransactions.erase(transaction);
    }
}

void PeriodicSession::check_active_requests() {
    VSCOPE_FUNCTION();

    std::vector<TransactionHandle> to_remove;

    auto now = std::chrono::steady_clock::now();
    for (auto it = mRequestTransactions.begin(); it != mRequestTransactions.end();) {
        auto transaction = it->first;
        auto started     = it->second;
        if (now - started > std::chrono::seconds(5)) {
            to_remove.push_back(transaction);
            stale_request(transaction);
            it = mRequestTransactions.erase(it);
        } else {
            ++it;
        }
    }

    for (auto& transaction : to_remove) {
        auto abort = lpp::create_abort();
        transaction.send_with_end(abort);
        unregister_request(transaction);
    }
}

void PeriodicSession::destroy() {
    VSCOPE_FUNCTION();

    // Remove request
    std::vector<TransactionHandle> to_remove;
    for (auto it = mRequestTransactions.begin(); it != mRequestTransactions.end(); ++it) {
        auto transaction = it->first;
        to_remove.push_back(transaction);
    }

    for (auto& transaction : to_remove) {
        auto abort = lpp::create_abort();
        transaction.send_with_end(abort);
        unregister_request(transaction);
    }

    to_remove.clear();
    mRequestTransactions.clear();

    // Remove periodic
    for (auto& transaction : mPeriodicTransactions) {
        to_remove.push_back(transaction);
    }

    for (auto& transaction : to_remove) {
        auto abort = lpp::create_abort();
        transaction.send_with_end(abort);
        handle_periodic_ended(transaction);
    }

    if (mClient) {
        mClient->deallocate_periodic_session_handle(mHandle);
    }

    mPeriodicTask.cancel();
}

void PeriodicSession::try_destroy() {
    VSCOPE_FUNCTION();

    if (mClient) {
        mClient->want_to_be_destroyed(mHandle);
    }
}

}  // namespace lpp
