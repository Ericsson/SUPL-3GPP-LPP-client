#pragma once
#include "coordinates/enu.hpp"
#include "coordinates/llh.hpp"

namespace coordinates {
namespace nkgrf17vel {

Enu<> lookup(Llh<> const& position);

}  // namespace nkgrf17vel
}  // namespace coordinates
