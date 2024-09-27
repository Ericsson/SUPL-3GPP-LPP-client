#pragma once
#include <lpp/assistance_data.hpp>
#include <lpp/types.hpp>
#include <supl/cell.hpp>
#include <scheduler/periodic_task.hpp>
#include "handler.hpp"

namespace lpp {

class AssistanceDataHandler : public PeriodicSession {
public:
    LPP_EXPLICIT AssistanceDataHandler(Client* client, Session* session,
                                       PeriodicSessionHandle handle, RequestAssistanceData data);
    virtual ~AssistanceDataHandler() override;

    virtual void request_response(const TransactionHandle& transaction, Message message) override;
    virtual void periodic_begin(const TransactionHandle& transaction) override;
    virtual void periodic_ended(const TransactionHandle& transaction) override;
    virtual void periodic_message(const TransactionHandle& transaction, Message message) override;

    bool request_assistance_data();
    bool update_assistance_data(supl::Cell cell);

private:
    RequestAssistanceData mData;
};

}  // namespace lpp
