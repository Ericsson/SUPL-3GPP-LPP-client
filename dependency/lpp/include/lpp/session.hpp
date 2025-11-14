#pragma once
#include <lpp/message.hpp>
#include <lpp/transaction.hpp>
#include <lpp/version.hpp>
#include <scheduler/scheduler.hpp>
#include <supl/cell.hpp>
#include <supl/identity.hpp>

#include <functional>
#include <unordered_map>
#include <unordered_set>
#include <vector>

class Scheduler;
class IoTask;

namespace supl {
class Session;
struct POS;
struct Payload;
}  // namespace supl

struct LPP_TransactionID;

namespace lpp {

enum class State {
    UNKNOWN,
    CONNECT,
    CONNECTING,
    CONNECTED,
    ConnectionFailed,
    DISCONNECTED,
    SuplHandshakeSend,
    SuplHandshakeRecv,
    SuplPosinit,
    ESTABLISHED,
    MESSAGE,
    ERROR,
    EXIT,
};

struct NextState {
    State read_state;
    State write_state;
    State error_state;
    State next_state;
    bool  schedule;

    static NextState make() {
        return {State::UNKNOWN, State::UNKNOWN, State::UNKNOWN, State::UNKNOWN, false};
    }

    NextState& read(State state) {
        read_state = state;
        schedule   = true;
        return *this;
    }

    NextState& write(State state) {
        write_state = state;
        schedule    = true;
        return *this;
    }

    NextState& error(State state) {
        error_state = state;
        schedule    = true;
        return *this;
    }

    NextState& next(State state) {
        next_state = state;
        return *this;
    }
};

struct TransactionData {
    TransactionHandle handle;
    bool              client_has_sent_end;
    bool              server_has_sent_end;
    bool              client_should_send_end;
    bool              single_side_endable;
};

class SessionTask {
public:
    SessionTask()
        : mFd(-1), mSession(nullptr), mScheduler(nullptr), mReadEnabled(false),
          mWriteEnabled(false), mErrorEnabled(false) {}
    SessionTask(Session* session, int fd);
    ~SessionTask() = default;

    bool schedule(scheduler::Scheduler& scheduler);
    bool cancel();

    void event(struct epoll_event* event);
    void update(int fd, bool read, bool write, bool error);

protected:
    int                   mFd;
    scheduler::EpollEvent mEvent;
    Session*              mSession;
    scheduler::Scheduler* mScheduler;
    bool                  mReadEnabled;
    bool                  mWriteEnabled;
    bool                  mErrorEnabled;
};

class Session {
public:
    explicit Session(Version version, supl::Identity identity, supl::Cell cell);
    ~Session();

    // Setup the connection information for the session and switch to the CONNECT state
    void connect(std::string const& host, uint16_t port, std::string const& interface);

    // Create a new transaction. 'single_side_endable' determines if the transaction is can be ended
    // when either sides sends a endTransaction, otherwise you need to end the transaction yourself
    // in `on_server_end_transaction`.
    TransactionHandle create_transaction(bool single_side_endable = true);
    void              delete_transaction(TransactionHandle const& transaction);

    void send(TransactionHandle const& handle, Message& message);

    // Send message with endTransaction. Only sends the message if the transaction is alive by both
    // parties.
    void send_with_end(TransactionHandle const& handle, Message& message);

    // Send an abort message with endTransaction. Only sends the message if the transaction is alive
    // by both parties.
    void abort(TransactionHandle const& handle);

    void schedule(scheduler::Scheduler* scheduler);
    void cancel();

    // Called when the session is connected to the server
    std::function<void(Session&)> on_connected;
    // Called when the session is disconnected from the server
    std::function<void(Session&)> on_disconnected;
    // Called when the session is established, i.e. the LPP/SUPL handshake is complete and the
    // session is ready to send and receive messages
    std::function<void(Session&)> on_established;
    // Called when a transaction begins (from the server or client)
    std::function<void(Session&, TransactionHandle const&)> on_begin_transaction;
    // Callback for transaction that what to handle server endTransaction themself.
    std::function<void(Session&, TransactionHandle const&)> on_server_end_transaction;
    // Called when a transaction ends (from the server or client)
    std::function<void(Session&, TransactionHandle const&)> on_end_transaction;
    // Called when a message was received for a transaction
    std::function<void(Session&, TransactionHandle const&, Message)> on_message;

    static Message              decode_lpp_message(uint8_t const* data, size_t size);
    static std::vector<uint8_t> encode_lpp_message(Message const& message);
    static std::string          encode_lpp_message_xer(Message const& message);

protected:
    void switch_state(State state);
    void process();

    void process_read();
    void process_write();
    void process_error();

    NextState state_unknown();
    NextState state_connect();
    NextState state_connecting();
    NextState state_connected();
    NextState state_connection_failed();
    NextState state_disconnected();
    NextState state_error();
    NextState state_handshake_send();
    NextState state_handshake_recv();
    NextState state_posinit();
    NextState state_established();
    NextState state_message();

    TransactionHandle allocate_transaction();

    TransactionData* find_transaction(LPP_TransactionID const& transaction_id);
    TransactionData* find_transaction(TransactionHandle const& handle);
    bool             add_transaction(TransactionHandle const& handle, bool single_side_endable);
    bool             remove_transaction(TransactionHandle const& handle);

    void server_end_transaction(TransactionHandle const& handle);
    void client_end_transaction(TransactionHandle const& handle);

    void process_supl_pos(supl::POS const& pos);
    void process_lpp_payload(supl::Payload const& payload);

private:
    State          mState;
    Version        mVersion;
    supl::Identity mIdentity;
    supl::Cell     mInitialCell;
    supl::Session* mSession;

    std::string mConnectionHost;
    uint16_t    mConnectionPort;
    std::string mConnectionInterface;

    std::unordered_map<TransactionLookup, TransactionHandle> mTransacionLookup;
    std::unordered_map<TransactionHandle, TransactionData>   mTransactions;
    std::unordered_set<long>                                 mClientTransactions;

    std::vector<supl::POS> mPosQueue;

    long mTransactionId;
    long mGenerationId;
    long mSequenceNumber;

    scheduler::Scheduler* mScheduler;
    SessionTask           mTask;
    State                 mNextReadState;
    State                 mNextWriteState;
    State                 mNextErrorState;

    friend SessionTask;
};

}  // namespace lpp
