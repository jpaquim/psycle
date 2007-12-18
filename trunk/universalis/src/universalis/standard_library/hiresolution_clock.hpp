#pragma once
#include "duration.hpp"
#include <universalis/operating_system/clocks.hpp>
namespace std {
	template<typename Time_Point>
	class hiresolution_clock {
		public:
			Time_Point static universal_time() {
				std::nanoseconds const ns(universalis::operating_system::clocks::utc_since_epoch::current());
				Time_Point t(ns);
				return t;
			}
	};
}
