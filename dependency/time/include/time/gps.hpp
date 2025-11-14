#pragma once
#include <time/timestamp.hpp>

namespace ts {

class Tai;
class Utc;
class Bdt;
class Gst;
class Glo;

class Gps {
public:
    friend Utc;

    Gps();
    EXPLICIT Gps(Timestamp const& timestamp);
    EXPLICIT Gps(Tai const& time);
    EXPLICIT Gps(Utc const& time);
    EXPLICIT Gps(Bdt const& time);
    EXPLICIT Gps(Gst const& time);
    EXPLICIT Gps(Glo const& time);

    NODISCARD int64_t   days() const;
    NODISCARD Timestamp time_of_day() const;
    NODISCARD int64_t   week() const;
    NODISCARD Timestamp time_of_week() const;
    NODISCARD Timestamp timestamp() const { return mTm; }
    NODISCARD Timestamp mod_timestamp() const;

    NODISCARD Gps       operator+(Timestamp delta) const { return Gps(mTm + delta); }
    NODISCARD Timestamp difference(Gps const& other) const;

    NODISCARD TimePoint to_timepoint() const;

    NODISCARD static Gps now();
    NODISCARD static Gps from_day_tod(int64_t day, double tod);
    NODISCARD static Gps from_week_tow(int64_t week, int64_t tow, double fractions);
    NODISCARD static Gps from_ymdhms(int64_t year, int64_t month, int64_t day, int64_t hour,
                                     int64_t min, double seconds);

    NODISCARD static int64_t days_from_ymd(int64_t year, int64_t month, int64_t day);

    NODISCARD bool operator<(Gps const& other) const { return mTm < other.mTm; }
    NODISCARD bool operator<=(Gps const& other) const { return mTm <= other.mTm; }
    NODISCARD bool operator>(Gps const& other) const { return mTm > other.mTm; }
    NODISCARD bool operator>=(Gps const& other) const { return mTm >= other.mTm; }

protected:
    NODISCARD Timestamp utc_timestamp() const;

private:
    Timestamp mTm;
};

}  // namespace ts
