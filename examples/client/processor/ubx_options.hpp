#pragma once
#include <format/ubx/cfg.hpp>
#include <memory>
#include <scheduler/scheduler.hpp>
#include "../config.hpp"

class UbxConfigApplicator {
public:
    explicit UbxConfigApplicator(UbxConfigConfig const& config, scheduler::Scheduler& scheduler);
    ~UbxConfigApplicator() = default;

    bool apply_configurations();

private:
    UbxConfigConfig const& mConfig;
    scheduler::Scheduler&  mScheduler;

    // Response collection state
    std::vector<std::pair<format::ubx::CfgKey, format::ubx::CfgValue>> mCollectedValues;
    bool                                                               mWaitingForResponse = false;
    uint16_t                                                           mExpectedPosition   = 0;
    bool                                                               mHasMoreData        = true;

    std::vector<uint8_t> create_cfg_valset_message(
        std::vector<std::pair<format::ubx::CfgKey, format::ubx::CfgValue>> const& options);
    std::vector<uint8_t> create_cfg_valget_message(std::vector<format::ubx::CfgKey> const& keys,
                                                   uint16_t position = 0);
    std::vector<uint8_t> create_cfg_valget_all_message(uint16_t position = 0);
    bool                 apply_to_interface(UbxConfigInterface& interface);
    bool                 print_current_config(UbxConfigInterface& interface);
    std::string          format_cfg_key_name(format::ubx::CfgKey key);

    bool collect_all_config(UbxConfigInterface& interface);
};
