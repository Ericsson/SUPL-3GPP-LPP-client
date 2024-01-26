#ifndef SPARTN_Generator_
#define SPARTN_Generator_

#include <generator/spartn/message.h>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <ctime>
#include <iostream>
#include <map>
#include <set>
#include <tuple>
#include <type_traits>
#include <vector>

#include <GNSS-SSR-GriddedCorrection-r16.h>
#include <GNSS-SSR-OrbitCorrections-r15.h>
#include <GNSS-SSR-ClockCorrections-r15.h>
#include <GNSS-SSR-PhaseBias-r16.h>
#include <GNSS-SSR-CodeBias-r15.h>
#include <GNSS-SSR-URA-r16.h>
#include <GNSS-SSR-STEC-Correction-r16.h>
#include <LPP-Message.h>
#include <LPP-MessageBody.h>

typedef struct CorrectionPoint {
    long    id;
    uint8_t num_of_grid_points;

    long referencePointLatitude_r16;
    long referencePointLongitude_r16;
    long numberOfStepsLatitude_r16;
    long numberOfStepsLongitude_r16;
    long stepOfLatitude_r16;
    long stepOfLongitude_r16;
} CorrectionPoint;

typedef struct HPACCorrection {
    const long                            iod_ssr;
    const GNSS_ID                         gnss_id;
    const CorrectionPoint*                correction_points;
    const GNSS_SSR_GriddedCorrection_r16* gridded_corrections;
    const GNSS_SSR_STEC_Correction_r16*   stec_corrections;
} HPACCorrection;

typedef struct IonosphereSatelliteData {
    const uint64_t                              SVID;
    const STEC_SatElement_r16* const            sat_elm;  // contains coefficients
    std::vector<STEC_ResidualSatElement_r16_t*> residuals;
} IonosphereSatelliteData;

typedef struct LastDiscontinuity {
    int8_t      discontinuity_indicator;
    std::time_t update_time;
} LastDiscontinuity;

typedef struct OCBCorrection {
    ~OCBCorrection() = default;

    const long                           iod_ssr;
    const GNSS_ID                        gnss_id;
    const GNSS_SSR_OrbitCorrections_r15* orbit_corrs;
    const GNSS_SSR_ClockCorrections_r15* clock_corrs;
    const GNSS_SSR_CodeBias_r15*         code_bias_corrs;
    const GNSS_SSR_PhaseBias_r16*        phase_bias_corrs;
    const GNSS_SSR_URA_r16*              ura;
} OCBCorrection;

typedef std::tuple<const SSR_OrbitCorrectionSatelliteElement_r15*,
                   const SSR_ClockCorrectionSatelliteElement_r15*,
                   const SSR_CodeBiasSatElement_r15*, const SSR_PhaseBiasSatElement_r16_t*>
    OCBSatCorrection;

class SPARTN_Generator {
public:
    SPARTN_Generator()  = default;
    ~SPARTN_Generator() = default;

    /** Generate SPARTN message(s) from a single LPP message. SPARTN
     * messages are represented by the internal struct SPARTN_Message,
     * meaning they can be passed to the SPARTN_Transmitter::transmit
     * function to be sent as a binary stream to various outputs (UART
     * etc.).
     *
     * @param[in] lpp_message a pointer to the LPP_Message struct that is
     *                        to be transcoded. This is stored in the class
     *                        so that it does not need to be passed around.
     *
     * @param[in] uraoveride  a hacky fix to set a nominal value for the
     *                        URA if the LPP message does not include it.
     *                        uraoveride will be clamped between 0-7.
     *
     * @return                a vector of the transcoded SPARTN messages,
     *                        ready to be transmitted. Messages will always
     *                        be in the order of OCB -> GAD -> HPAC (unless
     *                        the LPP message does not contain information
     *                        for a full message to be made).
     */
    std::vector<std::unique_ptr<SPARTN_Message>> generate(const LPP_Message* lpp_message,
                                                          int8_t             qualityoveride,
                                                          bool               ublox_clock_correction,
                                                          bool               force_continuity);

private:
    void intersect_svs();

