#include <chrono>
#include <format/ubx/decoder.hpp>
#include <format/ubx/encoder.hpp>
#include <format/ubx/messages/cfg_valget.hpp>
#include <format/ubx/messages/cfg_valset.hpp>
#include <format/ubx/parser.hpp>
#include <io/output.hpp>
#include <iomanip>
#include <loglet/loglet.hpp>
#include <thread>
#include "ubx_options.hpp"

LOGLET_MODULE(ubx_config);
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF(ubx_config)

UbxConfigApplicator::UbxConfigApplicator(const UbxConfigConfig& config,
                                         scheduler::Scheduler&  scheduler)
    : config_(config), scheduler_(scheduler) {
    collected_values_.reserve(1000);
}

std::vector<uint8_t> UbxConfigApplicator::create_cfg_valset_message(
    std::vector<std::pair<format::ubx::CfgKey, format::ubx::CfgValue>> const& options) {
    if (options.empty()) {
        return {};
    }

    size_t total_size = 0;
    for (auto const& entry : options) {
        auto const& value = entry.second;
        total_size += 8 + 4 + value.size();
    }

    std::vector<uint8_t> message;
    message.reserve(total_size);

    format::ubx::CfgLayer layers =
        format::ubx::CFG_LAYER_RAM | format::ubx::CFG_LAYER_BBR | format::ubx::CFG_LAYER_FLASH;

    for (auto const& entry : options) {
        auto const&          key      = entry.first;
        auto const&          value    = entry.second;
        size_t               msg_size = 64;
        std::vector<uint8_t> temp_buffer(msg_size);
        format::ubx::Encoder encoder(temp_buffer.data(), temp_buffer.size());

        auto bytes_written = format::ubx::UbxCfgValset::set(encoder, layers, key, value);
        if (bytes_written > 0) {
            message.insert(message.end(), temp_buffer.begin(), temp_buffer.begin() + bytes_written);
        } else {
            ERRORF("failed to encode CFG-VALSET for key 0x%08X", key);
        }
    }

    return message;
}

std::vector<uint8_t>
UbxConfigApplicator::create_cfg_valget_message(std::vector<format::ubx::CfgKey> const& keys,
                                               uint16_t                                position) {
    if (keys.empty()) {
        return {};
    }

    std::vector<format::ubx::CfgKey> limited_keys;
    size_t                           key_count = std::min(keys.size(), size_t(64));
    limited_keys.reserve(key_count);
    for (size_t i = 0; i < key_count; i++) {
        limited_keys.push_back(keys[i]);
    }

    size_t               msg_size = 64 + (limited_keys.size() * 4);
    std::vector<uint8_t> message(msg_size);
    format::ubx::Encoder encoder(message.data(), message.size());

    auto bytes_written = format::ubx::UbxCfgValget::poll(encoder, format::ubx::CFG_LAYER_RAM,
                                                         position, limited_keys);

    if (bytes_written == 0) {
        ERRORF("failed to encode CFG-VALGET message");
        return {};
    }

    message.resize(bytes_written);
    return message;
}

std::vector<uint8_t> UbxConfigApplicator::create_cfg_valget_all_message(uint16_t position) {
    std::vector<format::ubx::CfgKey> wildcard_keys = {0x0fffffff};

    size_t               msg_size = 64;
    std::vector<uint8_t> message(msg_size);
    format::ubx::Encoder encoder(message.data(), message.size());

    auto bytes_written = format::ubx::UbxCfgValget::poll(encoder, format::ubx::CFG_LAYER_RAM,
                                                         position, wildcard_keys);

    if (bytes_written == 0) {
        ERRORF("failed to encode CFG-VALGET all message");
        return {};
    }

    message.resize(bytes_written);
    return message;
}

std::string UbxConfigApplicator::format_cfg_key_name(format::ubx::CfgKey key) {
    switch (key) {
    case format::ubx::CFG_KEY_RATE_MEAS: return "CFG_KEY_RATE_MEAS";
    case format::ubx::CFG_KEY_NAVHPG_DGNSSMODE: return "CFG_KEY_NAVHPG_DGNSSMODE";
    case format::ubx::CFG_KEY_UART1_ENABLED: return "CFG_KEY_UART1_ENABLED";
    case format::ubx::CFG_KEY_UART1_BAUDRATE: return "CFG_KEY_UART1_BAUDRATE";
    case format::ubx::CFG_KEY_UART1_STOPBITS: return "CFG_KEY_UART1_STOPBITS";
    case format::ubx::CFG_KEY_UART1_DATABITS: return "CFG_KEY_UART1_DATABITS";
    case format::ubx::CFG_KEY_UART1_PARITY: return "CFG_KEY_UART1_PARITY";
    case format::ubx::CFG_KEY_UART1INPROT_UBX: return "CFG_KEY_UART1INPROT_UBX";
    case format::ubx::CFG_KEY_UART1INPROT_NMEA: return "CFG_KEY_UART1INPROT_NMEA";
    case format::ubx::CFG_KEY_UART1INPROT_RTCM3X: return "CFG_KEY_UART1INPROT_RTCM3X";
    case format::ubx::CFG_KEY_UART1INPROT_SPARTN: return "CFG_KEY_UART1INPROT_SPARTN";
    case format::ubx::CFG_KEY_UART1OUTPROT_UBX: return "CFG_KEY_UART1OUTPROT_UBX";
    case format::ubx::CFG_KEY_UART1OUTPROT_NMEA: return "CFG_KEY_UART1OUTPROT_NMEA";
    case format::ubx::CFG_KEY_UART1OUTPROT_RTCM3X: return "CFG_KEY_UART1OUTPROT_RTCM3X";
    case format::ubx::CFG_KEY_MSGOUT_NAV_PVT_UART1: return "CFG_KEY_MSGOUT_NAV_PVT_UART1";
    case format::ubx::CFG_KEY_INFMSG_UART1: return "CFG_KEY_INFMSG_UART1";
    default: return "0x" + std::to_string(key);
    }
}

