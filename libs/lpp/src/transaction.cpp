#include "lpp/transaction.hpp"
#include "lpp.hpp"

namespace lpp {

void TransactionHandle::send() {}
void TransactionHandle::end() {}

std::unique_ptr<Transaction> Transaction::create(TransactionHandle handle) {
    return std::unique_ptr<Transaction>(new Transaction(handle));
}

Transaction::Transaction(TransactionHandle handle) : mHandle(handle) {}

Transaction::~Transaction() {
    if (mHandle.is_valid()) {
        mHandle.end();
    }
}

void Transaction::send() {
    if (mHandle.is_valid()) {
        mHandle.send();
    }
}

}  // namespace lpp