    /** Generate OCB messages from the LPP message. Currently GPS, GLONASS
     * and Galileo constellations are supported, others will fail silently.
     *
     * @param[out] messages   vector of the currently transcoded messages,
     *                        newly generated messages will be appended to
     *                        this vector.
     */
    void generate_ocb_message(std::vector<std::unique_ptr<SPARTN_Message>>& messages);
    /** Generate the orbit block of the OCB message, for one satellite.
     *
     * @param[out] sat_block        the SPARTN satellite block where the
     *                              generated orbit block will be place.
     *
     * @param[in]  orbit_correction pointer to the information from LPP on
     *                              the orbit correction for *one*
     *                              satellite.
     *
     * @param[in]  gnss_id          orbit_correction parameter does not
     *                              contain knowledge of which
     *                              constellation the satellite it is
     *                              correcting is part of, keep store of it
     *                              here.
     *
     * @param[in]  generation_time  time stamp of when the generation of
     *                              the SPARTN messages started.
     *
     * @return                      true if block was successfully
     *                              generated and added to parent block,
     *                              false if something failed.
     */
    bool generate_orbit_block(const std::unique_ptr<SPARTN_Block>&           sat_block,
                              const SSR_OrbitCorrectionSatelliteElement_r15* orbit_correction,
                              GNSS_ID__gnss_id gnss_id, std::time_t generation_time);
    /** Generate the clock block of the OCB message, for one satellite.
     *
     * @param[out] sat_block        the SPARTN satellite block where the
     *                              generated clock block will be place.
     *
     * @param[in]  clock_correction pointer to the information from LPP on
     *                              the clock correction for *one*
     *                              satellite.
     *
     * @param[in]  URA              pointer to the user range error given
     *                              by LPP.
     *
     * @param[in]  gnss_id          clock_correction parameter does not
     *                              contain knowledge of which
     *                              constellation the satellite it is
     *                              correcting is part of, keep store of it
     *                              here.
     *
     * @param[in]  generation_time  time stamp of when the generation of
     *                              the SPARTN messages started.
     *
     * @param[in]  gnss_systemtime  should be retrieved from the LPP
     *                              message. Used to generate the clock
     *                              corrections (SF020).
     *
     * @param[in]  update_interval  should be retrieved from the LPP
     *                              message. Used to generate the clock
     *                              corrections (SF020).
     *
     * @param[in]  iode             Issue of Data Ephemeris from the LPP
     *                              message.
     *
     * @return                      true if block was successfully
     *                              generated and added to parent block,
     *                              false if something failed.
     */
    bool generate_clock_block(const std::unique_ptr<SPARTN_Block>&           sat_block,
                              const SSR_ClockCorrectionSatelliteElement_r15* clock_correction,
                              const GNSS_SSR_URA_r16* URA, GNSS_ID__gnss_id gnss_id,
                              std::time_t generation_time, const GNSS_SystemTime* gnss_systemtime,
                              long update_interval, uint8_t iode);
    /** Generate the bias block of the OCB message, for one satellite.
     *
     * @param[out] sat_block         the SPARTN satellite block where the
     *                               generated bias block will be place.
     *
     * @param[in]  sat_code_biases   pointer to the information from LPP on
     *                               the code bias corrections for each
     *                               signal for one satellite.
     *
     * @param[in]  sat_phase_biases  pointer to the information from LPP on
     *                               the phase bias corrections for each
     *                               signal for one satellite.
     *
     * @param[in]  gnss_id           neither correction contains knowledge
     *                               of which constellation the satellite
     *                               it is correcting is part of, keep
     *                               store of it here.
     *
     * @return                       true if block was successfully
     *                               generated and added to parent block,
     *                               false if something failed.
     */
    bool generate_bias_block(const std::unique_ptr<SPARTN_Block>& sat_block,
                             const SSR_CodeBiasSatElement_r15*    sat_code_biases,
                             const SSR_PhaseBiasSatElement_r16*   sat_phase_biases,
                             GNSS_ID__gnss_id                     gnss_id);
    /** Generate a phase bias block for each signal contained within the
     * LPP message, add it to the bias block for an individual satellite.
     *
     * @param[out] bias_block         the bias block for each phase bias
     *                                block to be added to.
     *
     * @param[in]  phase_biases       list of phase bias corrections given
     *                                by LPP for each signal for one
     *                                satellite.
     *
     * @param[in]  lpp_to_spartn_bias SPARTN and LPP give each signal a
     *                                numerical ID. Different
     *                                constellations have different signals
     *                                and therefore different IDs. SPARTN
     *                                and LPP also do not agree on the IDs,
     *                                one signal in LPP might have ID 8 but
     *                                1 in SPARTN. This is a mapping
     *                                between the LPP IDs and the SPARTN
     *                                IDs for each signal in *one*
     *                                constellation.
     *
     * @param[in]  bis                in SPARTN, different constellations
     *                                have different IDs for code and phase
     *                                bias fields and different sizes.
     *                                Store this information here.
     *
     * @param[in]  gnss_id            the phase bias correction does not
     *                                contain knowledge of which
     *                                constellation the satellite it is
     *                                correcting is part of, keep store of
     *                                it here.
     */
    void generate_phase_bias_block(const std::unique_ptr<SPARTN_Block>& bias_block,
                                   const SSR_PhaseBiasSatElement_r16*   phase_biases,
                                   std::map<long, uint8_t>              lpp_to_spartn_bias,
                                   Constants::bias_ids_size bis, GNSS_ID__gnss_id gnss_id);
    /** Generate the code bias block for a satellite.
     *
     * @param[out] bias_block         the bias block for the code bias
     *                                block to be added to.
     *
     * @param[in]  code_biases        list of phase bias corrections given
     *                                by LPP for each signal for one
     *                                satellite.
     *
     * @param[in]  lpp_to_spartn_bias SPARTN and LPP give each signal a
     *                                numerical ID. Different
     *                                constellations have different signals
     *                                and therefore different IDs. SPARTN
     *                                and LPP also do not agree on the IDs,
     *                                one signal in LPP might have ID 8 but
     *                                1 in SPARTN. This is a mapping
     *                                between the LPP IDs and the SPARTN
     *                                IDs for each signal in *one*
     *                                constellation.
     *
     * @param[in]  bis                in SPARTN, different constellations
     *                                have different IDs for code and phase
     *                                bias fields and different sizes.
     *                                Store this information here.
     *
     * @return                        true if all code bias block has
     *                                been added successfully, false if
     *                                not.
     */
    static bool generate_code_bias_block(const std::unique_ptr<SPARTN_Block>& bias_block,
                                         const SSR_CodeBiasSignalList_r15*    code_biases,
                                         std::map<long, uint8_t>              lpp_to_spartn_bias,
                                         Constants::bias_ids_size             bis);

