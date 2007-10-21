// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2007 johan boule <bohan@jabber.org>
// copyright 2004-2007 psycledelics http://psycle.pastnotecut.org

///\interface psycle::engine::event
#pragma once
#include "sample.hpp"
#define UNIVERSALIS__COMPILER__DYNAMIC_LINK  PSYCLE__ENGINE__EVENT
#include <universalis/compiler/dynamic_link/begin.hpp>
namespace psycle { namespace engine {

/// a sample with an associated, logical, index.
/// the index corresponds to a logical (i.e., not physical) position in a buffer.
class UNIVERSALIS__COMPILER__DYNAMIC_LINK event {
	///\name sample
	///\{
		public:
			/// the immutable value of the sample.
			///\return immutable the value of the sample
			real sample() const throw() { return sample_; }
			/// the mutable value of the sample.
			///\return mutable the value of the sample
			real & sample() throw() { return sample_; }
			/// sets the value of the sample.
			///\param the value of the sample
			void sample(real const & sample) throw() { sample_ = sample; }
			/// compares the logical indexes of two events.
			/// The value of the sample is not taken into account.
			///\return true if both events have the same logical index
		private:
			real sample_;
	///\}

	///\name index
	///\{
		public:
			/// the logical index of the sample.
			///\returns the logical index of the sample
			std::size_t index() const throw() { return index_; }
			/// the mutable logical index of the sample.
			///\returns mutable value of the logical index of the sample
			std::size_t & index() throw() { return index_; }
			/// sets the logical index of the sample.
			///\param the logical index of the sample
			void index(std::size_t index) throw() { index_ = index; }
			/// the const value of the sample.
			///\returns the const value of the sample
		private:
			std::size_t index_;
	///\}

	public:
		void operator()(std::size_t index, real const & sample) { index_ = index; sample_ = sample; }
	
		/// compares the logical indexes of two events.
		/// The value of the sample is not taken into account.
		///\return true if this event has a logical index equal to the one of the other event.
		bool operator==(event const & event) const throw() { return this->index() == event.index(); }
		/// compares the logical indexes of two events.
		/// The value of the sample is not taken into account.
		///\return true if this event has a logical index lower than the one of the other event.
		bool operator<(event const & event) const throw() { return this->index() < event.index(); }
};

}}
#include <universalis/compiler/dynamic_link/end.hpp>

