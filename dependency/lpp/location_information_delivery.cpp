#include "location_information_delivery.hpp"
#include "lpp.hpp"
#include "lpp/client.hpp"
#include "messages/provide_location_information.hpp"

#include <algorithm>
#include <loglet/loglet.hpp>
#include <scheduler/scheduler.hpp>

#define LOGLET_CURRENT_MODULE "lpp/lid"

namespace lpp {

LocationInformationDelivery::LocationInformationDelivery(
    Client* client, Session* session, TransactionHandle transaction,
    PeriodicLocationInformationDeliveryDescription description)
    : mClient(client), mSession(session), mTransaction(std::move(transaction)),
      mCoordinateType{description.coordinate_type}, mVelocityType{description.velocity_type},
      mReportingAmount{description.reporting_amount},
      mReportingAmountUnlimited{description.reporting_amount_unlimited},
      mReportingInterval{description.reporting_interval},
      mPeriodicTask{description.reporting_interval} {
    mPeriodicTask.callback = [this]() {
        deliver();
    };
    if (mClient && mClient->mScheduler) {
        mPeriodicTask.schedule(*mClient->mScheduler);
    }
}

LocationInformationDelivery::~LocationInformationDelivery() {
    if (mClient) {
        mPeriodicTask.cancel();
    }
}

void LocationInformationDelivery::deliver() {
    SCOPE_FUNCTION();
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