    /** Generate HPAC messages from the LPP message. Currently GPS, GLONASS
     * and Galileo constellations are supported, others will fail silently.
     *
     * @param[out] messages   vector of the currently transcoded messages,
     *                        newly generated messages will be appended to
     *                        this vector.
     */
    void generate_hpac_messages(std::vector<std::unique_ptr<SPARTN_Message>>& messages) const;
    /** Generate the area data block for a single correction.
     *
     * @param[out] atmo_block the atmosphere block for the area data block
     *                        to be added to.
     *
     * @param[in]  hpac_corr  the HPAC corrections given by LPP, organised
     *                        by SPARTN_Generator::group_hpac_corrections()
     *
     * @return                true if area data block has been added
     *                        successfully, false if not.
     */
    static bool generate_area_data(const std::unique_ptr<SPARTN_Block>& atmo_block,
                                   const HPACCorrection&                hpac_corr);
    /** Generate the troposphere data block.
     *
     * @param[out] atmo_block          the atmosphere block for the
     *                                 troposphere data block to be added
     *                                 to.
     *
     * @param[in]  gridded_corrections pointer to the gridded correction
     *                                 data given by LPP.
     */
    void generate_tropo_data(const std::unique_ptr<SPARTN_Block>&  atmo_block,
                             const GNSS_SSR_GriddedCorrection_r16* gridded_corrections) const;
    /** Generate the troposphere coefficients block.
     *
     * @param[out] tropo_data          the troposphere data block for the
     *                                 troposphere coefficients block to be
     *                                 added to.
     *
     * @param[in]  gridded_corrections pointer to the gridded correction
     *                                 data given by LPP.
     *
     * @return                         true if block has been added
     *                                 successfully, false if not.
     */
    bool generate_tropo_coef(const std::unique_ptr<SPARTN_Block>&  tropo_data,
                             const GNSS_SSR_GriddedCorrection_r16* gridded_corrections) const;
    /** Generate the troposphere grid block.
     *
     * @param[out] tropo_data          the troposphere data block for the
     *                                 troposphere grid block to be added
     *                                 to.
     *
     * @param[in]  gridded_corrections pointer to the gridded correction
     *                                 data given by LPP.
     */
    static void generate_tropo_grid(const std::unique_ptr<SPARTN_Block>&  tropo_data,
                                    const GNSS_SSR_GriddedCorrection_r16* gridded_corrections);
    /** Generate the ionosphere data block.
     *
     * @param[out] atmo_block          the atmosphere block for the
     *                                 ionosphere data block to be added
     *                                 to.
     *
     * @param[in]  gridded_corrections pointer to the gridded correction
     *                                 data given by LPP.
     */
    void generate_iono_data(const std::unique_ptr<SPARTN_Block>& atmo_block,
                            const HPACCorrection&                hpac_corr) const;
    /** Generate the ionosphere satellite polynomial block for one
     * satellite.
     *
     * @param[out] iono_data     the ionosphere data block for the
     *                           satellite block to be added to.
     *
     * @param[in]  sat_elm       ionosphere corrections for a single
     *                           satellite
     *
     * @param[in]  residual_size used for SF056, false means small
     *                           coefficient block is used, true means
     *                           large is used.
     */
    void generate_iono_sat_block(const std::unique_ptr<SPARTN_Block>& iono_data_block,
                                 const STEC_SatElement_r16* const&    sat_elm,
                                 bool                                 residual_size) const;
    /** Generate the ionosphere satellite coefficient block. Will generate
     * either small (table 6.21) or large (table 6.22) based on the
     * equ_type parameter.
     *
     * This function does not automatically add it's generated block to a
     * parent (like other functions in this library), as the coefficients
     * block must be added *after* the polynomial block but the polynomial
     * block relies on information that can only be known after the
     * coefficients block is generated (size of coefficients).
     *
     * @param[in] sat_elm  ionosphere corrections for a single
     *                     satellite
     *
     * @param[in] equ_type the equation type used (how many coefficients
     *                     used)
     *
     * @return             a pair of the newly generated coefficient block
     *                     and if small or large coefficients can be used,
     *                     small is false and large is true.
     */
    static std::pair<std::unique_ptr<SPARTN_Block>, bool>
    generate_iono_coef_block(const STEC_SatElement_r16* const& sat_elm, uint8_t equ_type);
    /** Generate the ionosphere satellite polynomial block for one
     * satellite.
     *
     * @param[out] iono_data_block the ionosphere data block for the
     *                             satellite block to be added to.
     *
     * @param[in]  residuals       vector of points to residuals for each
     *                             grid point for a single satellite.
     */
    static void
    generate_iono_grid_block(const std::unique_ptr<SPARTN_Block>&               iono_data_block,
                             const std::vector<STEC_ResidualSatElement_r16_t*>& residuals);

