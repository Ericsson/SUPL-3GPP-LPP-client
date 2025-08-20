#pragma once
#include <core/core.hpp>

struct GlobalPressureTempature {
    double pressure;
    double temperature;
    double undulation;
};

GlobalPressureTempature gpt(double dmjd, double dlat, double dlon, double h_ell);
