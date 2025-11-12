#include <cmath>
#include <time/timestamp.hpp>

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

}  // namespace ts
