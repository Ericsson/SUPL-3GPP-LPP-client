#pragma once
#include "../config.hpp"
#include <format/ubx/cfg.hpp>
#include <scheduler/scheduler.hpp>
#include <memory>

class UbxConfigApplicator {
public:
    explicit UbxConfigApplicator(const UbxConfigConfig& config, scheduler::Scheduler& scheduler);
    ~UbxConfigApplicator() = default;

    bool apply_configurations();

private:
    const UbxConfigConfig& config_;
    scheduler::Scheduler& scheduler_;
    
    // Response collection state
    std::vector<std::pair<format::ubx::CfgKey, format::ubx::CfgValue>> collected_values_;
    bool waiting_for_response_ = false;
    uint16_t expected_position_ = 0;
    bool has_more_data_ = true;
    
    std::vector<uint8_t> create_cfg_valset_message(const std::vector<std::pair<format::ubx::CfgKey, format::ubx::CfgValue>>& options);
    std::vector<uint8_t> create_cfg_valget_message(const std::vector<format::ubx::CfgKey>& keys, uint16_t position = 0);
    std::vector<uint8_t> create_cfg_valget_all_message(uint16_t position = 0);
    bool apply_to_interface(UbxConfigInterface& interface);
    bool print_current_config(UbxConfigInterface& interface);
    std::string format_cfg_key_name(format::ubx::CfgKey key);

    bool collect_all_config(UbxConfigInterface& interface);
};