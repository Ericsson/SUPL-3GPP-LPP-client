#pragma once
#include <lpp/initiator.hpp>
#include <lpp/message.hpp>

#include <memory>

namespace lpp {

class Session;

struct TransactionLookup {
    long      id;
    Initiator initiator;

    NODISCARD bool operator==(TransactionLookup const& other) const {
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

    TransactionHandle(TransactionHandle const& other)
        : mSession(other.mSession), mTransactionId(other.mTransactionId),
          mGenerationId(other.mGenerationId), mInitiator(other.mInitiator) {}

    TransactionHandle& operator=(TransactionHandle const& other) {
        if (this == &other) return *this;
        mSession       = other.mSession;
        mTransactionId = other.mTransactionId;
        mGenerationId  = other.mGenerationId;
        mInitiator     = other.mInitiator;
        return *this;
    }

    TransactionHandle(TransactionHandle&& other) NOEXCEPT {
        std::swap(mSession, other.mSession);
        std::swap(mTransactionId, other.mTransactionId);
        std::swap(mGenerationId, other.mGenerationId);
        std::swap(mInitiator, other.mInitiator);
    }

    TransactionHandle& operator=(TransactionHandle&& other) NOEXCEPT {
        std::swap(mSession, other.mSession);
        std::swap(mTransactionId, other.mTransactionId);
        std::swap(mGenerationId, other.mGenerationId);
        std::swap(mInitiator, other.mInitiator);
        return *this;
    }

    NODISCARD bool is_valid() const { return mSession != nullptr; }

    void send(Message& message);
    void send_with_end(Message& message);
    void abort();

    NODISCARD long      id() const { return mTransactionId; }
    NODISCARD long      generation_id() const { return mGenerationId; }
    NODISCARD Initiator initiator() const { return mInitiator; }

    NODISCARD bool operator==(TransactionHandle const& other) const {
        return mSession == other.mSession && mTransactionId == other.mTransactionId &&
               mGenerationId == other.mGenerationId && mInitiator == other.mInitiator;
    }

    NODISCARD bool operator!=(TransactionHandle const& other) const { return !(*this == other); }

    NODISCARD std::string       to_string() const;
    NODISCARD TransactionLookup lookup() const { return TransactionLookup{id(), initiator()}; }

    NODISCARD static TransactionHandle invalid() { return {}; }

private:
    Session* mSession;
    long     mTransactionId;
    long mGenerationId;  // Due to the fact that 'mTransactionId' are limit and may loop, this is
                         // added protection that a old handle cannot access new transaction that
                         // are created with the same id.
    Initiator mInitiator;
};

}  // namespace lpp

namespace std {
template <>
struct hash<lpp::TransactionHandle> {
    std::size_t operator()(lpp::TransactionHandle const& k) const {
        std::size_t result = 17;
        result             = result * 31 + hash<long>()(k.id());
        result             = result * 31 + hash<long>()(k.generation_id());
        result             = result * 31 + hash<lpp::Initiator>()(k.initiator());
        return result;
    }
};

template <>
struct hash<lpp::TransactionLookup> {
    std::size_t operator()(lpp::TransactionLookup const& k) const {
        std::size_t result = 17;
        result             = result * 31 + hash<long>()(k.id);
        result             = result * 31 + hash<lpp::Initiator>()(k.initiator);
        return result;
    }
};
}  // namespace std
