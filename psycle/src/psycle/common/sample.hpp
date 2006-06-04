/// \file
/// \brief
#pragma once
#include "pi.hpp"
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

	namespace common
	{
		namespace math
		{
			const Real pi(Real(3.14159265358979323846));

			namespace sample { const Sample pi(static_cast<Sample>(pi)); }

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
}
