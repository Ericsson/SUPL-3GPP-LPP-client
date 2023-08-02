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

UbloxReceiver::UbloxReceiver(Port port, interface::Interface& interface) UBLOX_NOEXCEPT
    : mPort(port),
      mInterface(interface),
      mSpartnSupport(false) {
    mParser          = new Parser();
    mSoftwareVersion = "unknown";
    mHardwareVersion = "unknown";

    // Clear residual data from the interface
    process();
    mParser->clear();

    // Load receiver configuration
    load_receiver_configuration();
}

UbloxReceiver::~UbloxReceiver() UBLOX_NOEXCEPT {
    if (mParser != nullptr) {
        delete mParser;
    }
}

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

void UbloxReceiver::enable_message(MessageId message_id) {
    auto key     = cfg_key_from_message_id(mPort, message_id);
    mConfig[key] = CfgValue::from_l(true);
}

void UbloxReceiver::disable_message(MessageId message_id) {
    auto key     = cfg_key_from_message_id(mPort, message_id);
    mConfig[key] = CfgValue::from_l(false);
}

void UbloxReceiver::poll_mon_ver() {
    process();
    mParser->clear();

    uint8_t buffer[64];
    auto    encoder             = Encoder{buffer, sizeof(buffer)};
    auto    poll_message_length = UbxMonVer::poll(encoder);
    mInterface.write(buffer, poll_message_length);

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
    mInterface.write(buffer, poll_message_length);

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
        break;
    }

    keys.push_back(CFG_KEY_RATE_MEAS);
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

#if 1
#define PRINT_CFG_VALUE(NAME, KEY)                                                                 \
    do {                                                                                           \
        if (mConfig.count(KEY) > 0) {                                                              \
            printf("    %08X %s: ", (KEY), (NAME));                                                \
            auto value = mConfig[KEY];                                                             \
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
            printf("    %08X %s: <not available>\n", (KEY), (NAME));                               \
        }                                                                                          \
    } while (false)

    printf("Configuration:\n");
    switch (mPort) {
    case Port::UART1:
        PRINT_CFG_VALUE("UART1-ENABLED", CFG_KEY_UART1_ENABLED);
        PRINT_CFG_VALUE("UART1-BAUDRATE", CFG_KEY_UART1_BAUDRATE);
        PRINT_CFG_VALUE("UART1-STOPBITS", CFG_KEY_UART1_STOPBITS);
        PRINT_CFG_VALUE("UART1-DATABITS", CFG_KEY_UART1_DATABITS);
        PRINT_CFG_VALUE("UART1-PARITY", CFG_KEY_UART1_PARITY);
        PRINT_CFG_VALUE("UART1INPROT-UBX", CFG_KEY_UART1INPROT_UBX);
        PRINT_CFG_VALUE("UART1INPROT-NMEA", CFG_KEY_UART1INPROT_NMEA);
        PRINT_CFG_VALUE("UART1INPROT-RTCM3X", CFG_KEY_UART1INPROT_RTCM3X);
        PRINT_CFG_VALUE("UART1INPROT-SPARTN", CFG_KEY_UART1INPROT_SPARTN);
        PRINT_CFG_VALUE("UART1OUTPROT-UBX", CFG_KEY_UART1OUTPROT_UBX);
        PRINT_CFG_VALUE("UART1OUTPROT-NMEA", CFG_KEY_UART1OUTPROT_NMEA);
        PRINT_CFG_VALUE("UART1OUTPROT-RTCM3X", CFG_KEY_UART1OUTPROT_RTCM3X);
        break;
    case Port::UART2:
        PRINT_CFG_VALUE("UART2-ENABLED", CFG_KEY_UART2_ENABLED);
        PRINT_CFG_VALUE("UART2-BAUDRATE", CFG_KEY_UART2_BAUDRATE);
        PRINT_CFG_VALUE("UART2-STOPBITS", CFG_KEY_UART2_STOPBITS);
        PRINT_CFG_VALUE("UART2-DATABITS", CFG_KEY_UART2_DATABITS);
        PRINT_CFG_VALUE("UART2-PARITY", CFG_KEY_UART2_PARITY);
        PRINT_CFG_VALUE("UART2INPROT-UBX", CFG_KEY_UART2INPROT_UBX);
        PRINT_CFG_VALUE("UART2INPROT-NMEA", CFG_KEY_UART2INPROT_NMEA);
        PRINT_CFG_VALUE("UART2INPROT-RTCM3X", CFG_KEY_UART2INPROT_RTCM3X);
        PRINT_CFG_VALUE("UART2INPROT-SPARTN", CFG_KEY_UART2INPROT_SPARTN);
        PRINT_CFG_VALUE("UART2OUTPROT-UBX", CFG_KEY_UART2OUTPROT_UBX);
        PRINT_CFG_VALUE("UART2OUTPROT-NMEA", CFG_KEY_UART2OUTPROT_NMEA);
        PRINT_CFG_VALUE("UART2OUTPROT-RTCM3X", CFG_KEY_UART2OUTPROT_RTCM3X);
        break;
    case Port::I2C:
        PRINT_CFG_VALUE("I2C-ENABLED", CFG_KEY_I2C_ENABLED);
        PRINT_CFG_VALUE("I2C-ADDRESS", CFG_KEY_I2C_ADDRESS);
        PRINT_CFG_VALUE("I2CINPROT-UBX", CFG_KEY_I2CINPROT_UBX);
        PRINT_CFG_VALUE("I2CINPROT-NMEA", CFG_KEY_I2CINPROT_NMEA);
        PRINT_CFG_VALUE("I2CINPROT-RTCM3X", CFG_KEY_I2CINPROT_RTCM3X);
        PRINT_CFG_VALUE("I2CINPROT-SPARTN", CFG_KEY_I2CINPROT_SPARTN);
        PRINT_CFG_VALUE("I2COUTPROT-UBX", CFG_KEY_I2COUTPROT_UBX);
        PRINT_CFG_VALUE("I2COUTPROT-NMEA", CFG_KEY_I2COUTPROT_NMEA);
        PRINT_CFG_VALUE("I2COUTPROT-RTCM3X", CFG_KEY_I2COUTPROT_RTCM3X);
        break;
    case Port::USB:
        PRINT_CFG_VALUE("USB-ENABLED", CFG_KEY_USB_ENABLED);
        PRINT_CFG_VALUE("USBINPROT-UBX", CFG_KEY_USBINPROT_UBX);
        PRINT_CFG_VALUE("USBINPROT-NMEA", CFG_KEY_USBINPROT_NMEA);
        PRINT_CFG_VALUE("USBINPROT-RTCM3X", CFG_KEY_USBINPROT_RTCM3X);
        PRINT_CFG_VALUE("USBINPROT-SPARTN", CFG_KEY_USBINPROT_SPARTN);
        PRINT_CFG_VALUE("USBOUTPROT-UBX", CFG_KEY_USBOUTPROT_UBX);
        PRINT_CFG_VALUE("USBOUTPROT-NMEA", CFG_KEY_USBOUTPROT_NMEA);
        PRINT_CFG_VALUE("USBOUTPROT-RTCM3X", CFG_KEY_USBOUTPROT_RTCM3X);
        break;
    }

