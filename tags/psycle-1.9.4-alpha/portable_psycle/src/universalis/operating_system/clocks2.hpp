///\file
///\brief time/tick clock counters
#pragma once

#if !defined Mu_Universalis && defined Mu_OperatingSystemMicrosoft
	#include <windows.h>
	// deprecated
	#include <mmsystem.h>
	#if defined Mu_CompilerFeatureAutoLink
		#pragma comment(lib,"kernel32") // win64?
		// deprecated
		#pragma comment(lib,"winmm")
	#endif
#else
	#include <unversalis/operating_system/clocks.hpp>
#endif

namespace Mu
{
	// recommended: http://icl.cs.utk.edu/papi/custom/index.html?lid=62&slid=96

	/// in seconds
	typedef Float TimeStamp;

	/// not used.
	class OpaqueTime
	{
	private:
		typedef
			#if defined Mu_Universalis || !defined Mu_OperatingSystemMicrosoft
				universalis::operating_system::time
			#else
				::ULONGLONG
			#endif
			Underlying;
		Underlying underlying;
		static Float Scaled(const Underlying& underlying)
		{
			return
				underlying
				#if !defined Mu_Universalis && defined Mu_OperatingSystemMicrosoft
					* 1e7
				#endif
				;
		}
		static Underlying UnScaled(const Float& si)
		{
			return
				#if defined Mu_Universalis || !defined Mu_OperatingSystemMicrosoft
					Underlying(si);
				#else
					si * 1e-7;
				#endif
		}
	public:
		OpaqueTime operator+ (const OpaqueTime& other) const { return this->underlying + other.underlying; }
		OpaqueTime operator- (const OpaqueTime& other) const { return this->underlying - other.underlying; }
		void operator+=(const OpaqueTime& other) { this->underlying += other.underlying; }
		void operator-=(const OpaqueTime& other) { this->underlying -= other.underlying; }

		OpaqueTime(Underlying underlying) : underlying(underlying) {}
		operator Underlying& () throw() { return underlying; }
		operator const Underlying& () const throw() { return underlying; }

		OpaqueTime(const Float& seconds) : underlying(UnScaled(underlying)) {}
		///\return the time value in SI units (i.e. seconds).
		Float /* in SI units */ operator()() const { return Scaled(underlying); }
	};

	#if 0
	///\internal
	namespace work_in_progress
	{
		template<typename Derived, typename Count>
		class clock
		{
			public:
				typedef Count count_type;
				count_type inline current() { return current_impl(); }
			protected:
				typedef clock clock_access;
				/// to be implemented in Derived
				count_type inline current_impl() { return Derived::current(); }
		};

		template<typename clock> typename clock::count_type static inline static_clock() { clock static instance; return instance.current(); }

		template<typename clock>
		class static_clock
		{
			public:
				typename clock::count_type static inline current() { clock static instance; return instance.current(); }
		};

		class wall_time_clock
		:
			public clock<wall_time_clock, time>
		{
			public:
				count static inline current() { boost::xtime xtime; boost::xtime_get(&xtime); return time::nanoseconds(xtime.sec * 1e9 + xtime.nsec); }
			protected:
				friend class clock_access::count clock_access::current();
				/// implements clock<>
				count inline current_impl() { return current(); }
		};

		typedef static_clock<wall_time_clock> static_wall_time_clock;

		static_wall_time_clock::count inline now() { return static_wall_time_clock::current(); }

		template<typename Clock>
		class clock_difference
		:
			public Clock
		{
			public:
				typedef Clock clock_type;
				clock_type inline & clock() throw() { return clock_; }
			private:
				clock_type clock_;

			public:
				typedef typename clock::count_type count_type;
			private:
				count_type initial;

			public:
				inline clock_difference() { reset(); }
				void inline reset() { initial = clock().current(); }
				count_type inline current() { return clock::current() - initial; }
				count_type inline current_and_reset() { count_type const current(clock::current()), difference(current - initial); initial = current; return difference; }
		};

		typedef clock_difference<wall_time_clock> wall_time_clock_difference;
	}
	#endif

	///\internal
	namespace Detail
	{
		template<typename Derived, typename DerivedCount>
		class Clock
		{
		protected:
			typedef Derived Derived;

			//typedef typename Derived::Count Count;
			typedef DerivedCount Count;

			typedef Clock ClockAccess;
			/// to be implemented in Derived.
			///\return the count value of the current clock tick.
			static Count GetCurrentTickCount() { return Derived::GetCurrentTickCount(); }
		public:
			///\return the count value of the current clock tick.
			static Count GetCurrent() { return GetCurrentTickCount(); }
		};

		template<typename Clock>
		class ScalableClock
		:
			public Clock
		{
		protected:
			typedef typename Clock::Count Count;
			typedef typename Clock::ClockAccess ClockAccess;

			typedef ScalableClock ScalableClockAccess;
			/// to be implemented in Derived.
			///\return the value in SI units (i.e. seconds for time clocks) corresponding to the given count value.
			static Float /* in SI Units */ Scaled(const Count& count) { return Derived::Scaled(count); }
		};

