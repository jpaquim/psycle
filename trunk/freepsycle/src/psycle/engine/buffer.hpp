// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2012 members of the psycle project http://psycle.pastnotecut.org : johan boule <bohan@jabber.org>

#pragma once
#include "forward_declarations.hpp"
#include <psycle/helpers/math.hpp>
#include <vector>
#include <cassert>
#define PSYCLE__DECL  PSYCLE__ENGINE
#include <psycle/detail/decl.hpp>
namespace psycle { namespace engine {

typedef double real;

namespace math = helpers::math;

/// vectors of channels of samples, and logical indexes.
class PSYCLE__DECL buffer {
	public:
		/// creates a new buffer with the given number of channels and the given number of events in each channel.
		///\param channels the number of channels
		///\param events the number of events in each channel
		buffer(std::size_t channels, std::size_t events);

		/// destructor.
		virtual ~buffer() throw();

		/// the number of channels (size of the vector of channels).
		///\return the number of channels
		std::size_t channels() const { return samples_.size(); }

		/// sets the number of channels.
		///\param the number of channels
		void channels(std::size_t channels) { resize(channels, events()); }

		/// the number of events in each channels.
		///\return the number of events in each channel
		std::size_t events() const { return events_; }

		/// sets the number of events in each channel.
		///\param the number of events in each channel
		void events(std::size_t events) { resize(channels(), events); }

		/// sets the number of channels and number of events in each channel.
		///\param channels the number of channels
		///\param events the number of events in each channel
		void resize(std::size_t channels, std::size_t events);

		/// clears all events.
		/// A complexity of o(1) is achieved by simply setting the index of the first event. TODO only setting flag is necessary.
		void clear() { *indexes_.begin() = events(); flag(flags::empty); /* TODO only setting flag is necessary */ }

		/// copies the first given number of channels from another buffer into this buffer.
		///\param buffer the buffer to copy from
		///\param channels the number of channels to copy
		void inline copy(buffer const &, std::size_t channels);

	private:
		std::size_t events_;

	///\name flags to give hint to process loops
	///\{
		public:
			enum class flags {
				empty, ///< indicates there is no event
				discrete, ///< indicates there are events, but not one for every sample
				continuous ///< indicates there is an event for every sample
			};
			
			flags flag() const { return flag_; }
			void flag(flags flag) { flag_ = flag; }
		private:
			flags flag_;
	///\}
	
	///\name access to samples
	///\{
		public:
			real const & sample(std::size_t event, std::size_t channel = 0) const {
				assert(event < events());
				assert(channel < channels());
				return samples_[channel][event];
			}

			real & sample(std::size_t event, std::size_t channel = 0) {
				assert(event < events());
				assert(channel < channels());
				return samples_[channel][event];
			}
		private:
			typedef std::vector<std::vector<real>> samples_type;
			samples_type samples_;
	///\}

	///\name vector of indexes
	/// Samples are associated a logical index.
	/// The index corresponds to a logical (i.e., not physical) position in a buffer.
	///\{
		private:
			typedef std::vector<std::size_t> indexes_type;
			indexes_type indexes_;
		public:
			indexes_type::const_reference index(indexes_type::size_type event) const { assert(event < events()); return indexes_[event]; }
			indexes_type::reference index(indexes_type::size_type event) { assert(event < events()); return indexes_[event]; }
	///\}
	
	public:
		operator bool() const { return events() && events() > index(0); }
};

}}
#include <psycle/detail/decl.hpp>

/**********************************************************************************************************************/
// implementation details

namespace psycle { namespace engine {

void buffer::copy(buffer const & buffer, std::size_t channels) {
	if(loggers::trace()()) {
		std::ostringstream s;
		s << "copying " << channels << " channels from buffer " << &buffer << " to buffer " << this;
		loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
	}
	assert("not copying itself: " && this != &buffer); // would not cause a bug, but this catches lacks of optimizations.
	// TODO We can even optimise the loop with the flag information.
	for(std::size_t event = 0; event < events() && buffer.indexes_[event] < events() ; ++event) {
		indexes_[event] = buffer.indexes_[event];
		for(std::size_t channel = 0; channel < channels ; ++channel)
			samples_[channel][event] = buffer.samples_[channel][event];
	}
	flag(buffer.flag());
}

}}
