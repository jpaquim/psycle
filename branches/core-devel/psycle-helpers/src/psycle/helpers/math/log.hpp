#pragma once
#include <diversalis/processor.hpp>
#include <universalis/compiler.hpp>
#if defined DIVERSALIS__PROCESSOR__X86 // we should verify the code for other architectures.
	#include <boost/static_assert.hpp>
	#include <cstdint>
#else
	#include <cmath>
#endif
#if defined BOOST_AUTO_TEST_CASE
	#include <universalis/operating_system/clocks.hpp>
	#include <cmath>
	#include <sstream>
	#include <iomanip>
#endif
namespace psycle
{
	namespace helpers
	{
		namespace math
		{
			/// an approximate but fast computation of the base-2 logarithm.
			/// the approximation error is:
			/// less than 10% for input values between 0 and 0.6,
			/// maximum 60% for input values between 0.6 and 1.7,
			/// less than 10% for input values above 1.7.
			float inline UNIVERSALIS__COMPILER__CONST fast_log2(float f)
			{
				#if defined DIVERSALIS__PROCESSOR__X86 // we should verify the code for other architectures.
					BOOST_STATIC_ASSERT((sizeof f == 4));
					//assert(f > 0); 
					union result_union {
						float f;
						std::uint32_t i;
					} result;
					result.f = f;
					#if 0 // slower
						/*
							speed tests:
							,----------------------------------------------------------------------------------------------.
							| host name | cpu   | compiler        | compiler target          | fast_log2 time | std time   |
							|-----------|-------|-----------------|--------------------------|----------------|------------|
							| anechoid  | amd64 | mingw 3.4.4     | x87 without amd64 tuning | 0.0265056s     | 0.0724436s |
							| anechoid  | amd64 | msvc 1400 (8.0) | sse2                     | 0.0664263s     | 0.1051030s |
							| factoid   | uml   | gcc 4.1.2       | x87                      | 0.0644470s     | 0.0999490s |
							`----------------------------------------------------------------------------------------------'
						*/
						return
							(  (result.i & 0x7f800000) >> 23 )
							+  (result.i & 0x007fffff)
							* 1.19209289550781e-07f // * 1 / float(0x00800000)
							-              0x0000007f;
					#else // faster
						/*
							speed tests:
							,----------------------------------------------------------------------------------------------.
							| host name | cpu   | compiler        | compiler target          | fast_log2 time | std time   |
							|-----------|-------|-----------------|--------------------------|----------------|------------|
							| anechoid  | amd64 | mingw 3.4.4     | x87 without amd64 tuning | 0.0156093s     | 0.0710476s |
							| anechoid  | amd64 | msvc 1400 (8.0) | sse2                     | 0.0529316s     | 0.1039730s |
							| factoid   | uml   | gcc 4.1.2       | x87                      | 0.0199000s     | 0.0966760s |
							`----------------------------------------------------------------------------------------------'
						*/
						int const log_2 = ((result.i >> 23) & 255) - 128;
						result.i &= ~(255 << 23);
						result.i += 127 << 23;
						return result.f + log_2;
					#endif
				#else
					#define PSYCLE__HELPERS__MATH__FAST_LOG2__SKIP_TEST_CASE
					#if defined DIVERSALIS__COMPILER__MICROSOFT // lacks log2
						return std::log(f) * 1.442695f; // * 1 / log(2)
					#else
						return ::log2(f);
					#endif
				#endif
			}

			#if defined BOOST_AUTO_TEST_CASE && !defined PSYCLE__HELPERS__MATH__FAST_LOG2__SKIP_TEST_CASE
				BOOST_AUTO_TEST_CASE(fast_log2_test)
				{
					float const input_values[] = {
						0.0000001f,
						0.000001f,
						0.0001f,
						0.001f,
						0.01f,
						0.1f,
						0.2f,
						0.4f,
						0.5f,
						0.6f,
						0.7f,
						0.8f,
						0.9f,
						0.999f,
						0.99999f,
						0.999999f,
						0.9999999f,
						1.0000001f,
						1.000001f,
						1.00001f,
						1.001f,
						1.01f,
						1.1f,
						1.2f,
						1.5f,
						1.6f,
						1.7f,
						1.8f,
						1.9f,
						2.f,
						3.f,
						4.f,
						5.f,
						10.f,
						16.f,
						100.f,
						128.f,
						1000.f,
						1024.f,
						10000.f,
						16384.f,
						100000.f,
						10000000.f,
						1000000000.f
					};
					for(int i(0); i < sizeof input_values / sizeof *input_values; ++i) {
						float const f(input_values[i]);
						double tolerance;
						if(0.6 < f && f < 1.7) tolerance = 0.60;
						else tolerance = 0.1;
						double ratio = fast_log2(f) /
						(
							#if defined DIVERSALIS__COMPILER__MICROSOFT // lacks log2
								std::log(f) * 1.442695f // * 1 / log(2)
							#else
								::log2(f)
							#endif
						);
						//std::ostringstream s; s << "ratio(" << f << "): " << ratio;
						//BOOST_MESSAGE(s.str());
						BOOST_CHECK(1 - tolerance < ratio && ratio < 1 + tolerance);
					}
					using namespace universalis::operating_system::clocks;
					typedef thread clock;
					int const iterations(1000000);
					opaque_time const t1(clock::current());
					float f1(2);
					for(int i(0); i < iterations; ++i) f1 += fast_log2(f1);
					opaque_time const t2(clock::current());
					float f2(2);
					for(int i(0); i < iterations; ++i) f2 += 
						#if defined DIVERSALIS__COMPILER__MICROSOFT // lacks log2
							std::log(f2) * 1.442695f // * 1 / log(2)
						#else
							::log2(f2)
						#endif
						;
					opaque_time const t3(clock::current());
					{
						std::ostringstream s; s << "fast_log2: " << f1;
						BOOST_MESSAGE(s.str());
					}
					{
						std::ostringstream s; s << "std::log2: " << f2;
						BOOST_MESSAGE(s.str());
					}
					{
						std::ostringstream s;
						s << (t2 - t1).to_real_time() << "s < " << (t3 - t2).to_real_time() << "s";
						BOOST_MESSAGE(s.str());
					}
					BOOST_CHECK(t2 - t1 < t3 - t2);
				}
			#endif
		}
	}
}