    /** Generate HPAC messages from the LPP message.
     *
     * @param[out] messages   vector of the currently transcoded messages,
     *                        newly generated messages will be appended to
     *                        this vector.
     */
    void generate_gad_message(std::vector<std::unique_ptr<SPARTN_Message>>& messages);
    /** Generate the area definition block from LPP's list representation
     * of the correction points.
     *
     * WARNING: very much untested code.
     *
     * @param[out] gad_message the GAD message for the area definition
     *                         blocks to be added to.
     *
     * @param[in]  corrpoints  correction points in LPP's list
     *                         representation.
     *
     * @param[in]  area_id     the id of the area from LPP.
     */
    static void generate_gad_message_list(const std::unique_ptr<SPARTN_Message>&     gad_message,
                                          const GNSS_SSR_ListOfCorrectionPoints_r16& corrpoints,
                                          uint32_t                                   area_id);
    /** Generate the area definition block from LPP's array representation
     * of the correction points.
     *
     * @param[out] gad_message the GAD message for the area definition
     *                         blocks to be added to.
     *
     * @param[in]  corrpoints  correction points in LPP's array
     *                         representation.
     *
     * @param[in]  area_id     the id of the area from LPP.
     */
    static void generate_gad_message_array(const std::unique_ptr<SPARTN_Message>& gad_message,
                                           const CorrectionPoint& corrpoints, uint32_t area_id);

    template <typename T, typename = void>
    struct has_iod_r16 : std::false_type {};
    template <typename T>
    struct has_iod_r16<T, decltype(std::declval<T>().iod_ssr_r16, void())> : std::true_type {};

    /** gets the IOD for types that have the attribute `iod_ssr_r15`.
     *
     * @param[in] corr            the information element (typically a
     *                            correction).
     *
     * @param[in] std::false_type
     *
     * @return                    the issue of data ephemeris for the IE.
     */
    template <class T>
    static long get_iod(const T* const corr, std::false_type) {
        return corr->iod_ssr_r15;
    }

    /** gets the IOD for types that have the attribute `iod_ssr_r16`.
     *
     * @param[in] corr           the information element (typically a
     *                           correction).
     *
     * @param[in] std::true_type
     *
     * @return                   the issue of data ephemeris for the IE.
     */
    template <class T>
    static long get_iod(const T* const corr, std::true_type) {
        return corr->iod_ssr_r16;
    }

    /** Overload of add_ocb_correction for the orbit corrections.
     *
     * @param[out] ocb_correction where the correction is be added.
     *
     * @param[in]  orbit_corrs    set of corrections to be added.
     */
    static void add_ocb_correction(OCBCorrection&                             ocb_correction,
                                   const GNSS_SSR_OrbitCorrections_r15* const orbit_corrs) {
        ocb_correction.orbit_corrs = orbit_corrs;
    }

