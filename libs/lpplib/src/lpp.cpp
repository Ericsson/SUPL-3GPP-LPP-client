#include "lpp.h"
#include "internal_lpp.h"
#include "supl.h"

#include <utility/cpp.h>

LPP_Client::LPP_Client(bool segmentation) {
    connected           = false;
    main_request        = AD_REQUEST_INVALID;
    provide_li.type     = -1;
    transaction_counter = 1;
    client_id           = 0xC0DEC0DE;
    mEnableSegmentation = segmentation;
    mSUPL               = std::make_unique<SUPL_Client>();
}

LPP_Client::~LPP_Client() {}

void LPP_Client::set_identity_msisdn(unsigned long msisdn) {
    if (connected) return;
    mSUPL->set_session(SUPL_Session::msisdn(0, msisdn));
}

void LPP_Client::set_identity_imsi(unsigned long imsi) {
    if (connected) return;
    mSUPL->set_session(SUPL_Session::imsi(0, imsi));
}

void LPP_Client::set_identity_ipv4(const std::string& ipv4) {
    if (connected) return;
    mSUPL->set_session(SUPL_Session::ip_address(0, ipv4));
}

bool LPP_Client::supl_start(CellID cell) {
    auto message = mSUPL->create_message(UlpMessage_PR_msSUPLSTART);

    auto start                                           = &message->message.choice.msSUPLSTART;
    start->locationId.status                             = Status_current;
    start->sETCapabilities.posTechnology.agpsSETassisted = true;
    start->sETCapabilities.posTechnology.agpsSETBased    = true;
    start->sETCapabilities.prefMethod                    = PrefMethod_agpsSETBasedPreferred;

    {
        // LPP Version
        auto lpp_pos_protocol                   = ALLOC_ZERO(PosProtocolVersion3GPP_t);
        lpp_pos_protocol->majorVersionField     = 16;
        lpp_pos_protocol->technicalVersionField = 4;
        lpp_pos_protocol->editorialVersionField = 0;

        auto pos_protocol_ext                   = ALLOC_ZERO(Ver2_PosProtocol_extension);
        pos_protocol_ext->lpp                   = true;
        pos_protocol_ext->posProtocolVersionLPP = lpp_pos_protocol;

        start->sETCapabilities.posProtocol.ver2_PosProtocol_extension = pos_protocol_ext;
    }

    {
        auto cell_info     = &start->locationId.cellInfo;
        cell_info->present = CellInfo_PR_ver2_CellInfo_extension;

        auto cell_v2     = &cell_info->choice.ver2_CellInfo_extension;
        cell_v2->present = Ver2_CellInfo_extension_PR_lteCell;

        auto lte_cell                                 = &cell_v2->choice.lteCell;
        lte_cell->cellGlobalIdEUTRA.plmn_Identity.mcc = supl_create_mcc(cell.mcc);
        supl_fill_mnc(&lte_cell->cellGlobalIdEUTRA.plmn_Identity.mnc, cell.mnc);
        supl_fill_tracking_area_code(&lte_cell->trackingAreaCode, cell.tac);
        supl_fill_cell_identity(&lte_cell->cellGlobalIdEUTRA.cellIdentity, cell.cell);

        /* Set the remaining LTE parameters */
        lte_cell->physCellId          = 0;
        lte_cell->rsrpResult          = OPTIONAL_MISSING;
        lte_cell->rsrqResult          = OPTIONAL_MISSING;
        lte_cell->measResultListEUTRA = OPTIONAL_MISSING;
    }

    return mSUPL->send(message);
}

bool LPP_Client::supl_response() {
    auto message = mSUPL->receive(-1);
    if (!message) {
        return false;
    }

    // Harvest the SlpSessionID
    if (message->message.present == UlpMessage_PR_msSUPLRESPONSE) {
        mSUPL->harvest(message);
    }

    return true;
}

