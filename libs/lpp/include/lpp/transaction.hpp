#pragma once
#include <lpp/initiator.hpp>
#include <lpp/message.hpp>
#include <lpp/types.hpp>

#include <memory>

namespace lpp {

class Session;

struct TransactionLookup {
    long      id;
    Initiator initiator;

    LPP_NODISCARD bool operator==(const TransactionLookup& other) const {
        return id == other.id && initiator == other.initiator;
    }
};

class TransactionHandle {
public:
    TransactionHandle() : mSession(nullptr), mTransactionId(0), mGenerationId(0), mInitiator() {}
    TransactionHandle(Session* session, long transaction_id, long generation_id,
                      Initiator initiator)
        : mSession(session), mTransactionId(transaction_id), mGenerationId(generation_id),
          mInitiator(initiator) {}

    TransactionHandle(const TransactionHandle& other)
        : mSession(other.mSession), mTransactionId(other.mTransactionId),
          mGenerationId(other.mGenerationId), mInitiator(other.mInitiator) {}

    TransactionHandle& operator=(const TransactionHandle& other) {
        mSession       = other.mSession;
        mTransactionId = other.mTransactionId;
        mGenerationId  = other.mGenerationId;
        mInitiator     = other.mInitiator;
        return *this;
    }

    TransactionHandle(TransactionHandle&& other) {
        std::swap(mSession, other.mSession);
        std::swap(mTransactionId, other.mTransactionId);
        std::swap(mGenerationId, other.mGenerationId);
        std::swap(mInitiator, other.mInitiator);
    }

    TransactionHandle& operator=(TransactionHandle&& other) {
        std::swap(mSession, other.mSession);
        std::swap(mTransactionId, other.mTransactionId);
        std::swap(mGenerationId, other.mGenerationId);
        std::swap(mInitiator, other.mInitiator);
        return *this;
    }

    LPP_NODISCARD bool is_valid() const { return mSession != nullptr; }

    void send(Message& message);
    void send_with_end(Message& message);
    void abort();

    LPP_NODISCARD long      id() const { return mTransactionId; }
    LPP_NODISCARD long      generation_id() const { return mGenerationId; }
    LPP_NODISCARD Initiator initiator() const { return mInitiator; }

    LPP_NODISCARD bool operator==(const TransactionHandle& other) const {
        return mSession == other.mSession && mTransactionId == other.mTransactionId &&
               mGenerationId == other.mGenerationId && mInitiator == other.mInitiator;
    }

    LPP_NODISCARD std::string       to_string() const;
    LPP_NODISCARD TransactionLookup lookup() const { return TransactionLookup{id(), initiator()}; }

    LPP_NODISCARD static TransactionHandle invalid() { return {}; }

private:
    Session* mSession;
    long     mTransactionId;
    long mGenerationId;  // Due to the fact that 'mTransactionId' are limit and may loop, this is
                         // added protection that a old handle cannot access new transaction that
                         // are created with the same id.
    Initiator mInitiator;
};

}  // namespace lpp

template <>
struct std::hash<lpp::TransactionHandle> {
    std::size_t operator()(const lpp::TransactionHandle& k) const {
        std::size_t result = 17;
        result             = result * 31 + hash<long>()(k.id());
        result             = result * 31 + hash<long>()(k.generation_id());
        result             = result * 31 + hash<lpp::Initiator>()(k.initiator());
        return result;
    }
};

template <>
struct std::hash<lpp::TransactionLookup> {
    std::size_t operator()(const lpp::TransactionLookup& k) const {
        std::size_t result = 17;
        result             = result * 31 + hash<long>()(k.id);
        result             = result * 31 + hash<lpp::Initiator>()(k.initiator);
        return result;
    }
};
