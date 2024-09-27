#include "generator.h"
#include <utility/time.h>

std::vector<std::unique_ptr<SPARTN_Message>>
SPARTN_Generator::generate(LPP_Message const* const lpp_message, int8_t const qualityoveride,
                           bool const ublox_clock_correction, bool const force_continuity) {
    if (!lpp_message->lpp_MessageBody) {
        std::cout << "[INFO] no message body!" << std::endl;
        return {};
    }

    if (lpp_message->lpp_MessageBody->present != LPP_MessageBody_PR_c1) {
        std::cout << "[INFO] no c1!" << std::endl;
        return {};
    }

    if (lpp_message->lpp_MessageBody->choice.c1.present !=
        LPP_MessageBody__c1_PR_provideAssistanceData) {
        std::cout << "[INFO] no provideAssistanceData!" << std::endl;
        return {};
    }

    auto& provide_assistance_data_base =
        lpp_message->lpp_MessageBody->choice.c1.choice.provideAssistanceData;
    if (provide_assistance_data_base.criticalExtensions.present !=
        ProvideAssistanceData__criticalExtensions_PR_c1) {
        std::cout << "[INFO] no c1!" << std::endl;
        return {};
    }

    if (provide_assistance_data_base.criticalExtensions.choice.c1.present !=
        ProvideAssistanceData__criticalExtensions__c1_PR_provideAssistanceData_r9) {
        std::cout << "[INFO] no provideAssistanceData_r9!" << std::endl;
        return {};
    }

    this->prov_ass_data_ =
        &provide_assistance_data_base.criticalExtensions.choice.c1.choice.provideAssistanceData_r9;
    this->qualityoveride_        = qualityoveride;
    this->ublox_clock_correction = ublox_clock_correction;
    this->force_continuity       = force_continuity;

    auto provide_assistance_data = this->prov_ass_data_->a_gnss_ProvideAssistanceData;
    if (!provide_assistance_data) {
        std::cout << "[INFO] no a_gnss_ProvideAssistanceData!" << std::endl;
        return {};
    }

    std::cout << "[INFO] Generating messages" << std::endl;
    std::vector<std::unique_ptr<SPARTN_Message>> messages = {};

    this->ocb_corrections_ = SPARTN_Generator::group_ocb_corrections(
        this->prov_ass_data_->a_gnss_ProvideAssistanceData->gnss_GenericAssistData);

    this->hpac_corrections_ = group_hpac_corrections(
        this->prov_ass_data_->a_gnss_ProvideAssistanceData->gnss_GenericAssistData,
        this->prov_ass_data_->a_gnss_ProvideAssistanceData->gnss_CommonAssistData);

    this->intersect_svs();

    SPARTN_Generator::generate_ocb_message(messages);
    SPARTN_Generator::generate_gad_message(messages);
    SPARTN_Generator::generate_hpac_messages(messages);

    return messages;
}

void SPARTN_Generator::intersect_svs() {
    std::map<uint16_t, std::set<uint16_t>> ocb_svs      = {};
    std::set<uint16_t>                     ocb_gnss_ids = {};
    for (auto const& ocb_corr : this->ocb_corrections_) {
        uint16_t const gnss_id = ocb_corr.gnss_id.gnss_id;
        ocb_gnss_ids.insert(gnss_id);
        if (ocb_svs.find(gnss_id) == ocb_svs.end()) {
            ocb_svs[gnss_id] = {};
        }

        if (ocb_corr.orbit_corrs) {
            auto const& orb_corrs = ocb_corr.orbit_corrs->ssr_OrbitCorrectionList_r15.list;
            for (int i = 0; i < orb_corrs.count; i++) {
                ocb_svs[gnss_id].insert(orb_corrs.array[i]->svID_r15.satellite_id);
            }
        }
        if (ocb_corr.clock_corrs) {
            auto const& clock_corrs = ocb_corr.clock_corrs->ssr_ClockCorrectionList_r15.list;
            for (int i = 0; i < clock_corrs.count; i++) {
                ocb_svs[gnss_id].insert(clock_corrs.array[i]->svID_r15.satellite_id);
            }
        }
        if (ocb_corr.code_bias_corrs) {
            auto const& code_bias_corrs = ocb_corr.code_bias_corrs->ssr_CodeBiasSatList_r15.list;
            for (int i = 0; i < code_bias_corrs.count; i++) {
                ocb_svs[gnss_id].insert(code_bias_corrs.array[i]->svID_r15.satellite_id);
            }
        }
        if (ocb_corr.phase_bias_corrs) {
            auto const& phase_bias_corrs = ocb_corr.phase_bias_corrs->ssr_PhaseBiasSatList_r16.list;
            for (int i = 0; i < phase_bias_corrs.count; i++) {
                ocb_svs[gnss_id].insert(phase_bias_corrs.array[i]->svID_r16.satellite_id);
            }
        }
    }

    std::map<uint16_t, std::set<uint16_t>> hpac_svs      = {};
    std::set<uint16_t>                     hpac_gnss_ids = {};
    for (auto const& hpac_corr : this->hpac_corrections_) {
        uint16_t const gnss_id = hpac_corr.gnss_id.gnss_id;
        hpac_gnss_ids.insert(gnss_id);
        if (hpac_svs.find(gnss_id) == hpac_svs.end()) {
            hpac_svs[gnss_id] = {};
        }

        if (hpac_corr.stec_corrections) {
            auto const& stec_corrs = hpac_corr.stec_corrections->stec_SatList_r16.list;

            for (int i = 0; i < stec_corrs.count; i++) {
                hpac_svs[gnss_id].insert(stec_corrs.array[i]->svID_r16.satellite_id);
            }
        }
    }

    std::vector<uint16_t> common_gnss = {};
    std::set_intersection(ocb_gnss_ids.begin(), ocb_gnss_ids.end(), hpac_gnss_ids.begin(),
                          hpac_gnss_ids.end(), std::back_inserter(common_gnss));

    for (auto const& gnss_id : common_gnss) {
        auto const& ocb  = ocb_svs[gnss_id];
        auto const& hpac = hpac_svs[gnss_id];

        this->sv_intersections_[gnss_id] = {};
        auto& inter                      = this->sv_intersections_[gnss_id];

        std::set_intersection(ocb.begin(), ocb.end(), hpac.begin(), hpac.end(),
                              std::back_inserter(inter));
    }
}

