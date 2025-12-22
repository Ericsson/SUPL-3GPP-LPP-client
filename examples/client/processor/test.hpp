#pragma once
#include <memory>
#include "../program_io.hpp"

#include <streamline/inspector.hpp>
#include <streamline/system.hpp>

#include "config.hpp"

namespace scheduler {
class Scheduler;
}

void test_outputer(scheduler::Scheduler& scheduler, ProgramOutput const& output, uint64_t tag);
