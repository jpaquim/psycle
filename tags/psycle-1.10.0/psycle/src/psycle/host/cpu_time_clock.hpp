// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2009-2010 members of the psycle project http://psycle.sourceforge.net

///\interface psycle::host::cpu_time_clock

#pragma once
#include <psycle/host/detail/project.hpp>
#include <universalis/os/clocks.hpp>

namespace psycle { namespace host {

/// This clock is meant to have the following characteristics:
/// - high resolution: can measure very short durations,
/// - monotonic: does not jump nor accelerate/deccelerate to adjust to official time,
/// - virtual thread time: ideally, this counts the time spent by cpu(s) in the current thread, blocked time not counted.
typedef universalis::os::clocks::hires_thread_or_fallback cpu_time_clock;

/// This clock is meant to have the following characteristics:
/// - high resolution: can measure very short durations,
/// - monotonic: does not jump nor accelerate/deccelerate to adjust to official time,
/// - real, wall time: counts the real time elapsed, since some unspecified origin.
typedef universalis::os::clocks::monotonic wall_time_clock;

}}
