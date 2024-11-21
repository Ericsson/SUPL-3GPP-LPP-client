#include <streamline/inspector.hpp>
#include <streamline/system.hpp>

#include "lpp.hpp"

namespace ssr_example {
struct SsrGlobals;
}

extern void tokoro_initialize(streamline::System& system, ssr_example::SsrGlobals const& globals,
                              OutputOptions const& options);