void SPARTN_Generator::generate_ocb_message(
    std::vector<std::unique_ptr<SPARTN_Message>>& messages) {
    if (!this->prov_ass_data_->a_gnss_ProvideAssistanceData->gnss_GenericAssistData) {
        std::cout << "[INFO] no generic assistance data!" << std::endl;
        return;
    }

    auto const& gen_ass =
        this->prov_ass_data_->a_gnss_ProvideAssistanceData->gnss_GenericAssistData;

    auto const set_size = this->ocb_corrections_.size();
    size_t     set_num  = 0;
    for (auto const& this_assist_data : this->ocb_corrections_) {
        std::unique_ptr<SPARTN_Message>        this_message(new SPARTN_Message);
        std::unique_ptr<SPARTN_Message_Header> this_header(new SPARTN_Message_Header);
        auto&                                  satellite_blocks = this_message->data;

        auto const        gnss_id         = (GNSS_ID__gnss_id)this_assist_data.gnss_id.gnss_id;
        std::time_t const generation_time = std::time(nullptr);

        std::cout << std::endl << "===== NEW OCB MESSAGE [";
        std::cout << Constants::constellation_names.at(gnss_id);
        std::cout << "] =====" << std::endl;

        auto const& orb_corrs = this_assist_data.orbit_corrs;

        auto const& clk_corrs = this_assist_data.clock_corrs;

        auto const& code_corrs = this_assist_data.code_bias_corrs;

        auto const& phase_corrs = this_assist_data.phase_bias_corrs;

        std::map<uint16_t, OCBSatCorrection> const all_sv_ocbs =
            SPARTN_Generator::get_ocb_for_svs(orb_corrs, clk_corrs, code_corrs, phase_corrs);

        // NOTE: remove "unallowed" satellites, if we don't do this then the
        // bitmask will be incorrect.
        std::map<uint16_t, OCBSatCorrection> sv_ocbs     = {};
        auto const&                          allowed_svs = this->sv_intersections_.at(gnss_id);
        for (auto const& sv_ocb : all_sv_ocbs) {
            if (std::count(allowed_svs.begin(), allowed_svs.end(), sv_ocb.first) != 0) {
                sv_ocbs[sv_ocb.first] = sv_ocb.second;
            } else {
                std::cout << "[INFO] SV: " << sv_ocb.first << " is not allowed" << std::endl;
            }
        }

        GNSS_SystemTime const* const gnss_time =
            SPARTN_Generator::get_gnss_systemtime(this_assist_data);

        SPARTN_Generator::add_time_to_message(this_message, gnss_time, gnss_id);

        this_message->message_type = 0;

        int8_t const subtype = Constants::constellation_subtypes.at(gnss_id);
        if (subtype == -1) {
            std::cout << "[ERR] unsupported GNSS: " << gnss_id << std::endl;
            continue;
        }
        this_message->message_sub_type = subtype;

        {
            SPARTN_Generator::add_field_to_header(this_header, 5, 9, this_assist_data.iod_ssr);

            // end of OCB set for epoch
            SPARTN_Generator::add_field_to_header(this_header, 10, 1,
                                                  (int64_t)(++set_num == set_size));

            SPARTN_Generator::add_field_to_header(this_header, 69, 1, 0);
            SPARTN_Generator::add_field_to_header(this_header, 8, 1, 0);

            if (SPARTN_Generator::check_is_itrf(orb_corrs)) {
                SPARTN_Generator::add_field_to_header(this_header, 9, 1, 0);
            } else {
                std::cout << "[INFO] Got unsupported reference datum" << std::endl;
                continue;
            }

            uint8_t const ephm_id            = Constants::ephemeris_id.at(gnss_id);
            uint8_t const ephm_bit_count     = Constants::ephemeris_bit_count.at(gnss_id);
            uint8_t const ephm_bits          = Constants::ephemeris_bits.at(gnss_id);
            uint8_t const sat_mask_id        = Constants::satellite_mask_ids.at(gnss_id);
            uint8_t const sat_mask_bit_count = Constants::satellite_mask_sizes.at(gnss_id);

            auto const sat_mask_bits =
                SPARTN_Generator::get_satellite_mask_from_ocbs(sv_ocbs, sat_mask_bit_count, 2);

            SPARTN_Generator::add_field_to_header(this_header, ephm_id, ephm_bit_count, ephm_bits);
            std::unique_ptr<SPARTN_Field> sat_mask_field(
                new SPARTN_Field(sat_mask_id, sat_mask_bit_count, sat_mask_bits));
            this_header->fields.push(std::move(sat_mask_field));
        }

        this_message->message_header = std::move(this_header);

        for (auto const& sv_ocb : sv_ocbs) {
            auto const svid = sv_ocb.first;

            std::unique_ptr<SPARTN_Block> this_sat_block(new SPARTN_Block);
            std::unique_ptr<SPARTN_Block> this_sat_block_pre(new SPARTN_Block);
            auto const&                   ocb_corr = sv_ocb.second;
            uint8_t                       ocb_mask = 0;

            if (orb_corrs && this->generate_orbit_block(this_sat_block_pre, std::get<0>(ocb_corr),
                                                        gnss_id, generation_time)) {
                ocb_mask |= 4;
            } else {
                std::cout << "[INFO] no orbit corrections for SV: " << svid << std::endl;
            }

            if (clk_corrs && this->generate_clock_block(
                                 this_sat_block_pre, std::get<1>(ocb_corr), this_assist_data.ura,
                                 gnss_id, generation_time, gnss_time,
                                 clk_corrs->ssrUpdateInterval_r15, clk_corrs->iod_ssr_r15)) {
                ocb_mask |= 2;
            } else {
                std::cout << "[INFO] no clock corrections for SV: " << svid << std::endl;
            }

            if (code_corrs && this->generate_bias_block(this_sat_block_pre, std::get<2>(ocb_corr),
                                                        std::get<3>(ocb_corr), gnss_id)) {
                ocb_mask |= 1;
            }

            SPARTN_Generator::add_field_to_block(this_sat_block, 13, 1, 0);

            SPARTN_Generator::add_field_to_block(this_sat_block, 14, 3, ocb_mask);

            if (this->last_generation_time_.find(gnss_id) == this->last_generation_time_.end()) {
                this->last_generation_time_[gnss_id] = {};
            }

            if (this->last_generation_time_.at(gnss_id).find(svid) ==
                this->last_generation_time_.at(gnss_id).end()) {
                this->last_generation_time_.at(gnss_id)[svid] = generation_time;
            }

            auto const since_last_generation =
                generation_time - this->last_generation_time_.at(gnss_id).at(svid);
            uint8_t spartn_continuity = 7;

            for (uint8_t i = 1; i < Constants::spartn_continuity_num; i++) {
                if (since_last_generation < Constants::allowed_iode_continuity_values[i]) {
                    spartn_continuity = i - 1;
                    break;
                }
            }

            if (this->force_continuity) {
                SPARTN_Generator::add_field_to_block(this_sat_block, 15, 3, 7 /* 320 secs */);
            } else {
                SPARTN_Generator::add_field_to_block(this_sat_block, 15, 3, spartn_continuity);
            }

            if (spartn_continuity == 7) {
                this->last_generation_time_.at(gnss_id).at(svid) = generation_time;
            }

            /*
             * Fields come before blocks in the satellite block, but we need to
             * know the result of the block to fill in the OCB mask. So store
             * the blocks in a temp queue and figure out the mask, add the
             * fields then move the data from the temp queue to the actual one.
             */
            while (!this_sat_block_pre->data.empty()) {
                this_sat_block->data.push(std::move(this_sat_block_pre->data.front()));
                this_sat_block_pre->data.pop();
            }

            satellite_blocks.push(std::move(this_sat_block));
        }

        messages.push_back(std::move(this_message));

        // TODO: REMOVE
        // messages.clear();
        // printf("-----------------------------------------------------------\n");
    }
}

bool SPARTN_Generator::generate_orbit_block(
    std::unique_ptr<SPARTN_Block> const&                 sat_block,
    SSR_OrbitCorrectionSatelliteElement_r15 const* const orbit_correction,
    GNSS_ID__gnss_id const gnss_id, std::time_t const generation_time) {
    if (!orbit_correction) {
        return false;
    }

    std::unique_ptr<SPARTN_Block> orbit_block(new SPARTN_Block);

    uint8_t const iod_id       = Constants::iode_ids.at(gnss_id);
    uint8_t const iod_bit_size = Constants::iode_bit_size.at(gnss_id);

    auto const& iod_bit_string = orbit_correction->iod_r15;
    long        iod_dec        = 0;
    for (size_t i = 0; i < iod_bit_string.size; i++) {
        iod_dec <<= 8;
        iod_dec |= iod_bit_string.buf[i];
    }
    iod_dec >>= iod_bit_string.bits_unused;
    iod_dec >>= 3;  // TODO: is this right?

    auto const svid = orbit_correction->svID_r15.satellite_id;
    if (this->iods_.find(gnss_id) == this->iods_.end()) {
        this->iods_[gnss_id] = {};
    }

    if (this->iods_.at(gnss_id).find(svid) == this->iods_.at(gnss_id).end()) {
        this->iods_.at(gnss_id)[svid] = {iod_dec, generation_time};
    }

    if (this->iods_.at(gnss_id).at(svid).first != iod_dec) {
        this->iods_.at(gnss_id)[svid] = {iod_dec, generation_time};
    }

    SPARTN_Generator::add_field_to_block(orbit_block, iod_id, iod_bit_size, iod_dec);

    double val_dec = SPARTN_Generator::decode_value_lpp(orbit_correction->delta_radial_r15,
                                                        Constants::orbit_lpp_radial_res);

    if (SPARTN_Generator::check_within_range(val_dec, Constants::orbit_clock_spartn_rng_min)) {
        SPARTN_Generator::add_field_to_block(
            orbit_block, 20, 14,
            SPARTN_Generator::encode_value_spartn(val_dec, Constants::orbit_clock_spartn_rng_min,
                                                  Constants::orbit_clock_spartn_res));
    } else {
        std::cout << "[INFO] Radial correction is out of range: " << val_dec << std::endl;
        return false;
    }

    val_dec = SPARTN_Generator::decode_value_lpp(orbit_correction->delta_AlongTrack_r15,
                                                 Constants::orbit_lpp_along_res);

    if (SPARTN_Generator::check_within_range(val_dec, Constants::orbit_clock_spartn_rng_min)) {
        SPARTN_Generator::add_field_to_block(
            orbit_block, 20, 14,
            SPARTN_Generator::encode_value_spartn(val_dec, Constants::orbit_clock_spartn_rng_min,
                                                  Constants::orbit_clock_spartn_res));
    } else {
        std::cout << "[INFO] Along correction is out of range: " << val_dec << std::endl;
        return false;
    }

    val_dec = SPARTN_Generator::decode_value_lpp(orbit_correction->delta_CrossTrack_r15,
                                                 Constants::orbit_lpp_cross_res);

    if (SPARTN_Generator::check_within_range(val_dec, Constants::orbit_clock_spartn_rng_min)) {
        SPARTN_Generator::add_field_to_block(
            orbit_block, 20, 14,
            SPARTN_Generator::encode_value_spartn(val_dec, Constants::orbit_clock_spartn_rng_min,
                                                  Constants::orbit_clock_spartn_res));
    } else {
        std::cout << "[INFO] Cross correction is out of range: " << val_dec << std::endl;
        return false;
    }

    SPARTN_Generator::add_field_to_block(orbit_block, 21, 0, 0);

    SPARTN_Generator::add_block_to_block(sat_block, orbit_block);

    return true;
}

