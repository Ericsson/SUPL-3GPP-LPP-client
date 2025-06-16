#include "assistance_data.hpp"
#include "../lpp.hpp"
#include "lpp/client.hpp"
#include "lpp/messages/request_assistance_data.hpp"
#include "lpp/session.hpp"

#include <loglet/loglet.hpp>

LOGLET_MODULE2(lpp, ad);
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF2(lpp, ad)

namespace lpp {

AssistanceDataHandler::AssistanceDataHandler(Client* client, Session* session,
                                             PeriodicSessionHandle handle,
                                             RequestAssistanceData data)
    : PeriodicSession(client, session, handle), mData(data) {
    VSCOPE_FUNCTION();
}

AssistanceDataHandler::~AssistanceDataHandler() {
    VSCOPE_FUNCTION();
}

void AssistanceDataHandler::request_response(TransactionHandle const& transaction,
                                             Message                  message) {
    VSCOPE_FUNCTIONF("%s", transaction.to_string().c_str());
    if (mData.on_non_periodic) {
        mData.on_non_periodic(*mClient, std::move(message));
    }
}

void AssistanceDataHandler::periodic_begin(TransactionHandle const& transaction) {
    VSCOPE_FUNCTIONF("%s", transaction.to_string().c_str());
    if (mData.on_started) {
        mData.on_started(*mClient, mHandle);
    }
}

void AssistanceDataHandler::periodic_ended(TransactionHandle const& transaction) {
    VSCOPE_FUNCTIONF("%s", transaction.to_string().c_str());
    if (mData.on_ended) {
        mData.on_ended(*mClient, mHandle);
    }
}

void AssistanceDataHandler::periodic_message(TransactionHandle const& transaction,
                                             Message                  message) {
    VSCOPE_FUNCTIONF("%s", transaction.to_string().c_str());
    if (mData.on_periodic) {
        mData.on_periodic(*mClient, mHandle, std::move(message));
    }
}

void AssistanceDataHandler::stale_request(TransactionHandle const& transaction) {
    VSCOPE_FUNCTIONF("%s", transaction.to_string().c_str());
    if (mData.on_error) {
        mData.on_error(*mClient);
    }

    try_destroy();
}

bool AssistanceDataHandler::request_assistance_data() {
    VSCOPE_FUNCTION();

    messages::RequestAssistanceData message_description{};
    message_description.cell               = mData.cell;
    message_description.periodic_session   = handle();
    message_description.gps                = mData.gnss.gps;
    message_description.glonass            = mData.gnss.glonass;
    message_description.galileo            = mData.gnss.galileo;
    message_description.bds                = mData.gnss.beidou;
    message_description.delivery_amount    = mData.config.delivery_amount;
    message_description.rtk_antenna_height = mData.config.antenna_height;

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
    VSCOPE_FUNCTION();

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

}  // namespace lpp