bool LPP_Client::supl_send_posinit(CellID cell) {
    auto message = mSUPL->create_message(UlpMessage_PR_msSUPLPOSINIT);

    auto posinit                                           = &message->message.choice.msSUPLPOSINIT;
    posinit->locationId.status                             = Status_current;
    posinit->sETCapabilities.posTechnology.agpsSETassisted = true;
    posinit->sETCapabilities.posTechnology.agpsSETBased    = true;
    posinit->sETCapabilities.prefMethod                    = PrefMethod_agpsSETBasedPreferred;

    {
        // LPP Version
        auto lpp_pos_protocol                   = ALLOC_ZERO(PosProtocolVersion3GPP_t);
        lpp_pos_protocol->majorVersionField     = 16;
        lpp_pos_protocol->technicalVersionField = 4;
        lpp_pos_protocol->editorialVersionField = 0;

        auto pos_protocol_ext                   = ALLOC_ZERO(Ver2_PosProtocol_extension);
        pos_protocol_ext->lpp                   = true;
        pos_protocol_ext->posProtocolVersionLPP = lpp_pos_protocol;

        posinit->sETCapabilities.posProtocol.ver2_PosProtocol_extension = pos_protocol_ext;
    }

    {
        auto cell_info     = &posinit->locationId.cellInfo;
        cell_info->present = CellInfo_PR_ver2_CellInfo_extension;

        auto cell_v2     = &cell_info->choice.ver2_CellInfo_extension;
        cell_v2->present = Ver2_CellInfo_extension_PR_lteCell;

        auto lte_cell                                 = &cell_v2->choice.lteCell;
        lte_cell->cellGlobalIdEUTRA.plmn_Identity.mcc = supl_create_mcc(cell.mcc);
        supl_fill_mnc(&lte_cell->cellGlobalIdEUTRA.plmn_Identity.mnc, cell.mnc);
        supl_fill_tracking_area_code(&lte_cell->trackingAreaCode, cell.tac);
        supl_fill_cell_identity(&lte_cell->cellGlobalIdEUTRA.cellIdentity, cell.cell);

        /* Set the remaining LTE parameters */
        lte_cell->physCellId          = 0;
        lte_cell->rsrpResult          = OPTIONAL_MISSING;
        lte_cell->rsrqResult          = OPTIONAL_MISSING;
        lte_cell->measResultListEUTRA = OPTIONAL_MISSING;
    }

    return mSUPL->send(message);
}

bool LPP_Client::supl_receive(std::vector<LPP_Message*>& messages, int milliseconds) {
    auto message = mSUPL->receive(milliseconds);
    if (!message) {
        return false;
    }

    if (message->message.present == UlpMessage_PR_msSUPLPOS) {
        auto pos = &message->message.choice.msSUPLPOS;
        if (pos->posPayLoad.present == PosPayLoad_PR_ver2_PosPayLoad_extension) {
            auto ext     = &pos->posPayLoad.choice.ver2_PosPayLoad_extension;
            auto payload = ext->lPPPayload;
            auto list    = &payload->list;
            for (auto i = 0; i < list->count; i++) {
                auto data = list->array[i];
                auto lpp  = decode(data);
                if (lpp) {
                    messages.push_back(lpp);
                } else {
                    printf("ERROR: Failed to decode LPP message\n");
                }
            }
        }
    }

    return true;
}

bool LPP_Client::supl_send(LPP_Message* message) {
    std::vector<LPP_Message*> messages;
    messages.push_back(message);
    return supl_send(messages);
}

bool LPP_Client::supl_send(const std::vector<LPP_Message*>& messages) {
    auto message = mSUPL->create_message(UlpMessage_PR_msSUPLPOS);

    {
        auto payload = ALLOC_ZERO(Ver2_PosPayLoad_extension::Ver2_PosPayLoad_extension__lPPPayload);
        asn_sequence_empty(&payload->list);

        for (auto& data : messages) {
            auto lpp = encode(data);
            if (lpp) {
                asn_sequence_add(&payload->list, lpp);
            }
        }

        auto supl_pos                = &message->message.choice.msSUPLPOS;
        supl_pos->posPayLoad.present = PosPayLoad_PR_ver2_PosPayLoad_extension;
        supl_pos->posPayLoad.choice.ver2_PosPayLoad_extension.lPPPayload = payload;
    }

    return mSUPL->send(message);
}

OCTET_STRING* LPP_Client::encode(LPP_Message* message) {
    return lpp_encode(message);
}

LPP_Message* LPP_Client::decode(OCTET_STRING* data) {
    return lpp_decode(data);
}

bool LPP_Client::connect(const std::string& host, int port, bool use_ssl, CellID supl_cell) {
    // Initialize and connect to the location server
    if (!mSUPL->connect(host, port, use_ssl)) {
        return false;
    }

    // Send SUPL-START request
    if (!supl_start(supl_cell)) {
        mSUPL->disconnect();
        return false;
    }

    // Wait for SUPL-RESPONSE with slp session
    if (!supl_response()) {
        mSUPL->disconnect();
        return false;
    }

    // Send SUPL-POSINIT
    if (!supl_send_posinit(supl_cell)) {
        mSUPL->disconnect();
        return false;
    }

    connected = true;

    // Old servers requires a few messages in the begining of the LPP session.
    struct timespec timeout;
    timeout.tv_sec  = 0;
    timeout.tv_nsec = 1000000 * 1000;  // 1 sec
    nanosleep(&timeout, NULL);

    process();

    return true;
}

bool LPP_Client::disconnect() {
    connected = false;
    return mSUPL->disconnect();
}

