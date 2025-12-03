#pragma once
#include <cmath>
#include <iomanip>
#include <sstream>

struct Tolerance {
    double value;
    double epsilon;

    explicit Tolerance(double v) : value(v), epsilon(1e-9) {}

    Tolerance& abs(double e) {
        epsilon = e;
        return *this;
    }

    friend bool operator==(double lhs, Tolerance const& rhs) {
        return std::fabs(lhs - rhs.value) < rhs.epsilon;
    }

    friend bool operator==(Tolerance const& lhs, double rhs) { return operator==(rhs, lhs); }
};

inline std::ostream& operator<<(std::ostream& os, Tolerance const& t) {
    int    precision = 0;
    double eps       = t.epsilon;
    while (eps < 1.0 && precision < 15) {
        eps *= 10.0;
        precision++;
    }
    os << std::fixed << std::setprecision(precision) << t.value;
    return os;
}