bool SPARTN_Generator::generate_clock_block(
    std::unique_ptr<SPARTN_Block> const&                 sat_block,
    SSR_ClockCorrectionSatelliteElement_r15 const* const clock_correction,
    GNSS_SSR_URA_r16 const* const URA, GNSS_ID__gnss_id const gnss_id,
    std::time_t const generation_time, GNSS_SystemTime const* const gnss_systemtime,
    long const update_interval, uint8_t const iode) {
    if (!clock_correction) {
        return false;
    }

    std::unique_ptr<SPARTN_Block> clock_block(new SPARTN_Block);

    auto const                             svid = clock_correction->svID_r15.satellite_id;
    std::pair<int32_t, std::time_t> const& iod_time =
        this->iods_.find(gnss_id)->second.find(svid)->second;

    auto const since_last_iod_change = generation_time - iod_time.second;
    uint8_t    spartn_iode           = 7;

    for (uint8_t i = 1; i < Constants::spartn_continuity_num; i++) {
        if (since_last_iod_change < Constants::allowed_iode_continuity_values[i]) {
            spartn_iode = i - 1;
            break;
        }
    }

    if (this->force_continuity) {
        SPARTN_Generator::add_field_to_block(clock_block, 22, 3, 7 /* 320 secs */);
    } else {
        SPARTN_Generator::add_field_to_block(clock_block, 22, 3, spartn_iode);
    }

    if (spartn_iode == 7) {
        this->iods_.at(gnss_id)[svid] = {iode, generation_time};
    }

    /** +---------------------------------------------------------------------+
     *  |                      Clock Correction Calculation                   |
     *  +---------------------------------------------------------------------+
     *  | LPP gives clock corrections as a polynomial, where-as SPARTN uses a |
     *  | single lump value. Therefore we must calculate the lump value from  |
     *  | the given polynomial.                                               |
     *  |                                                                     |
     *  | delta_c = c0 + c1(t - t0) + [c2(t-t0)] ^ 2                          |
     *  |   where: c0, c1, c2 are coefficients given by the LPP message,      |
     *  |          t          is the generation time of the LPP message,      |
     *  |          t0         is the reference time.                          |
     *  |                                                                     |
     *  | t0 = epochTime + (0.5 * ssrUpdateInterval)                          |
     *  +---------------------------------------------------------------------+
     */

    double const c0 = SPARTN_Generator::decode_value_lpp(clock_correction->delta_Clock_C0_r15,
                                                         Constants::clock_c0_lpp_res);
    double       c1 = 0;
    double       c2 = 0;

    if (clock_correction->delta_Clock_C1_r15) {
        if (*clock_correction->delta_Clock_C1_r15 != 0) {
            c1 = SPARTN_Generator::decode_value_lpp(*clock_correction->delta_Clock_C1_r15,
                                                    Constants::clock_c1_lpp_res);
        }
    }

    if (clock_correction->delta_Clock_C2_r15) {
        if (*clock_correction->delta_Clock_C2_r15 != 0) {
            c2 = SPARTN_Generator::decode_value_lpp(*clock_correction->delta_Clock_C2_r15,
                                                    Constants::clock_c2_lpp_res);
        }
    }

    // TODO: Why are we rounding TimeOfDayFrac_msec?
    long const tod = gnss_systemtime->gnss_TimeOfDay +
                     (gnss_systemtime->gnss_TimeOfDayFrac_msec ?
                          *gnss_systemtime->gnss_TimeOfDayFrac_msec >= 500 ? 1 : 0 :
                          0);

    auto standard_day_number =
        (double)SPARTN_LPP_Time::standard_day_number(gnss_systemtime->gnss_DayNumber, gnss_id);
    auto epoch_time = (standard_day_number * Constants::seconds_in_day) + (double)tod;

    auto update_interval_in_seconds = Constants::update_intervals.at(update_interval);
    auto reference_time             = epoch_time + (0.5 * update_interval_in_seconds);

    auto delta_gen_ref = reference_time - reference_time;  // TODO: what to do here?
    auto delta_c       = c0 + (c1 * delta_gen_ref) + pow(c2 * delta_gen_ref, 2);

    if (this->ublox_clock_correction) {
        delta_c = -delta_c;
    }

    if (SPARTN_Generator::check_within_range(delta_c, Constants::orbit_clock_spartn_rng_min)) {
        SPARTN_Generator::add_field_to_block(
            clock_block, 20, 14,
            SPARTN_Generator::encode_value_spartn(delta_c, Constants::orbit_clock_spartn_rng_min,
                                                  Constants::orbit_clock_spartn_res));
    } else {
        std::cout << "[INFO] Clock correction is out of range: " << delta_c << std::endl;
        return false;
    }

    if (URA == nullptr) {
        SPARTN_Generator::add_field_to_block(clock_block, 24, 3,
                                             this->qualityoveride_ < 0 ? 0 :
                                             7 < this->qualityoveride_ ? 7 :
                                                                         this->qualityoveride_);
    } else {
        std::cout << "[INFO] URA present" << std::endl;
        auto const& sats = URA->ssr_URA_SatList_r16.list;

        for (int i = 0; i < sats.count; i++) {
            SSR_URA_SatElement_r16 const* const this_sat = sats.array[i];

            if (this_sat->svID_r16.satellite_id == clock_correction->svID_r15.satellite_id) {
                auto const ura_bits  = (*this_sat->ssr_URA_r16.buf) >> this_sat->ssr_URA_r16.size;
                auto const ura_class = ura_bits >> 3;
                auto const ura_value = ura_bits & 3;
                auto const q = ((pow(3, ura_class)) * ((1 + ((double)ura_value))) - 1) / 100;

                for (size_t ura_idx = 0; ura_idx < Constants::ura_values.size(); ura_idx++) {
                    if (q <= Constants::ura_values[ura_idx]) {
                        SPARTN_Generator::add_field_to_block(clock_block, 24, 3, (int64_t)ura_idx);
                        break;
                    }
                }
                break;
            }
        }

        // TODO: what to do if we don't find the SV?
    }

    SPARTN_Generator::add_block_to_block(sat_block, clock_block);

    return true;
}

bool SPARTN_Generator::generate_bias_block(
    std::unique_ptr<SPARTN_Block> const&     sat_block,
    SSR_CodeBiasSatElement_r15 const* const  sat_code_biases,
    SSR_PhaseBiasSatElement_r16 const* const sat_phase_biases, GNSS_ID__gnss_id const gnss_id) {
    std::unique_ptr<SPARTN_Block> bias_block(new SPARTN_Block);

    SSR_CodeBiasSignalList_r15 const* code_biases =
        sat_code_biases ? &sat_code_biases->ssr_CodeBiasSignalList_r15 : nullptr;

    std::map<long, uint8_t>  lpp_to_spartn_bias = {};
    Constants::bias_ids_size bis;

    switch (gnss_id) {
    case GNSS_ID__gnss_id_gps: {
        lpp_to_spartn_bias = Constants::gps_lpp_to_spartn_bias;
        bis                = Constants::gps_bias_ids_size;
        break;
    }
    case GNSS_ID__gnss_id_qzss: {
        lpp_to_spartn_bias = Constants::qzss_lpp_to_spartn_bias;
        bis                = Constants::qzss_bias_ids_size;
        break;
    }
    case GNSS_ID__gnss_id_galileo: {
        lpp_to_spartn_bias = Constants::gal_lpp_to_spartn_bias;
        bis                = Constants::gal_bias_ids_size;
        break;
    }
    case GNSS_ID__gnss_id_glonass: {
        lpp_to_spartn_bias = Constants::glo_lpp_to_spartn_bias;
        bis                = Constants::glo_bias_ids_size;
        break;
    }
    case GNSS_ID__gnss_id_bds: {
        lpp_to_spartn_bias = Constants::bds_lpp_to_spartn_bias;
        bis                = Constants::bds_bias_ids_size;
        break;
    }
    default: {
        std::cout << "[INFO] bias, unsupported GNSS: " << gnss_id << std::endl;
        return false;
    }
    }

    this->generate_phase_bias_block(bias_block, sat_phase_biases, lpp_to_spartn_bias, bis, gnss_id);

    SPARTN_Generator::generate_code_bias_block(bias_block, code_biases, lpp_to_spartn_bias, bis);

    SPARTN_Generator::add_block_to_block(sat_block, bias_block);

    return true;
}

void SPARTN_Generator::generate_phase_bias_block(
    std::unique_ptr<SPARTN_Block> const&     bias_block,
    SSR_PhaseBiasSatElement_r16 const* const phase_biases,
    std::map<long, uint8_t> lpp_to_spartn_bias, Constants::bias_ids_size const bis,
    GNSS_ID__gnss_id const gnss_id) {
    if (phase_biases == nullptr) {
        std::cout << "[INFO] no phase bias for SV" << std::endl;

        SPARTN_Generator::add_field_to_block(bias_block, bis.phase_id, bis.bit_count, 0);

        return;
    }

    auto const phase_list = phase_biases->ssr_PhaseBiasSignalList_r16.list;
    std::vector<std::pair<long, SSR_PhaseBiasSignalElement_r16 const*>> sig_corrs = {};

    for (int i = 0; i < phase_list.count; i++) {
        SSR_PhaseBiasSignalElement_r16 const* const this_phase_bias = phase_list.array[i];

        uint8_t const lpp_code_bias_type =
            SPARTN_Generator::get_signalid(this_phase_bias->signal_and_tracking_mode_ID_r16);

        if (lpp_to_spartn_bias.find(lpp_code_bias_type) == lpp_to_spartn_bias.end()) {
            std::cout << "[INFO] got unsupported signal for phase bias: " << (int)lpp_code_bias_type
                      << std::endl;
            continue;
        }

        long const code_type = lpp_to_spartn_bias.at(lpp_code_bias_type);

        sig_corrs.emplace_back(std::make_pair(code_type, this_phase_bias));
    }

    std::bitset<Constants::max_bias_mask_bit_count> phase_bias_mask = 0;

    std::queue<std::unique_ptr<SPARTN_Block>> phase_bias_blocks = {};

    auto const svid = phase_biases->svID_r16.satellite_id;

    static constexpr std::bitset<Constants::max_bias_mask_bit_count> new_sig = 1;
    uint8_t const mask_length                                                = bis.bit_count - 1;
    for (std::pair<long, SSR_PhaseBiasSignalElement_r16 const*> const& this_sig_corr : sig_corrs) {
        auto const* const this_phase_bias = this_sig_corr.second;

        bool fix_flag = false;

        auto const* const integer_indicator = this_phase_bias->phaseBiasIntegerIndicator_r16;

        if (integer_indicator) {
            switch (*integer_indicator) {
            case 0:
            case 1: {
                fix_flag = true;
                break;
            }
            case 2: {
                fix_flag = false;
                break;
            }
            default: {
                std::cout << "[INFO] Unknown phase bias integer indicator: "
                          << *this_phase_bias->phaseBiasIntegerIndicator_r16 << std::endl;
                continue;
            }
            }
        } else {
            fix_flag = true;
        }

        std::unique_ptr<SPARTN_Block> this_phase_bias_block(new SPARTN_Block);

        SPARTN_Generator::add_field_to_block(this_phase_bias_block, 23, 1, (int64_t)fix_flag);

        if (this->discontinuities_.find(gnss_id) == this->discontinuities_.end()) {
            this->discontinuities_[gnss_id] = {};
        }

        if (this->discontinuities_.at(gnss_id).find(svid) ==
            this->discontinuities_.at(gnss_id).end()) {
            this->discontinuities_.at(gnss_id)[svid] = {};
        }

        if (this->discontinuities_.at(gnss_id).at(svid).find(this_sig_corr.first) ==
            this->discontinuities_.at(gnss_id).at(svid).end()) {
            this->discontinuities_.at(gnss_id).at(svid)[this_sig_corr.first] = {-1, 0};
        }

        LastDiscontinuity& this_last_disc =
            this->discontinuities_.at(gnss_id).at(svid).at(this_sig_corr.first);

        if (this_last_disc.discontinuity_indicator !=
            this_phase_bias->phaseDiscontinuityIndicator_r16) {
            // can narrow here because indicator has max of 3
            this_last_disc = {(int8_t)this_phase_bias->phaseDiscontinuityIndicator_r16,
                              std::time(nullptr)};
        }

        auto const since_last_disc = std::time(nullptr) - this_last_disc.update_time;

        uint8_t spartn_continuity = 7;

        for (uint8_t i = 1; i < Constants::spartn_continuity_num; i++) {
            if (since_last_disc < Constants::allowed_iode_continuity_values[i]) {
                spartn_continuity = i - 1;
                break;
            }
        }

        if (this->force_continuity) {
            SPARTN_Generator::add_field_to_block(this_phase_bias_block, 15, 3, 7 /* 320 secs */);
        } else {
            SPARTN_Generator::add_field_to_block(this_phase_bias_block, 15, 3, spartn_continuity);
        }

        if (spartn_continuity == 7) {
            this->discontinuities_.at(gnss_id).at(svid)[this_sig_corr.first] = {-1, 0};
        }

        double const val_dec = SPARTN_Generator::decode_value_lpp(
            this_sig_corr.second->phaseBias_r16, Constants::phase_bias_lpp_res);

        if (SPARTN_Generator::check_within_range(val_dec, Constants::phase_bias_spartn_rng_min)) {
            SPARTN_Generator::add_field_to_block(
                this_phase_bias_block, 20, 14,
                SPARTN_Generator::encode_value_spartn(val_dec, Constants::phase_bias_spartn_rng_min,
                                                      Constants::phase_bias_spartn_res));
        } else {
            continue;
        }
        phase_bias_blocks.push(std::move(this_phase_bias_block));

        phase_bias_mask |= new_sig << (mask_length - this_sig_corr.first - 1);
    }

    SPARTN_Generator::add_field_to_block(bias_block, bis.phase_id, bis.bit_count,
                                         (int64_t)phase_bias_mask.to_ullong());

    while (!phase_bias_blocks.empty()) {
        SPARTN_Generator::add_block_to_block(bias_block, phase_bias_blocks.front());
        phase_bias_blocks.pop();
    }
}

