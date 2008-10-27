// -*- mode:c++; indent-tabs-mode:t -*-
// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2008-2008 members of the psycle project http://psycle.sourceforge.net ; johan boule <bohan@jabber.org>

///\interface psycle::helpers::ring_buffer
#pragma once
#include <universalis/processor/compare_and_swap.hpp>
namespace psycle { namespace helpers {

class ring_buffer {
	public:
		typedef unsigned int size_type;
		
		ring_buffer(size_type size) : size_(size), read_position_(), write_position_() {}

	///\name size
	///\{		
		public:
			size_type size() const { return size_; }
		private:
			size_type size_;
	///\}

	///\name read position
	///\{		
		public:
			size_type read_position() const { return read_position_; }
			void advance_read_position(size_type amount) { add(read_position_, amount); }
		private:
			size_type read_position_;
	///\}

	///\name write position
	///\{		
		public:
			size_type write_position() const { return write_position_; }
			void advance_write_position(size_type amount) { add(write_position_, amount); }
		private:
			size_type write_position_;
	///\}

	private:
		bool compare_and_swap(size_type * address, size_type old_value, size_type new_value) {
			return universalis::processor::compare_and_swap(address, old_value, new_value);
		}

		bool add(size_type & var, size_type amount) {
			while(!compare_and_swap(&var, var, var + amount));
		}
};

}}
