
// compile with g++ --std=c++0x -pthread -Wall -O3

#include <cstddef>
#include <cstdatomic>
#include <cassert>
#include <algorithm>
#include <thread>
#include <utility>
#include <random>
#include <chrono>
#include <string>
#include <iostream>

/// ring buffer concept
class ring {
	public:
		ring(std::size_t size) { assert("power of 2" && !(size & (size - 1))); }
		std::size_t size() const;
		void commit_read(std::size_t count);
		void commit_write(std::size_t count);
		void avail_for_read(std::size_t max, std::size_t & begin, std::size_t & size1, std::size_t & size2) const;
		void avail_for_write(std::size_t max, std::size_t & begin, std::size_t & size1, std::size_t & size2) const;
};

/// ring buffer using c++0x atomic types
class ring_with_atomic_stdlib {
	std::size_t const size_, size_mask_;
	std::atomic<std::size_t> read_, write_;
	
	public:
		ring_with_atomic_stdlib(std::size_t size) : size_(size), size_mask_(size - 1), read_(), write_() {
			assert("power of 2" && !(size & size_mask_));
		}
		
		std::size_t size() const { return size_; }
		
		void commit_read(std::size_t count) {
			read_.fetch_add(count, std::memory_order_release);
		}

		void commit_write(std::size_t count) {
			write_.fetch_add(count, std::memory_order_release);
		}

		void avail_for_read(std::size_t max, std::size_t & begin, std::size_t & size1, std::size_t & size2) const {
			auto const read = read_.load(std::memory_order_relaxed);
			begin = read & size_mask_;
			auto const write = write_.load(std::memory_order_acquire);
			size1 = write - read;
			size2 = 0;
		}

		void avail_for_write(std::size_t max, std::size_t & begin, std::size_t & size1, std::size_t & size2) const {
			auto const write = write_.load(std::memory_order_relaxed);
			begin = write & size_mask_;
			auto const read = read_.load(std::memory_order_acquire);
			size1 = size_ - (write - read);
			size2 = 0;
		}
};

/// ring buffer using explict memory barriers
class ring_with_explicit_memory_barriers {
	std::size_t const size_, size_mask_, size_mask2_;
	std::size_t read_, write_;
	class memory_barriers {
		public:
			void inline static  full();
			void inline static  read();
			void inline static write();
	};

	public:
		ring_with_explicit_memory_barriers(std::size_t size) : size_(size), size_mask_(size - 1), size_mask2_(size * 2 - 1), read_(), write_() {
			assert("power of 2" && !(size & size_mask_));
		}
		
		std::size_t size() const { return size_; }
		
		void commit_read(std::size_t count) {
			memory_barriers::write();
			read_ = (read_ + count) & size_mask2_;
		}

		void commit_write(std::size_t count) {
			memory_barriers::write();
			write_ = (write_ + count) & size_mask2_;
		}

		void avail_for_read(std::size_t max, std::size_t & begin, std::size_t & size1, std::size_t & size2) const {
			begin = read_ & size_mask_;
			memory_barriers::read();
			auto avail = std::min(max, (write_ - read_) & size_mask2_);
			if(begin + avail > size_) {
				size1 = size_ - begin;
				size2 = avail - size1;
			} else {
				size1 = avail;
				size2 = 0;
			}
		}

		void avail_for_write(std::size_t max, std::size_t & begin, std::size_t & size1, std::size_t & size2) const {
			begin = write_ & size_mask_;
			memory_barriers::read();
			auto avail = std::min(max, size_ - ((write_ - read_) & size_mask2_));
			if(begin + avail > size_) {
				size1 = size_ - begin;
				size2 = avail - size1;
			} else {
				size1 = avail;
				size2 = 0;
			}
		}
};

/// ring buffer using compiler volatile
/// WARNING: It doesn't work reliably on cpu archs with weak memory ordering, and it also suffers from undeterministic wakeups which make it slower.
class ring_with_compiler_volatile {
	std::size_t const size_, size_mask_, size_mask2_;
	std::size_t volatile read_, write_;
	
