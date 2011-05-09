// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2002-2011 members of the psycle project http://psycle.pastnotecut.org : johan boule <bohan@jabber.org>

///\interface psycle::engine::event
#ifndef PSYCLE__ENGINE__EVENT__INCLUDED
#define PSYCLE__ENGINE__EVENT__INCLUDED
#pragma once
#include "sample.hpp"
#define PSYCLE__DECL  PSYCLE__ENGINE
#include <psycle/detail/decl.hpp>
namespace psycle { namespace engine {

/// a sample with an associated, logical, index.
/// the index corresponds to a logical (i.e., not physical) position in a buffer.
class event {
	///\name sample
	///\{
		public:
			/// the immutable value of the sample.
			///\return immutable value of the sample
			real sample() const { return sample_; }
			operator real() const { return sample_; }

			/// the mutable value of the sample.
			///\return mutable value of the sample
			real & sample() { return sample_; }
			operator real&() { return sample_; }

			/// sets the value of the sample.
			///\param the value of the sample
			void sample(real const & sample) { sample_ = sample; }
		private:
			real sample_;
	///\}

	///\name index
	///\{
		public:
			/// the logical index of the sample.
			///\returns the logical index of the sample
			std::size_t index() const { return index_; }

			/// the mutable logical index of the sample.
			///\returns mutable value of the logical index of the sample
			std::size_t & index() { return index_; }

			/// sets the logical index of the sample.
			///\param the logical index of the sample
			void index(std::size_t index) { index_ = index; }
		private:
			std::size_t index_;
	///\}

	public:
		/// sets both the logical index and the sample value.
		void operator()(std::size_t index, real const & sample) { index_ = index; sample_ = sample; }
	
		/// compares the logical indexes of two events.
		/// The value of the sample is not taken into account.
		///\return true if this event has a logical index equal to the one of the other event.
		bool operator==(event const & event) const { return index_ == event.index_; }

		/// compares the logical indexes of two events.
		/// The value of the sample is not taken into account.
		///\return true if this event has a logical index lower than the one of the other event.
		bool operator<(event const & event) const { return index_ < event.index_; }
};

}}
#include <psycle/detail/decl.hpp>
#endif
