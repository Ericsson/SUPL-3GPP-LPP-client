#include "message_stats.hpp"

#include <algorithm>
#include <cstdio>

#include <loglet/loglet.hpp>
#include <scheduler/periodic.hpp>

#include "tag_registry.hpp"

#ifdef INCLUDE_GENERATOR_RTCM
#include <gnss/satellite_id.hpp>
#endif

LOGLET_MODULE2(p, message_stats);
#undef LOGLET_CURRENT_MODULE
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF2(p, message_stats)

MessageStats& MessageStats::instance() {
    static MessageStats stats;
    return stats;
}

void MessageStats::record(std::string const& category, std::string const& name, uint64_t tag) {
    auto& counter = mCategories[category][name];
    counter.total++;
    counter.per_tag[tag]++;

    if (std::find(mTagOrder.begin(), mTagOrder.end(), tag) == mTagOrder.end()) {
        mTagOrder.push_back(tag);
    }
}

void MessageStats::print_report() const {
    if (mCategories.empty()) {
        printf("\nMessage statistics: no messages received yet\n\n");
        return;
    }

    std::vector<std::string> tag_names;
    tag_names.reserve(mTagOrder.size());
    for (auto tag : mTagOrder) {
        auto name = tags::to_string(tags::TagMask(tag));
        tag_names.push_back(name.empty() ? "-" : name);
    }

    printf("\nMessage statistics:\n");
    for (auto const& category_entry : mCategories) {
        auto const& category = category_entry.first;
        auto const& names    = category_entry.second;

        printf("\n  %s:\n", category.c_str());
        printf("    %-12s %10s", "name", "total");
        for (auto const& tag_name : tag_names) {
            printf(" %10s", tag_name.c_str());
        }
        printf("\n");

        uint64_t              grand_total = 0;
        std::vector<uint64_t> per_tag_totals(mTagOrder.size(), 0);

        for (auto const& name_entry : names) {
            auto const& name    = name_entry.first;
            auto const& counter = name_entry.second;

            printf("    %-12s %10llu", name.c_str(),
                   static_cast<unsigned long long>(counter.total));
            for (size_t i = 0; i < mTagOrder.size(); i++) {
                auto it    = counter.per_tag.find(mTagOrder[i]);
                auto count = it != counter.per_tag.end() ? it->second : 0;
                printf(" %10llu", static_cast<unsigned long long>(count));
                per_tag_totals[i] += count;
            }
            printf("\n");

            grand_total += counter.total;
        }

        printf("    %-12s %10llu", "total", static_cast<unsigned long long>(grand_total));
        for (auto total : per_tag_totals) {
            printf(" %10llu", static_cast<unsigned long long>(total));
        }
        printf("\n");
    }
    printf("\n");
}

void NmeaMessageStats::inspect(streamline::System&, DataType const& message,
                               uint64_t tag) NOEXCEPT {
    if (!message) return;
    MessageStats::instance().record("nmea", message->prefix(), tag);
}

void RtcmMessageStats::inspect(streamline::System&, DataType const& message,
                               uint64_t tag) NOEXCEPT {
    if (!message) return;
    char name[16];
    snprintf(name, sizeof(name), "%d", message->type());
    MessageStats::instance().record("rtcm", name, tag);
}

void UbxMessageStats::inspect(streamline::System&, DataType const& message, uint64_t tag) NOEXCEPT {
    if (!message) return;
    char name[16];
    snprintf(name, sizeof(name), "%02X-%02X", message->message_class(), message->message_id());
    MessageStats::instance().record("ubx", name, tag);
}

void CtrlMessageStats::inspect(streamline::System&, DataType const& message,
                               uint64_t tag) NOEXCEPT {
    if (!message) return;
    MessageStats::instance().record("ctrl", "ctrl", tag);
}

void LppMessageStats::inspect(streamline::System&, DataType const& message, uint64_t tag) NOEXCEPT {
    if (!message) return;
    MessageStats::instance().record("lpp", "lpp", tag);
}

#ifdef INCLUDE_GENERATOR_RTCM
void GpsEphMessageStats::inspect(streamline::System&, DataType const& message,
                                 uint64_t tag) NOEXCEPT {
    auto satellite_id = SatelliteId::from_gps_prn(message.prn);
    MessageStats::instance().record("eph", satellite_id.name(), tag);
}

void GalEphMessageStats::inspect(streamline::System&, DataType const& message,
                                 uint64_t tag) NOEXCEPT {
    auto satellite_id = SatelliteId::from_gal_prn(message.prn);
    MessageStats::instance().record("eph", satellite_id.name(), tag);
}

void BdsEphMessageStats::inspect(streamline::System&, DataType const& message,
                                 uint64_t tag) NOEXCEPT {
    auto satellite_id = SatelliteId::from_bds_prn(message.prn);
    MessageStats::instance().record("eph", satellite_id.name(), tag);
}

void QzsEphMessageStats::inspect(streamline::System&, DataType const& message,
                                 uint64_t tag) NOEXCEPT {
    auto satellite_id = SatelliteId::from_qzs_prn(message.prn);
    MessageStats::instance().record("eph", satellite_id.name(), tag);
}
#endif

static std::unique_ptr<scheduler::PeriodicTask> gMessageStatsTask = nullptr;

bool message_stats_start(streamline::System& system, scheduler::Scheduler& scheduler,
                         int interval) {
    if (interval <= 0) return false;

    system.add_inspector<NmeaMessageStats>();
    system.add_inspector<RtcmMessageStats>();
    system.add_inspector<UbxMessageStats>();
    system.add_inspector<CtrlMessageStats>();
    system.add_inspector<LppMessageStats>();
#ifdef INCLUDE_GENERATOR_RTCM
    system.add_inspector<GpsEphMessageStats>();
    system.add_inspector<GalEphMessageStats>();
    system.add_inspector<BdsEphMessageStats>();
    system.add_inspector<QzsEphMessageStats>();
#endif

    ASSERT(gMessageStatsTask == nullptr, "message stats task already exists");
    gMessageStatsTask = std::unique_ptr<scheduler::PeriodicTask>(
        new scheduler::PeriodicTask(std::chrono::seconds(interval)));
    gMessageStatsTask->set_event_name("message stats");
    gMessageStatsTask->callback = []() {
        MessageStats::instance().print_report();
    };

    if (!gMessageStatsTask->schedule(scheduler)) {
        ERRORF("failed to schedule periodic task for message stats");
        return false;
    }
    return true;
}
