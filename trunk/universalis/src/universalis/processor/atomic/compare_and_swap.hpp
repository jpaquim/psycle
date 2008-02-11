/* -*- mode:c++, indent-tabs-mode:t -*- */
// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2007 johan boule <bohan@jabber.org>
// copyright 2004-2007 psycledelics http://psycle.pastnotecut.org

///\interface universalis::processor::atomic::compare_and_swap
#pragma once
#include <universalis/operating_system/exception.hpp>
#include <diversalis/compiler.hpp>
#define UNIVERSALIS__COMPILER__DYNAMIC_LINK UNIVERSALIS__PROCESSOR__ATOMIC__COMPARE_AND_SWAP
#include <universalis/compiler/dynamic_link/begin.hpp>
namespace universalis
{
	namespace processor
	{
		namespace atomic
		{
			/// atomic compare and swap.
			/// with full memory barrier.
			/// does not fail spuriously.
			template<typename Value>
			bool inline compare_and_swap(Value * const address, Value const old_value, Value const new_value)
			{
				#if defined DIVERSALIS__COMPILER__GNU // since 4.1
					return __sync_bool_compare_and_swap(address, old_value, new_value);
				#else
					#error not yet implemented (could be emulated with locking primitives)
				#endif
			}
		}
	}
}
#include <universalis/compiler/dynamic_link/end.hpp>

#if 0
// http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2006/n2047.html


// The expectation is that this will usually be implemented in terms of
// low_level_atomics.h by checking whether the template argument has
// sufficient size and alignment constraints that it can be safely
// cast to a primitive integer type, and one of the low level primitives
// can be applied.

enum ordering_constraint {raw, acquire, release, ordered};
	// Informally:
	// raw ==> This operation is unordered, and may become
	//          visible to other threads in an order that is
	//          constrained only by ordering constraints on other
	//          operations.
	// release ==> All prior memory operations (including ordinary
	//          assignments) become visible to a an acquire
	//          operation on the same object that sees the resulting
	//          value.
	// acquire ==> See above.
	// ordered ==> Both acquire and release ordering properties.

// This version gives direct access to the hardware primitives, and fails
// if they don't exist.  As a result, it should be OK for inter-process
// and signal-handler communication, though that's beyond the standard.
template <class T>
class native_atomic {
	public:
	static bool basics_supported();
		// Are load/store primitives supported?
	native_atomic(T);
		// No ordering semantics, constructor itself not atomic.
	
	// The following may fail if basics_supported() returns false.
	// A store_release, or an atomic update with a release
	// argument, "synchronizes with" a load_acquire or an atomic
	// update with an acquire argument.  There are no other
	// such relationships.
	template <ordering_constraint c>
		void store(const T&);
		// Compile-time error if c is acquire.
	template <ordering_constraint c>
		T load();
		// Compile-time error if c is neither none nor acquire.
	
	static bool cas_supported();
		// Is compare_and_swap supported?
		// If so, the various fetch_and_... primitives
		// are also presumed to be supported for numeric
		// types T, since they
		// can be emulated with a CAS loop.
		// Cas is not guaranteed to be
		// wait-free, though it should be if the hardware
		// provides them.
	// There was a suggestion that the above be static, for this
	// version only.  I'm not sure that's right, given that portable
	// code should probably be prepared to adjust dynamically.

	// Compare-and-swap.  Does not fail spuriously.  Not wait-free
	// on ll-sc machines.
	template <ordering_constraint c>
		bool cas(const T& old, const T& new_val);
	static bool cas_is_wait_free();

	// Compare-and-swap.  May fail spuriously.  Wait-free
	// on ll-sc machines.
	template <ordering_constraint c>
		bool weak_cas(const T& old, const T& new_val);
	static bool weak_cas_is_wait_free();

	// I'm inclined to restrict double-width operations to
	// the low level interface, if we provide them at all.
	// They're very difficult to use,
	// due to architectural variation, and would mess up this
	// interface.
	
	T operator T() { return load<acquire>(); }
	void operator=(const T& x) { return store<release>(x); }
};

