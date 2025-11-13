#include <cmath>
#include <time/timestamp.hpp>
#include <version.hpp>

namespace ts {

void Timestamp::add(double sec) {
    CORE_ASSERT(!std::isnan(sec), "sec is NaN");
    mFraction += sec;
    normalize();
}

void Timestamp::subtract(double sec) {
    CORE_ASSERT(!std::isnan(sec), "sec is NaN");
    mFraction -= sec;
    normalize();
}

bool Timestamp::operator==(Timestamp const& other) const {
    return seconds() == other.seconds() && fabs(fraction() - other.fraction()) < 1e-9;
}

int64_t LeapSeconds::count() {
#define LAST_UPDATE_CHECKED 0x040016
#if LAST_UPDATE_CHECKED < CLIENT_VERSION_INT
#error "Please update the leap seconds table"
#endif
    // TODO(ewasjon): This should be based on a provided input time.
    return 37;  // TODO(ewasjon): SHOULD NOT BE HARDCODED!
}

}  // namespace ts
