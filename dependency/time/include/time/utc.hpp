#pragma once
#include <time/timestamp.hpp>

#include <string>

namespace ts {

class Tai;
class Gps;
class Glo;
class Gst;
class Bdt;

class Utc {
public:
    Utc();
    EXPLICIT Utc(Timestamp const& timestamp);
    EXPLICIT Utc(Tai const& time);
    EXPLICIT Utc(Gps const& time);
    EXPLICIT Utc(Glo const& time);
    EXPLICIT Utc(Gst const& time);
    EXPLICIT Utc(Bdt const& time);

    NODISCARD int64_t days() const;
    NODISCARD double  day_of_year() const;

    NODISCARD Timestamp ut1(double ut1_utc) const;
    NODISCARD double    gmst(double ut1_utc) const;

    NODISCARD Timestamp timestamp() const { return tm; }
    NODISCARD std::string rtklib_time_string() const;
    NODISCARD std::string rfc3339() const;
    NODISCARD std::string rinex_string() const;
    NODISCARD std::string rinex_filename() const;
    NODISCARD TimePoint time_point() const;

    Utc& add(int64_t seconds) {
        tm.add(seconds);
        return *this;
    }

    NODISCARD double julian_date(double ut1_utc) const;
    NODISCARD double j2000_century(double ut1_utc) const;

    NODISCARD static Utc now();
    NODISCARD static Utc from_day_tod(int64_t day, double tod);
    NODISCARD static Utc from_date_time(int64_t year, int64_t month, int64_t day, int64_t hour,
                                        int64_t minute, double second);
    NODISCARD static Utc from_time_point(TimePoint const& tp);

private:
    Timestamp tm;
};

}  // namespace ts
