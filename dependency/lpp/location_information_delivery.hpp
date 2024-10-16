#pragma once
#include <lpp/transaction.hpp>
#include <scheduler/periodic.hpp>

#include <chrono>
#include <unordered_map>

namespace lpp {

struct CoordinateType {
    bool ellipsoid_point;
    bool ellipsoid_point_with_uncertainty_circle;
    bool ellipsoid_point_with_uncertainty_ellipse;
    bool polygon;
    bool ellipsoid_point_with_altitude;
    bool ellipsoid_point_with_altitude_and_uncertainty_ellipsoid;
    bool ellipsoid_arc;
    bool ha_ellipsoid_point_with_uncertainty_ellipse;
    bool ha_ellipsoid_point_with_altitude_and_uncertainty_ellipsoid;
    bool ha_ellipsoid_point_with_scalable_uncertainty_ellipse;
    bool ha_ellipsoid_point_with_scalable_altitude_and_uncertainty_ellipse;
    bool local2d_point_with_uncertainty_ellipse;
    bool local3d_point_with_uncertainty_elipsoid;
};

struct VelocityType {
    bool horizontal;
    bool horizontal_with_vertical;
    bool horizontal_with_uncertainty;
    bool horizontal_with_vertical_and_uncertainty;
};

struct PeriodicLocationInformationDeliveryDescription {
    bool                                ha_gnss_metrics;
    bool                                reporting_amount_unlimited;
    long                                reporting_amount;
    std::chrono::steady_clock::duration reporting_interval;

    CoordinateType coordinate_type;
    VelocityType   velocity_type;
};

class Client;
class Session;
class LocationInformationDelivery {
public:
    EXPLICIT
    LocationInformationDelivery(Client* client, Session* session, TransactionHandle transaction,
                                PeriodicLocationInformationDeliveryDescription description);
    ~LocationInformationDelivery();

    NODISCARD TransactionHandle const& transaction() const { return mTransaction; }
    NODISCARD CoordinateType const& coordinate_type() const { return mCoordinateType; }
    NODISCARD VelocityType const&   velocity_type() const { return mVelocityType; }

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
