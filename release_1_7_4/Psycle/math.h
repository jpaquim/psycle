#ifndef PSYCLE__SAMPLE
#define PSYCLE__SAMPLE
#pragma once
#include <limits>
#include <cmath>
#include <vector>
/// \file
/// \brief
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
			#if PSYCLE__DEBUG_LEVEL > 1
			assert(x > 0); 
			assert(sizeof x == 8);
			#endif
			const Real result(x - 0.5 + 6755399441055744); // 2^51 + 2^52
			return *reinterpret_cast<const int *>(&result);
		};

		inline const int truncated(const Sample & x) 
		{ 
			#if PSYCLE__DEBUG_LEVEL > 1
			assert(x > 0);
			assert(sizeof x == sizeof(int)); 
			assert(sizeof x == 4);
			#endif
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
			#if PSYCLE__DEBUG_LEVEL > 1
			assert(x > 0); 
			assert(sizeof x == sizeof(int)); 
			assert(sizeof x == 4);
			#endif
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
}
#endif









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