		template<typename Clock>
		class ScaledClock
		:
			public ScalableClock<Clock>
		{
		public:
			///\return the value in SI units (i.e. seconds for time clocks) corresponding to the count value of the current tick of this clock.
			static Float GetCurrent() { return Scaled(Clock::GetCurrent()); }
		};

		template<typename Clock>
		class ClockDifference
		:
			public Clock
		{
		protected:
			typedef typename Clock::Count Count;
			typedef typename Clock::ClockAccess ClockAccess;
		private:
			Count initial;
		public:
			ClockDifference() { Reset(); }

			/// resets the counter.
			void Reset() { initial = Clock::GetCurrent(); }

			///\return the count difference since the instance was created, or since the previous call to Reset().
			Count GetCurrent() const { return Clock::GetCurrent() - initial; }

			// also resets the counter.
			/// For maximum precision, you should use this function rather than calling consecutively GetDifference() and Reset().
			///\return the count difference since the instance was created, or since the previous call to Reset().
			Count GetCurrentAndReset()
			{
				const Count
					current(Clock::GetCurrent()),
					difference(current - initial);
				initial = current;
				return difference;
			}
		};

		template<typename ClockDifference>
		class ScaledClockDifference
		:
			public ScalableClock<ClockDifference>
		{
		public:
			///\return the value in SI units (i.e. seconds for time clocks) corresponding to the count value of the current tick of this clock.
			Float GetCurrent() { return Scaled(ClockDifference::GetCurrent()); }

			// also resets the counter.
			/// For maximum precision, you should use this function rather than calling consecutively GetCurrent() and Reset().
			///\return the value in SI units (i.e. seconds for time clocks) corresponding to the count value of the current tick of this clock.
			Float GetCurrentAndReset() { return Scaled(ClockDifference::GetCurrentAndReset()); }
		};

		/// provides a consistent way of nesting the template instanciations of ScaledClock<> and ClockDifference<>.
		/// It is important to nest ScaledClock < ClockDifference<...> > in this order for precise computation of time differences.
		template<typename Derived, typename DerivedCount>
		class TimeClock
		:
			public ScaledClockDifference< ClockDifference < Clock<Derived,DerivedCount> > >
		{
		public:
			/// provides access to the scaled version of the clock, without support for computing differences, which need to create an instance of the class.
			typedef ScaledClock<ClockAccess> ScaledClock;
			//typedef ScaledClock< Clock<Derived,DerivedCount> > ScaledClock;
			
		};
	} // namespace Detail

	/// counts the time elapsed since some unspecified origin.
	///
	/// The implementation reads, if available, the tick count register of some unspecified CPU.
	/// On most CPU architectures, the register is updated at a rate based on the frequency of the cycles, but often the count value and the tick events are unrelated,
	/// i.e. the value might not be incremented one by one. So the period corresponding to 1 count unit may be even smaller than the period of a CPU cycle, but should probably stay in the same order of magnitude.
	/// If the counter is increased by 4,000,000,000 over a second, and is 64-bit long, it is possible to count an uptime period in the order of a century without wrapping.
	/// The implementation for x86, doesn't work well at all on some of the CPUs whose frequency varies over time. This will eventually be fixed http://www.x86-secret.com/?option=newsd&nid=845.
	/// The implementation for mswindows is unpsecified on SMP systems.
	class WallTimeClock
	:
		public Detail::TimeClock
			<
				WallTimeClock,
				#if defined Mu_Universalis || !defined Mu_OperatingSystemMicrosoft
					universalis::operating_system::clocks::wall::value_type
				#else
					::LONGLONG
				#endif
			>
	{
	protected: 
		friend static ClockAccess::Count ClockAccess::GetCurrentTickCount();
		/// implements Clock<>
		static Count GetCurrentTickCount()
		{
			#if defined Mu_Universalis || !defined Mu_OperatingSystemMicrosoft
				return universalis::operating_system::clocks::wall::current();
			#else
				::LARGE_INTEGER result;
				if(::QueryPerformanceCounter(&result))
				{
					return result.QuadPart;
				}
				else // The CPU has no tick count register.
				{
					/// Use the PIT/PIC PC hardware via mswindows' ::timeGetTime() or ::GetTickCount() instead.
					class SetTimerResolution
					{
					private:
						::UINT milliseconds;
					public:
						SetTimerResolution()
						{
							// tries to get the best possible resolution, starting with 1ms
							milliseconds = 1;
							retry:
							if(::timeBeginPeriod(milliseconds) == TIMERR_NOCANDO)
							{
								if(++milliseconds < 100) goto retry;
								else milliseconds = 0; // give up :-(
							}
						}
						~SetTimerResolution() throw()
						{
							if(!milliseconds) return; // wasn't set
							if(::timeEndPeriod(milliseconds) == TIMERR_NOCANDO)
								return; // cannot throw in a destructor
								//throw std::runtime_error(GetLastErrorString());
						}
					};
					static SetTimerResolution setTimerResolution;
					return
						::timeGetTime(); // equivalent to ::GetTickCount() but Microsoft very loosely tries to express the idea that it might be more precise, especially if calling ::timeBeginPeriod and ::timeEndPeriod.

						// uptime (i.e., time elapsed since computer was booted), in milliseconds. Microsoft doesn't even specifies wether it's monotonic and as linear as possible, but we can probably assume so.
						// This function returns a value which is read from a context value which is updated only on context switches, and hence is very inaccurate: it can lag behind the real clock value as much as 15ms.
						//::GetTickCount();
				}
			#endif
		}

		friend static Float ScalableClockAccess::Scaled(const ScalableClockAccess::Count& count);
		/// implements ScalableClock<>
		static Float Scaled(const Count& count)
		{
			#if defined Mu_Universalis || !defined Mu_OperatingSystemMicrosoft
				return count;
			#else
				class Scaled
				{
				private:
					Float period;
				public:
					Scaled()
					{
						::LARGE_INTEGER frequency;
						if(::QueryPerformanceFrequency(&frequency))
						{
							period = 1. / frequency.QuadPart;
						}
						else // The CPU has no tick count register.
						{
							/// Use the PIT/PIC PC hardware via mswindows' ::timeGetTime() or ::GetTickCount() instead.
							period = 1e-3;
						}
					}

					inline Float operator()(const Count& count) const
					{
						return count * period;
					}
				};
				static Scaled Scaled;
				return Scaled(count);
			#endif
		}

		#if !defined Mu_Universalis && defined Mu_OperatingSystemMicrosoft
			/// The implementation of mswindows' ::GetThreadTimes() is completly broken: http://blog.kalmbachnet.de/?postid=28
			/// Use the wall clock instead, which majorates all virtual subclocks.
			friend class ThreadTimeClock;
		#endif
	};

