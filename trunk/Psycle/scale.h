#ifndef PSYCLE__SCALE
#define PSYCLE__SCALE
#pragma once
#include "math.h"
/// \file
/// \brief scale facilities for machine parameters
namespace psycle
{
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