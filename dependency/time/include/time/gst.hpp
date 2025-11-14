#pragma once
#include <time/timestamp.hpp>

namespace ts {

class Tai;
class Utc;
class Gps;
class Glo;
class Bdt;

class Gst {
public:
    friend Utc;

    Gst();
    EXPLICIT Gst(Timestamp const& timestamp);
    EXPLICIT Gst(Tai const& time);
    EXPLICIT Gst(Utc const& time);
    EXPLICIT Gst(Gps const& time);
    EXPLICIT Gst(Glo const& time);
    EXPLICIT Gst(Bdt const& time);

    NODISCARD int64_t   days() const;
    NODISCARD Timestamp time_of_day() const;
    NODISCARD int64_t   week() const;
    NODISCARD Timestamp time_of_week() const;
    NODISCARD Timestamp timestamp() const { return mTm; }

    NODISCARD static Gst now();
    NODISCARD static Gst from_day_tod(int64_t day, double tod);
    NODISCARD static Gst from_week_tow(int64_t week, int64_t tow, double fractions);

protected:
    NODISCARD Timestamp utc_timestamp() const;

private:
    Timestamp mTm;
};

}  // namespace ts
