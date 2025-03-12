#pragma once
#include <memory>

#include <streamline/inspector.hpp>
#include <streamline/system.hpp>

#include "config.hpp"

namespace scheduler {
class Scheduler;
}

void test_outputer(scheduler::Scheduler& scheduler, OutputConfig const& output);
