#include "lpp/client.hpp"
#include "location_information_delivery.hpp"
#include "lpp.hpp"
#include "lpp/assistance_data.hpp"
#include "lpp/messages/request_assistance_data.hpp"
#include "lpp/provide_capabilities.hpp"
#include "periodic_session/assistance_data.hpp"
#include "single_session.hpp"

#include <loglet/loglet.hpp>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wreserved-macro-identifier"
#pragma GCC diagnostic ignored "-Wreserved-identifier"
#pragma GCC diagnostic ignored "-Wundef"
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Wunused-function"
#include <CommonIEsRequestLocationInformation.h>
#include <PeriodicalReportingCriteria.h>
#include <ProvideAssistanceData-r9-IEs.h>
#include <RequestLocationInformation-r9-IEs.h>
#pragma GCC diagnostic pop

LOGLET_MODULE2(lpp, client);
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF2(lpp, client)

namespace lpp {

Client::Client(supl::Identity identity, supl::Cell supl_cell, std::string const& host,
               uint16_t port)
    : mHost(host), mPort(port), mInterface(""),
      mSession{lpp::VERSION_18_4_0, std::move(identity), std::move(supl_cell)}, mScheduler{nullptr},
      mCapabilities{nullptr} {
    VSCOPE_FUNCTION();

    on_capabilities                          = nullptr;
    on_request_location_information          = nullptr;
    on_provide_location_information          = nullptr;
    on_provide_location_information_advanced = nullptr;
    on_connected                             = nullptr;
    on_disconnected                          = nullptr;

    mNextSessionId        = 1;
    mSession.on_connected = [](Session&) {
        DEBUGF("connected");
    };

    mSession.on_disconnected = [this](Session&) {
        DEBUGF("disconnected");
        cancel();

        if (on_disconnected) {
            on_disconnected(*this);
        }
    };

    mSession.on_established = [this](lpp::Session&) {
        DEBUGF("established");

        if (on_connected) {
            on_connected(*this);
        }
    };

    mSession.on_begin_transaction = [this](lpp::Session&,
                                           lpp::TransactionHandle const& transaction) {
        this->process_begin_transaction(transaction);
    };

    mSession.on_end_transaction = [this](lpp::Session&, lpp::TransactionHandle const& transaction) {
        this->process_end_transaction(transaction);
    };

    mSession.on_message = [this](lpp::Session&, lpp::TransactionHandle const& transaction,
                                 lpp::Message message) {
        this->process_message(transaction, std::move(message));
    };

    mHackBadTransactionInitiator = false;
    mHackNeverSendAbort          = false;
}

Client::~Client() {
    if (mScheduler) {
        mScheduler->unregister_tick(this);
    }
}

PeriodicSessionHandle
Client::request_assistance_data(PeriodicRequestAssistanceData const& request_assistance_data) {
    VSCOPE_FUNCTION();

    PeriodicSessionHandle handle{};
    if (!allocate_periodic_session_handle(handle)) {
        ERRORF("failed to create periodic session handle");
        return PeriodicSessionHandle::invalid();
    }

    auto periodic_session =
        std::make_shared<AssistanceDataHandler>(this, &mSession, handle, request_assistance_data);
    periodic_session->set_hack_bad_transaction_initiator(mHackBadTransactionInitiator);
    periodic_session->set_hack_never_send_abort(mHackNeverSendAbort);
    if (!periodic_session->request_assistance_data()) {
        ERRORF("failed to request assistance data");
        return PeriodicSessionHandle::invalid();
    }

    mSessions[handle] = std::move(periodic_session);
    return handle;
}

bool Client::update_assistance_data(PeriodicSessionHandle const& session, supl::Cell cell) {
    VSCOPE_FUNCTION();

    auto it = mSessions.find(session);
    if (it == mSessions.end()) {
        WARNF("periodic session not found");
        return false;
    }

    auto periodic_session = dynamic_cast<AssistanceDataHandler*>(it->second.get());
    if (!periodic_session) {
        WARNF("periodic session is null");
        return false;
    }

    return periodic_session->update_assistance_data(cell);
}

void Client::cancel_assistance_data(PeriodicSessionHandle const&) {
    VSCOPE_FUNCTION();
    ERRORF("not implemented");
}

bool Client::is_periodic_session_valid(PeriodicSessionHandle const& session) const {
    if (!session.is_valid()) return false;
    auto it = mSessions.find(session);
    if (it == mSessions.end()) return false;
    return it->second->is_valid();
}

bool Client::request_assistance_data(SingleRequestAssistanceData const& request_assistance_data) {
    VSCOPE_FUNCTION();

    messages::RequestAssistanceData message_description{};
    message_description.cell             = request_assistance_data.cell;
    message_description.periodic_session = PeriodicSessionHandle::invalid();
    message_description.gps              = request_assistance_data.gnss.gps;
    message_description.glonass          = request_assistance_data.gnss.glonass;
    message_description.galileo          = request_assistance_data.gnss.galileo;
    message_description.bds              = request_assistance_data.gnss.beidou;

    if (request_assistance_data.type == SingleRequestAssistanceData::Type::AGNSS) {
        message_description.reference_time    = 1;
        message_description.ionospheric_model = 1;
    } else {
        WARNF("unknown RequestAssistanceData type");
        return false;
    }

    auto message = messages::create_request_assistance_data(message_description);
    if (!message) {
        ERRORF("failed to create request assistance data message");
        return false;
    }

    auto transaction = mSession.create_transaction();
    if (!transaction.is_valid()) {
        ERRORF("failed to create transaction");
        return false;
    }

    transaction.send(message);

    auto session = std::make_shared<SingleRequestAssistanceSession>(
        this, &mSession, transaction, std::move(request_assistance_data));
    session->set_hack_never_send_abort(mHackNeverSendAbort);
    return add_single_session(session);
}

void Client::schedule(scheduler::Scheduler* scheduler) {
    ASSERT(scheduler, "scheduler is null");
    ASSERT(!mScheduler, "scheduler is already set");

    // If the client has sessions already, then we need to schedule
    // the sessions now.
    for (auto& session : mSessions) {
        (void)session.second->schedule(*scheduler);
    }

    for (auto& session : mSingleSessions) {
        (void)session.second->schedule(*scheduler);
    }

    mScheduler = scheduler;
    mSession.connect(mHost, mPort, mInterface);
    mSession.schedule(scheduler);
    scheduler->register_tick(this, [this]() {
        TRACEF("tick: %zu sessions, %zu single sessions", mSessions.size(), mSingleSessions.size());
        for (auto handle : mSessionsToDestroy) {
            deallocate_periodic_session_handle(handle);
        }

        mSessionsToDestroy.clear();

        for (auto transaction : mSingleSessionsToDestroy) {
            remove_single_session(transaction);
        }

        mSingleSessionsToDestroy.clear();
    });
}

void Client::cancel() {
    ASSERT(mScheduler, "scheduler is null");

    for (auto& session : mSessions) {
        session.second->cancel();
    }

    for (auto& session : mSingleSessions) {
        session.second->cancel();
    }

    mSession.cancel();
    mScheduler->unregister_tick(this);
    mScheduler = nullptr;

    while (!mSessions.empty()) {
        auto it      = mSessions.begin();
        auto session = it->second;
        mSessions.erase(it);
    }

    mRequestTransactions.clear();
    mPeriodicTransactions.clear();
    mSingleSessions.clear();
    mLocationInformationDeliveries.clear();
    mNextSessionId = 1;
}

bool Client::allocate_periodic_session_handle(PeriodicSessionHandle& result) {
    for (long i = 0; i < 256; i++) {
        auto id     = (mNextSessionId + i) % 256;
        auto handle = PeriodicSessionHandle{id, Initiator::TargetDevice};
        if (mSessions.find(handle) == mSessions.end()) {
            mNextSessionId = id;
            DEBUGF("allocated periodic session handle %s", handle.to_string().c_str());
            result = handle;
            return true;
        }
    }

    return false;
}

bool Client::deallocate_periodic_session_handle(PeriodicSessionHandle const& handle) {
    if (!handle.is_valid()) return false;
    mSessions.erase(handle);
    DEBUGF("deallocated periodic session handle %s", handle.to_string().c_str());
    return true;
}

bool Client::add_single_session(Sah session) {
    ASSERT(session, "session is null");
    ASSERT(session->transaction().is_valid(), "transaction is invalid");
    mSingleSessions[session->transaction()] = session;
    DEBUGF("added single session with transaction %s", session->transaction().to_string().c_str());
    return true;
}

bool Client::remove_single_session(TransactionHandle const& handle) {
    if (!handle.is_valid()) return false;
    mSingleSessions.erase(handle);
    DEBUGF("removed single session with transaction %s", handle.to_string().c_str());
    return true;
}

void Client::want_to_be_destroyed(PeriodicSessionHandle const& handle) {
    VSCOPE_FUNCTION();
    mSessionsToDestroy.push_back(handle);
}

void Client::single_session_want_to_be_destroyed(TransactionHandle const& handle) {
    VSCOPE_FUNCTION();
    mSingleSessionsToDestroy.push_back(handle);
}

void Client::process_message(lpp::TransactionHandle const& transaction, lpp::Message message) {
    VSCOPE_FUNCTIONF("%s", transaction.to_string().c_str());

    if (lpp::is_request_capabilities(message)) {
        process_request_capabilities(transaction, std::move(message));
    } else if (lpp::is_request_assistance_data(message)) {
        process_request_assistance_data(transaction, std::move(message));
    } else if (lpp::is_request_location_information(message)) {
        process_request_location_information(transaction, std::move(message));
    } else if (lpp::is_provide_capabilities(message)) {
        process_provide_capabilities(transaction, std::move(message));
    } else if (lpp::is_provide_assistance_data(message)) {
        process_provide_assistance_data(transaction, std::move(message));
    } else if (lpp::is_provide_location_information(message)) {
        process_provide_location_information(transaction, std::move(message));
    } else if (lpp::is_abort(message)) {
        process_abort(transaction, std::move(message));
    } else if (lpp::is_error(message)) {
        process_error(transaction, std::move(message));
    } else {
        ERRORF("unknown message type");
    }
}

void Client::process_request_capabilities(lpp::TransactionHandle const& transaction,
                                          lpp::Message                  message) {
    VSCOPE_FUNCTIONF("%s", transaction.to_string().c_str());

    auto capabilities_has_been_handled = false;
    if (on_capabilities) {
        capabilities_has_been_handled = on_capabilities(*this, transaction, message);
    }

    if (!capabilities_has_been_handled) {
        ProvideCapabilities default_capabilities{};
        default_capabilities.gnss.gps     = true;
        default_capabilities.gnss.glonass = true;
        default_capabilities.gnss.galileo = true;
        default_capabilities.gnss.beidou  = true;

        auto capabilities = mCapabilities.get();
        if (!capabilities) capabilities = &default_capabilities;
        auto response_message = create_provide_capabilities(*capabilities);
        mSession.send_with_end(transaction, response_message);
    }

    if (on_provide_capabilities) {
        on_provide_capabilities(*this);
    }
}

void Client::process_request_assistance_data(lpp::TransactionHandle const& transaction,
                                             lpp::Message) {
    VSCOPE_FUNCTIONF("%s", transaction.to_string().c_str());
    ERRORF("lpp::Client does not support RequestAssistanceData from the location server");
    // TODO(ewasjon): What should we respond with here?
}

void Client::process_request_location_information(lpp::TransactionHandle const& transaction,
                                                  lpp::Message                  message) {
    VSCOPE_FUNCTIONF("%s", transaction.to_string().c_str());

    auto it = mLocationInformationDeliveries.find(transaction);
    if (it != mLocationInformationDeliveries.end()) {
        WARNF("location information delivery already exists for transaction");
        return;
    }

    auto handled_by_user = false;
    if (on_request_location_information) {
        handled_by_user = on_request_location_information(*this, transaction, message);
    }

    if (handled_by_user) {
        DEBUGF("location information request handled by user");
        return;
    }

    auto inner = lpp::get_request_location_information(message);
    if (!inner) {
        WARNF(
            "request location information message does not contain a request location information");
        return;
    }

    auto common = inner->commonIEsRequestLocationInformation;
    if (!common) {
        WARNF("request location information message does not contain common IEs");
        return;
    }

    auto periodical_reporting = common->periodicalReporting;
    if (!periodical_reporting) {
        WARNF("only 'periodicalReporting' is supported and must be set");
        return;
    }

    auto reporting_amount_enum = (periodical_reporting->reportingAmount == nullptr ?
                                      PeriodicalReportingCriteria__reportingAmount_ra1 :
                                      *periodical_reporting->reportingAmount);
    auto reporting_amount_unlimited =
        (reporting_amount_enum == PeriodicalReportingCriteria__reportingAmount_ra_Infinity);
    long reporting_amount = 0;
    switch (reporting_amount_enum) {
    case PeriodicalReportingCriteria__reportingAmount_ra1: reporting_amount = 1; break;
    case PeriodicalReportingCriteria__reportingAmount_ra2: reporting_amount = 2; break;
    case PeriodicalReportingCriteria__reportingAmount_ra4: reporting_amount = 4; break;
    case PeriodicalReportingCriteria__reportingAmount_ra8: reporting_amount = 8; break;
    case PeriodicalReportingCriteria__reportingAmount_ra16: reporting_amount = 16; break;
    case PeriodicalReportingCriteria__reportingAmount_ra32: reporting_amount = 32; break;
    case PeriodicalReportingCriteria__reportingAmount_ra64: reporting_amount = 64; break;
    case PeriodicalReportingCriteria__reportingAmount_ra_Infinity: reporting_amount = 0; break;
    default: WARNF("unknown reporting amount"); return;
    }

    std::chrono::seconds reporting_interval{1};
    switch (periodical_reporting->reportingInterval) {
    case PeriodicalReportingCriteria__reportingInterval_noPeriodicalReporting:
        WARNF("no periodical reporting is not supported");
        return;
    case PeriodicalReportingCriteria__reportingInterval_ri0_25:
        reporting_interval = std::chrono::seconds{1};
        break;
    case PeriodicalReportingCriteria__reportingInterval_ri0_5:
        reporting_interval = std::chrono::seconds{2};
        break;
    case PeriodicalReportingCriteria__reportingInterval_ri1:
        reporting_interval = std::chrono::seconds{4};
        break;
    case PeriodicalReportingCriteria__reportingInterval_ri2:
        reporting_interval = std::chrono::seconds{8};
        break;
    case PeriodicalReportingCriteria__reportingInterval_ri4:
        reporting_interval = std::chrono::seconds{10};
        break;
    case PeriodicalReportingCriteria__reportingInterval_ri8:
        reporting_interval = std::chrono::seconds{16};
        break;
    case PeriodicalReportingCriteria__reportingInterval_ri16:
        reporting_interval = std::chrono::seconds{20};
        break;
    case PeriodicalReportingCriteria__reportingInterval_ri32:
        reporting_interval = std::chrono::seconds{32};
        break;
    case PeriodicalReportingCriteria__reportingInterval_ri64:
        reporting_interval = std::chrono::seconds{64};
        break;
    default: WARNF("unknown reporting interval"); return;
    }

    PeriodicLocationInformationDeliveryDescription description{};
    description.reporting_amount_unlimited = reporting_amount_unlimited;
    description.reporting_amount           = reporting_amount;
    description.reporting_interval         = reporting_interval;

    // TODO(ewasjon):
    // description.coordinate_type = get_location_coordinate_types(common->locationCoordinateTypes);
    // description.velocity_type   = get_velocity_types(common->velocityTypes);
    // description.ha_gnss_metrics = did_request_ha_gnss_metrics(inner);

    if (!start_periodic_location_information(transaction, description)) {
        WARNF("failed to start periodic location information");
    }
}

void Client::process_provide_capabilities(lpp::TransactionHandle const& transaction, lpp::Message) {
    VSCOPE_FUNCTIONF("%s", transaction.to_string().c_str());
}

PeriodicSession* Client::find_by_periodic_session_handle(PeriodicSessionHandle const& handle) {
    auto it = mSessions.find(handle);
    if (it != mSessions.end()) {
        return it->second.get();
    } else {
        return nullptr;
    }
}

PeriodicSession* Client::find_by_request_transaction_handle(TransactionHandle const& transaction) {
    auto it = mRequestTransactions.find(transaction);
    if (it != mRequestTransactions.end()) {
        return find_by_periodic_session_handle(it->second);
    } else {
        return nullptr;
    }
}

PeriodicSession* Client::find_by_periodic_transaction_handle(TransactionHandle const& transaction) {
    auto it = mPeriodicTransactions.find(transaction);
    if (it != mPeriodicTransactions.end()) {
        return find_by_periodic_session_handle(it->second);
    } else {
        return nullptr;
    }
}

SingleSession*
Client::find_single_session_by_request_transaction_handle(TransactionHandle const& transaction) {
    auto it = mSingleSessions.find(transaction);
    if (it != mSingleSessions.end()) {
        return it->second.get();
    } else {
        return nullptr;
    }
}

void Client::process_provide_assistance_data(lpp::TransactionHandle const& transaction,
                                             lpp::Message                  message) {
    VSCOPE_FUNCTIONF("%s", transaction.to_string().c_str());

    auto inner = lpp::get_provide_assistance_data(message);
    if (!inner) return;

    auto periodic_session = find_by_request_transaction_handle(transaction);
    if (periodic_session) {
        periodic_session->message(transaction, std::move(message));
        return;
    }

    auto single_session = find_single_session_by_request_transaction_handle(transaction);
    if (single_session) {
        single_session->message(transaction, std::move(message));
        return;
    }

    PeriodicSessionHandle handle{};
    if (!lpp::get_periodic_session(*inner, &handle)) {
        WARNF("provide assistance data message does not contain a periodic session id");
        // TODO(ewasjon): Handle single-shot requests
        return;
    }

    periodic_session = find_by_periodic_session_handle(handle);
    if (!periodic_session) {
        WARNF("provide assistance data with a unknown periodic session");
        // TODO(ewasjon): Send back an abort for the transactions
        return;
    }

    periodic_session->message(transaction, std::move(message));
}

void Client::process_provide_location_information(TransactionHandle const& transaction, Message) {
    VSCOPE_FUNCTIONF("%s", transaction.to_string().c_str());
}

void Client::process_abort(TransactionHandle const& transaction, Message) {
    VSCOPE_FUNCTIONF("%s", transaction.to_string().c_str());
}

void Client::process_error(TransactionHandle const& transaction, Message) {
    VSCOPE_FUNCTIONF("%s", transaction.to_string().c_str());
}

void Client::process_begin_transaction(lpp::TransactionHandle const& transaction) {
    VSCOPE_FUNCTIONF("%s", transaction.to_string().c_str());
}

void Client::process_end_transaction(lpp::TransactionHandle const& transaction) {
    VSCOPE_FUNCTIONF("%s", transaction.to_string().c_str());

    auto periodic_session = find_by_request_transaction_handle(transaction);
    if (periodic_session) {
        periodic_session->end(transaction);
    }

    auto single_session = find_single_session_by_request_transaction_handle(transaction);
    if (single_session) {
        // When a single session ends, it is destroyed
        remove_single_session(transaction);
    }

    periodic_session = find_by_periodic_transaction_handle(transaction);
    if (periodic_session) {
        periodic_session->end(transaction);
    }

    mLocationInformationDeliveries.erase(transaction);
}

TransactionHandle Client::start_periodic_location_information(
    PeriodicLocationInformationDeliveryDescription const& description) {
    VSCOPE_FUNCTION();
    auto transaction = mSession.create_transaction(false);
    start_periodic_location_information(transaction, description);
    return transaction;
}

bool Client::start_periodic_location_information(
    TransactionHandle const&                              transaction,
    PeriodicLocationInformationDeliveryDescription const& description) {
    VSCOPE_FUNCTIONF("%s", transaction.to_string().c_str());

    auto it = mLocationInformationDeliveries.find(transaction);
    if (it != mLocationInformationDeliveries.end()) {
        WARNF("location information delivery already exists for transaction");
        return false;
    }

    auto delivery =
        std::make_shared<LocationInformationDelivery>(this, &mSession, transaction, description);
    // TODO(ewasjon): Should we deliver the location information directly or wait for the next
    // interval?
    delivery->deliver();

    mLocationInformationDeliveries[transaction] = std::move(delivery);
    return true;
}

void Client::stop_periodic_location_information(TransactionHandle const& transaction) {
    VSCOPE_FUNCTIONF("%s", transaction.to_string().c_str());

    auto it = mLocationInformationDeliveries.find(transaction);
    if (it == mLocationInformationDeliveries.end()) {
        WARNF("location information delivery does not exist for transaction");
        return;
    }

    ERRORF("todo: stop periodic location information");
}

void Client::set_capabilities(ProvideCapabilities const& capabilities) {
    mCapabilities.reset(new ProvideCapabilities(capabilities));
}

}  // namespace lpp
