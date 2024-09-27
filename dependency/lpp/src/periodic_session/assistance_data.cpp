#include "assistance_data.hpp"
#include "../lpp.hpp"
#include "lpp/client.hpp"
#include "lpp/messages/request_assistance_data.hpp"
#include "lpp/session.hpp"

#include <loglet/loglet.hpp>

#define LOGLET_CURRENT_MODULE "lpp/ad"

namespace lpp {

AssistanceDataHandler::AssistanceDataHandler(Client* client, Session* session,
                                             PeriodicSessionHandle handle,
                                             RequestAssistanceData data)
    : PeriodicSession(client, session, handle), mData(data) {
    SCOPE_FUNCTION();
}

AssistanceDataHandler::~AssistanceDataHandler() {
    SCOPE_FUNCTION();
}

void AssistanceDataHandler::request_response(const TransactionHandle& transaction,
                                             Message                  message) {
    SCOPE_FUNCTIONF("%s", transaction.to_string().c_str());
    if (mData.on_non_periodic) {
        mData.on_non_periodic(*mClient, std::move(message));
    }
}

void AssistanceDataHandler::periodic_begin(const TransactionHandle& transaction) {
    SCOPE_FUNCTIONF("%s", transaction.to_string().c_str());
    if (mData.on_started) {
        mData.on_started(*mClient, mHandle);
    }
}

void AssistanceDataHandler::periodic_ended(const TransactionHandle& transaction) {
    SCOPE_FUNCTIONF("%s", transaction.to_string().c_str());
    if (mData.on_ended) {
        mData.on_ended(*mClient, mHandle);
    }
}

void AssistanceDataHandler::periodic_message(const TransactionHandle& transaction,
                                             Message                  message) {
    SCOPE_FUNCTIONF("%s", transaction.to_string().c_str());
    if (mData.on_periodic) {
        mData.on_periodic(*mClient, mHandle, std::move(message));
    }
}

bool AssistanceDataHandler::request_assistance_data() {
    SCOPE_FUNCTION();

    auto message_description = messages::RequestAssistanceData{
        .cell             = mData.cell,
        .periodic_session = handle(),
        .gps              = true,
        .glonass          = true,
        .galileo          = true,
        .bds              = false,
    };

    if (mData.type == RequestAssistanceData::Type::OSR) {
        message_description.rtk_observations           = 1;
        message_description.rtk_residuals              = 1;
        message_description.rtk_bias_information       = 1;
        message_description.rtk_reference_station_info = 1;
    } else if (mData.type == RequestAssistanceData::Type::SSR) {
        ERRORF("SSR not implemented");
        LPP_UNREACHABLE();
        return false;
    } else if (mData.type == RequestAssistanceData::Type::AGNSS) {
        ERRORF("AGNSS not implemented");
        LPP_UNREACHABLE();
        return false;
    } else {
        ERRORF("Unknown RequestAssistanceData type");
        LPP_UNREACHABLE();
        return false;
    }

    auto message = messages::create_request_assistance_data(message_description);
    if (!message) {
        ERRORF("failed to create request assistance data message");
        return false;
    }

    return send_new_request(std::move(message));
}

bool AssistanceDataHandler::update_assistance_data(supl::Cell cell) {
    SCOPE_FUNCTION();

    auto message_description = messages::RequestAssistanceData{
        .cell             = cell,
        .periodic_session = handle(),
        .gps              = true,
        .glonass          = true,
        .galileo          = true,
        .bds              = false,
    };

    if (mData.type == RequestAssistanceData::Type::OSR) {
        message_description.rtk_observations           = 1;
        message_description.rtk_residuals              = 1;
        message_description.rtk_bias_information       = 1;
        message_description.rtk_reference_station_info = 1;
    } else if (mData.type == RequestAssistanceData::Type::SSR) {
        ERRORF("SSR not implemented");
        LPP_UNREACHABLE();
        return false;
    } else if (mData.type == RequestAssistanceData::Type::AGNSS) {
        ERRORF("AGNSS not implemented");
        LPP_UNREACHABLE();
        return false;
    } else {
        ERRORF("Unknown RequestAssistanceData type");
        LPP_UNREACHABLE();
        return false;
    }

    auto message = messages::create_request_assistance_data(message_description);
    if (!message) {
        ERRORF("failed to create request assistance data message");
        return false;
    }

    return send_new_request(std::move(message));
}

}  // namespace lpp
