#include <iostream>

#if defined __unix__ || defined __APPLE__
	#include <pthread.h>
	#include <cerrno>
#elif defined _WIN64 || defined _WIN32
	#include <windows.h>
#else
	#error unsupported platform
#endif

int main(int, char**) {
	#if defined __unix__ || defined __APPLE__
		cpu_set_t set;
		CPU_ZERO(&set);
		if(
			#if 0
				pthread_getaffinity_np(pthread_self()
			#else
				sched_getaffinity(0
			#endif
			, sizeof set /* warning: do not use CPU_SETSIZE here */, &set)
		) {
			int error(errno);
			std::cerr << "standard error: " << error << '\n';
			return error ? error : 1;
		}
		std::cout << "cpu set: ";
		for(int i(0); i < CPU_SETSIZE; ++i) std::cout << (CPU_ISSET(i, &set) ? 1 : 0);
		std::cout << '\n';
		return 0;
	#elif defined _WIN64 || defined _WIN32
		DWORD process, system;
		if(!GetProcessAffinityMask(GetCurrentProcess(), &process, &system)) {
			DWORD error(GetLastError());
			std::cerr << "winapi error: " << error << '\n';
			return error ? error : 1;
		}
		std::cout << "process: " << process << ", system: " << system << '\n';
		return 0;
	#endif
}
