#pragma once
#include <features.h>
#include "types.hpp"

namespace ts {
using TsInt   = int64_t;
using TsFloat = double;

RTCM_CONSTEXPR static TsInt DAYS_PER_WEEK           = 7LL;
RTCM_CONSTEXPR static TsInt DAYS_PER_YEAR           = 365LL;
RTCM_CONSTEXPR static TsInt MINUTE_IN_SECONDS       = 60LL;
RTCM_CONSTEXPR static TsInt HOUR_IN_SECONDS         = MINUTE_IN_SECONDS * 60LL;
RTCM_CONSTEXPR static TsInt DAY_IN_SECONDS          = HOUR_IN_SECONDS * 24LL;
RTCM_CONSTEXPR static TsInt WEEK_IN_SECONDS         = DAY_IN_SECONDS * DAYS_PER_WEEK;
RTCM_CONSTEXPR static TsInt YEAR_IN_SECONDS         = DAY_IN_SECONDS * 365LL;
RTCM_CONSTEXPR static TsInt MILLISECONDS_PER_SECOND = 1000LL;

// Total time in seconds (+fractions) since a start date. The date depends on
// from where this object was created, i.e., for UTC start date is 1 Jan 1970
// 00:00:00.
class Timestamp {
public:
    Timestamp() : mSeconds(0), mFraction(0.0) {}
    RTCM_EXPLICIT Timestamp(TsFloat seconds) : Timestamp() { add(seconds); }
    RTCM_EXPLICIT Timestamp(TsInt seconds) : mSeconds(seconds), mFraction() {}
    RTCM_EXPLICIT Timestamp(TsInt seconds, TsFloat fraction) {
        mSeconds  = seconds;
        mFraction = fraction;
        normalize();
    }

    RTCM_NODISCARD TsInt   seconds() const { return mSeconds; }
    RTCM_NODISCARD TsFloat fraction() const { return mFraction; }
    RTCM_NODISCARD TsFloat full_seconds() const {
        return static_cast<TsFloat>(mSeconds) + mFraction;
    }

    RTCM_NODISCARD Timestamp operator+(const Timestamp& other) const {
        return Timestamp{seconds() + other.seconds(), fraction() + other.fraction()};
    }

    RTCM_NODISCARD Timestamp operator-(const Timestamp& other) const {
        return Timestamp{seconds() - other.seconds(), fraction() - other.fraction()};
    }

    void normalize() {
        auto overflow = static_cast<TsInt>(mFraction);
        mSeconds += overflow;
        mFraction -= static_cast<TsFloat>(overflow);
        assert(mFraction >= 0.0);
        assert(mFraction < 1.0);
    }

    void add(TsInt sec) { mSeconds += sec; }
    void subtract(TsInt sec) { mSeconds -= sec; }

    void add(TsFloat sec) {
        mFraction += sec;
        normalize();
    }

    void subtract(TsFloat sec) {
        mFraction -= sec;
        normalize();
    }

private:
    TsInt   mSeconds;
    TsFloat mFraction;
};

class LeapSeconds {
public:
    RTCM_NODISCARD static TsInt count() {
        return 37;  // TODO(ewasjon): SHOULD NOT BE HARDCODED!
    }
};
}  // namespace ts
