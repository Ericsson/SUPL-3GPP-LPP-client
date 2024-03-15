#pragma once
#include <lpp/message.hpp>
#include <lpp/transaction.hpp>
#include <lpp/types.hpp>
#include <lpp/version.hpp>
#include <supl/identity.hpp>

#include <functional>
#include <unordered_map>

class Scheduler;
class IoTask;

namespace supl {
class Session;
struct POS;
struct Payload;
}  // namespace supl

namespace lpp {

enum class State {
    UNKNOWN,
    CONNECTED,
    DISCONNECTED,
    SUPL_HANDSHAKE,
    SUPL_POSINIT,
    ESTABLISHED,
    MESSAGE,
};

struct NextState {
    State state;
    bool  do_next;
};

struct TransactionData {
    TransactionHandle handle;
};

class Session {
public:
    explicit Session(Version version, supl::Identity identity);
    ~Session();

    bool connect(const std::string& host, uint16_t port);

    std::unique_ptr<Transaction> create_transaction();
    void                         destroy_transaction(const Transaction& transaction);

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
    // Called when a transaction ends (from the server or client)
    std::function<void(Session&, const TransactionHandle&)> on_end_transaction;

protected:
    void switch_state(State state);
    void process();

    NextState state_connected();
    NextState state_handshake();
    NextState state_posinit();
    NextState state_established();
    NextState state_message();

    long find_next_client_transaction_id();
    bool add_transaction(const TransactionHandle& handle);
    bool remove_transaction(const TransactionHandle& handle);

    void    process_supl_pos(const supl::POS& pos);
    void    process_lpp_payload(const supl::Payload& payload);
    Message decode_lpp_message(const uint8_t* data, size_t size);

private:
    State          mState;
    Version        mVersion;
    supl::Identity mIdentity;
    supl::Session* mSession;

    std::unordered_map<long, TransactionData> mClientTransactions;
    std::unordered_map<long, TransactionData> mServerTransactions;

    long mNextClientTransactionId;

    IoTask* mTask;
};

}  // namespace lpp
