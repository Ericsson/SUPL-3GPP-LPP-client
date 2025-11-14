#pragma once
#include <time/timestamp.hpp>

namespace ts {

class Tai;
class Utc;
class Glo;
class Gst;
class Gps;

class Bdt {
public:
    friend Utc;

    Bdt();
    EXPLICIT Bdt(Timestamp const& timestamp);
    EXPLICIT Bdt(Tai const& time);
    EXPLICIT Bdt(Utc const& time);
    EXPLICIT Bdt(Glo const& time);
    EXPLICIT Bdt(Gst const& time);
    EXPLICIT Bdt(Gps const& time);

    NODISCARD int64_t   days() const;
    NODISCARD int64_t   week() const;
    NODISCARD Timestamp time_of_day() const;
    NODISCARD Timestamp time_of_week() const;
    NODISCARD Timestamp timestamp() const { return mTm; }

    NODISCARD static Bdt now();
    NODISCARD static Bdt from_day_tod(int64_t day, double tod);
    NODISCARD static Bdt from_week_tow(int64_t week, int64_t tow, double fractions);

protected:
    NODISCARD Timestamp utc_timestamp() const;

private:
    // NOTE: Seconds since the begining of BDT (00:00:00 on January 1st, 2006). BDT is -33 seconds
    // away from TAI as of (2022-09-22) and changes with added or subtracted leap-seconds.
    Timestamp mTm;
};

}  // namespace ts