    /** Overload of add_ocb_correction for the clock corrections.
     *
     * @param[out] ocb_correction where the correction is be added.
     *
     * @param[in]  clock_corrs    set of corrections to be added.
     */
    static void add_ocb_correction(OCBCorrection&                             ocb_correction,
                                   const GNSS_SSR_ClockCorrections_r15* const clock_corrs) {
        ocb_correction.clock_corrs = clock_corrs;
    }

    /** Overload of add_ocb_correction for the code bias corrections.
     *
     * @param[out] ocb_correction where the correction is be added.
     *
     * @param[in]  code_corrs    set of corrections to be added.
     */
    static void add_ocb_correction(OCBCorrection&                     ocb_correction,
                                   const GNSS_SSR_CodeBias_r15* const code_corrs) {
        ocb_correction.code_bias_corrs = code_corrs;
    }

    /** Overload of add_ocb_correction for the phase bias corrections.
     *
     * @param[out] ocb_correction where the correction is be added.
     *
     * @param[in]  phase_corrs    set of corrections to be added.
     */
    static void add_ocb_correction(OCBCorrection&                      ocb_correction,
                                   const GNSS_SSR_PhaseBias_r16* const phase_corrs) {
        ocb_correction.phase_bias_corrs = phase_corrs;
    }

    /** Add some generic orbit, clock or bias correction to the list of
     * `OCBCorrection`s. If this function can't find somewhere to put the
     * correction in the list (based on iod_ssr), it will create a new
     * record and put it there. See comments in implementation for more
     * details.
     *
     * There is some real grimy template stuff going on here to make this
     * work...
     *
     * The first issue is that the type of the corrections is different, so
     * we template that type (easy).
     *
     * Next all corrections store the iod in iod_ssr_r15, *except* for
     * phase bias which uses iod_ssr_r16. Therefore we have the type
     * `has_iod_r16` which takes the type of the correction as it's
     * template. Using a little SFINAE this can then be converted into a
     * boolean as to if iod_ssr16 is used. There is then the overloaded
     * function `get_iod` that takes a bool as it's second parameter, which
     * allows it to either call the function that gets the r15 or r16
     * member.
     *
     * The final issue is adding the correction to the right member of the
     * struct. Again, overloading is used and multiple `add_ocb_correction`
     * definitions with different arguments for each correction that could
     * be used.
     *
     * @param[out] ocb_corrections a list of grouped OCB corrections where
     *                             the new set of corrections will end up.
     *
     * @param[in]  corrs           a set of correction from LPP, i.e.
     *                             GNSS_SSR_PhaseBias_r16.
     *
     * @param[in]  ura             the user range error is also associated
     *                             with an iod-ssr so is grouped as well.
     *                             its passed to this function so that it
     *                             can be used when a new record must be
     *                             made.
     *
     * @param[in]  gnss_id         iod-ssr only identifies a correction
     *                             within a constellation, so keep track of
     *                             that as well.
     */
    template <class T>
    static void add_ocb_correction(std::vector<OCBCorrection>& ocb_corrections, const T* corrs,
                                   const GNSS_SSR_URA_r16* ura, GNSS_ID gnss_id);

    /** The purpose of this function (and group_hpac_corrections) is that
     * it is not ensured by LPP that the iod-ssr between correction types
     * is the same. If the iod-ssr changes, all previous corrections must
     * be discarded SPARTN has a similar idea with it's SIOU, but it only
     * allows one SIOU for all correction types. Therefore we must group
     * correction types by iod-ssr and create a new message for each value
     * found.
     *
     * @param[in] gen_ass The struct from LPP that contains all of the sets
     *                    of corrections.
     *
     * @return            A vector of `OCBCorrection` structs, grouped by
     *                    iod-ssr and constellation.
     */
    static std::vector<OCBCorrection> group_ocb_corrections(const GNSS_GenericAssistData* gen_ass);

    /** The same idea as SPARTN_Generator::group_ocb_corrections(), just
     * with the HPAC IEs instead. This time, the correction points are the
     * same between all of the elements in the vector.
     *
     * @param[in] gen_ass Generic assistance data from LPP, containing the
     *                    gridded and STEC correction.
     *
     * @param[in] com_ass Common assistance data from LPP, containing the
     *                    correction points.
     *
     * @return            A vector of `HPACCorrection` structs, grouped by
     *                    iod-ssr and constellation.
     */
    std::vector<HPACCorrection> group_hpac_corrections(const GNSS_GenericAssistData* gen_ass,
                                                       const GNSS_CommonAssistData*  com_ass);

