#include "lpp/client.hpp"
#include "lpp.hpp"
#include "lpp/assistance_data.hpp"
#include "lpp/provide_capabilities.hpp"
#include "periodic_session/assistance_data.hpp"

#include <loglet/loglet.hpp>

#include <ProvideAssistanceData-r9-IEs.h>

#define LOGLET_CURRENT_MODULE "lpp/c"

namespace lpp {

Client::Client(supl::Identity identity, const std::string& host, uint16_t port)
    : mHost(host), mPort(port), mSession{lpp::VERSION_16_4_0, identity}, mScheduler{nullptr} {
    SCOPE_FUNCTION();

    mNextSessionId        = 1;
    mSession.on_connected = [this](Session& session) {
        DEBUGF("connected");
    };

    mSession.on_disconnected = [this](Session& session) {
        DEBUGF("disconnected");

        if (on_disconnected) {
            on_disconnected(*this);
        }
    };

    mSession.on_established = [this](lpp::Session& session) {
        DEBUGF("established");

        if (on_connected) {
            on_connected(*this);
        }
    };

    mSession.on_begin_transaction = [this](lpp::Session&,
                                           const lpp::TransactionHandle& transaction) {
        this->process_begin_transaction(transaction);
    };

    mSession.on_end_transaction = [this](lpp::Session&, const lpp::TransactionHandle& transaction) {
        this->process_end_transaction(transaction);
    };

    mSession.on_message = [this](lpp::Session& session, const lpp::TransactionHandle& transaction,
                                 lpp::Message message) {
        this->process_message(transaction, std::move(message));
    };
}

PeriodicSessionHandle
Client::request_assistance_data(const RequestAssistanceData& request_assistance_data) {
    SCOPE_FUNCTION();

    PeriodicSessionHandle handle{};
    if (!allocate_periodic_session_handle(handle)) {
        ERRORF("failed to create periodic session handle");
        return PeriodicSessionHandle::invalid();
    }

    auto periodic_session =
        std::make_shared<AssistanceDataHandler>(this, &mSession, handle, request_assistance_data);
    if (!periodic_session->request_assistance_data()) {
        ERRORF("failed to request assistance data");
        return PeriodicSessionHandle::invalid();
    }

    mSessions[handle] = std::move(periodic_session);
    return handle;
}

bool Client::update_assistance_data(const PeriodicSessionHandle& session, supl::Cell cell) {
    SCOPE_FUNCTION();

    auto it = mSessions.find(session);
    if (it == mSessions.end()) {
        WARNF("periodic session not found");
        return false;
    }

    auto periodic_session = dynamic_cast<AssistanceDataHandler*>(it->second.get());
    if (!periodic_session) {
        WARNF("periodic session is null");
        return false;
    }

    return periodic_session->update_assistance_data(cell);
}

void Client::cancel_assistance_data(const PeriodicSessionHandle& session) {
    SCOPE_FUNCTION();
    ERRORF("not implemented");
}

bool Client::is_periodic_session_valid(const PeriodicSessionHandle& session) const {
    if (!session.is_valid()) return false;
    auto it = mSessions.find(session);
    if (it == mSessions.end()) return false;
    return it->second->is_valid();
}

void Client::schedule(Scheduler* scheduler) {
    LPP_ASSERT(scheduler, "scheduler is null");
    LPP_ASSERT(!mScheduler, "scheduler is already set");

    mScheduler = scheduler;
    mSession.connect(mHost, mPort);
    mSession.schedule(scheduler);
}

bool Client::allocate_periodic_session_handle(PeriodicSessionHandle& result) {
    for (long i = 0; i < 256; i++) {
        auto id     = (mNextSessionId + i) % 256;
        auto handle = PeriodicSessionHandle{id, Initiator::TargetDevice};
        if (mSessions.find(handle) == mSessions.end()) {
            mNextSessionId = id;
            DEBUGF("allocated periodic session handle %s", handle.to_string().c_str());
            result = handle;
            return true;
        }
    }

    return false;
}

bool Client::deallocate_periodic_session_handle(const PeriodicSessionHandle& handle) {
    if (!handle.is_valid()) return false;
    mSessions.erase(handle);
    DEBUGF("deallocated periodic session handle %s", handle.to_string().c_str());
    return true;
}

void Client::process_message(const lpp::TransactionHandle& transaction, lpp::Message message) {
    SCOPE_FUNCTIONF("%s", transaction.to_string().c_str());

    if (lpp::is_request_capabilities(message)) {
        process_request_capabilities(transaction, std::move(message));
    } else if (lpp::is_request_assistance_data(message)) {
        process_request_assistance_data(transaction, std::move(message));
    } else if (lpp::is_request_location_information(message)) {
        process_request_location_information(transaction, std::move(message));
    } else if (lpp::is_provide_capabilities(message)) {
        process_provide_capabilities(transaction, std::move(message));
    } else if (lpp::is_provide_assistance_data(message)) {
        process_provide_assistance_data(transaction, std::move(message));
    } else if (lpp::is_provide_location_information(message)) {
        process_provide_location_information(transaction, std::move(message));
    } else if (lpp::is_abort(message)) {
        process_abort(transaction, std::move(message));
    } else if (lpp::is_error(message)) {
        process_error(transaction, std::move(message));
    } else {
        ERRORF("unknown message type");
    }
}

void Client::process_request_capabilities(const lpp::TransactionHandle& transaction,
                                          lpp::Message                  message) {
    SCOPE_FUNCTIONF("%s", transaction.to_string().c_str());

    auto capabilities_has_been_handled = false;
    if (on_capabilities) {
        capabilities_has_been_handled = on_capabilities(*this, transaction, message);
    }

    if (!capabilities_has_been_handled) {
        auto message = create_provide_capabilities();
        mSession.send_with_end(transaction, message);
    }
}

void Client::process_request_assistance_data(const lpp::TransactionHandle& transaction,
                                             lpp::Message                  message) {
    SCOPE_FUNCTIONF("%s", transaction.to_string().c_str());
    ERRORF("lpp::Client does not support RequestAssistanceData from the location server");
    // TODO(ewasjon): What should we respond with here?
}

void Client::process_request_location_information(const lpp::TransactionHandle& transaction,
                                                  lpp::Message                  message) {
    SCOPE_FUNCTIONF("%s", transaction.to_string().c_str());
}

void Client::process_provide_capabilities(const lpp::TransactionHandle& transaction,
                                          lpp::Message                  message) {
    SCOPE_FUNCTIONF("%s", transaction.to_string().c_str());
}

PeriodicSession* Client::find_by_periodic_session_handle(const PeriodicSessionHandle& handle) {
    auto it = mSessions.find(handle);
    if (it != mSessions.end()) {
        return it->second.get();
    } else {
        return nullptr;
    }
}

PeriodicSession* Client::find_by_request_transaction_handle(const TransactionHandle& transaction) {
    auto it = mRequestTransactions.find(transaction);
    if (it != mRequestTransactions.end()) {
        return find_by_periodic_session_handle(it->second);
    } else {
        return nullptr;
    }
}

PeriodicSession* Client::find_by_periodic_transaction_handle(const TransactionHandle& transaction) {
    auto it = mPeriodicTransactions.find(transaction);
    if (it != mPeriodicTransactions.end()) {
        return find_by_periodic_session_handle(it->second);
    } else {
        return nullptr;
    }
}

void Client::process_provide_assistance_data(const lpp::TransactionHandle& transaction,
                                             lpp::Message                  message) {
    SCOPE_FUNCTIONF("%s", transaction.to_string().c_str());

    auto inner = lpp::get_provide_assistance_data(message);
    if (!inner) return;

    auto periodic_session = find_by_request_transaction_handle(transaction);
    if (periodic_session) {
        periodic_session->message(transaction, std::move(message));
        return;
    }

    PeriodicSessionHandle handle{};
    if (!lpp::get_periodic_session(*inner, &handle)) {
        WARNF("provide assistance data message does not contain a periodic session id");
        return;
    }

    periodic_session = find_by_periodic_session_handle(handle);
    if (!periodic_session) {
        WARNF("provide assistance data with a unknown periodic session");
        return;
    }

    periodic_session->message(transaction, std::move(message));
}

void Client::process_provide_location_information(const lpp::TransactionHandle& transaction,
                                                  lpp::Message                  message) {
    SCOPE_FUNCTIONF("%s", transaction.to_string().c_str());
}

void Client::process_abort(const TransactionHandle& transaction, Message message) {
    SCOPE_FUNCTIONF("%s", transaction.to_string().c_str());
}

void Client::process_error(const TransactionHandle& transaction, Message message) {
    SCOPE_FUNCTIONF("%s", transaction.to_string().c_str());
}

void Client::process_begin_transaction(const lpp::TransactionHandle& transaction) {
    SCOPE_FUNCTIONF("%s", transaction.to_string().c_str());
}

void Client::process_end_transaction(const lpp::TransactionHandle& transaction) {
    SCOPE_FUNCTIONF("%s", transaction.to_string().c_str());

    auto periodic_session = find_by_request_transaction_handle(transaction);
    if (periodic_session) {
        periodic_session->end(transaction);
    }

    periodic_session = find_by_periodic_transaction_handle(transaction);
    if (periodic_session) {
        periodic_session->end(transaction);
    }
}

}  // namespace lpp