#include "single_session.hpp"
#include "lpp.hpp"
#include "lpp/abort.hpp"
#include "lpp/client.hpp"

#include <algorithm>
#include <loglet/loglet.hpp>
#include <scheduler/scheduler.hpp>

LOGLET_MODULE2(lpp, ss);
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF2(lpp, ss)

namespace lpp {

SingleSession::SingleSession(Client* client, Session* session, TransactionHandle handle)
    : mClient(client), mSession(session), mPeriodicTask{std::chrono::seconds(5)} {
    mHackNeverSendAbort    = false;
    mTransaction           = std::move(handle);
    mLastRequestTime       = std::chrono::steady_clock::now();
    mPeriodicTask.callback = [this]() {
        check_active_requests();
    };
    if (mClient && mClient->mScheduler) {
        mPeriodicTask.schedule(*mClient->mScheduler);
    }
}

SingleSession::~SingleSession() {
    destroy();
}

void SingleSession::message(TransactionHandle const& transaction, Message message) {
    VSCOPE_FUNCTIONF("%s", transaction.to_string().c_str());

    // Check if the transaction is a request transaction
    if (transaction != mTransaction) {
        WARNF("unknown transaction: %s", transaction.to_string().c_str());
        return;
    }

    request_response(transaction, std::move(message));
}

void SingleSession::check_active_requests() {
    VSCOPE_FUNCTION();

    std::vector<TransactionHandle> to_remove;

    auto now = std::chrono::steady_clock::now();
    if (now - mLastRequestTime > std::chrono::seconds(5)) {
        DEBUGF("no response received for more than 5 seconds");
        stale_request(mTransaction);

        if (mHackNeverSendAbort) {
            WARNF("skipping abort for %s", mTransaction.to_string().c_str());
        } else {
            auto abort = lpp::create_abort();
            mTransaction.send_with_end(abort);
        }
        
        try_destroy();
    }
}

void SingleSession::destroy() {
    VSCOPE_FUNCTION();

    // Remove request
    if (mTransaction.is_valid()) {
        if (mClient) {
            mClient->remove_single_session(mTransaction);
        }

        mTransaction = TransactionHandle{};
    }

    mPeriodicTask.cancel();
}

void SingleSession::try_destroy() {
    VSCOPE_FUNCTION();

    if (mClient && mTransaction.is_valid()) {
        mClient->single_session_want_to_be_destroyed(mTransaction);
        mTransaction = TransactionHandle{};
    }
}

//
//
//

SingleRequestAssistanceSession::SingleRequestAssistanceSession(Client* client, Session* session, TransactionHandle handle,
                                                             SingleRequestAssistanceData data)
    : SingleSession(client, session, handle), mData(std::move(data)) {
    VSCOPE_FUNCTION();
}

SingleRequestAssistanceSession::~SingleRequestAssistanceSession() {
    VSCOPE_FUNCTION();
}

void SingleRequestAssistanceSession::request_response(TransactionHandle const& transaction, Message message) {
    VSCOPE_FUNCTIONF("%s", transaction.to_string().c_str());
    if (mData.on_message) {
        mData.on_message(*mClient, std::move(message));
    }
}

void SingleRequestAssistanceSession::stale_request(TransactionHandle const& transaction) {
    VSCOPE_FUNCTIONF("%s", transaction.to_string().c_str());
    if (mData.on_error) {
        mData.on_error(*mClient);
    }
}

}  // namespace lpp
