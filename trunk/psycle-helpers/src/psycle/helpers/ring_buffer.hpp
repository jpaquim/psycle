// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2008-2008 members of the psycle project http://psycle.sourceforge.net ; johan boule <bohan@jabber.org>

///\interface psycle::helpers::ring_buffer
#pragma once
#include <universalis/processor/atomic/compare_and_swap.hpp>
#include <cstddef>
namespace psycle { namespace helpers {

// see portaudio memory barrier http://portaudio.com/trac/browser/portaudio/trunk/src/common/pa_memorybarrier.h
// see portaudio ring buffer    http://portaudio.com/trac/browser/portaudio/trunk/src/common/pa_ringbuffer.c

/// lock-free ring buffer.
///
/// This ring buffer is useful for push-based multi-threaded processing.
/// It helps optimising thread-synchronisation by using lock-free atomic primitives
/// (normally implemented with specific CPU instructions),
/// instead of relying on OS-based synchronisation facilities.
///
/// This class does not store the actual buffer data, but only a read position and a write position.
/// This separation of concerns makes it possible to use this class with any kind of data access interface for the buffer.
template<typename Size_Type = std::size_t>
class ring_buffer {
	public:
		typedef Size_Type size_type;
		
		ring_buffer(size_type size = 0) { this->size(size); }

		void reset() { read_position_ = write_position_ = 0; }

	///\name size
	///\{
		public:
			size_type size() const { return size_; }
			void size(size_type size) { to_power_of_2(size); reset(); }
		private:
			size_type size_, size_mask_;
	///\}

	///\name read position
	///\{
		public:
			void read_position_and_sizes(size_type & position, size_type & size1, size_type & size2) const {
				size_type const r(read_position_), w(write_position_);
				position = r;
				if(r < w) {
					size1 = w - r;
					size2 = 0;
				} else {
					size1 = size_ - r;
					size2 = w - 1;
				}
			}
			void advance_read_position(size_type amount) { add(read_position_, amount); }
		private:
			size_type read_position_;
	///\}

	///\name write position
	///\{
		public:
			void write_position_and_sizes(size_type & position, size_type & size1, size_type & size2) const {
				size_type const r(read_position_), w(write_position_);
				position = w;
				if(w < r) {
					size1 = r - w;
					size2 = 0;
				} else {
					size1 = size_ - w;
					size2 = r - 1;
				}
			}
			void advance_write_position(size_type amount) { add(write_position_, amount); }
		private:
			size_type write_position_;
	///\}

	private:
		void to_power_of_2(size_type size) {
			if(!size) {
				size_ = size;
				size_mask_ = 0;
			} else {
				size_type power_of_2 = 1;
				while(1 << power_of_2 < size) ++power_of_2;
				size_ = 1 << power_of_2;
				size_mask_ = size - 1;
			}
		}
		
		bool static compare_and_swap(size_type * address, size_type old_value, size_type new_value) {
			return universalis::processor::atomic::compare_and_swap(address, old_value, new_value);
		}

		void add(size_type & var, size_type amount) {
			while(!compare_and_swap(&var, var, (var + amount) & size_mask_));
		}
};

}}
