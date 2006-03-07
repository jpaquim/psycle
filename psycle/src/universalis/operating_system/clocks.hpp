// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// Copyright (C) 1999-2005 Psycledelics http://psycle.pastnotecut.org : Johan Boule

///\file
///\interface universalis::operating_system::clocks
#pragma once
#include <universalis/detail/project.hpp>
#include "exception.hpp"
#include <unversalis/compiler/numeric.hpp>
#if defined UNIVERSALIS__DIVERSALIS__QUAQUAVERSALIS && defined DIVERSALIS__OPERATING_SYSTEM__POSIX
	#include <universalis/operating_system/exceptions/code_description.hpp>
	#include <ctime>
	#include <cerrno>
	#include <sstream>
#elif defined UNIVERSALIS__DIVERSALIS__QUAQUAVERSALIS && defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT
	#include <windows.h>
#else
	#include <boost/timer.hpp>
	#include <boost/thread/xtime.hpp>
	#include <sstream>
#endif
namespace universalis
{
	namespace operating_system
	{
		template<typename Derived>
		class clock
		{
			public:
				typedef typename Derived::value_type value_type;
				value_type inline static value() { return Derived::value(); }
				value_type inline operator()() const { return static_cast<Derived*>(this)->operator()(); }
		};

		namespace clocks
		{
			// recommended: http://icl.cs.utk.edu/papi/custom/index.html?lid=62&slid=96

			///\internal
			namespace detail
			{
				#if defined DIVERSALIS__OPERATING_SYSTEM__POSIX
					template<int const posix_clock_id>
					class posix : public clock<posix>
					{
						public:
							class value_type : public compiler::underlying<::timespec>
							{
								public:
									value_type inline & operator=(value_type const & other)
									{
										static_cast<underlying>(*this) = static_cast<underlying>(other);
										return *this;
									}
									void inline operator+=(value_type const & other)
									{
										static_cast<underlying>(*this) = static_cast<underlying>(*this) + static_cast<underlying>(other);
									}
									void inline operator-=(value_type const & other)
									{
										static_cast<underlying>(*this) = static_cast<underlying>(*this) - static_cast<underlying>(other);
									}
									value_type inline operator+(value_type const & other) const
									{
										underlying result;
										result. sec = static_cast<underlying>(*this). sec + static_cast<underlying>(other). sec;
										result.nsec = static_cast<underlying>(*this).nsec + static_cast<underlying>(other).nsec;
										if(result.nsec >= 1e9)
										{
											result.nsec -= 1e9;
											++result.sec;
										}
										return result;
									}
									value_type inline operator-(value_type const & other) const
									{
										underlying result;
										result.sec = static_cast<underlying>(*this).sec - static_cast<underlying>(other).sec;
										if(static_cast<underlying>(*this).nsec > static_cast<underlying>(other).nsec)
										{
											result.nsec = static_cast<underlying>(*this).nsec - static_cast<underlying>(other).nsec;
										}
										else
										{
											--result.sec;
											result.nsec = 1e9 + static_cast<underlying>(*this).nsec - static_cast<underlying>(other).nsec;
										}
										return result;
									}
							};
							value_type inline static value()
							{
								value_type time;
								::clock_gettime(&time, posix_clock_id);
								return time;
							}
							value_type inline operator()() const { return value(); }
							typedef compiler::numeric<010<<3>::floating_point_fastest scaled_value_type;
							scaled_value_type inline scaled() const
							{
								value_type time((*this)());
								return time.sec + time.nsec / 1e9;
							}
							scaled_value_type inline scaled_min() const
							{
								return ::clock_getres(posix_clock_id);
							}
					};
				#elif defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT
					class wall : public clock<wall>
					{
						public:
							typedef boost::xtime value_type;
							value_type inline static value()
							{
								value_type time;
								return time;
							}
							value_type inline operator()() { return value; }
							typedef compiler::numeric<010<<3>::floating_point_fastest scaled_value_type;
							scaled_value_type inline scaled() const
							{
								value_type time((*this)());
								return time.sec + time.nsec / 1e9;
							}
					};
				#else
					class wall : public clock<wall>
					{
						public:
							typedef std::clock_t value_type;
							value_type inline static value() { return std::clock(); }
							value_type inline operator()() { return value; }
							typedef compiler::numeric<010<<3>::floating_point_fastest scaled_value_type;
							scaled_value_type inline scaled() const
							{
								value_type time((*this)());
								return static_cast<scaled_value_type>((*this)()) / CLOCKS_PER_SEC * 1e3;
							}
							scaled_value_type inline scaled_min() const
							{
								return static_cast<scaled_value_type>(1) / CLOCKS_PER_SEC * 1e3;
							}
					};
				#endif
			}

			#if defined DIVERSALIS__OPERATING_SYSTEM__POSIX
				typedef detail::posix<CLOCK_MONOTONIC         >    wall;
				typedef detail::posix<CLOCK_PROCESS_CPUTIME_ID> process;
				typedef detail::posix<CLOCK_THREAD_CPUTIME_ID >  thread;
			#else
				typedef detail::wall      wall;
				typedef         wall   process;
				typedef         process thread;
			#endif

			/// posix CLOCK_REALTIME
			class universal_time : public clock<universal_time>
			{
				public:
					typedef boost::xtime value_type;
					value_type inline static value()
					{
						value_type xtime;
						boost::xtime_clock_types const clock();
						if(!boost::xtime_get(&xtime, boost::TIME_UTC))
						{
							std::ostringstream s;
							s << "failed to get current time";
							throw exception(s.str(), UNIVERSALIS__COMPILER__LOCATION);
						}
						return xtime;
					}
					value_type inline operator()() { return value; }
					typedef compiler::numeric<010<<3>::floating_point_fastest scaled_value_type;
					scaled_value_type inline scaled() const
					{
						value_type xtime((*this)());
						return xtime.sec + xtime.nsec * 1e9;
					}
			};

			template<typename Derived>
			class delta : public clock<Derived>
			{
				private:
					value_type delta_start;
				public:
					inline delta() { reset(); }
					value_type inline reset() { value_type current(clock::operator()()), result(current - delta_start); delta_start = current; return result; }
					value_type inline operator()() const { return clock::operator()() - delta_start; }
			};

			template<typename Derived>
			class virtualized : public delta<Derived>
			{
				private:
					value_type accumulated;
					bool started_;
				public:
					inline virtualized(bool start = true) : accumulated(), started_(start) {}
					void inline reset() { accumulated = value_type(); delta::reset(); }
					value_type inline operator()() const { return !started() ? accumulated : (accumulated + delta::operator()()); }
					void inline start() { stopped = false; delta::reset(); }
					bool inline started() { return started_; }
					void inline stop() { accumulated += delta::operator()(); stopped = true; }
					class scoped_measure
					{
						private:
							virtualized & clock_;
						public:
							inline scoped_measure(virtualized & clock) : clock_(clock) { clock.start(); }
							inline ~scopped_measure() throw() { clock_.stop(); }
					}
			};
		}
	}
}