	public:
		ring_with_compiler_volatile(std::size_t size) : size_(size), size_mask_(size - 1), size_mask2_(size * 2 - 1), read_(), write_() {
			assert("power of 2" && !(size & size_mask_));
		}
		
		std::size_t size() const { return size_; }
		
		void commit_read(std::size_t count) {
			read_ = (read_ + count) & size_mask2_;
		}

		void commit_write(std::size_t count) {
			write_ = (write_ + count) & size_mask2_;
		}

		void avail_for_read(std::size_t max, std::size_t & begin, std::size_t & size1, std::size_t & size2) const {
			begin = read_ & size_mask_;
			auto avail = std::min(max, (write_ - read_) & size_mask2_);
			if(begin + avail > size_) {
				size1 = size_ - begin;
				size2 = avail - size1;
			} else {
				size1 = avail;
				size2 = 0;
			}
		}

		void avail_for_write(std::size_t max, std::size_t & begin, std::size_t & size1, std::size_t & size2) const {
			begin = write_ & size_mask_;
			auto avail = std::min(max, size_ - ((write_ - read_) & size_mask2_));
			if(begin + avail > size_) {
				size1 = size_ - begin;
				size2 = avail - size1;
			} else {
				size1 = avail;
				size2 = 0;
			}
		}
};

template<typename Ring, typename RandGen>
void writer_loop(std::size_t buf[], Ring & ring, RandGen & rand_gen, std::size_t elements_to_process) {
	std::size_t counter = 0;
	while(counter < elements_to_process) {
		std::size_t begin, size1, size2;
		ring.avail_for_write(rand_gen(), begin, size1, size2);
		if(size1) {
			for(std::size_t i = begin, e = begin + size1; i < e; ++i) buf[i] = ++counter;
			if(size2) {
				for(std::size_t i = 0; i < size2; ++i) buf[i] = ++counter;
				ring.commit_write(size1 + size2);
			} else ring.commit_write(size1);
		}
	}
}

template<typename Ring, typename RandGen>
void reader_loop(std::size_t buf[], Ring & ring, RandGen & rand_gen, std::size_t elements_to_process) {
	std::size_t counter = 0;
	while(counter < elements_to_process) {
		std::size_t begin, size1, size2;
		ring.avail_for_read(rand_gen(), begin, size1, size2);
		if(size1) {
			for(std::size_t i = begin, e = begin + size1; i < e; ++i) assert(buf[i] == ++counter);
			if(size2) {
				for(std::size_t i = 0; i < size2; ++i) assert(buf[i] == ++counter);
				ring.commit_read(size1 + size2);
			} else ring.commit_read(size1);
		}
	}
}

std::string demangle(std::string const & mangled_symbol);

template<typename Ring>
void test(std::random_device::result_type writer_rand_gen_seed, std::random_device::result_type reader_rand_gen_seed) {
	std::size_t const size = 256;
	std::size_t const elements_to_process = 1000 * 1000 * 1000;
	std::size_t buf[size];
	Ring ring(size);
	typedef std::variate_generator<std::mt19937, std::uniform_int<std::size_t>> rand_gen_type;
	rand_gen_type::result_type const rand_gen_dist_lower = ring.size() / 4;
	rand_gen_type::result_type const rand_gen_dist_upper = ring.size() / 2;
	rand_gen_type writer_rand_gen { rand_gen_type::engine_type(reader_rand_gen_seed), rand_gen_type::distribution_type(rand_gen_dist_lower, rand_gen_dist_upper) };
	rand_gen_type reader_rand_gen { rand_gen_type::engine_type(writer_rand_gen_seed), rand_gen_type::distribution_type(rand_gen_dist_lower, rand_gen_dist_upper) };
	std::cout <<
		"____________________________\n\n"
		"ring typename: " << demangle(typeid(ring).name()) << "\n"
		"ring buffer size: " << size << "\n"
		"elements to process: " << double(elements_to_process) << "\n"
		"rand gen typename: " << demangle(typeid(rand_gen_type).name()) << "\n"
		"rand gen dist range: " << rand_gen_dist_lower << ' ' << rand_gen_dist_upper << "\n"
		"writer rand gen seed: " << writer_rand_gen_seed << "\n"
		"reader rand gen seed: " << reader_rand_gen_seed << "\n"
		"running ... " << std::flush;
	auto const t0 = std::chrono::high_resolution_clock::now();
	std::thread writer_thread(writer_loop<Ring, rand_gen_type>, buf, std::ref(ring), std::ref(writer_rand_gen), elements_to_process);
	std::thread reader_thread(reader_loop<Ring, rand_gen_type>, buf, std::ref(ring), std::ref(reader_rand_gen), elements_to_process);
	writer_thread.join();
	reader_thread.join();
	auto const t1 = std::chrono::high_resolution_clock::now();
	std::cout << "done.\n";
	auto const duration = std::chrono::nanoseconds(t1 - t0).count() * 1e-9;
	std::cout <<
		"duration: " << duration << " seconds\n"
		"troughput: " << elements_to_process / duration << " elements/second\n";
}