	/// counts the time spent by the CPU in the current process, kernel included.
	class ProcessTimeClock
	:
		public Detail::TimeClock
			<
				ProcessTimeClock,
				#if defined Mu_Universalis || !defined Mu_OperatingSystemMicrosoft
					universalis::operating_system::clocks::process::value_type
				#else
					::ULONGLONG
				#endif
			>
	{
	protected:
		friend static ClockAccess::Count ClockAccess::GetCurrentTickCount();
		/// implements Clock<>
		static Count GetCurrentTickCount()
		{
			#if defined Mu_Universalis || !defined Mu_OperatingSystemMicrosoft
				return universalis::operating_system::clocks::process::current();
			#else
				::FILETIME creation, exit, kernel, user;
				::GetProcessTimes(::GetCurrentProcess(), &creation, &exit, &kernel, &user);
				return reinterpret_cast<Count&>(user) + reinterpret_cast<Count&>(kernel);
			#endif
		}

		friend static Float ScalableClockAccess::Scaled(const ScalableClockAccess::Count& count);
		/// implements ScalableClock<>
		static Float Scaled(const Count& count)
		{
			return
				count
				#if !defined Mu_Universalis && defined Mu_OperatingSystemMicrosoft
					* 1e-7
				#endif
				;
		}
	};

	/// counts the time spent by the CPU in the current thread, kernel included.
	class ThreadTimeClock
	:
		public Detail::TimeClock
			<
				ThreadTimeClock,
				#if defined Mu_Universalis || !defined Mu_OperatingSystemMicrosoft
					universalis::operating_system::clocks::thread::value_type
				#elif 0 /// The implementation of mswindows' ::GetThreadTimes() is completly broken: http://blog.kalmbachnet.de/?postid=28
					::ULONGLONG
				#else /// Use the wall clock instead, which majorates all virtual subclocks.
					WallTimeClock::Count
				#endif
			>
	{
	protected:
		friend static ClockAccess::Count ClockAccess::GetCurrentTickCount();
		/// implements Clock<>
		static Count GetCurrentTickCount()
		{
			#if defined Mu_Universalis || !defined Mu_OperatingSystemMicrosoft
				return universalis::operating_system::clocks::thread::current();
			#elif 0 /// The implementation of mswindows' ::GetThreadTimes() is completly broken: http://blog.kalmbachnet.de/?postid=28
				::FILETIME creation, exit, kernel, user;
				::GetThreadTimes(&creation, &exit, &kernel, &user);
				return reinterpret_cast<Count&>(user) + reinterpret_cast<Count&>(kernel);
			#else /// Use the wall clock instead, which majorates all virtual subclocks.
				WallTimeClock::GetCurrentTickCount();
			#endif
		}

		friend static Float ScalableClockAccess::Scaled(const ScalableClockAccess::Count& count);
		/// implements ScalableClock<>
		static Float Scaled(const Count& count)
		{
			return
				#if defined Mu_Universalis || !defined Mu_OperatingSystemMicrosoft
					count;
				#elif 0 /// The implementation of mswindows' ::GetThreadTimes() is completly broken: http://blog.kalmbachnet.de/?postid=28
					count * 1e-7;
				#else /// Use the wall clock instead, which majorates all virtual subclocks.
					WallTimeClock::Scaled(count);
				#endif
		}
	};
}
