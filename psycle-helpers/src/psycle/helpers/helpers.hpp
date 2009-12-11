///\interface psycle::helpers
#pragma once
#include "math/pi.hpp"
#include "math/rint.hpp"
#include "math/truncate.hpp"
#include "math/round.hpp"
#include "math/log.hpp"
#include "math/clip.hpp"
#include <string> // to declare hexstring_to_integer
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

	/// combines float to signed integer conversion with clipping.
	template<typename Result, unsigned int const bits> UNIVERSALIS__COMPILER__CONST
	Result inline clipped_rint(float f) {
		int const max(1 << (bits - 1)); // The compiler is able to compute this statically.
		int const min(1 - max);
		return math::clipped(min, math::rint<int>(f), max);
	}

	/// combines float to signed integer conversion with clipping.
	template<typename Result> UNIVERSALIS__COMPILER__CONST
	Result inline clipped_rint(float f) {
		return clipped_rint<Result, sizeof(Result) >> 3>(f);
	}

}}
