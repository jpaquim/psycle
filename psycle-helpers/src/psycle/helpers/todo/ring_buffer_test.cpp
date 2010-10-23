
// compile with g++ --std=c++0x -pthread -Wall -O3

#include <cstddef>
#include <cstdatomic>
#include <cassert>
#include <thread>
#include <chrono>
#include <string>
#include <iostream>

/// ring buffer using c++0x atomic types
class ring_with_atomic_stdlib {
	std::size_t const size_, size_mask_;
	std::atomic<std::size_t> read_, write_;
	
	public:
		ring_with_atomic_stdlib(std::size_t size) : size_(size), size_mask_(size - 1), read_(), write_() {
			assert("power of 2" && !(size & size_mask_));
		}
		
		void commit_read(std::size_t count) {
			read_.fetch_add(count, std::memory_order_release);
		}

		void commit_write(std::size_t count) {
			write_.fetch_add(count, std::memory_order_release);
		}

		void avail_for_read(std::size_t & begin, std::size_t & size1, std::size_t & size2) const {
			auto const read = read_.load(std::memory_order_relaxed);
			begin = read & size_mask_;
			auto const write = write_.load(std::memory_order_acquire);
			size1 = write - read;
			size2 = 0;
		}

		void avail_for_write(std::size_t & begin, std::size_t & size1, std::size_t & size2) const {
			auto const write = write_.load(std::memory_order_relaxed);
			begin = write & size_mask_;
			auto const read = read_.load(std::memory_order_acquire);
			size1 = size_ - (write - read);
			size2 = 0;
		}
};

#if defined __GNUG__
	// nothing to include
#elif defined _MSC_VER
	#include <intrin.h>
	#pragma intrinsic(_ReadWriteBarrier)
	#pragma intrinsic(_ReadBarrier)
	#pragma intrinsic(_WriteBarrier)
#elif defined__APPLE__
	#include <libkern/OSAtomic.h> // for OSMemoryBarrier()
#endif

/// ring buffer using explict memory barriers
class ring_with_explicit_memory_barriers {
	std::size_t const size_, size_mask_;
	std::size_t read_, write_;

	public:
		ring_with_explicit_memory_barriers(std::size_t size) : size_(size), size_mask_(size - 1), read_(), write_() {
			assert("power of 2" && !(size & size_mask_));
		}
		
		void commit_read(std::size_t count) {
			memory_barriers::write();
			read_ += count;
		}

		void commit_write(std::size_t count) {
			memory_barriers::write();
			write_ += count;
		}

		void avail_for_read(std::size_t & begin, std::size_t & size1, std::size_t & size2) const {
			memory_barriers::read();
			begin = read_ & size_mask_;
			size1 = write_ - read_;
			size2 = 0;
		}

		void avail_for_write(std::size_t & begin, std::size_t & size1, std::size_t & size2) const {
			memory_barriers::read();
			begin = write_ & size_mask_;
			size1 = size_ - (write_ - read_);
			size2 = 0;
		}
		
	private:
		struct memory_barriers {
			#if defined __GNUG__
				#if \
					defined __x86_64__ || defined __k8__ || defined __nocona__ || defined __pentium4__ || \
					defined __i686__ || defined __i586__ || defined __i486__ || defined __i386__
					void static  full() { asm volatile ("mfence":::"memory"); }
					void static  read() { asm volatile ("lfence":::"memory"); }
					void static write() { asm volatile ("sfence":::"memory"); }
				#else // defined __ia64__ || defined __hppa__ || defined __alpha__ || defined __sparc__ || defined __mips__ || defined __arm__ || defined __powerpc__
					void static  full() { __sync_synchronize(); }
					void static  read() { full(); }
					void static write() { full(); }
				#endif
			#elif defined _MSC_VER
				void static  full() { _ReadWriteBarrier(); }
				void static  read() { _ReadBarrier(); }
				void static write() { _WriteBarrier(); }
			#elif defined __APPLE__
				void static  full() { OSMemoryBarrier(); }
				void static  read() { full(); }
				void static write() { full(); }
			#else
				#error unimplemented
				void static  full() {}
				void static  read() {}
				void static write() {}
			#endif
		};
};

/// ring buffer using compiler volatile
/// WARNING: It doesn't work reliably on cpu archs with weak memory ordering, and it also suffers from undeterministic wakeups which make it slower.
class ring_with_compiler_volatile {
	std::size_t const size_, size_mask_;
	std::size_t volatile read_, write_;
	
	public:
		ring_with_compiler_volatile(std::size_t size) : size_(size), size_mask_(size - 1), read_(), write_() {
			assert("power of 2" && !(size & size_mask_));
		}
		
		void commit_read(std::size_t count) {
			read_ += count;
		}

		void commit_write(std::size_t count) {
			write_ += count;
		}

		void avail_for_read(std::size_t & begin, std::size_t & size1, std::size_t & size2) const {
			begin = read_ & size_mask_;
			size1 = write_ - read_;
			size2 = 0;
		}

		void avail_for_write(std::size_t & begin, std::size_t & size1, std::size_t & size2) const {
			begin = write_ & size_mask_;
			size1 = size_ - (write_ - read_);
			size2 = 0;
		}
};

template<typename Ring>
void writer_loop(std::size_t buf[], Ring & ring, std::size_t elements_to_process) {
	std::size_t counter = 0;
	while(counter < elements_to_process) {
		std::size_t begin, size1, size2;
		ring.avail_for_write(begin, size1, size2);
		if(size1) {
			for(std::size_t i = begin; i < size1; ++i) buf[i] = ++counter;
			if(size2) {
				for(std::size_t i = 0; i < size2; ++i) buf[i] = ++counter;
				ring.commit_write(size1 + size2);
			} else ring.commit_write(size1);
		}
	}
}

template<typename Ring>
void reader_loop(std::size_t buf[], Ring & ring, std::size_t elements_to_process) {
	std::size_t counter = 0;
	while(counter < elements_to_process) {
		std::size_t begin, size1, size2;
		ring.avail_for_read(begin, size1, size2);
		if(size1) {
			for(std::size_t i = begin; i < size1; ++i) assert(buf[i] == ++counter);
			if(size2) {
				for(std::size_t i = 0; i < size2; ++i) assert(buf[i] == ++counter);
				ring.commit_read(size1 + size2);
			} else ring.commit_read(size1);
		}
	}
}

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

template<typename Ring>
void test() {
	std::size_t const size = 64;
	std::size_t const elements_to_process = 400 * 1000 * 1000;
	std::size_t buf[size];
	Ring ring(size);
	std::chrono::high_resolution_clock clock;
	std::cout <<
		"____________________________\n\n"
		"ring typename: " << demangle(typeid(ring).name()) << "\n"
		"ring buffer size: " << size << "\n"
		"elements to process: " << double(elements_to_process) << "\n"
		"running ... " << std::flush;
	auto const t0 = clock.now();
	std::thread writer_thread(writer_loop<decltype(ring)>, buf, std::ref(ring), elements_to_process);
	std::thread reader_thread(reader_loop<decltype(ring)>, buf, std::ref(ring), elements_to_process);
	writer_thread.join();
	reader_thread.join();
	auto const t1 = clock.now();
	std::cout << "done.\n";
	auto const duration = std::chrono::nanoseconds(t1 - t0).count() * 1e-9;
	std::cout <<
		"duration: " << duration << " seconds\n"
		"troughput: " << elements_to_process / duration << " elements/second\n";
}

int main() {
	//test<ring_with_atomic_stdlib>();
	test<ring_with_explicit_memory_barriers>();
	test<ring_with_compiler_volatile>();
	return 0;
}