    /** Helper function to take add a `SPARTN_LPP_Time` member to a
     * `SPARTN_Message`.
     *
     * @param[out] message    a pointer to the message to have the time
     *                        added to.
     *
     * @param[in]  epoch_time LPP's representation of time, to be converted
     *                        into this library's representation.
     *
     * @param[in]  gnss_id    to interpret epoch_time correctly, the
     *                        constellation that it represents must also be
     *                        known.
     */
    static void add_time_to_message(const std::unique_ptr<SPARTN_Message>& message,
                                    const GNSS_SystemTime* epoch_time, GNSS_ID__gnss_id gnss_id);

    template <typename T, typename = void>
    struct has_svid_r16 : std::false_type {};
    template <typename T>
    struct has_svid_r16<T, decltype(std::declval<T>()->svID_r16, void())> : std::true_type {};

    /** gets the SV-ID for types that have the attribute `svID_r15`.
     *
     * @param[in] corr           the information element (typically a
     *                           correction).
     *
     * @param[in] std::false_type
     *
     * @return                   the SV-ID that the correction is for.
     */
    template <class T>
    static long get_svid(const T* const corr, std::false_type) {
        return corr->svID_r15.satellite_id;
    }

    /** gets the SV-ID for types that have the attribute `svID_r16`.
     *
     * @param[in] corr           the information element (typically a
     *                           correction).
     *
     * @param[in] std::true_type
     *
     * @return                   the SV-ID that the correction is for.
     */
    template <class T>
    static long get_svid(const T* const corr, std::true_type) {
        return corr->svID_r16.satellite_id;
    }

    static void add_sv(OCBSatCorrection&                              ocb_corr,
                       SSR_OrbitCorrectionSatelliteElement_r15* const orb_corr) {
        std::get<0>(ocb_corr) = orb_corr;
    }

    static void add_sv(OCBSatCorrection&                              ocb_corr,
                       SSR_ClockCorrectionSatelliteElement_r15* const clk_corr) {
        std::get<1>(ocb_corr) = clk_corr;
    }

    static void add_sv(OCBSatCorrection& ocb_corr, SSR_CodeBiasSatElement_r15* const code_corr) {
        std::get<2>(ocb_corr) = code_corr;
    }

    static void add_sv(OCBSatCorrection& ocb_corr, SSR_PhaseBiasSatElement_r16* const phase_corr) {
        std::get<3>(ocb_corr) = phase_corr;
    }

    template <class T>
    static void add_ocb_for_svs(std::map<uint16_t, OCBSatCorrection>& ocb_for_svs, T corr);

    /** Group the OCB corrections by SV-ID.
     *
     * LPP & SPARTN have different ways of structuring their correction
     * type and SV relationships:
     *
     * LPP                             | SPARTN
     * --------------------------------|-------------------------------
     * Correction Type                 | Each SV
     *  |                              |  |
     *  |- Correction for SV           |  |- All correction types for SV
     *
     * For LPP each correction type is sorted in a different IE and has a
     * list of SV in each of those IEs. This makes it very hard to easily
     * structure the code inorder for the SPARTN structure to be generated.
     * So instead we do most of the heavy lifting here and make a list of
     * SVs and have all the corrections types of correction stored next to
     * them.
     *
     * @param[in] orbit_corrs      orbit corrections.
     * @param[in] clock_corrs      clock corrections.
     * @param[in] code_bias_corrs  code corrections.
     * @param[in] phase_bias_corrs phase bias corrections.
     *
     * @return                     a map between SV-IDs and corrections for
     *                             that satellite.
     */
    static std::map<uint16_t, OCBSatCorrection>
    get_ocb_for_svs(const GNSS_SSR_OrbitCorrections_r15* orbit_corrs,
                    const GNSS_SSR_ClockCorrections_r15* clock_corrs,
                    const GNSS_SSR_CodeBias_r15*         code_bias_corrs,
                    const GNSS_SSR_PhaseBias_r16*        phase_bias_corrs);

