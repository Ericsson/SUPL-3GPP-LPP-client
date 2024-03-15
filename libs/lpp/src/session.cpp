#include "lpp/session.hpp"
#include "lpp.hpp"

#include <scheduler/scheduler.hpp>
#include <scheduler/task.hpp>
#include <supl/end.hpp>
#include <supl/pos.hpp>
#include <supl/posinit.hpp>
#include <supl/response.hpp>
#include <supl/session.hpp>
#include <supl/start.hpp>

#include "LPP-Message.h"

namespace lpp {

Session::Session(Version version, supl::Identity identity)
    : mState(State::UNKNOWN), mVersion(version), mIdentity(identity), mSession(nullptr),
      mNextClientTransactionId(1), mTask(nullptr) {
    SCOPE_FUNCTION();
}

Session::~Session() {
    SCOPE_FUNCTION();
    if (mSession) {
        delete mSession;
    }
    if (mTask) {
        delete mTask;
    }
}

bool Session::connect(const std::string& host, uint16_t port) {
    SCOPE_FUNCTIONF("%s, %d", host.c_str(), port);
    if (mSession) {
        DEBUGF("session already connected");
        return false;
    } else if (mState != State::UNKNOWN && mState != State::DISCONNECTED) {
        DEBUGF("session already connected");
        return false;
    }

    mSession = new supl::Session(supl::VERSION_2_0, mIdentity);
    if (!mSession->connect(host, port)) {
        WARNF("failed to connect to %s:%d", host.c_str(), port);
        switch_state(State::DISCONNECTED);
        return false;
    }

    if (on_connected) {
        on_connected(*this);
    }

    switch_state(State::CONNECTED);
    process();
    return true;
}

static const char* state_to_string(State state) {
    switch (state) {
    case State::UNKNOWN: return "UNKNOWN";
    case State::CONNECTED: return "CONNECTED";
    case State::DISCONNECTED: return "DISCONNECTED";
    case State::SUPL_HANDSHAKE: return "SUPL_HANDSHAKE";
    case State::SUPL_POSINIT: return "SUPL_POSINIT";
    case State::ESTABLISHED: return "ESTABLISHED";
    case State::MESSAGE: return "MESSAGE";
    default: return "INVALID";
    }
}

void Session::switch_state(State state) {
    SCOPE_FUNCTIONF("%s -> %s", state_to_string(mState), state_to_string(state));
    mState = state;
}

void Session::process() {
    auto do_next = true;
    while (do_next) {
        SCOPE_FUNCTIONF("%s", state_to_string(mState));
        do_next = false;

        NextState next_state{};
        switch (mState) {
        case State::CONNECTED: next_state = state_connected(); break;
        case State::SUPL_HANDSHAKE: next_state = state_handshake(); break;
        case State::SUPL_POSINIT: next_state = state_posinit(); break;
        case State::ESTABLISHED: next_state = state_established(); break;
        case State::MESSAGE: next_state = state_message(); break;
        default: ERRORF("invalid state: %s", state_to_string(mState)); SUPL_UNREACHABLE();
        }

        switch_state(next_state.state);
        do_next = next_state.do_next;
    }
}

NextState Session::state_connected() {
    SCOPE_FUNCTION();
    return NextState{State::SUPL_HANDSHAKE, true};
}

NextState Session::state_handshake() {
    SCOPE_FUNCTION();
    LPP_ASSERT(mSession != nullptr, "session is null");

    auto cell         = supl::Cell::lte(240, 1, 1, 0);
    auto capabilities = supl::SETCapabilities{
        .posTechnology = {},
        .prefMethod    = supl::PrefMethod::noPreference,
        .posProtocol   = {.lpp = {.enabled               = true,
                                  .majorVersionField     = mVersion.major,
                                  .technicalVersionField = mVersion.technical,
                                  .editorialVersionField = mVersion.editorial}}};

    auto start = supl::START{
        .sETCapabilities = capabilities,
        .applicationID   = {.name     = "SUPL Example Client",
                            .provider = "Application-Provider",
                            .version  = "1.0.0.0"},
        .locationID      = {cell},
    };

    if (!mSession->handshake(start)) {
        ERRORF("failed to establish SUPL handshake");
        SUPL_UNREACHABLE();
        return NextState{State::DISCONNECTED};
    }

    return NextState{State::SUPL_POSINIT, true};
}

NextState Session::state_posinit() {
    SCOPE_FUNCTION();
    LPP_ASSERT(mSession != nullptr, "session is null");

    auto cell         = supl::Cell::lte(240, 1, 1, 0);
    auto capabilities = supl::SETCapabilities{
        .posTechnology = {},
        .prefMethod    = supl::PrefMethod::noPreference,
        .posProtocol   = {.lpp = {.enabled               = true,
                                  .majorVersionField     = mVersion.major,
                                  .technicalVersionField = mVersion.technical,
                                  .editorialVersionField = mVersion.editorial}}};

    auto posinit = supl::POSINIT{
        .sETCapabilities = capabilities,
        .locationID      = {cell},
    };
    if (!mSession->send(posinit)) {
        ERRORF("failed to send SUPL POSINIT");
        SUPL_UNREACHABLE();
        return NextState{State::DISCONNECTED};
    }

    return NextState{State::ESTABLISHED, true};
}

NextState Session::state_established() {
    SCOPE_FUNCTION();

    if (on_established) {
        on_established(*this);
    }

    return NextState{State::MESSAGE};
}

NextState Session::state_message() {
    SCOPE_FUNCTION();
    LPP_ASSERT(mSession != nullptr, "session is null");

    mSession->fill_receive_buffer();

    for (;;) {
        supl::END end{};
        supl::POS pos{};
        auto      received = mSession->try_receive(nullptr, &end, &pos);
        if (received == supl::Session::Received::NO_DATA) {
            return NextState{State::MESSAGE, false};
        } else if (received == supl::Session::Received::END) {
            return NextState{State::DISCONNECTED};
        } else if (received == supl::Session::Received::POS) {
            process_supl_pos(pos);
            continue;
        } else {
            WARNF("problem receiving SUPL message");
            continue;
        }
    }
}

void Session::schedule(Scheduler* scheduler) {
    SCOPE_FUNCTION();

    if (mTask) {
        DEBUGF("task already scheduled");
        return;
    }

    LPP_ASSERT(scheduler != nullptr, "scheduler is null");
    LPP_ASSERT(mSession != nullptr, "session is null");

    auto fd = mSession->fd();
    LPP_ASSERT(fd != -1, "invalid file descriptor");

    mTask          = new IoTask(fd);
    mTask->on_read = [this](int fd) {
        this->process();
    };

    DEBUGF("task added to scheduler");
    scheduler->schedule(mTask);
}

std::unique_ptr<Transaction> Session::create_transaction() {
    SCOPE_FUNCTION();
    LPP_ASSERT(mSession != nullptr, "session is null");

    auto transaction_id = find_next_client_transaction_id();
    if (transaction_id < 0) {
        ERRORF("failed to find next client transaction id");
        return nullptr;
    }

    auto transaction_handle = TransactionHandle{this, transaction_id, true};
    if (!add_transaction(transaction_handle)) {
        ERRORF("failed to add transaction");
        return nullptr;
    }

    return Transaction::create(transaction_handle);
}

void Session::destroy_transaction(const Transaction& transaction) {
    SCOPE_FUNCTION();
    LPP_ASSERT(mSession != nullptr, "session is null");

    remove_transaction(transaction.handle());
}

bool Session::add_transaction(const TransactionHandle& transaction) {
    SCOPE_FUNCTION();
    LPP_ASSERT(mSession != nullptr, "session is null");

    auto  transaction_id = transaction.id();
    auto& transactions   = transaction.is_client() ? mClientTransactions : mServerTransactions;

    if (transaction_id < 0 || transaction_id > 255) {
        ERRORF("invalid transaction id");
        return false;
    } else if (transactions.find(transaction_id) != transactions.end()) {
        ERRORF("transaction already exists");
        return false;
    }

    DEBUGF("new transaction %s%ld", transaction.is_client() ? "C" : "S", transaction_id);

    if (on_begin_transaction) {
        on_begin_transaction(*this, transaction);
    }

    mClientTransactions[transaction_id] = TransactionData{transaction};
    return true;
}

bool Session::remove_transaction(const TransactionHandle& transaction) {
    SCOPE_FUNCTION();
    LPP_ASSERT(mSession != nullptr, "session is null");

    auto  transaction_id = transaction.id();
    auto& transactions   = transaction.is_client() ? mClientTransactions : mServerTransactions;

    if (transactions.find(transaction_id) == transactions.end()) {
        ERRORF("transaction does not exist");
        return false;
    }

    DEBUGF("del transaction %s%ld", transaction.is_client() ? "C" : "S", transaction_id);

    if (on_end_transaction) {
        on_end_transaction(*this, transaction);
    }

    transactions.erase(transaction_id);
    return true;
}

long Session::find_next_client_transaction_id() {
    SCOPE_FUNCTION();

    for (long i = 0; i < 255; i++) {
        auto id = (mNextClientTransactionId + i) % 256;
        if (mClientTransactions.find(id) == mClientTransactions.end()) {
            mNextClientTransactionId = (id + 1) % 256;
            return i;
        }
    }

    return -1;
}

void Session::process_supl_pos(const supl::POS& pos) {
    SCOPE_FUNCTION();

    for (const auto& payload : pos.payloads) {
        switch (payload.type) {
        case supl::Payload::Type::LPP:
            DEBUGF("lpp payload: %d bytes", payload.data.size());
            process_lpp_payload(payload);
            break;
        default: WARNF("unsupport payload type"); break;
        }
    }
}

void Session::process_lpp_payload(const supl::Payload& payload) {
    SCOPE_FUNCTION();
    LPP_ASSERT(mSession != nullptr, "session is null");
    LPP_ASSERT(payload.type == supl::Payload::Type::LPP, "invalid payload type");

    auto message = decode_lpp_message(payload.data.data(), payload.data.size());
    if (!message) {
        WARNF("failed to decode LPP message");
        return;
    }

    INFOF("received LPP message");
}

Message Session::decode_lpp_message(const uint8_t* data, size_t size) {
    SCOPE_FUNCTION();
    LPP_ASSERT(mSession != nullptr, "session is null");

    // NOTE: Increase default max stack size to handle large messages.
    // TODO(ewasjon): Is this correct?
    asn_codec_ctx_t stack_ctx{};
    stack_ctx.max_stack_size = 1024 * 1024 * 4;

    auto message = (LPP_Message*)nullptr;
    auto result =
        uper_decode_complete(&stack_ctx, &asn_DEF_LPP_Message, (void**)&message, data, size);
    if (result.code == RC_FAIL) {
        WARNF("failed to decode uper: %zd bytes consumed", result.consumed);
        ASN_STRUCT_FREE(asn_DEF_LPP_Message, message);
        return nullptr;
    } else if (result.code == RC_WMORE) {
        WARNF("failed to decode uper: need more data");
        ASN_STRUCT_FREE(asn_DEF_LPP_Message, message);
        return nullptr;
    } else {
        return Message{message};
    }
}

}  // namespace lpp