int main() {
	std::random_device rand_dev;
	std::random_device::result_type const writer_rand_gen_seed = rand_dev();
	std::random_device::result_type const reader_rand_gen_seed = rand_dev();
	//test<ring_with_atomic_stdlib>(writer_rand_gen_seed, reader_rand_gen_seed);
	test<ring_with_explicit_memory_barriers>(writer_rand_gen_seed, reader_rand_gen_seed);
	test<ring_with_compiler_volatile>(writer_rand_gen_seed, reader_rand_gen_seed);
	return 0;
}

/*********************************************************************/
// platform-specific code below

#if defined __GNUG__
	#if \
		defined __x86_64__ || defined __k8__ || defined __nocona__ || defined __pentium4__ || \
		defined __i686__ || defined __i586__ || defined __i486__ || defined __i386__
		void inline ring_with_explicit_memory_barriers::memory_barriers:: full() { asm volatile ("mfence":::"memory"); }
		void inline ring_with_explicit_memory_barriers::memory_barriers:: read() { asm volatile ("lfence":::"memory"); }
		void inline ring_with_explicit_memory_barriers::memory_barriers::write() { asm volatile ("sfence":::"memory"); }
	#else // defined __ia64__ || defined __hppa__ || defined __alpha__ || defined __sparc__ || defined __mips__ || defined __arm__ || defined __powerpc__
		void inline ring_with_explicit_memory_barriers::memory_barriers:: full() { __sync_synchronize(); }
		void inline ring_with_explicit_memory_barriers::memory_barriers:: read() { full(); }
		void inline ring_with_explicit_memory_barriers::memory_barriers::write() { full(); }
	#endif
#elif defined _MSC_VER
	#include <intrin.h>
	#pragma intrinsic(_ReadWriteBarrier)
	#pragma intrinsic(_ReadBarrier)
	#pragma intrinsic(_WriteBarrier)
	void inline ring_with_explicit_memory_barriers::memory_barriers:: full() { _ReadWriteBarrier(); }
	void inline ring_with_explicit_memory_barriers::memory_barriers:: read() { _ReadBarrier(); }
	void inline ring_with_explicit_memory_barriers::memory_barriers::write() { _WriteBarrier(); }
#elif defined __APPLE__
	#include <libkern/OSAtomic.h> // for OSMemoryBarrier()
	void inline ring_with_explicit_memory_barriers::memory_barriers:: full() { OSMemoryBarrier(); }
	void inline ring_with_explicit_memory_barriers::memory_barriers:: read() { full(); }
	void inline ring_with_explicit_memory_barriers::memory_barriers::write() { full(); }
#else
	#error unimplemented
	void inline ring_with_explicit_memory_barriers::memory_barriers:: full() {}
	void inline ring_with_explicit_memory_barriers::memory_barriers:: read() {}
	void inline ring_with_explicit_memory_barriers::memory_barriers::write() {}
#endif

#if defined __GNUG__
	#include <cxxabi.h>
	#include <cstdlib>
#endif

std::string demangle(std::string const & mangled_symbol) {
	#if defined __GNUG__
		int status;
		char * c(abi::__cxa_demangle(mangled_symbol.c_str(), 0, 0, &status));
		std::string s(c);
		std::free(c);
		return s;
	#else
		return mangled_symbol;
	#endif
}