bool UbxConfigApplicator::collect_all_config(UbxConfigInterface& interface) {
    collected_values_.clear();
    expected_position_ = 0;
    has_more_data_     = true;

    printf("\n=== UBX Configuration (All Values) ===\n");

    format::ubx::Parser parser;

    interface.input_interface->callback = [&](io::Input&, uint8_t* data, size_t size) {
        parser.append(data, size);
        auto message = parser.try_parse();
        if (message && message->message_class() == 0x06 && message->message_id() == 0x8B) {
            waiting_for_response_ = false;
        }
    };

    while (has_more_data_ && expected_position_ < 1000) {
        auto message = create_cfg_valget_all_message(expected_position_);
        if (message.empty()) break;

        interface.output_interface->write(message.data(), message.size());
        waiting_for_response_ = true;

        auto timeout = std::chrono::steady_clock::now() + std::chrono::seconds(2);

        scheduler_.execute_while([&]() {
            if (std::chrono::steady_clock::now() > timeout) {
                waiting_for_response_ = false;
                return false;
            }
            return waiting_for_response_;
        });

        if (!waiting_for_response_) {
            WARNF("timeout waiting for VALGET response at position %u", expected_position_);
            break;
        }
    }

    printf("%-40s %-12s %s\n", "Key Name", "Hex Key", "Value");
    printf("%-40s %-12s %s\n", "--------", "-------", "-----");

    for (auto const& entry : collected_values_) {
        auto const& key      = entry.first;
        auto const& value    = entry.second;
        std::string key_name = format_cfg_key_name(key);
        std::string value_str;

        switch (value.type()) {
        case format::ubx::CfgValue::Type::L: value_str = value.l() ? "true" : "false"; break;
        case format::ubx::CfgValue::Type::U1: value_str = std::to_string(value.u1()); break;
        case format::ubx::CfgValue::Type::U2: value_str = std::to_string(value.u2()); break;
        case format::ubx::CfgValue::Type::U4: value_str = std::to_string(value.u4()); break;
        case format::ubx::CfgValue::Type::U8: value_str = std::to_string(value.u8()); break;
        case format::ubx::CfgValue::Type::UNKNOWN: value_str = "unknown"; break;
        }

        printf("%-40s 0x%08X   %s\n", key_name.c_str(), key, value_str.c_str());
    }

    printf("\nTotal configuration items: %zu\n\n", collected_values_.size());
    return true;
}

bool UbxConfigApplicator::print_current_config(UbxConfigInterface& interface) {
    if (!interface.output_interface->schedule(scheduler_) ||
        !interface.input_interface->schedule(scheduler_)) {
        ERRORF("failed to schedule interfaces");
        return false;
    }

    if (interface.print_mode == UbxPrintMode::OPTIONS) {
        std::vector<format::ubx::CfgKey> requested_keys;
        for (auto const& entry : interface.options) {
            auto const& key = entry.first;
            requested_keys.push_back(key);
        }

        auto message = create_cfg_valget_message(requested_keys);
        if (message.empty()) return false;

        interface.output_interface->write(message.data(), message.size());

        printf("\n=== Current UBX Configuration (Requested Keys) ===\n");
        printf("%-40s %-12s %s\n", "Key Name", "Hex Key", "Value");
        printf("%-40s %-12s %s\n", "--------", "-------", "-----");

        scheduler_.execute_timeout(std::chrono::milliseconds(500));
        printf("\n");

    } else if (interface.print_mode == UbxPrintMode::ALL) {
        return collect_all_config(interface);
    }

    return true;
}

bool UbxConfigApplicator::apply_to_interface(UbxConfigInterface& interface) {
    bool success = true;

    if (interface.print_mode != UbxPrintMode::NONE) {
        if (!print_current_config(interface)) {
            WARNF("failed to query current configuration");
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }

    if (!interface.options.empty()) {
        auto message = create_cfg_valset_message(interface.options);
        if (message.empty()) {
            ERRORF("failed to create CFG-VALSET message");
            success = false;
        } else {
            INFOF("applying %zu UBX options", interface.options.size());

            if (!interface.output_interface->schedule(scheduler_)) {
                ERRORF("failed to schedule output interface");
                success = false;
            } else {
                interface.output_interface->write(message.data(), message.size());
                DEBUGF("UBX configuration message sent (%zu bytes)", message.size());
            }
        }
    }

    interface.output_interface->cancel();
    interface.input_interface->cancel();

    return success;
}

bool UbxConfigApplicator::apply_configurations() {
    if (config_.interfaces.empty()) {
        DEBUGF("no UBX configuration interfaces");
        return true;
    }

    bool all_success = true;
    for (auto& interface : const_cast<UbxConfigConfig&>(config_).interfaces) {
        if (!apply_to_interface(interface)) {
            all_success = false;
        }
    }

    return all_success;
}
