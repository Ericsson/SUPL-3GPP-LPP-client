#pragma once
#include <lpp/asnlib.h>
#include <utility/types.h>
#include "osr/osr.h"

bool gather_reference_station(LPP_Message* lpp, OSR* osr);
bool gather_observations(LPP_Message* lpp, OSR* osr);

double from_msm_lock_ex(long value);
double from_msm_lock(long value);
