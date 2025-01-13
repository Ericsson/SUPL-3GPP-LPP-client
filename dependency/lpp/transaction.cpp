#include "lpp/transaction.hpp"
#include "lpp.hpp"
#include "lpp/abort.hpp"
#include "lpp/session.hpp"

#include <loglet/loglet.hpp>

#define LOGLET_CURRENT_MODULE "lpp/t"

namespace lpp {

void TransactionHandle::send(Message& message) {
    VSCOPE_FUNCTION();
    if (mSession != nullptr) {
        mSession->send(*this, message);
    }
}

void TransactionHandle::send_with_end(Message& message) {
    VSCOPE_FUNCTION();
    if (mSession != nullptr) {
        mSession->send_with_end(*this, message);
    }
}

void TransactionHandle::abort() {
    VSCOPE_FUNCTION();
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
