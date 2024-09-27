#pragma once
#include <time/timestamp.hpp>

#include <string>

namespace ts {

class Gps;
class Glo;
class Gst;
class Utc;
class Bdt;

class Tai {
public:
    friend Utc;

    Tai();
    EXPLICIT Tai(Timestamp const& timestamp);
    EXPLICIT Tai(Gps const& time);
    EXPLICIT Tai(Glo const& time);
    EXPLICIT Tai(Gst const& time);
    EXPLICIT Tai(Utc const& time);
    EXPLICIT Tai(Bdt const& time);

    NODISCARD Timestamp timestamp() const { return tm; }
    NODISCARD std::string rtklib_time_string() const;

    Tai& add_seconds(double seconds) {
        tm.add(seconds);
        return *this;
    }

    NODISCARD Tai operator+(Timestamp delta) const;
    NODISCARD Timestamp difference(Tai const& other) const;

    NODISCARD static Tai now();

protected:
    NODISCARD Timestamp utc_timestamp() const;

private:
    // There is no date for when the TAI timestamp began. Although, TAI - UTC
    // should be equal to the leap seconds since 1972. Which is, as of today
    // (2021-08-13), 37 seconds. Thus, the start date will be the same as UTC (1
    // january 1970).
    Timestamp tm;
};

}  // namespace ts
