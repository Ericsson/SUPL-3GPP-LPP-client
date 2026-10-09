#pragma once
#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include <streamline/inspector.hpp>
#include <streamline/system.hpp>

#include "processor/control.hpp"
#include "processor/lpp.hpp"
#include "processor/nmea.hpp"
#include "processor/rtcm.hpp"
#include "processor/ubx.hpp"

#ifdef INCLUDE_GENERATOR_RTCM
#include <ephemeris/bds.hpp>
#include <ephemeris/gal.hpp>
#include <ephemeris/gps.hpp>
#include <ephemeris/qzs.hpp>
#endif

#include "config.hpp"

namespace scheduler {
class Scheduler;
}

/// Tracks how many messages have been received, broken down by a category
/// (e.g. "nmea", "rtcm", "eph"), a name within that category (e.g. "GGA",
/// "1019", "G01"), and the exact tag mask the message carried.
///
/// Single-threaded by design: all recording and reporting happens on the
/// scheduler thread, since inspectors are dispatched from the scheduler's
/// event loop and the periodic report is also a scheduled task.
class MessageStats {
public:
    static MessageStats& instance();

    void record(std::string const& category, std::string const& name, uint64_t tag);

    /// Print a table of total and per-tag counts to stdout. Each column is
    /// one registered tag; a message contributes to every column whose bit
    /// is set in its tag mask (no merging of combined masks into one column).
    void print_report() const;

private:
    struct Counter {
        uint64_t                     total = 0;
        std::map<uint64_t, uint64_t> per_tag;  // raw tag mask -> message count
    };

    // category -> name -> counter
    std::map<std::string, std::map<std::string, Counter>> mCategories;
};

class NmeaMessageStats : public streamline::Inspector<NmeaMessage> {
public:
    NODISCARD char const* name() const NOEXCEPT override { return "NmeaMessageStats"; }
    void inspect(streamline::System&, DataType const& message, uint64_t tag) NOEXCEPT override;
};

class RtcmMessageStats : public streamline::Inspector<RtcmMessage> {
public:
    NODISCARD char const* name() const NOEXCEPT override { return "RtcmMessageStats"; }
    void inspect(streamline::System&, DataType const& message, uint64_t tag) NOEXCEPT override;
};

class UbxMessageStats : public streamline::Inspector<UbxMessage> {
public:
    NODISCARD char const* name() const NOEXCEPT override { return "UbxMessageStats"; }
    void inspect(streamline::System&, DataType const& message, uint64_t tag) NOEXCEPT override;
};

class CtrlMessageStats : public streamline::Inspector<CtrlMessage> {
public:
    NODISCARD char const* name() const NOEXCEPT override { return "CtrlMessageStats"; }
    void inspect(streamline::System&, DataType const& message, uint64_t tag) NOEXCEPT override;
};

class LppMessageStats : public streamline::Inspector<lpp::Message> {
public:
    NODISCARD char const* name() const NOEXCEPT override { return "LppMessageStats"; }
    void inspect(streamline::System&, DataType const& message, uint64_t tag) NOEXCEPT override;
};

#ifdef INCLUDE_GENERATOR_RTCM
class GpsEphMessageStats : public streamline::Inspector<ephemeris::GpsEphemeris> {
public:
    NODISCARD char const* name() const NOEXCEPT override { return "GpsEphMessageStats"; }
    void inspect(streamline::System&, DataType const& message, uint64_t tag) NOEXCEPT override;
};

class GalEphMessageStats : public streamline::Inspector<ephemeris::GalEphemeris> {
public:
    NODISCARD char const* name() const NOEXCEPT override { return "GalEphMessageStats"; }
    void inspect(streamline::System&, DataType const& message, uint64_t tag) NOEXCEPT override;
};

class BdsEphMessageStats : public streamline::Inspector<ephemeris::BdsEphemeris> {
public:
    NODISCARD char const* name() const NOEXCEPT override { return "BdsEphMessageStats"; }
    void inspect(streamline::System&, DataType const& message, uint64_t tag) NOEXCEPT override;
};

class QzsEphMessageStats : public streamline::Inspector<ephemeris::QzsEphemeris> {
public:
    NODISCARD char const* name() const NOEXCEPT override { return "QzsEphMessageStats"; }
    void inspect(streamline::System&, DataType const& message, uint64_t tag) NOEXCEPT override;
};
#endif

/// Register the message-stats inspectors on `system` and start a periodic
/// task that prints the report every `interval` seconds. Returns false if
/// `interval` is <= 0 (reporting disabled).
bool message_stats_start(streamline::System& system, scheduler::Scheduler& scheduler, int interval);
