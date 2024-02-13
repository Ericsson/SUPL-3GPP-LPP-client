#include "receiver.hpp"
#include <cinttypes>
#include <interface/interface.hpp>
#include <stdexcept>
#include "encoder.hpp"
#include "message.hpp"
#include "parser.hpp"
#include "ubx_ack_ack.hpp"
#include "ubx_ack_nak.hpp"
#include "ubx_cfg_valget.hpp"
#include "ubx_cfg_valset.hpp"
#include "ubx_mon_ver.hpp"

namespace receiver {
namespace ublox {

static CfgKey cfg_key_from_message_id(Port port, MessageId message_id) {
    switch (message_id) {
    case MessageId::UbxNavPvt: {
        switch (port) {
        case Port::UART1: return CFG_KEY_MSGOUT_UBX_NAV_PVT_UART1;
        case Port::UART2: return CFG_KEY_MSGOUT_UBX_NAV_PVT_UART2;
        case Port::I2C: return CFG_KEY_MSGOUT_UBX_NAV_PVT_I2C;
        case Port::USB: return CFG_KEY_MSGOUT_UBX_NAV_PVT_USB;
        }
        UBLOX_UNREACHABLE();
    }
    }

    UBLOX_UNREACHABLE();
}
static const char* configuration_name_from_key(CfgKey key) {
    switch (key) {
    case CFG_KEY_UART1_ENABLED: return "UART1-ENABLED";
    case CFG_KEY_UART1_BAUDRATE: return "UART1-BAUDRATE";
    case CFG_KEY_UART1_STOPBITS: return "UART1-STOPBITS";
    case CFG_KEY_UART1_DATABITS: return "UART1-DATABITS";
    case CFG_KEY_UART1_PARITY: return "UART1-PARITY";
    case CFG_KEY_UART1INPROT_UBX: return "UART1INPROT-UBX";
    case CFG_KEY_UART1INPROT_NMEA: return "UART1INPROT-NMEA";
    case CFG_KEY_UART1INPROT_RTCM3X: return "UART1INPROT-RTCM3X";
    case CFG_KEY_UART1INPROT_SPARTN: return "UART1INPROT-SPARTN";
    case CFG_KEY_UART1OUTPROT_UBX: return "UART1OUTPROT-UBX";
    case CFG_KEY_UART1OUTPROT_NMEA: return "UART1OUTPROT-NMEA";
    case CFG_KEY_UART1OUTPROT_RTCM3X: return "UART1OUTPROT-RTCM3X";
    case CFG_KEY_UART2_ENABLED: return "UART2-ENABLED";
    case CFG_KEY_UART2_BAUDRATE: return "UART2-BAUDRATE";
    case CFG_KEY_UART2_STOPBITS: return "UART2-STOPBITS";
    case CFG_KEY_UART2_DATABITS: return "UART2-DATABITS";
    case CFG_KEY_UART2_PARITY: return "UART2-PARITY";
    case CFG_KEY_UART2INPROT_UBX: return "UART2INPROT-UBX";
    case CFG_KEY_UART2INPROT_NMEA: return "UART2INPROT-NMEA";
    case CFG_KEY_UART2INPROT_RTCM3X: return "UART2INPROT-RTCM3X";
    case CFG_KEY_UART2INPROT_SPARTN: return "UART2INPROT-SPARTN";
    case CFG_KEY_UART2OUTPROT_UBX: return "UART2OUTPROT-UBX";
    case CFG_KEY_UART2OUTPROT_NMEA: return "UART2OUTPROT-NMEA";
    case CFG_KEY_UART2OUTPROT_RTCM3X: return "UART2OUTPROT-RTCM3X";
    case CFG_KEY_I2C_ENABLED: return "I2C-ENABLED";
    case CFG_KEY_I2C_ADDRESS: return "I2C-ADDRESS";
    case CFG_KEY_I2CINPROT_UBX: return "I2CINPROT-UBX";
    case CFG_KEY_I2CINPROT_NMEA: return "I2CINPROT-NMEA";
    case CFG_KEY_I2CINPROT_RTCM3X: return "I2CINPROT-RTCM3X";
    case CFG_KEY_I2CINPROT_SPARTN: return "I2CINPROT-SPARTN";
    case CFG_KEY_I2COUTPROT_UBX: return "I2COUTPROT-UBX";
    case CFG_KEY_I2COUTPROT_NMEA: return "I2COUTPROT-NMEA";
    case CFG_KEY_I2COUTPROT_RTCM3X: return "I2COUTPROT-RTCM3X";
    case CFG_KEY_USB_ENABLED: return "USB-ENABLED";
    case CFG_KEY_USBINPROT_UBX: return "USBINPROT-UBX";
    case CFG_KEY_USBINPROT_NMEA: return "USBINPROT-NMEA";
    case CFG_KEY_USBINPROT_RTCM3X: return "USBINPROT-RTCM3X";
    case CFG_KEY_USBINPROT_SPARTN: return "USBINPROT-SPARTN";
    case CFG_KEY_USBOUTPROT_UBX: return "USBOUTPROT-UBX";
    case CFG_KEY_USBOUTPROT_NMEA: return "USBOUTPROT-NMEA";
    case CFG_KEY_USBOUTPROT_RTCM3X: return "USBOUTPROT-RTCM3X";
    case CFG_KEY_RATE_MEAS: return "RATE-MEAS";
    case CFG_KEY_NAVHPG_DGNSSMODE: return "NAVHPG-DGNSSMODE";
    case CFG_KEY_MSGOUT_UBX_NAV_PVT_UART1: return "MSGOUT-UBX-NAV-PVT-UART1";
    case CFG_KEY_MSGOUT_UBX_NAV_PVT_UART2: return "MSGOUT-UBX-NAV-PVT-UART2";
    case CFG_KEY_MSGOUT_UBX_NAV_PVT_I2C: return "MSGOUT-UBX-NAV-PVT-I2C";
    case CFG_KEY_MSGOUT_UBX_NAV_PVT_USB: return "MSGOUT-UBX-NAV-PVT-USB";
    case CFG_KEY_INFMSG_UBX_UART1: return "INFMSG-UBX-UART1";
    case CFG_KEY_INFMSG_UBX_UART2: return "INFMSG-UBX-UART2";
    case CFG_KEY_INFMSG_UBX_I2C: return "INFMSG-UBX-I2C";
    case CFG_KEY_INFMSG_UBX_USB: return "INFMSG-UBX-USB";
    default: return "<unknown>";
    }
}

static void print_configuration(const std::unordered_map<CfgKey, CfgValue>& configuration,
                                Port                                        port) {
#if 1
#define PRINT_CFG_VALUE(KEY)                                                                       \
    do {                                                                                           \
        printf("    %08X %-26s: ", (KEY), configuration_name_from_key(KEY));                       \
        auto it = configuration.find(KEY);                                                         \
        if (it != configuration.end()) {                                                           \
            auto value = it->second;                                                               \
            switch (value.type()) {                                                                \
            case CfgValue::U1:                                                                     \
                printf("%02X unsigned=%u signed=%i\n", value.u1(),                                 \
                       static_cast<uint32_t>(value.u1()), static_cast<int32_t>(value.u1()));       \
                break;                                                                             \
            case CfgValue::U2:                                                                     \
                printf("%04X unsigned=%u signed=%i\n", value.u2(),                                 \
                       static_cast<uint32_t>(value.u2()), static_cast<int32_t>(value.u2()));       \
                break;                                                                             \
            case CfgValue::U4:                                                                     \
                printf("%08X unsigned=%u signed=%i\n", value.u4(),                                 \
                       static_cast<uint32_t>(value.u4()), static_cast<int32_t>(value.u4()));       \
                break;                                                                             \
            case CfgValue::U8:                                                                     \
                printf("%016" PRIx64 " unsigned=%" PRIu64 " signed=%" PRId64 "\n", value.u8(),     \
                       static_cast<uint64_t>(value.u8()), static_cast<int64_t>(value.u8()));       \
                break;                                                                             \
            case CfgValue::L: printf("%s\n", value.l() ? "true" : "false"); break;                 \
            default: printf("<unknown>\n"); break;                                                 \
            }                                                                                      \
        } else {                                                                                   \
            printf("<not available>\n");                                                           \
        }                                                                                          \
    } while (false)

    printf("Configuration:\n");
    switch (port) {
    case Port::UART1:
        PRINT_CFG_VALUE(CFG_KEY_UART1_ENABLED);
        PRINT_CFG_VALUE(CFG_KEY_UART1_BAUDRATE);
        PRINT_CFG_VALUE(CFG_KEY_UART1_STOPBITS);
        PRINT_CFG_VALUE(CFG_KEY_UART1_DATABITS);
        PRINT_CFG_VALUE(CFG_KEY_UART1_PARITY);
        PRINT_CFG_VALUE(CFG_KEY_UART1INPROT_UBX);
        PRINT_CFG_VALUE(CFG_KEY_UART1INPROT_NMEA);
        PRINT_CFG_VALUE(CFG_KEY_UART1INPROT_RTCM3X);
        PRINT_CFG_VALUE(CFG_KEY_UART1INPROT_SPARTN);
        PRINT_CFG_VALUE(CFG_KEY_UART1OUTPROT_UBX);
        PRINT_CFG_VALUE(CFG_KEY_UART1OUTPROT_NMEA);
        PRINT_CFG_VALUE(CFG_KEY_UART1OUTPROT_RTCM3X);
        PRINT_CFG_VALUE(CFG_KEY_INFMSG_UBX_UART1);
        break;
    case Port::UART2:
        PRINT_CFG_VALUE(CFG_KEY_UART2_ENABLED);
        PRINT_CFG_VALUE(CFG_KEY_UART2_BAUDRATE);
        PRINT_CFG_VALUE(CFG_KEY_UART2_STOPBITS);
        PRINT_CFG_VALUE(CFG_KEY_UART2_DATABITS);
        PRINT_CFG_VALUE(CFG_KEY_UART2_PARITY);
        PRINT_CFG_VALUE(CFG_KEY_UART2INPROT_UBX);
        PRINT_CFG_VALUE(CFG_KEY_UART2INPROT_NMEA);
        PRINT_CFG_VALUE(CFG_KEY_UART2INPROT_RTCM3X);
        PRINT_CFG_VALUE(CFG_KEY_UART2INPROT_SPARTN);
        PRINT_CFG_VALUE(CFG_KEY_UART2OUTPROT_UBX);
        PRINT_CFG_VALUE(CFG_KEY_UART2OUTPROT_NMEA);
        PRINT_CFG_VALUE(CFG_KEY_UART2OUTPROT_RTCM3X);
        PRINT_CFG_VALUE(CFG_KEY_INFMSG_UBX_UART2);
        break;
    case Port::I2C:
        PRINT_CFG_VALUE(CFG_KEY_I2C_ENABLED);
        PRINT_CFG_VALUE(CFG_KEY_I2C_ADDRESS);
        PRINT_CFG_VALUE(CFG_KEY_I2CINPROT_UBX);
        PRINT_CFG_VALUE(CFG_KEY_I2CINPROT_NMEA);
        PRINT_CFG_VALUE(CFG_KEY_I2CINPROT_RTCM3X);
        PRINT_CFG_VALUE(CFG_KEY_I2CINPROT_SPARTN);
        PRINT_CFG_VALUE(CFG_KEY_I2COUTPROT_UBX);
        PRINT_CFG_VALUE(CFG_KEY_I2COUTPROT_NMEA);
        PRINT_CFG_VALUE(CFG_KEY_I2COUTPROT_RTCM3X);
        PRINT_CFG_VALUE(CFG_KEY_INFMSG_UBX_I2C);
        break;
    case Port::USB:
        PRINT_CFG_VALUE(CFG_KEY_USB_ENABLED);
        PRINT_CFG_VALUE(CFG_KEY_USBINPROT_UBX);
        PRINT_CFG_VALUE(CFG_KEY_USBINPROT_NMEA);
        PRINT_CFG_VALUE(CFG_KEY_USBINPROT_RTCM3X);
        PRINT_CFG_VALUE(CFG_KEY_USBINPROT_SPARTN);
        PRINT_CFG_VALUE(CFG_KEY_USBOUTPROT_UBX);
        PRINT_CFG_VALUE(CFG_KEY_USBOUTPROT_NMEA);
        PRINT_CFG_VALUE(CFG_KEY_USBOUTPROT_RTCM3X);
        PRINT_CFG_VALUE(CFG_KEY_INFMSG_UBX_USB);
        break;
    }

    PRINT_CFG_VALUE(CFG_KEY_RATE_MEAS);
    PRINT_CFG_VALUE(CFG_KEY_NAVHPG_DGNSSMODE);
    PRINT_CFG_VALUE(cfg_key_from_message_id(port, MessageId::UbxNavPvt));

#undef PRINT_CFG_VALUE
#endif
}

//
//
//

UbloxReceiver::UbloxReceiver(Port                                  port,
                             std::unique_ptr<interface::Interface> interface) UBLOX_NOEXCEPT
    : mPort(port),
      mInterface(std::move(interface)),
      mSpartnSupport(false) {
    mParser          = new Parser();
    mSoftwareVersion = "unknown";
    mHardwareVersion = "unknown";

    // Clear residual data from the interface
    process();
    mParser->clear();
}

UbloxReceiver::~UbloxReceiver() UBLOX_NOEXCEPT {
    if (mParser != nullptr) {
        delete mParser;
    }
}

void UbloxReceiver::enable_message(MessageId message_id) {
    auto key     = cfg_key_from_message_id(mPort, message_id);
    mConfig[key] = CfgValue::from_u1(1);
}

void UbloxReceiver::disable_message(MessageId message_id) {
    auto key     = cfg_key_from_message_id(mPort, message_id);
    mConfig[key] = CfgValue::from_u1(0);
}

void UbloxReceiver::poll_mon_ver() {
    process();
    mParser->clear();

    uint8_t buffer[64];
    auto    encoder             = Encoder{buffer, sizeof(buffer)};
    auto    poll_message_length = UbxMonVer::poll(encoder);
    mInterface->write(buffer, poll_message_length);

    auto message = wait_for_specific_message<UbxMonVer>(false, false);
    if (message) {
        mSoftwareVersion = message->sw_version();
        mHardwareVersion = message->hw_version();
        mExtensions      = message->extensions();
    }
}

bool UbloxReceiver::poll_cfg_valget(CfgKey key, CfgValue& value) {
    process();
    mParser->clear();

    std::vector<CfgKey> keys{key};

    uint8_t buffer[1024];
    auto    encoder             = Encoder{buffer, sizeof(buffer)};
    auto    poll_message_length = UbxCfgValget::poll(encoder, CFG_LAYER_RAM, 0, keys);
    mInterface->write(buffer, poll_message_length);

    auto message = wait_for_specific_message<UbxCfgValget>(true, true);
    if (message) {
        value = message->get(key);
        return true;
    } else {
        return false;
    }
}

void UbloxReceiver::load_receiver_configuration() {
    std::vector<CfgKey> keys;

    switch (mPort) {
    case Port::UART1:
        keys.push_back(CFG_KEY_UART1_ENABLED);
        keys.push_back(CFG_KEY_UART1_BAUDRATE);
        keys.push_back(CFG_KEY_UART1_STOPBITS);
        keys.push_back(CFG_KEY_UART1_DATABITS);
        keys.push_back(CFG_KEY_UART1_PARITY);
        keys.push_back(CFG_KEY_UART1INPROT_UBX);
        keys.push_back(CFG_KEY_UART1INPROT_NMEA);
        keys.push_back(CFG_KEY_UART1INPROT_RTCM3X);
        keys.push_back(CFG_KEY_UART1INPROT_SPARTN);
        keys.push_back(CFG_KEY_UART1OUTPROT_UBX);
        keys.push_back(CFG_KEY_UART1OUTPROT_NMEA);
        keys.push_back(CFG_KEY_UART1OUTPROT_RTCM3X);
        keys.push_back(CFG_KEY_INFMSG_UBX_UART1);
        break;
    case Port::UART2:
        keys.push_back(CFG_KEY_UART2_ENABLED);
        keys.push_back(CFG_KEY_UART2_BAUDRATE);
        keys.push_back(CFG_KEY_UART2_STOPBITS);
        keys.push_back(CFG_KEY_UART2_DATABITS);
        keys.push_back(CFG_KEY_UART2_PARITY);
        keys.push_back(CFG_KEY_UART2INPROT_UBX);
        keys.push_back(CFG_KEY_UART2INPROT_NMEA);
        keys.push_back(CFG_KEY_UART2INPROT_RTCM3X);
        keys.push_back(CFG_KEY_UART2INPROT_SPARTN);
        keys.push_back(CFG_KEY_UART2OUTPROT_UBX);
        keys.push_back(CFG_KEY_UART2OUTPROT_NMEA);
        keys.push_back(CFG_KEY_UART2OUTPROT_RTCM3X);
        keys.push_back(CFG_KEY_INFMSG_UBX_UART2);
        break;
    case Port::I2C:
        keys.push_back(CFG_KEY_I2C_ENABLED);
        keys.push_back(CFG_KEY_I2C_ADDRESS);
        keys.push_back(CFG_KEY_I2CINPROT_UBX);
        keys.push_back(CFG_KEY_I2CINPROT_NMEA);
        keys.push_back(CFG_KEY_I2CINPROT_RTCM3X);
        keys.push_back(CFG_KEY_I2CINPROT_SPARTN);
        keys.push_back(CFG_KEY_I2COUTPROT_UBX);
        keys.push_back(CFG_KEY_I2COUTPROT_NMEA);
        keys.push_back(CFG_KEY_I2COUTPROT_RTCM3X);
        keys.push_back(CFG_KEY_INFMSG_UBX_I2C);
        break;
    case Port::USB:
        keys.push_back(CFG_KEY_USB_ENABLED);
        keys.push_back(CFG_KEY_USBINPROT_UBX);
        keys.push_back(CFG_KEY_USBINPROT_NMEA);
        keys.push_back(CFG_KEY_USBINPROT_RTCM3X);
        keys.push_back(CFG_KEY_USBINPROT_SPARTN);
        keys.push_back(CFG_KEY_USBOUTPROT_UBX);
        keys.push_back(CFG_KEY_USBOUTPROT_NMEA);
        keys.push_back(CFG_KEY_USBOUTPROT_RTCM3X);
        keys.push_back(CFG_KEY_INFMSG_UBX_USB);
        break;
    }

    keys.push_back(CFG_KEY_RATE_MEAS);
    keys.push_back(CFG_KEY_NAVHPG_DGNSSMODE);
    keys.push_back(cfg_key_from_message_id(mPort, MessageId::UbxNavPvt));

    // NOTE: Although, requesting every configuration key one-by-one seems stupid, it is necessary
    // because the receiver will return UBX-ACK-NAK if any of the requested keys are invalid. E.g.,
    // if we ask for UART1INPORT_SPARTN and the receiver does not support SPARTN, it will return
    // UBX-ACK-NAK, thus preventing us from getting the rest of the configuration.
    for (auto key : keys) {
        CfgValue value{};
        if (poll_cfg_valget(key, value)) {
            if (key == CFG_KEY_USBINPROT_SPARTN || key == CFG_KEY_I2CINPROT_SPARTN ||
                key == CFG_KEY_UART1INPROT_SPARTN || key == CFG_KEY_UART2INPROT_SPARTN) {
                mSpartnSupport = true;
            }

            mConfig[key] = value;
        }
    }

    print_configuration(mConfig, mPort);
}

void UbloxReceiver::store_receiver_configuration() {
    for (auto& kvp : mConfig) {
        auto key   = kvp.first;
        auto value = kvp.second;

        uint8_t buffer[1024];
        auto    encoder = Encoder{buffer, sizeof(buffer)};
        auto    length  = UbxCfgValset::set(encoder, CFG_LAYER_RAM, key, value);
        if (length == 0) {
            continue;
        }

        mInterface->write(buffer, length);
        wait_for_specific_message<UbxAckAck>(true, true);
    }
}

void UbloxReceiver::load_configuration() {
    load_receiver_configuration();

    // By default, enable UBX-NAV-PVT
    enable_message(MessageId::UbxNavPvt);
}

void UbloxReceiver::store_configuration() {
    poll_mon_ver();

    // Enable fixed DGNSS mode
    mConfig[CFG_KEY_NAVHPG_DGNSSMODE] = CfgValue::from_u1(3 /* RTK_FIXED */);

    // Enable information messages
    switch (mPort) {
    case Port::UART1:
        mConfig[CFG_KEY_INFMSG_UBX_UART1] =
            CfgValue::from_u1(INF_ERROR | INF_WARNING | INF_NOTICE | INF_TEST | INF_DEBUG);
        mConfig[CFG_KEY_UART1OUTPROT_UBX]  = CfgValue::from_l(true);
        mConfig[CFG_KEY_UART1OUTPROT_NMEA] = CfgValue::from_l(false);
        break;
    case Port::UART2:
        mConfig[CFG_KEY_INFMSG_UBX_UART2] =
            CfgValue::from_u1(INF_ERROR | INF_WARNING | INF_NOTICE | INF_TEST | INF_DEBUG);
        mConfig[CFG_KEY_UART2OUTPROT_UBX]  = CfgValue::from_l(true);
        mConfig[CFG_KEY_UART2OUTPROT_NMEA] = CfgValue::from_l(false);
        break;
    case Port::I2C:
        mConfig[CFG_KEY_INFMSG_UBX_I2C] =
            CfgValue::from_u1(INF_ERROR | INF_WARNING | INF_NOTICE | INF_TEST | INF_DEBUG);
        mConfig[CFG_KEY_I2COUTPROT_UBX]  = CfgValue::from_l(true);
        mConfig[CFG_KEY_I2COUTPROT_NMEA] = CfgValue::from_l(false);
        break;
    case Port::USB:
        mConfig[CFG_KEY_INFMSG_UBX_USB] =
            CfgValue::from_u1(INF_ERROR | INF_WARNING | INF_NOTICE | INF_TEST | INF_DEBUG);
        mConfig[CFG_KEY_USBOUTPROT_UBX]  = CfgValue::from_l(true);
        mConfig[CFG_KEY_USBOUTPROT_NMEA] = CfgValue::from_l(false);
        break;
    }

    store_receiver_configuration();
    print_configuration(mConfig, mPort);
}

void UbloxReceiver::process() {
    if (!mInterface->is_open()) {
        mInterface->open();
    }

    if (mInterface->can_read()) {
        uint8_t buffer[1024];
        auto    length = mInterface->read(buffer, sizeof(buffer));
        if (length <= 0) {
            // This will only happen if the interface is closed or disconnected. In this case, we
            // will try to re-open the interface in the next iteration of the main loop.
            mInterface->close();
            return;
        }

        // Ignore the return value, this should never fail if the parser buffer is >= 1024 bytes
        mParser->append(buffer, length);
    }
}

std::unique_ptr<Message> UbloxReceiver::wait_for_message() {
    for (;;) {
        process();

        auto message = try_parse();
        if (message) {
            return message;
        }

        mInterface->wait_for_read();
    }
}

std::unique_ptr<Message> UbloxReceiver::try_parse() {
    return mParser->try_parse();
}

template <typename T>
std::unique_ptr<T> UbloxReceiver::wait_for_specific_message(bool expect_ack, bool expect_nak) {
    for (;;) {
        auto message = wait_for_message();
        message->print();

        auto response = dynamic_cast<T*>(message.get());
        if (response != nullptr) {
            auto casted_message = std::unique_ptr<T>(response);
            message.release();
            return casted_message;
        }

        if (expect_ack) {
            auto ack = dynamic_cast<UbxAckAck*>(message.get());
            if (ack != nullptr && ack->cls_id() == T::CLASS_ID && ack->msg_id() == T::MESSAGE_ID) {
                return nullptr;
            }
        }

        if (expect_nak) {
            auto nak = dynamic_cast<UbxAckNak*>(message.get());
            if (nak != nullptr && nak->cls_id() == T::CLASS_ID && nak->msg_id() == T::MESSAGE_ID) {
                return nullptr;
            }
        }
    }
}

}  // namespace ublox
}  // namespace receiver
