///\interface psycle::helpers
#pragma once
#include "math/pi.hpp"
#include "math/rint.hpp"
#include "math/truncate.hpp"
#include "math/round.hpp"
#include "math/log.hpp"
#include "math/clip.hpp"
#include <string> // to declare hexstring_to_integer
#include <cstdint>
#include <limits>
#include <boost/static_assert.hpp>

namespace psycle { namespace helpers {

	/// the pi constant as a 32-bit floating point number
	float const F_PI = math::pi_f;

	/// parses an hexadecimal string to convert it to an integer
	unsigned long long int hexstring_to_integer(std::string const &);

	/// parses an hexadecimal string to convert it to an integer
	template<typename X>
	void hexstring_to_integer(std::string const & s, X & x) { x = static_cast<X>(hexstring_to_integer(s)); }

	/// linearly maps a byte (0 to 255) to a float (0 to 1).
	///\todo needs some explanation about why the tables have a length of 257.
	///\todo make that a namespace
	class CValueMapper
	{
		public:
			/// contructor.
			///\todo not needed since everything is static
			CValueMapper() {}

			/// destructor.
			///\todo not needed since everything is static
			~CValueMapper() {}

			/// maps a byte (0 to 255) to a float (0 to 1).
			static inline float Map_255_1(int byte)
			{
				///\todo the conditional branches probably make it slower than direct calculation
				if(0 <= byte && byte <= 256)
					return CValueMapper::fMap_255_1[byte];
				else
					return byte * 0.00390625f;
			}

			/// maps a byte (0 to 255) to a float (0 to 100).
			static inline float Map_255_100(int byte)
			{
				///\todo the conditional branches probably make it slower than direct calculation
				if(0 <= byte && byte <= 256)
					return CValueMapper::fMap_255_100[byte];
				else
					return byte * 0.390625f;
			}
		private:
			static float fMap_255_1[257];
			static float fMap_255_100[257];
	};

#if 1

	/******* first convert to integer, then clip **********/

	/// combines float to signed integer conversion with clipping.
	///\todo: WARNING!!! This only works with signed types, not with unsigned.
	///\todo: WARNING!!! maybe we should clip in float and convert. but that has the other
	/// problem of integral types not being equal on both sides.
	template<typename Result, unsigned int const bits> UNIVERSALIS__COMPILER__CONST
	Result inline clipped_lrint(float f) {
		// check that Result is signed
		BOOST_STATIC_ASSERT((std::numeric_limits<Result>::is_signed));

		//This is not exactly correct (one value less) but at least works.
		Result const max(((1 << (bits - 2)) - 1) << 1); // The compiler is able to compute this statically.
		Result const min(-max-1);
		return math::clipped(min, math::lrint<Result>(f), max);
	}

#elif 0

	/******* first convert to integer, then clip **********/
	
	/// combines float to signed integer conversion with clipping.
	///\todo: WARNING!!! This only works with signed types, not with unsigned.
	template<typename Result, unsigned int const bits> UNIVERSALIS__COMPILER__CONST
	Result inline clipped_lrint(float f) {
		// check that Result is signed
		BOOST_STATIC_ASSERT((std::numeric_limits<Result>::is_signed));

		// use 64-bit if needed, otherwise 32-bit
		if(sizeof(Result) >= sizeof(std::int32_t)) { // The compiler *should* be able to compute this statically.
			std::int64_t const max((1 << (bits - 1)) - 1); // The compiler is able to compute this statically.
			std::int64_t const min(-max - 1);
			return math::clipped(min, math::lrint<std::int64_t>(f), max);
		} else {
			std::int32_t const max((1 << (bits - 1)) - 1); // The compiler is able to compute this statically.
			std::int32_t const min(-max - 1);
			return math::clipped(min, math::lrint<std::int32_t>(f), max);
		}
	}

#elif 0

	/******* first clip, then convert to integer (like the previous f2iclipXX functions) **********/
	
	/// combines float to signed integer conversion with clipping.
	///\todo: WARNING!!! This only works with signed types, not with unsigned.
	template<typename Result, unsigned int const bits> UNIVERSALIS__COMPILER__CONST
	Result inline clipped_lrint(float f) {
		// check that Result is signed
		BOOST_STATIC_ASSERT((std::numeric_limits<Result>::is_signed));

		int const max((1 << (bits - 1)) - 1);
		int const min(-max - 1);
		return math::lrint<Result>(math::clipped(float(min), f, float(max)));
	}
	
#elif 0

	/******* first clip, then convert to integer **********/

	/// clipping.
	inline int  UNIVERSALIS__COMPILER__CONST
	f2iclip16(float f)
	{ 
		int const l(32767);
		if(f < -l) return -l;
		if(f > +l) return +l;
		return math::rounded(f);
	}

	/// clipping.
	inline int  UNIVERSALIS__COMPILER__CONST
	f2iclip18(float f)
	{ 
		int const l(131071);
		if(f < -l) return -l;
		if(f > +l) return +l;
		return math::rounded(f);
	}

	/// clipping.
	inline int  UNIVERSALIS__COMPILER__CONST
	f2iclip20(float f)
	{ 
		int const l(524287);
		if(f < -l) return -l;
		if(f > +l) return +l;
		return math::rounded(f);
	}

	/// clipping.
	inline int  UNIVERSALIS__COMPILER__CONST
	f2iclip24(float f)
	{ 
		int const l(8388607);
		if(f < -l) return -l;
		if(f > +l) return +l;
		return math::rounded(f);
	}

	/// clipping.
	inline int  UNIVERSALIS__COMPILER__CONST
	f2iclip32(float f)
	{ 
		int const l(2147483647);
		if(f < -l) return -l;
		if(f > +l) return +l;
		return math::rounded(f);
	}

#endif

	/// combines float to signed integer conversion with clipping.
	template<typename Result> UNIVERSALIS__COMPILER__CONST
	Result inline clipped_lrint(float f) {
		return clipped_lrint<Result, (sizeof(Result) << 3)>(f);
	}

}}
