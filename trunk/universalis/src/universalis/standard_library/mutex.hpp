#pragma once
#include <universalis/operating_system/threads/mutex.hpp>
namespace std {
	typedef universalis::operating_system::threads::mutex mutex;
	typedef universalis::operating_system::threads::recursive_mutex recursive_mutex;
}
