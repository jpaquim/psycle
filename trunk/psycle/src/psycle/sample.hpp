/// \file
/// \brief
#pragma once
#include <diversalis/processor.hpp>
#include <diversalis/compiler.hpp>
#include <limits>
#include <cmath>
#include <vector>
#include <cfloat>
#include <cstdint>
#include <boost/static_assert.hpp>
namespace psycle
{
	#define PSYCLE__MATH__REAL 8
	typedef double Real;
	typedef float Sample;

	namespace math
	{
		const Real pi(Real(3.14159265358979323846));

		namespace sample { const Sample pi(static_cast<Sample>(pi)); }

		template<typename Real> inline const Real remainder(const Real & numerator, const Real & denominator)
		{
			Real result(numerator);
			if(result >= denominator) result -= denominator;
			if(result >= denominator) result = std::fmod(result, denominator);
			return result;
		}

		template<typename Real> inline const Real clipped(const Real & minimum, const Real & sample, const Real & maximum)
		{
			if(minimum > sample) return minimum;
			else if(sample > maximum) return maximum;
			else return sample;
		}

		template<typename Real> inline const int rounded(const Real & x)
		{
			if(x > 0) return std::floor(x + Real(0.5));
			else return std::ceil(x - Real(0.5));
		}

		#if PSYCLE__MATH__REAL == 8
			/// when Real is 64-bit
			inline const int truncated(const Real & x)
			{
				BOOST_STATIC_ASSERT(sizeof x == 8);
				const Real result(x - 0.5 + 6755399441055744ULL); // 2^51 + 2^52
				return *reinterpret_cast<const int *>(&result);
			};
		#endif

		#if 0
			/// for 32-bit
			inline const Sample log(const Sample & x)
			{ 
				BOOST_STATIC_ASSERT((sizeof x == 4));
				//assert(f > 0); 
				const std::int32_t i(*reinterpret_cast<const std::int32_t *>(&x));
				return
					( (i & 0x7f800000) >> 23 )
					+ (i & 0x007fffff) / Sample(0x800000)
					- 0x7f;
			}
		#endif

		/// Cure for malicious samples
		/// Type : Filters Denormals, NaNs, Infinities
		/// References : Posted by urs[AT]u-he[DOT]com
		void inline erase_all_nans_infinities_and_denormals(Sample samples [], unsigned int const sample_count)
		{
			std::uint32_t* inArrayOfFloats(reinterpret_cast<std::uint32_t*>(samples));
			std::uint32_t sample;
			std::uint32_t exponent;
			for(unsigned int i(0); i < sample_count; ++i)
			{
				sample = *inArrayOfFloats;
				exponent = sample & 0x7F800000;

				// exponent < 0x7F800000 is 0 if NaN or Infinity, otherwise 1
				// exponent > 0 is 0 if denormalized, otherwise 1

				*inArrayOfFloats++ = sample * ((exponent < 0x7F800000) & (exponent > 0));
			}
		}

		class Sin_Sequence
		{
		public:
			/// <bohan> use 64-bit floating point numbers or else accuracy is not sufficient
			typedef double Real;
			inline Sin_Sequence() : index_(0) {}
			inline void operator()(const Real & phase, const Real & radians_per_sample) throw()
			{
				step_ = static_cast<Real>(2 * std::cos(radians_per_sample));
				sequence_[0] = static_cast<Real>(std::sin(phase - radians_per_sample));
				sequence_[1] = static_cast<Real>(std::sin(phase - 2 * radians_per_sample));
				index_ = 0;
			}
			inline const Real & operator()() throw()
			{
				const int swapped_index((index_ + 1) % 2);
				const Real sin(clipped<Real>(-1, sequence_[index_] * step_ - sequence_[swapped_index], +1));
				index_ = swapped_index;
				return sequence_[swapped_index] = sin;
			}
		private:
			Real step_, sequence_[2];
			int index_;
		};
	}
}