bool SPARTN_Generator::generate_code_bias_block(std::unique_ptr<SPARTN_Block> const&    bias_block,
                                                SSR_CodeBiasSignalList_r15 const* const code_biases,
                                                std::map<long, uint8_t>        lpp_to_spartn_bias,
                                                Constants::bias_ids_size const bis) {
    if (!code_biases) {
        return false;
    }

    std::vector<std::pair<uint8_t, double>> signal_corrs = {};
    for (int i = 0; i < code_biases->list.count; i++) {
        SSR_CodeBiasSignalElement_r15 const* const this_code_bias = code_biases->list.array[i];
        uint8_t                                    spartn_signal_id;

        uint8_t const lpp_code_bias_type =
            SPARTN_Generator::get_signalid(this_code_bias->signal_and_tracking_mode_ID_r15);

        if (lpp_to_spartn_bias.find(lpp_code_bias_type) == lpp_to_spartn_bias.end()) {
            std::cout << "INFO[code]: unsupported signal id: " << (int)lpp_code_bias_type
                      << std::endl;
            continue;
        }

        uint8_t const code_type  = lpp_to_spartn_bias.at(lpp_code_bias_type);
        auto const    correction = SPARTN_Generator::decode_value_lpp(this_code_bias->codeBias_r15,
                                                                      Constants::bias_lpp_res);

        signal_corrs.emplace_back(std::make_pair(code_type, correction));
    }

    std::bitset<Constants::max_bias_mask_bit_count>                  code_bias_mask = 0;
    std::vector<long>                                                encoded_values = {};
    static constexpr std::bitset<Constants::max_bias_mask_bit_count> new_sig        = 1;
    uint8_t const mask_length = bis.bit_count - 1;

    for (auto const& type_corr : signal_corrs) {
        if (SPARTN_Generator::check_within_range(type_corr.second,
                                                 Constants::code_bias_spartn_rng_min)) {
            code_bias_mask |= new_sig << (mask_length - type_corr.first - 1);

            encoded_values.emplace_back(SPARTN_Generator::encode_value_spartn(
                type_corr.second, Constants::code_bias_spartn_rng_min, Constants::bias_spartn_res));
        }
    }

    SPARTN_Generator::add_field_to_block(bias_block, bis.code_id, bis.bit_count,
                                         (int64_t)code_bias_mask.to_ullong());

    for (long const& v : encoded_values) {
        SPARTN_Generator::add_field_to_block(bias_block, 29, 11, v);
    }

    return true;
}

void SPARTN_Generator::generate_hpac_messages(
    std::vector<std::unique_ptr<SPARTN_Message>>& messages) const {
    if (!this->prov_ass_data_->a_gnss_ProvideAssistanceData->gnss_GenericAssistData) {
        std::cout << "[INFO] no generic assistance data!" << std::endl;
        return;
    }

    auto const& gen_ass =
        this->prov_ass_data_->a_gnss_ProvideAssistanceData->gnss_GenericAssistData;
    for (auto const& this_assist_data : this->hpac_corrections_) {
        auto const gnss_id = this_assist_data.gnss_id.gnss_id;

        std::unique_ptr<SPARTN_Message> this_message(new SPARTN_Message);
        this_message->message_type     = 1;
        this_message->message_sub_type = Constants::gnss_id_to_hpac_subtype.at(gnss_id);

        GNSS_SystemTime const* const gnss_time =
            SPARTN_Generator::get_gnss_systemtime(this_assist_data);

        SPARTN_Generator::add_time_to_message(this_message, gnss_time, (GNSS_ID__gnss_id)gnss_id);

        std::unique_ptr<SPARTN_Message_Header> this_header(new SPARTN_Message_Header);
        SPARTN_Generator::add_field_to_header(this_header, 5, 9, this_assist_data.iod_ssr);
        SPARTN_Generator::add_field_to_header(this_header, 68, 4, 0);
        SPARTN_Generator::add_field_to_header(this_header, 69, 1, 0);

        std::cout << std::endl << "==== HPAC MESSAGE [";
        std::cout << Constants::constellation_names.at(gnss_id);
        std::cout << "] =====" << std::endl;

        if (!this_assist_data.correction_points) {
            std::cout << "[INFO] no correction points!" << std::endl;
            continue;
        }

        if (!this_assist_data.stec_corrections) {
            std::cout << "[INFO] no STEC corrections!" << std::endl;
            continue;
        }

        if (!this_assist_data.gridded_corrections) {
            std::cout << "[INFO] no gridded corrections!" << std::endl;
            continue;
        }

        std::unique_ptr<SPARTN_Block> this_atmo_block(new SPARTN_Block);

        if (!SPARTN_Generator::generate_area_data(this_atmo_block, this_assist_data)) {
            continue;
        }
        this->generate_tropo_data(this_atmo_block, this_assist_data.gridded_corrections);
        this->generate_iono_data(this_atmo_block, this_assist_data);

        this_message->data.push(std::move(this_atmo_block));

        SPARTN_Generator::add_field_to_header(
            this_header, 30, 5,
            (int64_t)(this_message->data.size() - 1));  // SPARTN counts from zero here

        this_message->message_header = std::move(this_header);

        messages.push_back(std::move(this_message));
    }
}

bool SPARTN_Generator::generate_area_data(std::unique_ptr<SPARTN_Block> const& atmo_block,
                                          HPACCorrection const&                hpac_corr) {
    std::unique_ptr<SPARTN_Block> area_data_block(new SPARTN_Block);

    CorrectionPoint const& corr_points = *hpac_corr.correction_points;

    SPARTN_Generator::add_field_to_block(area_data_block, 31, 8, corr_points.id);

    uint8_t num_of_grid_points = corr_points.num_of_grid_points;

    SPARTN_Generator::add_field_to_block(area_data_block, 39, 7, num_of_grid_points);

    /* [SPARTN] 0: none 1: poly 2: poly & grid. i.e. if we want to define
     * residuals for the grid, we must provide a poly. The issue is that LPP
     * does not provide a poly for the tropo, so this value can either only be
     * zero or two.
     *
     * This is also for the entire grid, in SPARTN, either every grid point has
     * a poly / poly & grid, or it does not. In LPP, it is per grid point, i.e.
     * one grid point could have just a poly and another could have nothing and
     * so on.
     */
    uint8_t    tropo_block_indicator = 2;
    auto const grid_list             = hpac_corr.gridded_corrections->gridList_r16.list;

    for (int i = 0; i < grid_list.count; i++) {
        if (!grid_list.array[i]->tropospericDelayCorrection_r16) {
            tropo_block_indicator = 0;
            break;
        }
    }

    SPARTN_Generator::add_field_to_block(area_data_block, 40, 2, tropo_block_indicator);

    uint8_t iono_block_indicator = 0;

    if (hpac_corr.stec_corrections) {
        iono_block_indicator++;

        if (hpac_corr.gridded_corrections) {
            iono_block_indicator++;
        }
    }

    SPARTN_Generator::add_field_to_block(area_data_block, 40, 2, iono_block_indicator);

    SPARTN_Generator::add_block_to_block(atmo_block, area_data_block);
    return true;
}

void SPARTN_Generator::generate_tropo_data(
    std::unique_ptr<SPARTN_Block> const&        atmo_block,
    GNSS_SSR_GriddedCorrection_r16 const* const gridded_corrections) const {
    if (!gridded_corrections) {
        std::cout << "[INFO] no tropo data" << std::endl;
        return;
    }

    std::unique_ptr<SPARTN_Block> tropo_data_block(new SPARTN_Block);

    if (!this->generate_tropo_coef(tropo_data_block, gridded_corrections)) {
        return;
    }
    SPARTN_Generator::generate_tropo_grid(tropo_data_block, gridded_corrections);

    SPARTN_Generator::add_block_to_block(atmo_block, tropo_data_block);
}