    /** Generates a SPARTN bitmask from the output of
     * SPARTN_Generator::get_ocb_for_svs(). The size of mask is the second
     * largest available for each field for each constellation.
     *
     * @param[in] sv_ocbs                 a map of SV-IDs to a set of
     *                                    corrections (only the keys are
     *                                    used).
     *
     * @param[in] sat_mask_bit_count      size of the resulting bitmask.
     *
     * @param[in] sat_mask_bit_count_flag prepended to the start of the
     *                                    mask, to indicate it's size, as
     *                                    defined by the SPARTN ICD.
     *
     * @return                            a bitset of length defined by
     *                                    Constants::max_spartn_bit_count.
     */
    static std::bitset<Constants::max_spartn_bit_count>
    get_satellite_mask_from_ocbs(const std::map<uint16_t, OCBSatCorrection>& sv_ocbs,
                                 const uint16_t                              sat_mask_bit_count,
                                 const uint8_t sat_mask_bit_count_flag) {
        std::bitset<Constants::max_spartn_bit_count> sat_mask_bitset = sat_mask_bit_count_flag;
        sat_mask_bitset <<= sat_mask_bit_count - 2;

        const uint16_t sat_mask_length = sat_mask_bit_count - 3;
        static constexpr std::bitset<Constants::max_spartn_bit_count> new_sv = 1;

        for (const auto& sv_ocb : sv_ocbs) {
            sat_mask_bitset |= new_sv << (sat_mask_length - sv_ocb.first);
        }

        return sat_mask_bitset;
    }

    /** The value of a residual can either be stored in the b7 or b16 field
     * in LPP. This function figures out which it is then retrieves the
     * value.
     *
     * @param[in] res the residual struct from the LPP message.
     *
     * @return        the residual's numerical value or 99999 (error value)
     *                if it can't figure out where the residual is stored.
     *                It is safe to return 99999 as the error value because
     *                LPP does not allow values larger than 32767.
     */
    static long get_residual(const STEC_ResidualSatElement_r16* const res) {
        long this_residual;
        switch (res->stecResidualCorrection_r16.present) {
        case STEC_ResidualSatElement_r16__stecResidualCorrection_r16_PR_b7_r16: {
            this_residual = res->stecResidualCorrection_r16.choice.b7_r16;
            break;
        }
        case STEC_ResidualSatElement_r16__stecResidualCorrection_r16_PR_b16_r16: {
            this_residual = res->stecResidualCorrection_r16.choice.b16_r16;
            break;
        }
        default: return 99999;
        }
        return this_residual;
    }

    /** Retrieve the signal id from the `GNSS_SignalID` struct. LPP lets
     * the signal ID either be stored directly in this struct or nested
     * within the `ext1` attribute.
     *
     * @param[in] sig a reference to the struct where the signal ID is
     *                stored.
     *
     * @return        the signal ID's numerical value.
     */
    static uint8_t get_signalid(const GNSS_SignalID& sig) {
        return sig.ext1 ? *sig.ext1->gnss_SignalID_Ext_r15 : sig.gnss_SignalID;
    }

    /** It is not guaranteed that the `OCBCorrection` struct will have all
     * of it's correction fields assigned, which means searching when
     * trying to find the `GNSS_SystemTime`. Therefore this function
     * conveniently searching for this struct.
     *
     * @param[in] ocb_correction an `OCBCorrection` struct where it is not
     *                           guaranteed that all of the fields are
     *                           assigned
     *
     * @return                   a pointer to the first system time that
     *                           can be found. If no fields are set, a
     *                           nullptr is returned. This is safe as if
     *                           there is a correction assigned, LPP
     *                           guarantees that the `GNSS_SystemTime` will
     *                           also be defined.
     */
    static const GNSS_SystemTime* get_gnss_systemtime(const OCBCorrection& ocb_corrections) {
        if (ocb_corrections.orbit_corrs) {
            return &ocb_corrections.orbit_corrs->epochTime_r15;
        }

        if (ocb_corrections.clock_corrs) {
            return &ocb_corrections.clock_corrs->epochTime_r15;
        }

        if (ocb_corrections.code_bias_corrs) {
            return &ocb_corrections.code_bias_corrs->epochTime_r15;
        }

        if (ocb_corrections.phase_bias_corrs) {
            return &ocb_corrections.phase_bias_corrs->epochTime_r16;
        }

        return nullptr;
    }

    static const GNSS_SystemTime* get_gnss_systemtime(const HPACCorrection& hpac_corrections) {
        if (hpac_corrections.gridded_corrections) {
            return &hpac_corrections.gridded_corrections->epochTime_r16;
        }

        if (hpac_corrections.stec_corrections) {
            return &hpac_corrections.stec_corrections->epochTime_r16;
        }

        return nullptr;
    }

