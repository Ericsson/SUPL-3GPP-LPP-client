#include "client.hpp"

#include <loglet/loglet.hpp>

#define LOGLET_CURRENT_MODULE "client"

static void process_rtcm(Config const& config, lpp::Message message) {
    INFOF("processing RTCM message");
}

static void process_spartn_old(Config const& config, lpp::Message message) {
    INFOF("processing SPARTN_OLD message");
}

static void process_spartn(Config const& config, lpp::Message message) {
    INFOF("processing SPARTN message");
}

static void process_xer(Config const& config, lpp::Message message) {
    INFOF("processing XER message");
}

static void process_uper(Config const& config, lpp::Message message) {
    INFOF("processing UPER message");
}

static void process_lpp_rtcm(Config const& config, lpp::Message message) {
    INFOF("processing LPP_RTCM message");
}

void process_assistance_data(Config const& config, lpp::Message message) {
    INFOF("received assistance data");
}
