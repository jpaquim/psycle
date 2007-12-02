#pragma once
#include <universalis/operating_system/threads/condition.hpp>
namespace std {
	template <class Lock> class condition;
	typedef condition<mutex>              condition_mtx;
	typedef condition<unique_lock<mutex>> condition_ulm;
}