bool SPARTN_Generator::generate_tropo_coef(
    std::unique_ptr<SPARTN_Block> const&        tropo_data_block,
    GNSS_SSR_GriddedCorrection_r16 const* const gridded_corrections) const {
    std::unique_ptr<SPARTN_Block> tropo_coef_block(new SPARTN_Block);

    auto const grid_list = gridded_corrections->gridList_r16.list;

    /*
     * LPP gives no coefficients for the troposphere but SPARTN requires at
     * least T00, so just give a constant zero.
     */
    SPARTN_Generator::add_field_to_block(tropo_coef_block, 41, 3, 0 /* just T00 */);

    auto const* const qual_indi  = gridded_corrections->troposphericDelayQualityIndicator_r16;
    uint8_t const     qual_class = qual_indi ? *qual_indi->buf >> 3 : 0;
    uint8_t const     qual_value = qual_indi ? *qual_indi->buf & 3 : 0;

    uint8_t tropo_qual = 7;

    if (qual_class == 0 && qual_value == 0) {
        tropo_qual = this->qualityoveride_ < 0 ? 0 :
                     7 < this->qualityoveride_ ? 7 :
                                                 this->qualityoveride_;
    } else if (qual_class <= 1 && qual_value < 1) {
        double const q       = (pow(3, qual_class)) * (1 + ((double)qual_value / 4)) - 1;
        auto         q_meter = q / 1000.0;

        for (uint64_t i = 0; i < Constants::spartn_tropo_qual_num; i++) {
            if (Constants::spartn_tropo_qualities_maximums[i] < q_meter) {
                tropo_qual = i;
                break;
            }
        }
    }

    printf("tropo class=%i, value=%i => qual=%i\n", qual_class, qual_value, tropo_qual);

    SPARTN_Generator::add_field_to_block(tropo_coef_block, 42, 3, tropo_qual);

    /*
     * SPARTN only specifies the average hydrostatic delay, where-as LPP will
     * give a delay for each grid point. Therefore we calculate the average
     * from LPP
     */
    double hydrostatic_delay_average = 0;
    double count                     = 0;
    for (int i = 0; i < grid_list.count; i++) {
        if (!grid_list.array[i]->tropospericDelayCorrection_r16) {
            return false;
        }

        long const this_delay =
            grid_list.array[i]->tropospericDelayCorrection_r16->tropoHydroStaticVerticalDelay_r16;

        auto const val_dec =
            SPARTN_Generator::decode_value_lpp(this_delay, Constants::tropo_dry_res);

        if (SPARTN_Generator::check_within_range(val_dec, Constants::tropo_dry_spartn_rng_min)) {
            hydrostatic_delay_average += val_dec;
            ++count;
        }
    }
    hydrostatic_delay_average /= count;

    SPARTN_Generator::add_field_to_block(
        tropo_coef_block, 43, 8,
        SPARTN_Generator::encode_value_spartn(hydrostatic_delay_average,
                                              Constants::tropo_dry_spartn_rng_min,
                                              Constants::tropo_dry_res));

    SPARTN_Generator::add_field_to_block(tropo_coef_block, 44, 1, 0);

    {
        /*
         * LPP contains no troposphere coefficients, but SPARTN requires at
         * least one set with at least T00 defined, so add a 0 one.
         *
         * We use a constant value of 63 because:
         * (0 - -0.252) / 0.0004 =  63
         */
        std::unique_ptr<SPARTN_Block> zero_coef(new SPARTN_Block);
        SPARTN_Generator::add_field_to_block(zero_coef, 45, 7, 63);
        SPARTN_Generator::add_block_to_block(tropo_coef_block, zero_coef);
    }

    SPARTN_Generator::add_block_to_block(tropo_data_block, tropo_coef_block);
    return true;
}

void SPARTN_Generator::generate_tropo_grid(
    std::unique_ptr<SPARTN_Block> const&        tropo_data_block,
    GNSS_SSR_GriddedCorrection_r16 const* const gridded_corrections) {
    std::unique_ptr<SPARTN_Block> grid_block(new SPARTN_Block);

    SPARTN_Generator::add_field_to_block(grid_block, 51, 1, 1);

    auto const gridlist = gridded_corrections->gridList_r16.list;

    for (int i = 0; i < gridlist.count; i++) {
        long const wetdelay =
            gridlist.array[i]->tropospericDelayCorrection_r16->tropoWetVerticalDelay_r16;

        double const val_dec =
            SPARTN_Generator::decode_value_lpp(wetdelay, Constants::wetdelay_res);

        // scale and res are the same between LPP and SPARTN so no need to
        // check range
        SPARTN_Generator::add_field_to_block(
            grid_block, 53, 8,
            SPARTN_Generator::encode_value_spartn(val_dec, Constants::wetdelay_spartn_rng_min,
                                                  Constants::wetdelay_res));
    }

    SPARTN_Generator::add_block_to_block(tropo_data_block, grid_block);
}

void SPARTN_Generator::generate_iono_data(std::unique_ptr<SPARTN_Block> const& atmo_block,
                                          HPACCorrection const&                hpac_corr) const {
    std::unique_ptr<SPARTN_Block> iono_data_block(new SPARTN_Block);
    /*
     * When mergeing the STEC_Corrections and GriddedCorrections, it is
     * possible to safely assume that the correction ID is the same for all
     * IEs. This is because GNSS-SSR-CorrectionPoints is sent once per message
     * and contains only one ID for the entire message.
     */
    std::vector<IonosphereSatelliteData> satellite_data = {};

    auto const& sat_list            = hpac_corr.stec_corrections->stec_SatList_r16.list;
    auto const& gridded_corrections = hpac_corr.gridded_corrections->gridList_r16.list;
    auto const  gnss_id             = hpac_corr.gnss_id;
    auto const& allowed_svs         = this->sv_intersections_.at(gnss_id.gnss_id);

    for (int i = 0; i < sat_list.count; i++) {
        STEC_SatElement_r16 const* const this_sat = sat_list.array[i];
        auto const                       svid     = this_sat->svID_r16.satellite_id;

        if (std::count(allowed_svs.begin(), allowed_svs.end(), svid) == 0) {
            if (std::count(allowed_svs.begin(), allowed_svs.end(), svid) == 0) {
                // std::cout << "[INFO] " << (int)svid << " not allowed in iono"
                // << std::endl;
                continue;
            }
        }

        satellite_data.emplace_back(IonosphereSatelliteData{(uint64_t)svid, this_sat, {}});
    }

    for (int i = 0; i < gridded_corrections.count; i++) {
        GridElement_r16 const* const this_grid_elm = gridded_corrections.array[i];

        if (!this_grid_elm->stec_ResidualSatList_r16) {
            continue;
        }

        auto const reslist = this_grid_elm->stec_ResidualSatList_r16->list;

        for (int j = 0; j < reslist.count; j++) {
            long const this_sv = reslist.array[j]->svID_r16.satellite_id;

            for (IonosphereSatelliteData& iono_data : satellite_data) {
                if (iono_data.SVID == static_cast<uint64_t>(this_sv)) {
                    iono_data.residuals.emplace_back(reslist.array[j]);
                    break;
                }
            }
        }
    }

    std::unique_ptr<SPARTN_Block> iono_block(new SPARTN_Block);

    uint8_t const sat_mask_bit_count = Constants::satellite_mask_sizes.at(gnss_id.gnss_id);
    uint8_t const field_id           = Constants::satellite_mask_ids.at(gnss_id.gnss_id);

    uint8_t                                      min_coefs_found = 3;
    std::bitset<Constants::max_spartn_bit_count> sat_mask_bitset = 2;
    sat_mask_bitset <<= sat_mask_bit_count - 2;

    uint16_t const sat_mask_length                                       = sat_mask_bit_count - 3;
    static constexpr std::bitset<Constants::max_spartn_bit_count> new_sv = 1;

    for (IonosphereSatelliteData const& this_sat : satellite_data) {
        uint8_t coefs_found = 0;
        if (this_sat.sat_elm->stec_C01_r16) {
            coefs_found++;
            if (this_sat.sat_elm->stec_C10_r16) {
                coefs_found++;
                if (this_sat.sat_elm->stec_C11_r16) {
                    coefs_found++;
                }
            }
        }

        if (coefs_found < min_coefs_found) {
            min_coefs_found = coefs_found;
        }

        sat_mask_bitset |= new_sv << (sat_mask_length - this_sat.SVID);
    }

    SPARTN_Generator::add_field_to_block(iono_block, 54, 3, min_coefs_found);

    std::unique_ptr<SPARTN_Field> satellite_mask(
        new SPARTN_Field(field_id, sat_mask_bit_count, sat_mask_bitset));
    iono_block->data.push(std::move(satellite_mask));

    SPARTN_Generator::add_block_to_block(iono_data_block, iono_block);

    for (IonosphereSatelliteData const& this_sat : satellite_data) {
        std::unique_ptr<SPARTN_Block> iono_sat_block(new SPARTN_Block);

        std::pair<std::unique_ptr<SPARTN_Block>, bool> coefs_res_size =
            SPARTN_Generator::generate_iono_coef_block(this_sat.sat_elm, min_coefs_found);

        this->generate_iono_sat_block(iono_sat_block, this_sat.sat_elm, coefs_res_size.second);
        SPARTN_Generator::add_block_to_block(iono_sat_block, coefs_res_size.first);

        SPARTN_Generator::generate_iono_grid_block(iono_sat_block, this_sat.residuals);
        SPARTN_Generator::add_block_to_block(iono_data_block, iono_sat_block);
    }
    SPARTN_Generator::add_block_to_block(atmo_block, iono_data_block);
}

