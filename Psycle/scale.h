#ifndef PSYCLE__SCALE
#define PSYCLE__SCALE
#pragma once
#include <limits>
#include <cmath>
#include <vector>
/// \file
/// \brief scale facilities for machine parameters
namespace psycle
{
	typedef double Real;
	typedef float Sample;

	namespace math
	{
		const Real pi(3.14159265358979323846);
		namespace sample { const Sample pi(static_cast<Sample>(pi)); }

		template<typename Real> inline const Real remainder(const Real & numerator, const Real & denominator)
		{
			Real result(numerator);
			if(result >= denominator) result -= denominator;
			if(result >= denominator) result = ::fmod(result, denominator);
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
			if(x > 0) return static_cast<int>(x + 0.5);
			else return static_cast<int>(x - 0.5);
		}

		inline const int truncated(const Real & x)
		{
			const Real result(x - 0.5 + 6755399441055744); // 2^51 + 2^52
			return *reinterpret_cast<const int *>(&result);
		};

		inline const int truncated(const Sample & x) 
		{ 
			const double half(0.5); 
			int i; 
			__asm 
			{ 
				fld x
				fsub half
				fistp i
			}
			return i;
		}

		inline const Sample log(const Sample & x) 
		{ 
			const int i(*reinterpret_cast<const int *>(&x));
			return
				(( i & 0x7f800000) >> 23)
				+ (i & 0x007fffff) / Sample(0x800000)
				-0x7f;
		} 

		template<typename Real> inline const bool denormal(const Real & x)
		{
			//return ::fabs(x) < std::numeric_limits<Real>::denorm_min();
			return Real(::fabs(x)) < std::numeric_limits<Real>::epsilon();
		}

		inline const bool denormal(const Sample & x)
		{
			#if PSYCLE__DEBUG_LEVEL > 1
			assert(sizeof x == sizeof(int)); 
			assert(sizeof x == 4);
			#endif
			return !(*reinterpret_cast<const unsigned int *>(&x) & 0x7f800000);
		}

		template<typename Real> inline const Real renormalized(const Real & x)
		{
			return denormal(x) ? 0 : x;
			
		}

		template<typename Real> inline const Real renormalize(Real & x)
		{
			x = renormalized(x);
		}

		inline void renormalize(Sample samples [], const int & sample_count)
		{
			float normal_dither(Sample(1e-20));
			for(int sample = 0 ; sample < sample_count ; ++sample)
			{
				samples[sample] += normal_dither;
				normal_dither = -normal_dither;
			}
		}
	}

	class Scale
	{
	public:
		Scale(const Real & input_maximum, const Real & output_minimum, const Real & output_maximum)
		: input_maximum_(input_maximum), output_minimum_(output_minimum), output_maximum_(output_maximum)
		{}
		virtual const Real apply(const Real & sample) const = 0;
		virtual const Real apply_inverse(const Real & sample) const = 0;
		inline const Real & input_maximum() const throw() { return input_maximum_; }
		inline const Real & output_minimum() const throw() { return output_minimum_; }
		inline const Real & output_maximum() const throw() { return output_maximum_; }
	private:
		const Real input_maximum_, output_minimum_, output_maximum_;
	};

	namespace scale
	{
		class Discrete : public Scale
		{
		public:
			Discrete(const Real & input_maximum)
			: Scale(input_maximum, 0, input_maximum)
			{}
			inline virtual const Real apply(const Real & sample) const
			{
				return sample;
			}
			inline virtual const Real apply_inverse(const Real & sample) const
			{
				return sample;
			}
		};

		class Linear : public Scale
		{
		public:
			Linear(const Real & input_maximum, const Real & output_minimum, const Real & output_maximum)
			: Scale(input_maximum, output_minimum, output_maximum)
			{
				ratio_ = (output_maximum - output_minimum) / input_maximum;
				offset_ = output_minimum / ratio_;
			}
			inline virtual const Real apply(const Real & sample) const
			{
				return (sample + offset_) * ratio_;
			}
			inline virtual const Real apply_inverse(const Real & sample) const
			{
				return sample / ratio_ - offset_;
			}
		private:
			Real offset_, ratio_;
		};

		class Exponential : public Scale
		{
		public:
			Exponential(const Real & input_maximum, const Real & output_minimum, const Real & output_maximum)
			: Scale(input_maximum, ::log(output_minimum), ::log(output_maximum))
			{
				ratio_ = (this->output_maximum() - this->output_minimum()) / input_maximum;
				offset_ = this->output_minimum() / ratio_;
			};
			inline virtual const Real apply(const Real & sample) const
			{
				return ::exp((sample + offset_) * ratio_);
			}
			inline virtual const Real apply_inverse(const Real & sample) const
			{
				return ::log(sample) / ratio_ - offset_;
			}
		private:
			Real offset_, ratio_;
		};

		class Logarithmic : public Scale
		{
		public:
			Logarithmic(const Real & input_maximum, const Real & output_minimum, const Real & output_maximum)
			: Scale(input_maximum, ::log(output_minimum), ::log(output_maximum))
			{
				ratio_ = (this->output_maximum() - this->output_minimum()) / input_maximum;
				offset_ = this->output_minimum() / ratio_;
				offset_plus_input_maximum_ = offset_ + input_maximum;
				output_minimum_maximum_sum_ = output_minimum + output_maximum;
			};
			inline virtual const Real apply(const Real & sample) const
			{
				return output_minimum_maximum_sum_ - ::exp((offset_plus_input_maximum_ - sample) * ratio_);
			}
			inline virtual const Real apply_inverse(const Real & sample) const
			{
				return offset_plus_input_maximum_ - ::log(output_minimum_maximum_sum_ - sample) / ratio_;
			}
		private:
			Real offset_, ratio_, output_minimum_maximum_sum_, offset_plus_input_maximum_;
		};
	}
}
#endif