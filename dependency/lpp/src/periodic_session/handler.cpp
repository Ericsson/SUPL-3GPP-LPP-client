#include "handler.hpp"
#include "../lpp.hpp"
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
    : mClient(client), mSession(session), mHandle(handle),
      mPeriodicTask(PeriodicCallbackTask{
          std::chrono::seconds(5),
          [this](std::chrono::steady_clock::duration) {
              check_active_requests();
          },
      }) {
    if (mClient) {
        mClient->mScheduler->schedule(&mPeriodicTask);
    }
}

PeriodicSession::~PeriodicSession() {
    if (mClient) {
        mClient->deallocate_periodic_session_handle(mHandle);
        mClient->mPeriodicTransactions.erase(mPeriodicTransaction);
        for (auto& rt : mRequestTransactions) {
            mClient->mRequestTransactions.erase(rt.first);
        }

        mClient->mScheduler->cancel(&mPeriodicTask);
    }
}

void PeriodicSession::message(const TransactionHandle& transaction, Message message) {
    SCOPE_FUNCTIONF("%s", transaction.to_string().c_str());

    // Check if the transaction is a request transaction
    auto it = mRequestTransactions.find(transaction);
    if (it != mRequestTransactions.end()) {
        request_response(transaction, std::move(message));
        return;
    }

    // Check if the transaction is the periodic transaction
    if (mPeriodicTransaction.is_valid() && mPeriodicTransaction == transaction) {
        periodic_message(transaction, std::move(message));
        return;
    }

    // Otherwise, this is the new transaction for the periodic session
    if (!mPeriodicTransaction.is_valid()) {
        mPeriodicTransaction = transaction;
        if (mClient) {
            mClient->mPeriodicTransactions[transaction] = mHandle;
        }

        periodic_begin(transaction);
        periodic_message(transaction, std::move(message));
        return;
    }

    WARNF("unknown transaction received");
}

void PeriodicSession::end(const TransactionHandle& transaction) {
    SCOPE_FUNCTIONF("%s", transaction.to_string().c_str());

    auto it = mRequestTransactions.find(transaction);
    if (it != mRequestTransactions.end()) {
        mRequestTransactions.erase(it);
        return;
    }

    if (mPeriodicTransaction == transaction) {
        periodic_ended(transaction);

        LPP_ASSERT(mPeriodicTransaction == transaction, "transaction mismatch");
        if (mClient) {
            mClient->mPeriodicTransactions.erase(transaction);
        }

        mPeriodicTransaction = TransactionHandle{};
        LPP_UNREACHABLE();
    }
}

void PeriodicSession::periodic_begin(const TransactionHandle& transaction) {}

void PeriodicSession::periodic_ended(const TransactionHandle& transaction) {}

bool PeriodicSession::send_new_request(Message message) {
    SCOPE_FUNCTION();
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

void PeriodicSession::register_request(const TransactionHandle& transaction) {
    SCOPE_FUNCTION();
    mRequestTransactions[transaction] = std::chrono::steady_clock::now();
    if (mClient) {
        mClient->mRequestTransactions[transaction] = mHandle;
    }
}

void PeriodicSession::unregister_request(const TransactionHandle& transaction) {
    SCOPE_FUNCTION();
    auto it = mRequestTransactions.find(transaction);
    if (it != mRequestTransactions.end()) {
        mRequestTransactions.erase(it);
    }

    if (mClient) {
        mClient->mRequestTransactions.erase(transaction);
    }
}

void PeriodicSession::check_active_requests() {
    SCOPE_FUNCTION();

    std::vector<TransactionHandle> to_remove;

    auto now = std::chrono::steady_clock::now();
    for (auto it = mRequestTransactions.begin(); it != mRequestTransactions.end();) {
        auto transaction = it->first;
        auto started     = it->second;
        if (now - started > std::chrono::seconds(5)) {
            to_remove.push_back(transaction);
            it = mRequestTransactions.erase(it);
        } else {
            ++it;
        }
    }

    for (auto& transaction : to_remove) {
        mSession->delete_transaction(transaction);
        unregister_request(transaction);
    }
}

}  // namespace lpp