void SPARTN_Generator::generate_iono_sat_block(std::unique_ptr<SPARTN_Block> const& iono_data_block,
                                               STEC_SatElement_r16 const* const&    sat_elm,
                                               bool const residual_size) const {
    std::unique_ptr<SPARTN_Block> iono_poly_block(new SPARTN_Block);

    uint8_t spartn_iono_quality = 0;

    auto stec_buf = *sat_elm->stecQualityIndicator_r16.buf;

    uint8_t stec_class = stec_buf & 7;
    stec_class = ((stec_class & 1) << 2) | ((stec_class & 2) << 0) | ((stec_class & 4) >> 2);
    uint8_t stec_value = stec_buf >> 3;
    stec_value = ((stec_value & 1) << 2) | ((stec_value & 2) << 0) | ((stec_value & 4) >> 2);
    uint8_t const index = (8 * stec_class) + stec_value;

    if (index == 64) {
        spartn_iono_quality = 15;
    } else if (index == 0) {
        spartn_iono_quality = this->qualityoveride_ < 0  ? 0 :
                              15 < this->qualityoveride_ ? 15 :
                                                           this->qualityoveride_;
    } else {
        double const lpp_stec_qual = Constants::lpp_stec_qualities_maxiumums.at(64 - index);

        for (uint64_t i = 0; i < Constants::spartn_stec_qual_num; i++) {
            double const this_max = Constants::spartn_stec_qualities_maximums.at(i);

            if (this_max > lpp_stec_qual) {
                spartn_iono_quality = i + 1;
                break;
            }
        }
    }

    printf("iono class=%i, value=%i, index=%2i => qual=%2i\n", stec_class, stec_value, index,
           spartn_iono_quality);

    SPARTN_Generator::add_field_to_block(iono_poly_block, 55, 4, spartn_iono_quality);

    SPARTN_Generator::add_field_to_block(iono_poly_block, 56, 1, (int64_t)residual_size);

    SPARTN_Generator::add_block_to_block(iono_data_block, iono_poly_block);
}

std::pair<std::unique_ptr<SPARTN_Block>, bool>
SPARTN_Generator::generate_iono_coef_block(STEC_SatElement_r16 const* const& sat_elm,
                                           uint8_t const                     equ_type) {
    std::unique_ptr<SPARTN_Block> iono_coef_block(new SPARTN_Block);

    bool use_small_coef = true;

    double  c00_dec      = -1;
    double  c01_dec      = -1;
    double  c10_dec      = -1;
    double  c11_dec      = -1;
    uint8_t coef_counter = 0;

    double val_dec =
        SPARTN_Generator::decode_value_lpp(sat_elm->stec_C00_r16, Constants::iono_lpp_c00_res);

    if (SPARTN_Generator::check_within_range(val_dec, Constants::iono_spartn_c00_small_rng_min)) {
        c00_dec = val_dec;
    } else if (SPARTN_Generator::check_within_range(val_dec,
                                                    Constants::iono_spartn_c00_large_rng_min)) {
        use_small_coef = false;
        c00_dec        = val_dec;
    }

    if (coef_counter < equ_type && c00_dec != -1 && sat_elm->stec_C01_r16) {
        coef_counter++;
        val_dec =
            SPARTN_Generator::decode_value_lpp(*sat_elm->stec_C01_r16, Constants::iono_lpp_c01_res);

        if (use_small_coef && SPARTN_Generator::check_within_range(
                                  val_dec, Constants::iono_spartn_c01_small_rng_min)) {
            c01_dec = val_dec;
        } else {
            use_small_coef = false;

            if (SPARTN_Generator::check_within_range(val_dec,
                                                     Constants::iono_spartn_c01_large_rng_min)) {
                c01_dec = val_dec;
            }
        }

        if (coef_counter < equ_type && c01_dec != -1 && sat_elm->stec_C10_r16) {
            coef_counter++;
            val_dec = SPARTN_Generator::decode_value_lpp(*sat_elm->stec_C10_r16,
                                                         Constants::iono_lpp_c10_res);

            if (use_small_coef && SPARTN_Generator::check_within_range(
                                      val_dec, Constants::iono_spartn_c10_small_rng_min)) {
                c10_dec = val_dec;
            } else {
                use_small_coef = false;

                if (SPARTN_Generator::check_within_range(
                        val_dec, Constants::iono_spartn_c10_large_rng_min)) {
                    c10_dec = val_dec;
                }
            }

            if (coef_counter < equ_type && c10_dec != -1 && sat_elm->stec_C11_r16) {
                val_dec = SPARTN_Generator::decode_value_lpp(*sat_elm->stec_C11_r16,
                                                             Constants::iono_lpp_c11_res);

                if (use_small_coef && SPARTN_Generator::check_within_range(
                                          val_dec, Constants::iono_spartn_c11_small_rng_min)) {
                    c11_dec = val_dec;
                } else {
                    use_small_coef = false;

                    if (SPARTN_Generator::check_within_range(
                            val_dec, Constants::iono_spartn_c11_large_rng_min)) {
                        c11_dec = val_dec;
                    }
                }
            }
        }
    }

    uint8_t const base_bit_count = use_small_coef ? 12 : 14;
    uint8_t const base_id        = use_small_coef ? 57 : 60;

    if (c00_dec != -1) {
        SPARTN_Generator::add_field_to_block(
            iono_coef_block, base_id, base_bit_count,
            SPARTN_Generator::encode_value_spartn(c00_dec,
                                                  use_small_coef ?
                                                      Constants::iono_spartn_c00_small_rng_min :
                                                      Constants::iono_spartn_c00_large_rng_min,
                                                  Constants::iono_spartn_c00_res));
    } else {
        throw std::invalid_argument("did not get a valid c00 value");
    }

    if (c01_dec != -1) {
        SPARTN_Generator::add_field_to_block(
            iono_coef_block, base_id + 1, base_bit_count,
            SPARTN_Generator::encode_value_spartn(c01_dec,
                                                  use_small_coef ?
                                                      Constants::iono_spartn_c01_small_rng_min :
                                                      Constants::iono_spartn_c01_large_rng_min,
                                                  Constants::iono_spartn_c01_res));
    }

    if (c10_dec != -1) {
        SPARTN_Generator::add_field_to_block(
            iono_coef_block, base_id + 1, base_bit_count,
            SPARTN_Generator::encode_value_spartn(c10_dec,
                                                  use_small_coef ?
                                                      Constants::iono_spartn_c10_small_rng_min :
                                                      Constants::iono_spartn_c10_large_rng_min,
                                                  Constants::iono_spartn_c10_res));
    }

    if (c11_dec != -1) {
        SPARTN_Generator::add_field_to_block(
            iono_coef_block, base_id + 2, base_bit_count + 1,
            SPARTN_Generator::encode_value_spartn(c11_dec,
                                                  use_small_coef ?
                                                      Constants::iono_spartn_c11_small_rng_min :
                                                      Constants::iono_spartn_c11_large_rng_min,
                                                  Constants::iono_spartn_c11_res));
    }

    return std::make_pair(std::move(iono_coef_block), !use_small_coef);
}

void SPARTN_Generator::generate_iono_grid_block(
    std::unique_ptr<SPARTN_Block> const&               iono_data_block,
    std::vector<STEC_ResidualSatElement_r16_t*> const& residuals) {
    std::unique_ptr<SPARTN_Block> grid_block(new SPARTN_Block);

    // small = 0, medium = 1, large = 2, extra large = 3
    uint8_t             residual_size     = 0;
    std::vector<double> decoded_residuals = {};
    for (STEC_ResidualSatElement_r16 const* const residual_sat : residuals) {
        long const this_res = SPARTN_Generator::get_residual(residual_sat);
        if (this_res == 99999) {
            throw std::invalid_argument("[ERR] unset residual, grid invliad. unrecoverable state.");
        }

        double const val_dec =
            SPARTN_Generator::decode_value_lpp(this_res, Constants::residual_res);

        uint8_t this_residual_size = 0;
        if (SPARTN_Generator::check_within_range(val_dec,
                                                 Constants::residual_spartn_small_rng_min)) {
            this_residual_size = 0;
        } else if (SPARTN_Generator::check_within_range(
                       val_dec, Constants::residual_spartn_medium_rng_min)) {
            this_residual_size = 1;
        } else if (SPARTN_Generator::check_within_range(val_dec,
                                                        Constants::residual_spartn_large_rng_min)) {
            this_residual_size = 2;
        } else if (SPARTN_Generator::check_within_range(
                       val_dec, Constants::residual_spartn_extra_large_rng_min)) {
            this_residual_size = 3;
        } else {
            continue;
        }

        decoded_residuals.emplace_back(val_dec);

        if (this_residual_size > residual_size) {
            residual_size = this_residual_size;
        }
    }

    uint8_t field_id    = 64;
    uint8_t field_size  = 4;
    double  res_rng_min = Constants::residual_spartn_small_rng_min;

    if (residual_size == 1) {
        field_id    = 65;
        field_size  = 7;
        res_rng_min = Constants::residual_spartn_medium_rng_min;
    } else if (residual_size == 2) {
        field_id    = 66;
        field_size  = 10;
        res_rng_min = Constants::residual_spartn_large_rng_min;
    } else if (residual_size == 3) {
        field_id    = 67;
        field_size  = 14;
        res_rng_min = Constants::residual_spartn_extra_large_rng_min;
    }

    SPARTN_Generator::add_field_to_block(grid_block, 63, 2, residual_size);

    for (double const& residual : decoded_residuals) {
        SPARTN_Generator::add_field_to_block(
            grid_block, field_id, field_size,
            SPARTN_Generator::encode_value_spartn(residual, res_rng_min, Constants::residual_res));
    }

    SPARTN_Generator::add_block_to_block(iono_data_block, grid_block);
}