// The following provides the same interface, but the primitives
// always have functional implementations, possibly because they are
// emulated with locks.
// The implementation should avoid emulation whenever the hardware
// provides suitable primitives.
// We expect that the canonical implementation will provide a static
// hash table of locks, and map each address to a location in the hash
// table.
// Since the implementation may be lock-based, this version is NOT useful
// for either signal-handler or inter-process communication.  (Again,
// this is beyond the scope of the standard, but non-normative text
// should make that clear to avoid accidents.)  It is only useful
// for inter-thread communication, which would like to avoid the overhead of
// a full lock in most cases, but needs to run everywhere to some
// extent.  Based on limited experience, we nonetheless believe
// that this is a common case.

template <class T>
class atomic {
	public:
	static bool basics_supported();
		// Always yields true.
	atomic(T);
		// No ordering semantics, constructor not atomic.
	template <ordering_constraint c>
		void store(const T&);
		// Compile-time error if c is neither none nor release.
	template <ordering_constraint c>
		T load();
		// Compile-time error if c is neither none nor acquire.
	
	static bool cas_supported();
		// Always yields true.
		// Lock free if native_atomic::cas_supported()
		// yields true.

	// Compare-and-swap.  Does not fail spuriously.  Not wait-free
	// on ll-sc machines.
	template <ordering_constraint c>
		bool cas(const T& old, const T& new_val);
	static bool cas_is_wait_free();

	// Compare-and-swap.  May fail spuriously.  Wait-free
	// on ll-sc machines.
	template <ordering_constraint c>
		bool weak_cas(const T& old, const T& new_val);
	static bool weak_cas_is_wait_free();

	T operator T() { return load<acquire>(); }
	void operator=(const T& x) { return store<release>(x); }


// Atomic integral data type with fetch_and_... operations.
// Meaningful unly if the argument T is an integral type.  
template <class T=int>
class native_atomic_int : public native_atomic<T> {
	public:
	native_atomic_int(T);
		// No ordering semantics, not atomic.
	// The fetch_op functions may fail if cas_supported() yields false.
	// These are also not guaranteed to be wait-free, and hence
	// can be trivially emulated with cas.  We provide them
	// directly for convenience, and since they may have slightly
	// faster implementations.
	// They return the original value of the atomic.
	template <ordering_constraint c>
		T fetch_add(T);
	static bool fetch_add_is_wait_free();
		// Is the operation wait-free?
	template <ordering_constraint c>
		T fetch_and(T);
	static bool fetch_and_is_wait_free();
	template <ordering_constraint c>
		T fetch_or(T);
	static bool fetch_or_is_wait_free();
};

template <class T=int>
class atomic_int : public atomic<T> {
	public:
	atomic_int(T);
		// No ordering semantics, not atomic.
	// The fetch_op functions always succeed, but are guaranteed
	// to be lock-free only id native_atomic::cas_supported yields
	// true.
	template <ordering_constraint c>
		T fetch_add(T);
	static bool fetch_add_is_wait_free();
		// Is the operation wait-free?
	template <ordering_constraint c>
		T fetch_and(T);
	static bool fetch_and_is_wait_free();
	template <ordering_constraint c>
		T fetch_or(T);
	static bool fetch_or_is_wait_free();
};

// Adds fetch_and_... operations to a pointer type.
// Meaningful unly if T is a pointer type.
template <class T>
class native_atomic_ptr : public native_atomic<T> {
	public:
	native_atomic_ptr(T);
		// No ordering semantics, not atomic.
	// The following may fail if native_atomic<T>::cas_supported()
	// yields false.
	// This atomic pointer addition or
	// subtraction, i.e. a multiple of the object size is added
	// to the address.
	// Also by analogy to basic pointer types, we do not directly
	// provide for tags.
	template <ordering_constraint c> T fetch_and_add(ptr_diff_t);
	static bool fetch_add_is_wait_free();
};

// Adds fetch_and_... operations to a pointer type.
// Meaningful unly if T is a pointer type.
template <class T>
class atomic_ptr : public atomic<T> {
	public:
	atomic_ptr(T);
		// No ordering semantics, not atomic.
	// Always succeeds, since it may be emulated with locks.
	template <ordering_constraint c> T fetch_and_add(ptr_diff_t);
	static bool fetch_add_is_wait_free();
};

#endif
