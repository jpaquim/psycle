///\file
/// thread local storage
/// variable stored in a per thread local storage.
#pragma once
#include INCLUDE(PROJECT/project.hpp)
#include INCLUDE(PROJECT/compilers.hpp)
#include "pragma.hpp"
#if defined COMPILER__GNU
	#define thread_local_storage pragma(thread)
#elif defined COMPILER__MICROSOFT
	#define thread_local_storage __thread
#else
	#define thread_local_storage
#endif