void SPARTN_Generator::generate_gad_message(
    std::vector<std::unique_ptr<SPARTN_Message>>& messages) {
    if (this->hpac_corrections_.size() == 0) {
        std::cout << "[INFO] [GAD] only send GAD with HPAC" << std::endl;
        return;
    } else if (!this->last_correction_point_) {
        std::cout << "[INFO] [GAD] no correction points (stored)" << std::endl;
        return;
    }

    // NOTE(ewasjon): LPP doesn't include IOD SSR for GAD, so we use one from the HPAC corrections.
    auto iod_ssr = this->hpac_corrections_.at(0).iod_ssr;

    std::cout << std::endl << "==== NEW GAD MESSAGE ====" << std::endl;
    std::unique_ptr<SPARTN_Message> this_message(new SPARTN_Message);

    this_message->message_type     = 2;
    this_message->message_sub_type = 0;

    std::unique_ptr<SPARTN_Message_Header> this_header(new SPARTN_Message_Header);
    SPARTN_Generator::add_field_to_header(this_header, 5, 9, iod_ssr);
    SPARTN_Generator::add_field_to_header(this_header, 68, 4, 0);
    SPARTN_Generator::add_field_to_header(this_header, 69, 1, 0);

    // LPP only provides one area at a time
    // SPARTN counts from zero
    SPARTN_Generator::add_field_to_header(this_header, 30, 5, 0);

    this_message->message_header = std::move(this_header);

    uint32_t const area_id = this->last_correction_point_->id;

    SPARTN_Generator::generate_gad_message_array(this_message, *this->last_correction_point_,
                                                 area_id);

    messages.push_back(std::move(this_message));
}

// NOTE(ewasjon): This function may not be correct.
void SPARTN_Generator::generate_gad_message_list(
    std::unique_ptr<SPARTN_Message> const&     gad_message,
    GNSS_SSR_ListOfCorrectionPoints_r16 const& corrpoints, uint32_t const area_id) {
    std::unique_ptr<SPARTN_Block> area_def(new SPARTN_Block);

    SPARTN_Generator::add_field_to_block(area_def, 31, 8, area_id);

    SPARTN_Generator::add_field_to_block(area_def, 32, 11, corrpoints.referencePointLatitude_r16);

    SPARTN_Generator::add_field_to_block(area_def, 33, 12, corrpoints.referencePointLongitude_r16);

    SPARTN_Generator::add_field_to_block(area_def, 34, 3,
                                         corrpoints.relativeLocationsList_r16.list.count);

    SPARTN_Generator::add_field_to_block(area_def, 35, 3,
                                         corrpoints.relativeLocationsList_r16.list.count);

    uint16_t const curr_delta_lat =
        corrpoints.relativeLocationsList_r16.list.array[0]->deltaLatitude_r16;
    uint16_t const curr_delta_lng =
        corrpoints.relativeLocationsList_r16.list.array[0]->deltaLongitude_r16;

    // LPP spec allows delta to be -ve & 0, SPARTN does not. we check max after
    // sanity check
    if (curr_delta_lng <= 0) {
        std::cout << "[INFO] got a negative longitude delta" << std::endl;
        return;
    }

    if (curr_delta_lat <= 0) {
        std::cout << "[INFO] got a negative latitude delta" << std::endl;
        return;
    }

    /* sanity check */
    for (int i = 1; i < corrpoints.relativeLocationsList_r16.list.count; i++) {
        RelativeLocation const* const this_elm = corrpoints.relativeLocationsList_r16.list.array[i];

        if (this_elm->deltaLatitude_r16 != curr_delta_lat) {
            std::cout << "[INFO] got different delta lat from list, aborting" << std::endl;
            return;
        }
        if (this_elm->deltaLongitude_r16 != curr_delta_lng) {
            std::cout << "[INFO] got different delta lng from list, aborting" << std::endl;
            return;
        }
    }

    auto const dec_delta_lat =
        SPARTN_Generator::decode_value_lpp(curr_delta_lat, Constants::delta_lpp_res);
    auto const dec_delta_lng =
        SPARTN_Generator::decode_value_lpp(curr_delta_lng, Constants::delta_lpp_res);

    if (dec_delta_lat > Constants::delta_spartn_rng_max) {
        std::cout << "[INFO] got a latitude delta outside range" << std::endl;
        return;
    }

    if (dec_delta_lng > Constants::delta_spartn_rng_max) {
        std::cout << "[INFO] got a longitude delta outside range" << std::endl;
        return;
    }

    SPARTN_Generator::add_field_to_block(
        area_def, 36, 5,
        SPARTN_Generator::encode_value_spartn(dec_delta_lat, Constants::delta_spartn_rng_min,
                                              Constants::delta_spartn_res));
    SPARTN_Generator::add_field_to_block(
        area_def, 37, 5,
        SPARTN_Generator::encode_value_spartn(dec_delta_lng, Constants::delta_spartn_rng_min,
                                              Constants::delta_spartn_res));

    gad_message->data.push(std::move(area_def));
}

void SPARTN_Generator::generate_gad_message_array(
    std::unique_ptr<SPARTN_Message> const& gad_message, CorrectionPoint const& corrpoints,
    uint32_t const area_id) {
    std::unique_ptr<SPARTN_Block> area_def(new SPARTN_Block);

    SPARTN_Generator::add_field_to_block(area_def, 31, 8, area_id);

    auto ref_point_lat_lpp = corrpoints.referencePointLatitude_r16;
    auto ref_point_lng_lpp = corrpoints.referencePointLongitude_r16;

    auto ref_point_lat_real =
        ((double)ref_point_lat_lpp / Constants::two_to_the_fourteen) * Constants::deg_in_lat;
    auto ref_point_lng_real =
        ((double)ref_point_lng_lpp / Constants::two_to_the_fifteen) * Constants::deg_in_lng;

    auto ref_point_lat_spartn = SPARTN_Generator::encode_value_spartn(
        ref_point_lat_real, -1 * Constants::deg_in_lat, Constants::lat_lng_res);
    auto ref_point_lng_spartn = SPARTN_Generator::encode_value_spartn(
        ref_point_lng_real, -1 * Constants::deg_in_lng, Constants::lat_lng_res);

    SPARTN_Generator::add_field_to_block(area_def, 32, 11, ref_point_lat_spartn);
    SPARTN_Generator::add_field_to_block(area_def, 33, 12, ref_point_lng_spartn);

    auto num_of_steps_lat = corrpoints.numberOfStepsLatitude_r16;
    auto num_of_steps_lng = corrpoints.numberOfStepsLongitude_r16;

    auto grid_count_lat = num_of_steps_lat + 1;
    auto grid_count_lng = num_of_steps_lng + 1;

    // NOTE(ewasjon): This cannot happen, but we check anyway
    if (grid_count_lat == 0 || grid_count_lng == 0) {
        std::cout << "[ERR] lpp provided a grid with zero steps";
        std::cout << ", lat steps " << num_of_steps_lat;
        std::cout << ", long steps " << num_of_steps_lng << std::endl;
        return;
    }

    if (grid_count_lat > 8) {
        std::cout << "[ERR] too many grid points in latitude: " << grid_count_lat << " (max 8)";
        return;
    } else if (grid_count_lng > 8) {
        std::cout << "[ERR] too many grid points in longitude: " << grid_count_lng << " (max 8)";
        return;
    }

    // NOTE(ewasjon): SF034 and SF035 encode the number of grid points with 3 bits. Where 0b000 is 1
    // grid point, 0b001 is 2 grid points, etc. So we subtract 1 from the number of grid points to
    // get the correct encoding.
    SPARTN_Generator::add_field_to_block(area_def, 34, 3, grid_count_lat - 1);
    SPARTN_Generator::add_field_to_block(area_def, 35, 3, grid_count_lng - 1);

    auto delta_lat_lpp  = corrpoints.stepOfLatitude_r16;
    auto delta_lng_lpp  = corrpoints.stepOfLongitude_r16;
    auto delta_lat_real = (double)delta_lat_lpp * Constants::step_lpp_res;
    auto delta_lng_real = (double)delta_lng_lpp * Constants::step_lpp_res;

    // NOTE(ewasjon): SPARTN encodes the delta with 5 bits, for the range 0.1 to 3.2 degrees. Thus,
    // the 0 integer value is not equal to 0 degrees, but rather 0.1 degrees. We subtract 0.1 from
    // the delta to get the correct encoding.
    auto delta_lng_spartn =
        std::lround((delta_lng_real - Constants::step_spartn_res) / Constants::step_spartn_res);
    auto delta_lat_spartn =
        std::lround((delta_lat_real - Constants::step_spartn_res) / Constants::step_spartn_res);

    SPARTN_Generator::add_field_to_block(area_def, 36, 5, delta_lat_spartn);
    SPARTN_Generator::add_field_to_block(area_def, 37, 5, delta_lng_spartn);

    gad_message->data.push(std::move(area_def));
}

template <class T>
void SPARTN_Generator::add_ocb_correction(std::vector<OCBCorrection>& ocb_corrections,
                                          T const* const corrs, GNSS_SSR_URA_r16 const* ura,
                                          const GNSS_ID gnss_id) {
    if (!corrs) {
        return;
    }

    bool found_ocb = false;
    long iod_ssr   = 0;
    for (OCBCorrection& ocb : ocb_corrections) {
        if (gnss_id.gnss_id != ocb.gnss_id.gnss_id) {
            continue;
        }

        iod_ssr = SPARTN_Generator::get_iod(corrs, has_iod_r16<T>{});

        if (iod_ssr == ocb.iod_ssr) {
            SPARTN_Generator::add_ocb_correction(ocb, corrs);
            found_ocb = true;
            break;
        }
    }

    if (!found_ocb) {
        ocb_corrections.push_back({iod_ssr, gnss_id, nullptr, nullptr, nullptr, nullptr, ura});
        SPARTN_Generator::add_ocb_correction(ocb_corrections.back(), corrs);
    }
}