    /* Check if the satellite reference datum is ITRF. Reference datum is
     * only relevant to orbit.
     *
     * @param[in] orb point to orbit correction to be checked
     *
     * @return        true if reference datum is ITRF, false if not.
     */
    static inline bool check_is_itrf(const GNSS_SSR_OrbitCorrections_r15* const orb) {
        static constexpr auto itrf =
            GNSS_SSR_OrbitCorrections_r15__satelliteReferenceDatum_r15_itrf;

        return (bool)orb && orb->satelliteReferenceDatum_r15 == itrf;
    }

    /** Add a block to it's parent. In a function because it's not a
     * trivial operation.
     *
     * @param[out] parent block to have it's children updated.
     * @param[in]  child  child to add to parent's children.
     */
    static void add_block_to_block(const std::unique_ptr<SPARTN_Block>& parent,
                                   std::unique_ptr<SPARTN_Block>&       child) {
        parent->data.push(std::move(child));
    }

    /** Create and add a field to header.
     *
     * @param[out] header    the header to add the field to
     * @param[in]  id        id of the new field
     * @param[in]  bit_count number of bits in the new field
     * @param[in]  bits      bits in the new field
     */
    static void add_field_to_header(const std::unique_ptr<SPARTN_Message_Header>& header,
                                    const uint8_t id, const uint8_t bit_count, const int64_t bits) {
        std::unique_ptr<SPARTN_Field> new_field(new SPARTN_Field(id, bit_count, bits));
        header->fields.push(std::move(new_field));
    }

    /** Create and add a field to block.
     *
     * @param[out] block     the block to add the field to
     * @param[in]  id        id of the new field
     * @param[in]  bit_count number of bits in the new field
     * @param[in]  bits      bits in the new field
     */
    static void add_field_to_block(const std::unique_ptr<SPARTN_Block>& block, const uint8_t id,
                                   const uint8_t bit_count, const int64_t bits) {
        std::unique_ptr<SPARTN_Field> new_field(new SPARTN_Field(id, bit_count, bits));
        block->data.push(std::move(new_field));
    }

    /** Check a value is within a range defined by the minimum value, i.e.
     * range of -10..+10.
     *
     * @param[in] value     value to be checked.
     * @param[in] range_min the lower bound of the range.
     *
     * @return              true if value in range, false if not.
     */
    static bool check_within_range(double value, double range_min) {
        return (value > range_min) && (value < (-1 * range_min));
    }

    /** Convert a double to a long representation, using a range and
     * resolution.
     *
     * The range and resolution can be found in the SPARTN ICD.
     *
     * A pretty common usage pattern for this function is:
     *
     * ```
     *  if (SPARTN_Generator::check_within_range(
     *              val_dec, Constants::orbit_clock_spartn_rng_min))
     *  {
     *      add_field_to_block(
     *          orbit_block,
     *          20,
     *          14,
     *          SPARTN_Generator::encode_value_spartn(
     *              val_dec,
     *              Constants::orbit_clock_spartn_rng_min,
     *              Constants::orbit_clock_spartn_res));
     *  }
     *  ```
     *
     * @param[in] v_original double representation of number.
     * @param[in] range_min  minimum allowed.
     * @param[in] resolution resolution of the to be encoded value
     *
     * @return               encoded integer representation of the double.
     */
    static long encode_value_spartn(double v_original, double range_min, double resolution) {
        return std::lround((v_original - range_min) / resolution);
    }

    /** Convert a long representation of a double to a double, using it's
     * resolution.
     *
     * @param[in] v_encoded  encoded integer representation of the double.
     * @param[in] resolution resolution of the encoded integer.
     *
     * @return               decoded representation of double.
     */
    static double decode_value_lpp(long v_encoded, double resolution) {
        return (double)v_encoded * resolution;
    }

    const ProvideAssistanceData_r9_IEs_t* prov_ass_data_;

    /*
     * For each SV in each constellation, we must store the last known
     * IOD and time at which the IOD changed, for SF022
     */
    std::map<GNSS_ID__gnss_id, std::map<long, std::pair<int32_t, std::time_t>>> iods_ = {};

    // map of signal ids and discontinuities
    std::map<GNSS_ID__gnss_id, std::map<long, std::map<uint16_t, LastDiscontinuity>>>
        discontinuities_ = {};

    std::map<GNSS_ID__gnss_id, std::map<long, std::time_t>> last_generation_time_ = {};

    int8_t qualityoveride_ = 0;

    bool ublox_clock_correction = false;
    bool force_continuity  = false;

    std::unique_ptr<CorrectionPoint> last_correction_point_ = nullptr;

    std::vector<OCBCorrection>  ocb_corrections_  = {};
    std::vector<HPACCorrection> hpac_corrections_ = {};

    std::map<uint16_t, std::vector<uint16_t>> sv_intersections_ = {};
};
#endif
