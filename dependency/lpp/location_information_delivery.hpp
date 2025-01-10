#pragma once
#include <lpp/location_information.hpp>
#include <lpp/transaction.hpp>
#include <scheduler/periodic.hpp>

#include <chrono>
#include <unordered_map>

namespace lpp {

class Client;
class Session;
class LocationInformationDelivery {
public:
    EXPLICIT
    LocationInformationDelivery(Client* client, Session* session, TransactionHandle transaction,
                                PeriodicLocationInformationDeliveryDescription description);
    ~LocationInformationDelivery();

    NODISCARD TransactionHandle const& transaction() const { return mTransaction; }
    NODISCARD CoordinateType const&    coordinate_type() const { return mCoordinateType; }
    NODISCARD VelocityType const&      velocity_type() const { return mVelocityType; }

    void deliver();

protected:
    Client*                             mClient;
    Session*                            mSession;
    TransactionHandle                   mTransaction;
    CoordinateType                      mCoordinateType;
    VelocityType                        mVelocityType;
    std::chrono::steady_clock::duration mReportingInterval;
    long                                mReportingAmount;
    bool                                mReportingAmountUnlimited;
    scheduler::PeriodicTask             mPeriodicTask;
};

}  // namespace lpp
