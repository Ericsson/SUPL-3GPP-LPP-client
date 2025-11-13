#pragma once
#include <core/core.hpp>

namespace ts {

CONSTEXPR static int64_t DAYS_PER_WEEK = 7LL;
CONSTEXPR static int64_t DAYS_PER_YEAR = 365LL;

CONSTEXPR static int64_t MINUTE_IN_SECONDS = 60LL;
CONSTEXPR static int64_t HOUR_IN_SECONDS   = MINUTE_IN_SECONDS * 60LL;
CONSTEXPR static int64_t DAY_IN_SECONDS    = HOUR_IN_SECONDS * 24LL;
CONSTEXPR static int64_t WEEK_IN_SECONDS   = DAY_IN_SECONDS * DAYS_PER_WEEK;
CONSTEXPR static int64_t YEAR_IN_SECONDS   = DAY_IN_SECONDS * 365LL;

CONSTEXPR static int64_t MILLISECONDS_PER_SECOND = 1000LL;

// Total time in seconds (+fractions) since a start date. The date depends on
// from where this object was created, i.e., for UTC stat is 1 Jan 1970
// 00:00:00.
class Timestamp {
public:
    Timestamp() : mSeconds(0), mFraction(0.0) {}
    EXPLICIT Timestamp(int64_t seconds) : mSeconds(seconds), mFraction() {}
    EXPLICIT Timestamp(double seconds) : Timestamp() { add(seconds); }
    EXPLICIT Timestamp(int64_t seconds, double fraction) : mSeconds(seconds), mFraction(fraction) {
        normalize();
    }

    NODISCARD int64_t seconds() const { return mSeconds; }
    NODISCARD double  fraction() const { return mFraction; }
    NODISCARD double  full_seconds() const { return static_cast<double>(mSeconds) + mFraction; }

    NODISCARD int64_t days() const { return mSeconds / DAY_IN_SECONDS; }

    Timestamp operator+(Timestamp const& other) const {
        return Timestamp{seconds() + other.seconds(), fraction() + other.fraction()};
    }
    Timestamp operator-(Timestamp const& other) const {
        return Timestamp{seconds() - other.seconds(), fraction() - other.fraction()};
    }

    void normalize() {
        while (mFraction < 0.0) {
            auto underflow = static_cast<int64_t>(-mFraction) + 1;
            mSeconds -= underflow;
            mFraction += static_cast<double>(underflow);
        }

        while (mFraction >= 1.0) {
            auto overflow = static_cast<int64_t>(mFraction);
            mSeconds += overflow;
            mFraction -= static_cast<double>(overflow);
        }

        CORE_ASSERT(mFraction >= 0.0 && mFraction < 1.0, "Fraction out of bounds");
    }

    void add(int64_t sec) { mSeconds += sec; }
    void subtract(int64_t sec) { mSeconds -= sec; }

    void add(double sec);
    void subtract(double sec);

    NODISCARD bool operator==(Timestamp const& other) const;

    NODISCARD bool operator<(Timestamp const& other) const {
        return seconds() < other.seconds() ||
               (seconds() == other.seconds() && fraction() < other.fraction());
    }
    NODISCARD bool operator<=(Timestamp const& other) const {
        return *this < other || *this == other;
    }
    NODISCARD bool operator>(Timestamp const& other) const { return !(*this <= other); }
    NODISCARD bool operator>=(Timestamp const& other) const { return !(*this < other); }

private:
    int64_t mSeconds;
    double  mFraction;
};

struct TimePoint {
    int64_t year;
    int64_t month;
    int64_t day;
    int64_t hour;
    int64_t minutes;
    double  seconds;
};

class LeapSeconds {
public:
    NODISCARD static int64_t count();
};

}  // namespace ts
