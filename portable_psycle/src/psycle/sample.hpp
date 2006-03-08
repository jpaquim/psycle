/// \file
/// \brief
#pragma once
#include <limits>
#include <cmath>
#include <vector>
#include <cfloat>
namespace psycle
{
	
	#define PSYCLE__MATH__REAL 8
	typedef double Real;
	typedef float Sample;

	namespace math
	{
		const Real pi(Real(3.14159265358979323846));
		/*
		class Pi
		{
		public:
			inline operator float() const throw() { return 3.14159265358979323846f; }
			inline operator double() const throw() { return 3.14159265358979323846; }
		} pi;
		*/

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
				const Real result(x - 0.5 + 6755399441055744); // 2^51 + 2^52
				return *reinterpret_cast<const int *>(&result);
			};
		#endif

		/// for 32-bit (does it also work with 64-bit ?)
		/// beware: only works with positive numbers
		inline const int truncated(const Sample & x) 
		{ 
			const double half(0.5); 
			int i; 
			__asm // shit, this is not be portable!
			{ 
				fld x
				fsub half
				fistp i
			}
			return i;
		}

		#if 0
		/// for 32-bit
		inline const Sample log(const Sample & x) /// <bohan> i don't remember from where i got that bit of crunching... looks weird.
		{ 
			const int i(*reinterpret_cast<const int *>(&x));
			return
				( (i & 0x7f800000) >> 23 )
				+ (i & 0x007fffff) / Sample(0x800000)
				- 0x7f;
		}
		#endif

		#if PSYCLE__MATH__REAL == 8
			/// when Real is 64-bit
			template<typename Real> inline const bool denormal(const Real & x)
			{
				//return std::fabs(x) < std::numeric_limits<Real>::denorm_min();
				return std::fabs(x) < std::numeric_limits<Real>::epsilon();
			}
		#endif

		/// for 32-bit
		inline const bool denormal(const Sample & x)
		{
			return !(*reinterpret_cast<const unsigned int *>(&x) & 0x7f800000);
		}

		template<typename Real> inline const Real renormalized(const Real & x)
		{
			return denormal(x) ? 0 : x;
			
		}

		template<typename Real> inline void renormalize(Real & x)
		{
			x = renormalized(x);
		}

		inline void renormalize(Sample samples [], const int & sample_count)
		{
			Sample normal_dither(Sample(1e-20));
			for(int sample(0) ; sample < sample_count ; ++sample)
			{
				samples[sample] += normal_dither;
				normal_dither = -normal_dither;
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

				/*
				const Real sin(clipped<Real>(-1, sequence_[0] * step_ - sequence_[1], +1));
				sequence_[1] = sequence_[0];
				sequence_[0] = sin;
				return sequence_[0];
				*/
			}
		private:
			Real step_, sequence_[2];
			int index_;
		};
	}
}









/////////////////////// todo //////////////////////////
/*
class Interpolator
{
public:
	enum Quality
	{
		none,
		linear,
		spline,
	};
	Quality quality_;
	typedef const Sample (*Interpolate)
	(
		const float &, const float & sample, const float &, const float &,
		const unsigned __int32 res, const unsigned __int64 offset, const unsigned __int64 length
	);
	Interpolate interpolate_;

	Interpolator()
	{ 
		quality_ = Quality::none;
		interpolate_ = none;
	};
	virtual void quality(const Quality & quality) = 0;
protected:
	static const float none
	(
		const float &, const float & sample, const float &, const float &,
		const unsigned __int32 resolution, const unsigned __int64 offset, const unsigned __int64 length
	)
	{
		return sample;
	}
};

class Cubic : public Interpolator
{
public:
	Cubic();
	virtual void quality(const Quality & quality)
	{
		this->quality_ = quality;
		switch(quality)
		{
		case none:
			interpolate_ = none;
			break;
		case linear:
			interpolate_ = linear;
			break;
		case spline:
			interpolate_ = spline;
			break;
		}
	}
protected:
	static const float linear
	(
		const float & previous_sample, const float & sample0, const float & sample1, const float &,
		const unsigned __int32 resolution, const unsigned __int64 offset, const unsigned __int64 length
	)
	{
		if(offset + 2 > length) sample1 = 0;
		return (sample0 + (sample1 - sample0) * linear_table[resolution >> 21]);
	}
	
	static float spline
	(
		const float & previous_sample, const float & sample0, const float & sample1, const float & sample2,
		const unsigned __int32 resolution, const unsigned __int64 offset, const unsigned __int64 length
	)
	{
		if(offset == 0) previous_sample = 0;
		if(offset + 2 > length) sample1 = 0;
		if(offset + 3 > length) sample2 = 0;
		const int index(res >> 21);
		return
			tables[0][index] * previous_sample +
			tables[1][index] * sample0 +
			tables[2][index] * sample1 +
			tables[3][index] * sample2;
	}
private:
	/* static const int *//* enum { resolution = 2048 };
	static const float tables[4][resolution], linear_table[resolution];
};

template<typename Sample> class Interpolated_Buffer : private::vector<Sample>
{
	inline const operator[](const Real & index) {}
};

template<typename Sample> class Circular_Buffer : private std::vector<Sample>
{
public:
	Circular_Buffer(const int & size)
		: std::vector<Sample>(size)
	{}
	inline const Sample & operator[](const int & index) const
	{
		return std::vector<Sample>::operator[](index);
	}
	inline Sample & operator[](const int & index)
	{
		return std::vector<Sample>::operator[](index);
	}
	class Iterator
	{
		inline void operator++(int)
		{
			++index_ %= ::std::vector<Sample>::size();
		}
	private:
		int index_;
	};
	
};

template<typename Sample> class Interpolated_Circular_Buffer : private::vector<Sample>
{
	
};
*/