std::vector<OCBCorrection>
SPARTN_Generator::group_ocb_corrections(GNSS_GenericAssistData const* const gen_ass) {
    std::vector<OCBCorrection> ocb_corrections = {};
    for (int i = 0; gen_ass && i < gen_ass->list.count; i++) {
        GNSS_GenericAssistDataElement const* const this_assist_data = gen_ass->list.array[i];

        auto const& gnss_id = this_assist_data->gnss_ID;

        GNSS_SSR_OrbitCorrections_r15 const* orb_corrs   = nullptr;
        GNSS_SSR_ClockCorrections_r15 const* clk_corrs   = nullptr;
        GNSS_SSR_CodeBias_r15 const*         code_corrs  = nullptr;
        GNSS_SSR_PhaseBias_r16 const*        phase_corrs = nullptr;

        GNSS_SSR_URA_r16 const* ura = nullptr;

        if (this_assist_data->ext2) {
            auto const& ext2 = this_assist_data->ext2;

            orb_corrs =
                ext2->gnss_SSR_OrbitCorrections_r15 ? ext2->gnss_SSR_OrbitCorrections_r15 : nullptr;

            clk_corrs =
                ext2->gnss_SSR_ClockCorrections_r15 ? ext2->gnss_SSR_ClockCorrections_r15 : nullptr;

            code_corrs = ext2->gnss_SSR_CodeBias_r15 ? ext2->gnss_SSR_CodeBias_r15 : nullptr;
        }

        if (this_assist_data->ext3) {
            auto const& ext3 = this_assist_data->ext3;

            phase_corrs = ext3->gnss_SSR_PhaseBias_r16 ? ext3->gnss_SSR_PhaseBias_r16 : nullptr;

            ura = ext3->gnss_SSR_URA_r16 ? ext3->gnss_SSR_URA_r16 : nullptr;
        }

        SPARTN_Generator::add_ocb_correction(ocb_corrections, orb_corrs, ura, gnss_id);
        SPARTN_Generator::add_ocb_correction(ocb_corrections, clk_corrs, ura, gnss_id);
        SPARTN_Generator::add_ocb_correction(ocb_corrections, code_corrs, ura, gnss_id);
        SPARTN_Generator::add_ocb_correction(ocb_corrections, phase_corrs, ura, gnss_id);
    }

    return ocb_corrections;
}

std::vector<HPACCorrection>
SPARTN_Generator::group_hpac_corrections(GNSS_GenericAssistData const* const gen_ass,
                                         GNSS_CommonAssistData const* const  com_ass) {
    std::vector<HPACCorrection> hpac_corrections = {};

    GNSS_SSR_CorrectionPoints_r16 const* correction_points =
        (com_ass && com_ass->ext2) ? com_ass->ext2->gnss_SSR_CorrectionPoints_r16 ?
                                     com_ass->ext2->gnss_SSR_CorrectionPoints_r16 :
                                     nullptr :
                                     nullptr;

    if (correction_points) {
        this->last_correction_point_ = nullptr;

        this->last_correction_point_     = std::unique_ptr<CorrectionPoint>(new CorrectionPoint{});
        this->last_correction_point_->id = correction_points->correctionPointSetID_r16;

        switch (correction_points->correctionPoints_r16.present) {
        case GNSS_SSR_CorrectionPoints_r16__correctionPoints_r16_PR_listOfCorrectionPoints_r16: {
            this->last_correction_point_->num_of_grid_points =
                correction_points->correctionPoints_r16.choice.listOfCorrectionPoints_r16
                    .relativeLocationsList_r16.list.count;
            break;
        }
        case GNSS_SSR_CorrectionPoints_r16__correctionPoints_r16_PR_arrayOfCorrectionPoints_r16: {
            auto const& array =
                correction_points->correctionPoints_r16.choice.arrayOfCorrectionPoints_r16;
            this->last_correction_point_->num_of_grid_points =
                (array.numberOfStepsLatitude_r16 + 1) * (array.numberOfStepsLongitude_r16 + 1);

            this->last_correction_point_->referencePointLatitude_r16 =
                array.referencePointLatitude_r16;
            this->last_correction_point_->referencePointLongitude_r16 =
                array.referencePointLongitude_r16;
            this->last_correction_point_->numberOfStepsLatitude_r16 =
                array.numberOfStepsLatitude_r16;
            this->last_correction_point_->numberOfStepsLongitude_r16 =
                array.numberOfStepsLongitude_r16;
            this->last_correction_point_->stepOfLatitude_r16  = array.stepOfLatitude_r16;
            this->last_correction_point_->stepOfLongitude_r16 = array.stepOfLongitude_r16;

            break;
        }
        default: {
            this->last_correction_point_ = nullptr;
            break;
        }
        }
    }

    for (int i = 0; gen_ass && i < gen_ass->list.count; i++) {
        auto const& this_assist_data = gen_ass->list.array[i];

        auto const& gnss_id = this_assist_data->gnss_ID;

        GNSS_SSR_GriddedCorrection_r16 const* gridded_corrections = nullptr;
        GNSS_SSR_STEC_Correction_r16 const*   stec_corrections    = nullptr;

        if (this_assist_data->ext3) {
            auto const& ext3 = this_assist_data->ext3;

            gridded_corrections = ext3->gnss_SSR_GriddedCorrection_r16 ?
                                      ext3->gnss_SSR_GriddedCorrection_r16 :
                                      nullptr;

            stec_corrections =
                ext3->gnss_SSR_STEC_Correction_r16 ? ext3->gnss_SSR_STEC_Correction_r16 : nullptr;
        }

        if ((gridded_corrections && stec_corrections)) {
            if (gridded_corrections->iod_ssr_r16 == stec_corrections->iod_ssr_r16) {
                hpac_corrections.push_back({gridded_corrections->iod_ssr_r16, gnss_id,
                                            this->last_correction_point_.get(), gridded_corrections,
                                            stec_corrections});
            } else {
                hpac_corrections.push_back({gridded_corrections->iod_ssr_r16, gnss_id,
                                            this->last_correction_point_.get(), gridded_corrections,
                                            nullptr});
                hpac_corrections.push_back({stec_corrections->iod_ssr_r16, gnss_id,
                                            this->last_correction_point_.get(), nullptr,
                                            stec_corrections});
            }
        }
        if (gridded_corrections && !stec_corrections) {
            hpac_corrections.push_back({gridded_corrections->iod_ssr_r16, gnss_id,
                                        this->last_correction_point_.get(), gridded_corrections,
                                        nullptr});
        } else if (!gridded_corrections && stec_corrections) {
            hpac_corrections.push_back({stec_corrections->iod_ssr_r16, gnss_id,
                                        this->last_correction_point_.get(), nullptr,
                                        stec_corrections});
        }
    }
    return hpac_corrections;
}

static TAI_Time time_from_epoch_time(GNSS_SystemTime const* time) {
    auto day_number  = time->gnss_DayNumber;
    auto time_of_day = time->gnss_TimeOfDay;
    auto msec        = time->gnss_TimeOfDayFrac_msec ? *time->gnss_TimeOfDayFrac_msec / 1000.0 : 0;

    switch (time->gnss_TimeID.gnss_id) {
    case GNSS_ID__gnss_id_gps: {
        auto gps_time = GPS_Time(day_number, static_cast<f64>(time_of_day) + msec);
        return TAI_Time{gps_time};
    }

    case GNSS_ID__gnss_id_glonass: {
        auto glo_time = GLO_Time(day_number, static_cast<f64>(time_of_day) + msec);
        return TAI_Time{glo_time};
    }

    case GNSS_ID__gnss_id_galileo: {
        auto gal_time = GST_Time(day_number, static_cast<f64>(time_of_day) + msec);
        return TAI_Time{gal_time};
    }

    case GNSS_ID__gnss_id_bds: {
        auto bds_time = BDT_Time(day_number, static_cast<f64>(time_of_day) + msec);
        return TAI_Time{bds_time};
    }

    default: assert(false);
    }
}

void SPARTN_Generator::add_time_to_message(std::unique_ptr<SPARTN_Message> const& message,
                                           GNSS_SystemTime const*                 epoch_time,
                                           GNSS_ID__gnss_id const                 gnss_id) {
    long const tod =
        epoch_time->gnss_TimeOfDay + (epoch_time->gnss_TimeOfDayFrac_msec ?
                                          *epoch_time->gnss_TimeOfDayFrac_msec >= 500 ? 1 : 0 :
                                          0);

    std::unique_ptr<SPARTN_LPP_Time> time(
        new SPARTN_LPP_Time(gnss_id, epoch_time->gnss_DayNumber, tod));

    message->time = std::move(time);
}

template <class T>
void SPARTN_Generator::add_ocb_for_svs(std::map<uint16_t, OCBSatCorrection>& ocb_for_svs, T corr) {
    auto const corr_list = corr.list;

    for (int i = 0; i < corr_list.count; i++) {
        auto const svid = SPARTN_Generator::get_svid(corr_list.array[i],
                                                     has_svid_r16<decltype(corr_list.array[i])>{});

        if (ocb_for_svs.find(svid) == ocb_for_svs.end()) {
            OCBSatCorrection ocb_for_sv{nullptr, nullptr, nullptr, nullptr};
            ocb_for_svs[svid] = ocb_for_sv;
        }

        SPARTN_Generator::add_sv(ocb_for_svs.at(svid), corr_list.array[i]);
    }
}

std::map<uint16_t, OCBSatCorrection>
SPARTN_Generator::get_ocb_for_svs(GNSS_SSR_OrbitCorrections_r15 const* const orbit_corrs,
                                  GNSS_SSR_ClockCorrections_r15 const* const clock_corrs,
                                  GNSS_SSR_CodeBias_r15 const* const         code_bias_corrs,
                                  GNSS_SSR_PhaseBias_r16 const* const        phase_bias_corrs) {
    std::map<uint16_t, OCBSatCorrection> ocb_for_svs = {};

    if (orbit_corrs) {
        SPARTN_Generator::add_ocb_for_svs(ocb_for_svs, orbit_corrs->ssr_OrbitCorrectionList_r15);
    }

    if (clock_corrs) {
        SPARTN_Generator::add_ocb_for_svs(ocb_for_svs, clock_corrs->ssr_ClockCorrectionList_r15);
    }

    if (code_bias_corrs) {
        SPARTN_Generator::add_ocb_for_svs(ocb_for_svs, code_bias_corrs->ssr_CodeBiasSatList_r15);
    }

    if (phase_bias_corrs) {
        SPARTN_Generator::add_ocb_for_svs(ocb_for_svs, phase_bias_corrs->ssr_PhaseBiasSatList_r16);
    }

    return ocb_for_svs;
}
