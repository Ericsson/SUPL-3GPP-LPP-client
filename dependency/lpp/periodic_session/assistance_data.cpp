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

void AssistanceDataHandler::request_response(TransactionHandle const& transaction,
                                             Message                  message) {
    SCOPE_FUNCTIONF("%s", transaction.to_string().c_str());
    if (mData.on_non_periodic) {
        mData.on_non_periodic(*mClient, std::move(message));
    }
}

void AssistanceDataHandler::periodic_begin(TransactionHandle const& transaction) {
    SCOPE_FUNCTIONF("%s", transaction.to_string().c_str());
    if (mData.on_started) {
        mData.on_started(*mClient, mHandle);
    }
}

void AssistanceDataHandler::periodic_ended(TransactionHandle const& transaction) {
    SCOPE_FUNCTIONF("%s", transaction.to_string().c_str());
    if (mData.on_ended) {
        mData.on_ended(*mClient, mHandle);
    }
}

void AssistanceDataHandler::periodic_message(TransactionHandle const& transaction,
                                             Message                  message) {
    SCOPE_FUNCTIONF("%s", transaction.to_string().c_str());
    if (mData.on_periodic) {
        mData.on_periodic(*mClient, mHandle, std::move(message));
    }
}

bool AssistanceDataHandler::request_assistance_data() {
    SCOPE_FUNCTION();

    messages::RequestAssistanceData message_description{};
    message_description.cell             = mData.cell;
    message_description.periodic_session = handle();
    message_description.gps              = true;
    message_description.glonass          = true;
    message_description.galileo          = true;
    message_description.bds              = false;

    if (mData.type == RequestAssistanceData::Type::OSR) {
        message_description.rtk_observations           = 1;
        message_description.rtk_residuals              = 1;
        message_description.rtk_bias_information       = 1;
        message_description.rtk_reference_station_info = 1;
    } else if (mData.type == RequestAssistanceData::Type::SSR) {
        message_description.ssr_clock             = 5;
        message_description.ssr_orbit             = 5;
        message_description.ssr_code_bias         = 5;
        message_description.ssr_phase_bias        = 5;
        message_description.ssr_stec              = 5;
        message_description.ssr_gridded           = 5;
        message_description.ssr_ura               = 5;
        message_description.ssr_correction_points = 1;
    } else if (mData.type == RequestAssistanceData::Type::AGNSS) {
        UNIMPLEMENTED("AGNSS not implemented");
        return false;
    } else {
        WARNF("unknown RequestAssistanceData type");
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

    messages::RequestAssistanceData message_description{};
    message_description.cell             = cell;
    message_description.periodic_session = handle();
    message_description.gps              = true;
    message_description.glonass          = true;
    message_description.galileo          = true;
    message_description.bds              = false;

    if (mData.type == RequestAssistanceData::Type::OSR) {
        message_description.rtk_observations           = 1;
        message_description.rtk_residuals              = 1;
        message_description.rtk_bias_information       = 1;
        message_description.rtk_reference_station_info = 1;
    } else if (mData.type == RequestAssistanceData::Type::SSR) {
        UNIMPLEMENTED("SSR not implemented");
        return false;
    } else if (mData.type == RequestAssistanceData::Type::AGNSS) {
        UNIMPLEMENTED("AGNSS not implemented");
        return false;
    } else {
        WARNF("unknown RequestAssistanceData type");
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
