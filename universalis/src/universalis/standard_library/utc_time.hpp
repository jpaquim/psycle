// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007-2007 psycledelics http://psycle.pastnotecut.org ; johan boule <bohan@jabber.org>
#pragma once
#include "duration.hpp"
#include <ctime> // for std::time_t in std::utc_time
namespace std {

	template<typename Time_Point>
	class hiresolution_clock;

	class utc_time {
		public:
			/// storage type for number of nanoseconds since epoch
			typedef nanoseconds::tick_type tick_type;

			/// number of ticks per second
			tick_type static ticks_per_second() { return nanoseconds::ticks_per_second(); }

			/// number of seconds per tick
			tick_type static seconds_per_tick() { return nanoseconds::seconds_per_tick(); }

			/// true if resolution is less than 1 second
			bool static is_subsecond() { return nanoseconds::is_subsecond(); }

			/// zero time (i.e. epoch)
			utc_time() : ns_() {}

			#if 0 ///\todo
			/// constructs a time point from the given separate seconds and nanoseconds since epoch
			utc_time(time_t t, nanoseconds ns);
			#endif

			/// number of seconds since epoch
			std::time_t seconds_since_epoch() const { return ns_ / ticks_per_second(); }

			/// number of nanoseconds since epoch
			nanoseconds nanoseconds_since_epoch() const { return ns_; }

			/// returns true if this == that
			bool operator==(utc_time const & that) const { return this->ns_ == that.ns_; }

			/// returns true if this != that
			bool operator!=(utc_time const & that) const { return this->ns_ != that.ns_; }

			/// returns true if this < that
			bool operator<(utc_time const & that) const { return this->ns_ < that.ns_; }

			/// returns true if this > that
			bool operator>(utc_time const & that) const { return this->ns_ > that.ns_; }

			/// returns true if this <= that
			bool operator<=(utc_time const & that) const { return this->ns_ <= that.ns_; }

			/// returns true if this >= that
			bool operator>=(utc_time const & that) const { return this->ns_ >= that.ns_; }

			/// returns duration between this and that
			nanoseconds operator-(utc_time const & that) const { return this->ns_ - that.ns_; }

			/// returns this time point shifted by the given duration
			template<typename Duration>
			utc_time operator+(Duration const & d) const { nanoseconds ns(d); return utc_time(ns_ + d); }

			/// returns this time point shifted by the given duration
			template<typename Duration>
			utc_time operator-(Duration const & d) const { nanoseconds ns(d); return utc_time(ns_ - d); }

			/// returns shifts this time point by the given duration
			template<typename Duration>
			utc_time operator+=(Duration const & d) const { nanoseconds ns(d); this->ns_ += ns.get_count(); return *this; }

			/// returns shifts this time point by the given duration
			template<typename Duration>
			utc_time operator-=(Duration const & d) const { nanoseconds ns(d); this->ns_ -= ns.get_count(); return *this; }

		private:
			tick_type ns_;
			utc_time(tick_type t) : ns_(t) {} friend class hiresolution_clock<utc_time>;
	};
}