LPP_Transaction LPP_Client::new_transaction() {
    LPP_Transaction transaction;
    transaction.id        = transaction_counter;
    transaction.end       = 0;
    transaction.initiator = 0;

    transaction_counter = (transaction_counter + 1) % 256;
    return transaction;
}

bool LPP_Client::process_message(LPP_Message* message, LPP_Transaction* transaction) {
    auto periodic_id = lpp_get_periodic_id(message);
    if (lpp_is_provide_assistance_data(message)) {
        if ((periodic_id == main_request || transaction->id == main_request_transaction.id) &&
            main_request >= 0) {
            if (main_request_callback) {
                main_request_callback(this, transaction, message, main_request_userdata);
            }
        }
        if (agnss_request_callback && transaction->id == agnss_request_transaction.id) {
            agnss_request_callback(this, transaction, message, agnss_request_userdata);
        }
        return true;
    } else if (lpp_is_request_capabilities(message)) {
        transaction->end = true;

        auto message = lpp_provide_capabilities(transaction, mEnableSegmentation);
        if (!supl_send(message)) {
            lpp_destroy(message);
            disconnect();
            return false;
        }

        lpp_destroy(message);
    } else if (lpp_is_request_location_information(message)) {
        return handle_request_location_information(message, transaction);
    }

    return false;
}

bool LPP_Client::process() {
    if (!mSUPL->is_connected()) {
        return false;
    }

    if (provide_li.type >= 0) {
        using namespace std::chrono;
        auto current  = system_clock::now();
        auto duration = current - provide_li.last;
        if (duration > std::chrono::seconds(1)) {
            handle_provide_location_information(&provide_li);
            provide_li.last = current - (duration - std::chrono::seconds(1));
        }
    }

    int timeout = -1;

    std::vector<LPP_Message*> messages;
    if (!supl_receive(messages, timeout)) {
        if (!mSUPL->is_connected()) {
            return false;
        }

        return true;
    }

    for (auto& message : messages) {
        LPP_Transaction transaction;
        if (!lpp_harvest_transaction(&transaction, message)) {
            lpp_destroy(message);
            continue;
        }

        process_message(message, &transaction);
        lpp_destroy(message);
    }

    return true;
}

bool LPP_Client::wait_for_assistance_data_response(LPP_Transaction* transaction) {
    int  timeout_seconds = 2;
    bool ok              = false;
    auto last            = time(NULL);
    while (time(NULL) - last < timeout_seconds && !ok) {
        std::vector<LPP_Message*> messages;
        if (!supl_receive(messages, timeout_seconds * 1000)) {
            disconnect();
            return false;
        }

        for (auto& message : messages) {
            LPP_Transaction message_transaction;
            if (!lpp_harvest_transaction(&message_transaction, message)) {
                lpp_destroy(message);
                continue;
            }

            process_message(message, &message_transaction);

            // NOTE: message_transaction.initiator == transaction->initiator
            // cannot be use as old server didn't correctly handle initiator
            if (message_transaction.id == transaction->id) {
                if (message && message->lpp_MessageBody &&
                    message->lpp_MessageBody->present == LPP_MessageBody_PR_c1 &&
                    message->lpp_MessageBody->choice.c1.present ==
                        LPP_MessageBody__c1_PR_provideAssistanceData) {
                    ok = true;
                } else {
                    last -= 100000;
                }
            }

            lpp_destroy(message);
        }
    }

    return ok;
}

LPP_Client::AD_Request LPP_Client::request_assistance_data(CellID cell, void* userdata,
                                                           AD_Callback callback) {
    if (main_request != AD_REQUEST_INVALID) return AD_REQUEST_INVALID;

    auto periodic_id = 1;  // TODO(ewasjon): support multiple requests
    auto transaction = new_transaction();
    auto message     = lpp_request_assistance_data(&transaction, cell, periodic_id, 1);
    if (!supl_send(message)) {
        lpp_destroy(message);
        disconnect();
        return AD_REQUEST_INVALID;
    }

    lpp_destroy(message);

    main_request             = (AD_Request)periodic_id;
    main_request_callback    = callback;
    main_request_userdata    = userdata;
    main_request_transaction = transaction;
    if (!wait_for_assistance_data_response(&transaction)) {
        main_request = AD_REQUEST_INVALID;
        return AD_REQUEST_INVALID;
    }

    return (AD_Request)periodic_id;
}

