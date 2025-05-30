
namespace lpp2frame_rtcm {

static args::Group gGroup{"LPP to framed RTCM:"};
static args::Flag  gEnable{
    gGroup,
    "enable",
    "Enable wrapping LPP messages in RTCM messages",
     {"lpp2fr"},
};

static args::ValueFlag<int> gRtcmMessageId{
    gGroup,
    "id",
    "RTCM message ID for LPP messages",
    {"l2f-id"},
};

static args::Flag gOutputInRtcm{
    gGroup,
    "rtcm",
    "Output as both 'lrf' and 'rtcm' messages",
    {"l2f-rtcm"},
};

static void setup() {
    gRtcmMessageId.HelpDefault("355");
}

static void parse(Config* config) {
    auto& lpp2frame_rtcm           = config->lpp2frame_rtcm;
    lpp2frame_rtcm.enabled         = false;
    lpp2frame_rtcm.rtcm_message_id = 355;
    lpp2frame_rtcm.output_in_rtcm  = gOutputInRtcm.Get();

    if (gEnable) lpp2frame_rtcm.enabled = true;

    if (gRtcmMessageId) {
        lpp2frame_rtcm.rtcm_message_id = gRtcmMessageId.Get();
        if (lpp2frame_rtcm.rtcm_message_id < 0 || lpp2frame_rtcm.rtcm_message_id > 1023) {
            throw args::ValidationError("--l2f-id must be between 0 and 1023, got `" +
                                        std::to_string(lpp2frame_rtcm.rtcm_message_id) + "`");
        }
    }
}

static void dump(Lpp2FrameRtcmConfig const& config) {
    DEBUGF("status: %s", config.enabled ? "enabled" : "disabled");
    if (!config.enabled) return;

    DEBUGF("rtcm_message_id: %d", config.rtcm_message_id);
    DEBUGF("output_in_rtcm:  %s", config.output_in_rtcm ? "true" : "false");
}

}  // namespace lpp2frame_rtcm
