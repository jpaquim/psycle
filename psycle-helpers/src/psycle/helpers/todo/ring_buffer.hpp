// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2008-2008 members of the psycle project http://psycle.sourceforge.net ; johan boule <bohan@jabber.org>

///\interface psycle::helpers::ring_buffer
#pragma once
#include <universalis.hpp>
#include <universalis/cpu/memory_barriers.hpp>
#include <cstddef>
namespace psycle { namespace helpers {

// see portaudio memory barrier http://portaudio.com/trac/browser/portaudio/trunk/src/common/pa_memorybarrier.h
// see portaudio ring buffer    http://portaudio.com/trac/browser/portaudio/trunk/src/common/pa_ringbuffer.c

/// lock-free ring buffer.
///
/// This ring buffer is useful for push-based multi-threaded processing.
/// It helps optimising thread-synchronisation by using lock-free atomic primitives
/// (normally implemented with specific CPU instructions),
/// instead of relying on OS-based synchronisation facilities,
/// which incur the overhead of context switching and re-scheduling delay.
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
			void size(size_type size) {
				if(!size) {
					size_ = size;
					size_mask_ = 0;
				} else {
					size_type power_of_2 = 1;
					while(1 << power_of_2 < size) ++power_of_2;
					size_ = 1 << power_of_2;
					size_mask_ = size - 1;
				}
				reset();
			}
		private:
			size_type size_, size_mask_;
	///\}

	///\name read position
	///\{
		public:
			void wait_for_read_avail(
				size_type wanted_size,
				size_type & UNIVERSALIS__COMPILER__RESTRICT_REF position,
				size_type & UNIVERSALIS__COMPILER__RESTRICT_REF size1,
				size_type & UNIVERSALIS__COMPILER__RESTRICT_REF size2
			) {
				do get_read_position_and_sizes(position, size1, size2);
				while(size1 + size2 < wanted_size);
			}
			
			void get_read_position_and_sizes(
				size_type & UNIVERSALIS__COMPILER__RESTRICT_REF position,
				size_type & UNIVERSALIS__COMPILER__RESTRICT_REF size1,
				size_type & UNIVERSALIS__COMPILER__RESTRICT_REF size2
			) const {
				universalis::cpu::memory_barriers::read();
				size_type const r = read_position_, w = write_position_;
				position = r;
				if(r < w) {
					size1 = w - r;
					size2 = 0;
				} else {
					size1 = size_ - r;
					size2 = w;
				}
			}
			
			void advance_read_position(size_type amount) {
				universalis::cpu::memory_barriers::write();
				read_position_ += amount;
				read_position_ &= size_mask_;
			}
		private:
			size_type read_position_;
	///\}

	///\name write position
	///\{
		public:
			void wait_for_write_avail(
				size_type wanted_size,
				size_type & UNIVERSALIS__COMPILER__RESTRICT_REF position,
				size_type & UNIVERSALIS__COMPILER__RESTRICT_REF size1,
				size_type & UNIVERSALIS__COMPILER__RESTRICT_REF size2
			) {
				do get_write_position_and_sizes(position, size1, size2);
				while(size1 + size2 < wanted_size);
			}

			void get_write_position_and_sizes(
				size_type & UNIVERSALIS__COMPILER__RESTRICT_REF position,
				size_type & UNIVERSALIS__COMPILER__RESTRICT_REF size1,
				size_type & UNIVERSALIS__COMPILER__RESTRICT_REF size2
			) const {
				universalis::cpu::memory_barriers::read();
				size_type const r = read_position_, w = write_position_;
				position = w;
				if(w < r) {
					size1 = r - w;
					size2 = 0;
				} else {
					size1 = size_ - w;
					size2 = r;
				}
			}
			
			void advance_write_position(size_type amount) {
				universalis::cpu::memory_barriers::write();
				write_position_ += amount;
				write_position_ &= size_mask_;
			}
		private:
			size_type write_position_;
	///\}
};

}}

	#include <universalis/stdlib/thread.hpp>
	#include <universalis/os/clocks.hpp>
	#include <vector>
	#include <boost/bind.hpp>
	#include <sstream>
	namespace {
		using namespace universalis::stdlib;

		class ring_buffer {
			std::size_t const size_;
			std::ptrdiff_t read_, write_;
			public:
				ring_buffer(std::size_t size) : size_(size), read_(), write_() {}

				void get_write_avail(std::ptrdiff_t & begin, std::size_t & end1, std::size_t & end2) {
					begin = write_;
					std::ptrdiff_t const read = read_;
					if(read <= write_) {
						end1 = size_;
						end2 = read;
					} else {
						end1 = read;
						end2 = 0;
					}
					std::clog << "wa: b: " << begin << ", e1: " << end1 << ", e2: " << end2 << '\n';
				}

				void advance_write(std::ptrdiff_t count) {
					if(count) write_ = (write_ + count) % size_;
				}

				void get_read_avail(std::ptrdiff_t & begin, std::size_t & end1, std::size_t & end2) {
					begin = read_;
					std::ptrdiff_t const write = write_;
					if(write < read_) {
						end1 = size_;
						end2 = write;
					} else {
						end1 = write;
						end2 = 0;
					}
					std::clog << "ra: b: " << begin << ", e1: " << end1 << ", e2: " << end2 << '\n';
				}

				void advance_read(std::ptrdiff_t count) {
					if(count) read_ = (read_ + count) % size_;
				}
		};

		typedef nanoseconds::tick_type counter;

		void writer_loop(counter buf[], ring_buffer & ring, counter iterations) {
			counter c = 0;
			while(c < iterations) {
				std::ptrdiff_t begin;
				std::size_t end1, end2;
				ring.get_write_avail(begin, end1, end2);
				for(std::ptrdiff_t i = begin; i < end1; ++i) buf[i] = ++c;
				for(std::ptrdiff_t i = 0; i < end2; ++i) buf[i] = ++c;
				ring.advance_write(end1 - begin + end2);
			}
		}

		void reader_loop(counter buf[], ring_buffer & ring, counter iterations) {
			counter c = 0;
			while(c < iterations) {
				std::ptrdiff_t begin;
				std::size_t end1, end2;
				ring.get_read_avail(begin, end1, end2);
				if(end1) {
					for(std::ptrdiff_t i = begin; i < end1; ++i) if(buf[i] != ++c) throw 0;
					if(end2) for(std::ptrdiff_t i = 0; i < end2; ++i) if(buf[i] != ++c) throw 0;
					ring.advance_read(end1 - begin + end2);
				}
			}
		}

		nanoseconds static inline cpu_time_clock() {
			return universalis::os::clocks::monotonic::current();
		}

		BOOST_AUTO_TEST_CASE(x) {
			counter const iterations = 100000;
			std::size_t const size = 512;
			counter buf[size];
			for(std::size_t i = 0; i < size; ++i) buf[i] = 0;
			ring_buffer ring(size);
			thread writer_thread(boost::bind(writer_loop, buf, ring, iterations));
			thread reader_thread(boost::bind(reader_loop, buf, ring, iterations));
			nanoseconds const t0 = cpu_time_clock();
			writer_thread.join();
			reader_thread.join();
			nanoseconds const t1 = cpu_time_clock();
			{
				std::ostringstream s;
				s << buf[0] << '\n' << (t1 - t0).get_count() * 1e-9 << 's';
				BOOST_MESSAGE(s.str());
			}
		}
	}
#endif
