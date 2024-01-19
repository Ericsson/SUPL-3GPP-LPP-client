#pragma once
#include <features.h>
#include <utility/types.h>

constexpr s64 DAYS_PER_WEEK = 7LL;
constexpr s64 DAYS_PER_YEAR = 365LL;

constexpr s64 MINUTE_IN_SECONDS = 60LL;
constexpr s64 HOUR_IN_SECONDS   = MINUTE_IN_SECONDS * 60LL;
constexpr s64 DAY_IN_SECONDS    = HOUR_IN_SECONDS * 24LL;
constexpr s64 WEEK_IN_SECONDS   = DAY_IN_SECONDS * DAYS_PER_WEEK;
constexpr s64 YEAR_IN_SECONDS   = DAY_IN_SECONDS * 365LL;

constexpr s64 MILLISECONDS_PER_SECOND = 1000LL;

// Total time in seconds (+fractions) since a start date. The date depends on
// from where this object was created, i.e., for UTC stat is 1 Jan 1970
// 00:00:00.
class Timestamp {
public:
    Timestamp() : mSeconds(0), mFraction(0.0) {}
    explicit Timestamp(f64 seconds) : Timestamp() { add(seconds); }
    explicit Timestamp(s64 seconds) : mSeconds(seconds), mFraction() {}
    explicit Timestamp(s64 seconds, f64 fraction) {
        mSeconds  = seconds;
        mFraction = fraction;
        normalize();
    }

    NO_DISCARD s64 seconds() const { return mSeconds; }
    NO_DISCARD f64 fraction() const { return mFraction; }
    NO_DISCARD f64 full_seconds() const { return static_cast<f64>(mSeconds) + mFraction; }

    Timestamp operator+(const Timestamp& other) const {
        return Timestamp{seconds() + other.seconds(), fraction() + other.fraction()};
    }
    Timestamp operator-(const Timestamp& other) const {
        return Timestamp{seconds() - other.seconds(), fraction() - other.fraction()};
    }

    void normalize() {
        auto overflow = static_cast<s64>(mFraction);
        mSeconds += overflow;
        mFraction -= static_cast<f64>(overflow);
        assert(mFraction >= 0.0);
        assert(mFraction < 1.0);
    }

    void add(s64 sec) { mSeconds += sec; }
    void subtract(s64 sec) { mSeconds -= sec; }

    void add(f64 sec) {
        mFraction += sec;
        normalize();
    }

    void subtract(f64 sec) {
        mFraction -= sec;
        normalize();
    }

private:
    s64 mSeconds;
    f64 mFraction;
};

class LeapSeconds {
public:
    NO_DISCARD static s64 count() {
        return 37;  // TODO(ewasjon): SHOULD NOT BE HARDCODED!
    }
};

#include <utility/time/bdt_time.h>
#include <utility/time/glo_time.h>
#include <utility/time/gps_time.h>
#include <utility/time/gst_time.h>
#include <utility/time/tai_time.h>
#include <utility/time/utc_time.h>
