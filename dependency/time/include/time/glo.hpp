#pragma once
#include <time/timestamp.hpp>

namespace ts {

class Tai;
class Utc;
class Gst;
class Bdt;
class Gps;

class Glo {
public:
    friend Utc;

    Glo();
    EXPLICIT Glo(Timestamp const& timestamp);
    EXPLICIT Glo(Tai const& time);
    EXPLICIT Glo(Utc const& time);
    EXPLICIT Glo(Gst const& time);
    EXPLICIT Glo(Bdt const& time);
    EXPLICIT Glo(Gps const& time);

    NODISCARD int64_t   days() const;
    NODISCARD Timestamp time_of_day() const;
    NODISCARD Timestamp timestamp() const { return tm; }

    NODISCARD static Glo now();
    NODISCARD static Glo from_day_tod(int64_t day, double tod);

protected:
    NODISCARD Timestamp utc_timestamp() const;

private:
    Timestamp tm;
};

}  // namespace ts