LPP_Client::AD_Request LPP_Client::request_assistance_data_ssr(CellID cell, void* userdata,
                                                               AD_Callback callback) {
    if (main_request != AD_REQUEST_INVALID) return AD_REQUEST_INVALID;

    auto periodic_id = 1;  // TODO(ewasjon): support multiple requests
    auto transaction = new_transaction();
    auto message     = lpp_request_assistance_data_ssr(&transaction, cell, periodic_id, 1);
    if (!supl_send(message)) {
        lpp_destroy(message);
        disconnect();
        return AD_REQUEST_INVALID;
    }

    lpp_destroy(message);

    main_request             = (AD_Request)periodic_id;
    main_request_callback    = callback;
    main_request_userdata    = userdata;
    main_request_transaction = transaction;
    if (!wait_for_assistance_data_response(&transaction)) {
        main_request = AD_REQUEST_INVALID;
        return AD_REQUEST_INVALID;
    }

    return (AD_Request)periodic_id;
}

bool LPP_Client::request_agnss(CellID cell, void* userdata, AD_Callback callback) {
    auto request_agnss_for = [this, cell, userdata, callback](long gnss_id) {
        auto transaction = new_transaction();
        auto message     = lpp_request_agnss(&transaction, cell, gnss_id);
        if (!supl_send(message)) {
            lpp_destroy(message);
            disconnect();
            return false;
        }

        lpp_destroy(message);

        agnss_request_callback    = callback;
        agnss_request_userdata    = userdata;
        agnss_request_transaction = transaction;
        if (!wait_for_assistance_data_response(&transaction)) {
            return false;
        }

        return true;
    };

    request_agnss_for(GNSS_ID__gnss_id_gps);
    request_agnss_for(GNSS_ID__gnss_id_glonass);
    request_agnss_for(GNSS_ID__gnss_id_galileo);
    request_agnss_for(GNSS_ID__gnss_id_bds);
    agnss_request_callback = nullptr;

    return true;
}

bool LPP_Client::update_assistance_data(AD_Request id, CellID cell) {
    if (id != main_request) return false;
    if (id == AD_REQUEST_INVALID) return false;

    auto transaction = new_transaction();
    auto message     = lpp_request_assistance_data(&transaction, cell, (long)id, 1);
    if (!supl_send(message)) {
        disconnect();
        return false;
    }

    main_request_transaction = transaction;
    if (!wait_for_assistance_data_response(&transaction)) {
        return false;
    }

    return true;
}

bool LPP_Client::cancel_assistance_data(AD_Request id) {
    if (id != main_request) return false;
    return false;
}

void LPP_Client::provide_location_information_callback(void*                    userdata,
                                                       LPP_Client::PLI_Callback callback) {
    pli_callback = callback;
    pli_userdata = userdata;
}

void LPP_Client::provide_ecid_callback(void* userdata, LPP_Client::PECID_Callback callback) {
    pecid_callback = callback;
    pecid_userdata = userdata;
}

bool LPP_Client::handle_request_location_information(LPP_Message*     message,
                                                     LPP_Transaction* transaction) {
    switch (lpp_get_request_location_information_type(message)) {
    case LocationInformationType_locationEstimateRequired:
        provide_li.type        = LocationInformationType_locationEstimateRequired;
        provide_li.last        = std::chrono::system_clock::now();
        provide_li.transaction = *transaction;
        return true;
    case LocationInformationType_locationMeasurementsRequired:
        provide_li.type        = LocationInformationType_locationMeasurementsRequired;
        provide_li.last        = std::chrono::system_clock::now();
        provide_li.transaction = *transaction;
        return true;
    case LocationInformationType_locationMeasurementsPreferred:
        provide_li.type        = LocationInformationType_locationMeasurementsPreferred;
        provide_li.last        = std::chrono::system_clock::now();
        provide_li.transaction = *transaction;
        return true;
    case LocationInformationType_locationEstimatePreferred:
        provide_li.type        = LocationInformationType_locationEstimatePreferred;
        provide_li.last        = std::chrono::system_clock::now();
        provide_li.transaction = *transaction;
        return true;
    }

    return false;
}

bool LPP_Client::handle_provide_location_information(LPP_Client::ProvideLI* pli) {
    LPP_Message* message = NULL;
    if (pli->type == LocationInformationType_locationEstimateRequired) {
        LocationInformation li{};
        HaGnssMetrics       metrics{};
        bool                has_information = false;
        if (pli_callback && pli_callback(li, metrics, pli_userdata)) {
            has_information = true;
        }

        message = lpp_PLI_location_estimate(&pli->transaction, &li, has_information);
    } else if (pli->type == LocationInformationType_locationMeasurementsRequired) {
        ECIDInformation ecid{};
        bool            has_information = false;
        if (!pecid_callback || !pecid_callback(ecid, pecid_userdata)) {
            has_information = true;
        }

        message = lpp_PLI_location_measurements(&pli->transaction, &ecid, has_information);
    } else {
        return false;
    }

    assert(message);
    if (!supl_send(message)) {
        return false;
    }

    return false;
}