#pragma once
#include <lpp/message.hpp>
#include <lpp/transaction.hpp>
#include <lpp/types.hpp>
#include <lpp/version.hpp>
#include <scheduler/task.hpp>
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
    CONNECTION_FAILED,
    DISCONNECTED,
    SUPL_HANDSHAKE_SEND,
    SUPL_HANDSHAKE_RECV,
    SUPL_POSINIT,
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

class SessionTask : public Task {
public:
    SessionTask()
        : mFd(-1), mSession(nullptr), mReadEnabled(false), mWriteEnabled(false),
          mErrorEnabled(false), mRegistered(false) {}
    SessionTask(Session* session, int fd);
    virtual ~SessionTask() = default;

    virtual void register_task(Scheduler* scheduler) override;
    virtual void unregister_task(Scheduler* scheduler) override;

    void event(struct epoll_event* event);
    void update(int fd, bool read, bool write, bool error);

protected:
    int        mFd;
    EpollEvent mEvent;
    Session*   mSession;
    bool       mReadEnabled;
    bool       mWriteEnabled;
    bool       mErrorEnabled;
    bool       mRegistered;
};

class Session {
public:
    explicit Session(Version version, supl::Identity identity);
    ~Session();

    // Setup the connection information for the session and switch to the CONNECT state
    void connect(const std::string& host, uint16_t port);

    // Create a new transaction. 'single_side_endable' determines if the transaction is can be ended
    // when either sides sends a endTransaction, otherwise you need to end the transaction yourself
    // in `on_server_end_transaction`.
    TransactionHandle create_transaction(bool single_side_endable = true);
    void              delete_transaction(const TransactionHandle& transaction);

    void send(const TransactionHandle& handle, Message& message);

    // Send message with endTransaction. Only sends the message if the transaction is alive by both
    // parties.
    void send_with_end(const TransactionHandle& handle, Message& message);

    // Send an abort message with endTransaction. Only sends the message if the transaction is alive
    // by both parties.
    void abort(const TransactionHandle& handle);

    void schedule(Scheduler* scheduler);

    // Called when the session is connected to the server
    std::function<void(Session&)> on_connected;
    // Called when the session is disconnected from the server
    std::function<void(Session&)> on_disconnected;
    // Called when the session is established, i.e. the LPP/SUPL handshake is complete and the
    // session is ready to send and receive messages
    std::function<void(Session&)> on_established;
    // Called when a transaction begins (from the server or client)
    std::function<void(Session&, const TransactionHandle&)> on_begin_transaction;
    // Callback for transaction that what to handle server endTransaction themself.
    std::function<void(Session&, const TransactionHandle&)> on_server_end_transaction;
    // Called when a transaction ends (from the server or client)
    std::function<void(Session&, const TransactionHandle&)> on_end_transaction;
    // Called when a message was received for a transaction
    std::function<void(Session&, const TransactionHandle&, Message)> on_message;

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

    TransactionData* find_transaction(const LPP_TransactionID& transaction_id);
    TransactionData* find_transaction(const TransactionHandle& handle);
    bool             add_transaction(const TransactionHandle& handle, bool single_side_endable);
    bool             remove_transaction(const TransactionHandle& handle);

    void server_end_transaction(const TransactionHandle& handle);
    void client_end_transaction(const TransactionHandle& handle);

    void                 process_supl_pos(const supl::POS& pos);
    void                 process_lpp_payload(const supl::Payload& payload);
    Message              decode_lpp_message(const uint8_t* data, size_t size);
    std::vector<uint8_t> encode_lpp_message(const Message& message);

private:
    State          mState;
    Version        mVersion;
    supl::Identity mIdentity;
    supl::Session* mSession;

    std::string mConnectionHost;
    uint16_t    mConnectionPort;

    std::unordered_map<TransactionLookup, TransactionHandle> mTransacionLookup;
    std::unordered_map<TransactionHandle, TransactionData>   mTransactions;
    std::unordered_set<long>                                 mClientTransactions;

    long mTransactionId;
    long mGenerationId;
    long mSequenceNumber;

    Scheduler*  mScheduler;
    SessionTask mTask;
    State       mNextReadState;
    State       mNextWriteState;
    State       mNextErrorState;

    friend SessionTask;
};

}  // namespace lpp