#undef PRINT_CFG_VALUE
#endif
}

void UbloxReceiver::store_receiver_configuration() {
    for (auto& kvp : mConfig) {
        auto key   = kvp.first;
        auto value = kvp.second;

        uint8_t buffer[1024];
        auto    encoder = Encoder{buffer, sizeof(buffer)};
        auto    length  = UbxCfgValset::set(encoder, CFG_LAYER_RAM, key, value);
        mInterface.write(buffer, length);

        auto message = wait_for_specific_message<UbxAckAck>(true, true);
        if (!message) {
            printf("*** failed to write %08X configuration ***\n", key);
        }
    }
}

template <typename T>
std::unique_ptr<T> UbloxReceiver::wait_for_specific_message(bool expect_ack, bool expect_nak) {
    for (;;) {
        process();

        auto message = try_parse();
        if (message != nullptr) {
            auto response = dynamic_cast<T*>(message.get());
            if (response != nullptr) {
                auto casted_message = std::unique_ptr<T>(response);
                message.release();
                return casted_message;
            }

            if (expect_ack) {
                auto ack = dynamic_cast<UbxAckAck*>(message.get());
                if (ack != nullptr && ack->cls_id() == T::CLASS_ID &&
                    ack->msg_id() == T::MESSAGE_ID) {
                    return nullptr;
                }
            }

            if (expect_nak) {
                auto nak = dynamic_cast<UbxAckNak*>(message.get());
                if (nak != nullptr && nak->cls_id() == T::CLASS_ID &&
                    nak->msg_id() == T::MESSAGE_ID) {
                    return nullptr;
                }
            }
        }

        mInterface.wait_for_read();
    }
}

void UbloxReceiver::configure() {
    poll_mon_ver();

    // TODO:
    mConfig.clear();

    // Enable fixed DGNSS mode
    mConfig[CFG_KEY_NAVHPG_DGNSSMODE] = CfgValue::from_u1(3 /* RTK_FIXED */);

    // Enable information messages
    switch (mPort) {
    case Port::UART1:
        mConfig[CFG_KEY_INFMSG_UBX_UART1] =
            CfgValue::from_u1(INF_ERROR | INF_WARNING | INF_NOTICE | INF_TEST | INF_DEBUG);
        break;
    case Port::UART2:
        mConfig[CFG_KEY_INFMSG_UBX_UART2] =
            CfgValue::from_u1(INF_ERROR | INF_WARNING | INF_NOTICE | INF_TEST | INF_DEBUG);
        break;
    case Port::I2C:
        mConfig[CFG_KEY_INFMSG_UBX_I2C] =
            CfgValue::from_u1(INF_ERROR | INF_WARNING | INF_NOTICE | INF_TEST | INF_DEBUG);
        break;
    case Port::USB:
        mConfig[CFG_KEY_INFMSG_UBX_USB] =
            CfgValue::from_u1(INF_ERROR | INF_WARNING | INF_NOTICE | INF_TEST | INF_DEBUG);
        break;
    }

    store_receiver_configuration();
}

void UbloxReceiver::process() {
    if (mInterface.can_read()) {
        uint8_t buffer[256];
        auto    length = mInterface.read(buffer, sizeof(buffer));
        if (length <= 0) {
            throw std::runtime_error("Failed to read from interface");
        }

        if (!mParser->append(buffer, length)) {
            throw std::runtime_error("Failed to append to parser");
        }
    }
}

std::unique_ptr<Message> UbloxReceiver::wait_for_message() {
    for (;;) {
        process();

        auto message = try_parse();
        if (message) {
            return message;
        }

        mInterface.wait_for_read();
    }
}

std::unique_ptr<Message> UbloxReceiver::try_parse() {
    return mParser->try_parse();
}

}  // namespace ublox
}  // namespace receiver
