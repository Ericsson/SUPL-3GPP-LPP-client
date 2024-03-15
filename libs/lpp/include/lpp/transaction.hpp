#pragma once
#include <lpp/types.hpp>

#include <memory>

namespace lpp {

class Session;

class TransactionHandle {
public:
    TransactionHandle() : mSession(nullptr), mId(0) {}
    TransactionHandle(Session* session, long id, bool clientInitiated)
        : mSession(session), mId(id), mClientInitiated(clientInitiated) {}

    bool is_valid() const { return mSession != nullptr; }

    void send();
    void end();

    long id() const { return mId; }
    bool is_client() const { return mClientInitiated; }

private:
    Session* mSession;
    long     mId;
    bool     mClientInitiated;
};

class Transaction {
public:
    ~Transaction();

    void send();

    const TransactionHandle& handle() const { return mHandle; }

    static std::unique_ptr<Transaction> create(TransactionHandle handle);

private:
    explicit Transaction(TransactionHandle handle);

    Transaction(const Transaction& other)            = delete;
    Transaction& operator=(const Transaction& other) = delete;
    Transaction(Transaction&& other)                 = delete;
    Transaction& operator=(Transaction&& other)      = delete;

    TransactionHandle mHandle;
};

}  // namespace lpp
