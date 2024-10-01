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
    NODISCARD double day_of_year() const;

    NODISCARD Timestamp timestamp() const { return tm; }
    NODISCARD std::string rtklib_time_string() const;

    Utc& add(int64_t seconds) {
        tm.add(seconds);
        return *this;
    }

    NODISCARD static Utc now();
    NODISCARD static Utc from_day_tod(int64_t day, double tod);

private:
    Timestamp tm;
};

}  // namespace ts