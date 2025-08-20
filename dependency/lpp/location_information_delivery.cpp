#include "location_information_delivery.hpp"
#include "lpp.hpp"
#include "lpp/client.hpp"
#include "messages/provide_location_information.hpp"

#include <algorithm>
#include <loglet/loglet.hpp>
#include <scheduler/scheduler.hpp>

LOGLET_MODULE2(lpp, lid);
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF2(lpp, lid)

namespace lpp {

LocationInformationDelivery::LocationInformationDelivery(
    Client* client, Session* session, TransactionHandle transaction,
    PeriodicLocationInformationDeliveryDescription description)
    : mClient(client), mSession(session), mTransaction(std::move(transaction)),
      mCoordinateType{description.coordinate_type}, mVelocityType{description.velocity_type},
      mReportingInterval{description.reporting_interval},
      mReportingAmount{description.reporting_amount},
      mReportingAmountUnlimited{description.reporting_amount_unlimited},
      mPeriodicTask{description.reporting_interval} {
    mPeriodicTask.callback = [this]() {
        deliver();
    };
    if (mClient && mClient->mScheduler) {
        if(!mPeriodicTask.schedule(*mClient->mScheduler)) {
            ERRORF("failed to schedule periodic location information delivery");
        }
    }
}

LocationInformationDelivery::~LocationInformationDelivery() {
    if (mClient) {
        mPeriodicTask.cancel();
    }
}

void LocationInformationDelivery::deliver() {
    VSCOPE_FUNCTION();
    ASSERT(mClient, "client is null");

    if (mClient->on_provide_location_information_advanced) {
        if (mClient->on_provide_location_information_advanced(*mClient, *this)) {
            VERBOSEF("location information delivered (handled by user)");
            return;
        }
    }

    if (mClient->on_provide_location_information) {
        messages::ProvideLocationInformation data{};
        if (mClient->on_provide_location_information(*mClient, *this, data)) {
            VERBOSEF("location information delivered (handled by client)");
            auto message = messages::create_provide_location_information(data);
            mTransaction.send(message);
            return;
        }
    }

    WARNF("location information not handled");
    messages::ProvideLocationInformation empty{};
    auto message = messages::create_provide_location_information(empty);
    mTransaction.send(message);
}

}  // namespace lpp
