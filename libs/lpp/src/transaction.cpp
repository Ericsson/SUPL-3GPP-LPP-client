#include "lpp/transaction.hpp"
#include "lpp.hpp"
#include "lpp/abort.hpp"
#include "lpp/session.hpp"

#include <loglet/loglet.hpp>

#define LOGLET_CURRENT_MODULE "lpp/t"
#define SCOPE_TRANSACTION()                                                                        \
    char _loglet_buffer[64];                                                                       \
    if (mHandle.is_valid()) {                                                                      \
        snprintf(_loglet_buffer, sizeof(_loglet_buffer), "%s%ld", mHandle.is_client() ? "C" : "S", \
                 mHandle.id());                                                                    \
    } else {                                                                                       \
        snprintf(_loglet_buffer, sizeof(_loglet_buffer), "invalid");                               \
    }                                                                                              \
    SCOPE_FUNCTIONF("%s", _loglet_buffer)

namespace lpp {

void TransactionHandle::send(Message& message) {
    if (mSession != nullptr) {
        mSession->send(*this, message);
    }
}

void TransactionHandle::send_with_end(Message& message) {
    if (mSession != nullptr) {
        mSession->send_with_end(*this, message);
    }
}

void TransactionHandle::abort() {
    if (mSession != nullptr) {
        mSession->abort(*this);
    }
}

std::string TransactionHandle::to_string() const {
    char buffer[64];
    snprintf(buffer, sizeof(buffer), "%s%ld#%ld", mInitiator == Initiator::TargetDevice ? "T" : "S",
             mTransactionId, mGenerationId);
    return std::string{buffer};
}

}  // namespace lpp
