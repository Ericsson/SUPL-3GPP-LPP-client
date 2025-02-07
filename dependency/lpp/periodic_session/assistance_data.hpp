#pragma once
#include "handler.hpp"

#include <lpp/assistance_data.hpp>
#include <supl/cell.hpp>

namespace lpp {

class AssistanceDataHandler : public PeriodicSession {
public:
    EXPLICIT AssistanceDataHandler(Client* client, Session* session, PeriodicSessionHandle handle,
                                   RequestAssistanceData data);
    virtual ~AssistanceDataHandler() override;

    virtual void request_response(TransactionHandle const& transaction, Message message) override;
    virtual void periodic_begin(TransactionHandle const& transaction) override;
    virtual void periodic_ended(TransactionHandle const& transaction) override;
    virtual void periodic_message(TransactionHandle const& transaction, Message message) override;
    virtual void stale_request(TransactionHandle const& transaction) override;

    bool request_assistance_data();
    bool update_assistance_data(supl::Cell cell);

private:
    RequestAssistanceData mData;
};

}  // namespace lpp
