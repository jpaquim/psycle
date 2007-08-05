// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2003-2007 johan boule <bohan@jabber.org>
// copyright 2003-2007 psycledelics http://psycle.pastnotecut.org

/// \file
/// \brief delay modulated by a sine
#include <packageneric/pre-compiled.private.hpp>
#include "plugin.hpp"
#include <psycle/helpers/math/sinus_sequence.hpp>
#include <psycle/helpers/math/pi.hpp>
#include <psycle/helpers/math/remainder.hpp>
#include <psycle/helpers/math/erase_all_nans_infinities_and_denormals.hpp>
#include <cassert>
#include <vector>
namespace psycle { namespace plugin {

namespace math = common::math;

class Flanger : public Plugin
{
public:
	virtual void help(std::ostream & out) const throw()
	{
		out << "delay modulated by a sine" << std::endl;
		out << "compatible with original psycle 1 arguru's flanger" << std::endl;
		out << std::endl;
		out << "commands:" << std::endl;
		out << "0x01 0x00-0xff: delay length modulation phase" << std::endl;
	}

	enum Parameters
	{
		delay,
		modulation_amplitude, modulation_radians_per_second, modulation_stereo_dephase,
		interpolation,
		dry, wet,
		left_feedback, right_feedback,
	};

	enum Interpolation { no, yes };
	
	static const Information & information() throw()
	{
		static const Information::Parameter parameters [] =
		{
			Information::Parameter::linear("central delay", 0, 0, 0.1),
			Information::Parameter::linear("mod. amplitude", 0, 0, 1),
			Information::Parameter::exponential("mod. frequency", 0.0001 * math::pi * 2, 0, 100 * math::pi * 2),
			Information::Parameter::linear("mod. stereodephase", 0, 0, math::pi),
			Information::Parameter::discrete("interpolation", yes, yes),
			Information::Parameter::linear("dry", -1, 1, 1),
			Information::Parameter::linear("wet", -1, 0, 1),
			Information::Parameter::linear("feedback left", -1, 0, 1),
			Information::Parameter::linear("feedback right", -1, 0, 1),
		};
		static const Information information(Information::Types::effect, "ayeternal Flanger", "Flanger", "jaz, bohan, and the psycledelics community", 2, parameters, sizeof parameters / sizeof *parameters);
		return information;
	}

	virtual void describe(std::ostream & out, const int & parameter) const
	{
		switch(parameter)
		{
		case interpolation:
			switch((*this)[interpolation])
			{
			case no:
				out << "no";
				break;
			case yes:
				out << "yes";
				break;
			default:
				out << "yes or no ???";
			}
			break;
		case delay:
			out << (*this)(delay) << " seconds";
			break;
		case modulation_radians_per_second:
			out << (*this)(modulation_radians_per_second) / math::pi / 2 << " hertz";
			break;
		case modulation_amplitude:
			 out << (*this)(delay) * (*this)(modulation_amplitude) << " seconds";
			 break;
		case modulation_stereo_dephase:
			if((*this)(modulation_stereo_dephase) == 0) out << 0;
			else if((*this)(modulation_stereo_dephase) == Sample(math::pi)) out << "pi";
			else out << "pi / " << math::pi / (*this)(modulation_stereo_dephase);
			break;
		case left_feedback:
		case right_feedback:
		case dry:
		case wet:
			if(std::fabs((*this)(parameter)) < 2e-5)
			{
				out << 0;
				break;
			}
		default:
			Plugin::describe(out, parameter);
		}
	};

	Flanger() : Plugin(information()), modulation_phase_(0)
	{
		for(int channel(0) ; channel < channels ; ++channel)
			writes_[channel] = 0;
	}
	virtual inline ~Flanger() throw() {}
	virtual void init();
	virtual void process(Sample l [], Sample r [], int samples, int);
	virtual void parameter(const int &);
protected:
	virtual void sequencer_note_event(const int, const int, const int, const int command, const int value);
	virtual void samples_per_second_changed();
	inline void process(math::sinus_sequence &, std::vector<Real> & buffer, int & write, Sample input [], const int & samples, const Real & feedback) throw();
	inline void resize(const Real & delay);
	enum Channels { left, right, channels };
	std::vector<Real> buffers_[channels];
	int delay_in_samples_, writes_[channels];
	Real modulation_amplitude_in_samples_, modulation_radians_per_sample_, modulation_phase_;
	math::sinus_sequence sin_sequences_[channels];
};

PSYCLE__PLUGIN__INSTANCIATOR(Flanger)

void Flanger::init()
{
	resize(0); // resizes the buffers not to 0, but to 1, the smallest length possible for the algorithm to work
}

void Flanger::sequencer_note_event(const int, const int, const int, const int command, const int value)
{
	switch(command)
	{
	case 1:
		modulation_phase_ = value * math::pi * 2 / 0x100;
		sin_sequences_[left](modulation_phase_, modulation_radians_per_sample_);
		sin_sequences_[right](modulation_phase_ + (*this)(modulation_stereo_dephase), modulation_radians_per_sample_);
		break;
	}
}

void Flanger::samples_per_second_changed()
{
	parameter(modulation_radians_per_second);
}

void Flanger::parameter(const int & parameter)
{
	switch(parameter)
	{
	case delay:
	case modulation_amplitude:
		resize((*this)(delay));
		break;
	case modulation_stereo_dephase:
	case modulation_radians_per_second:
		modulation_radians_per_sample_ = (*this)(modulation_radians_per_second) * seconds_per_sample();
		sin_sequences_[left](modulation_phase_, modulation_radians_per_sample_);
		sin_sequences_[right](modulation_phase_ + (*this)(modulation_stereo_dephase), modulation_radians_per_sample_);
		break;
	case interpolation:
		switch((*this)[interpolation])
		{
		case no:
		case yes:
			break;
		default:
			throw Exception("interpolation must be yes or no");
		}
		break;
	}
}

inline void Flanger::resize(const Real & delay)
{
	delay_in_samples_ = static_cast<int>(delay * samples_per_second());
	modulation_amplitude_in_samples_ = (*this)(modulation_amplitude) * delay_in_samples_;
	for(int channel(0) ; channel < channels ; ++channel)
	{
		buffers_[channel].resize(1 + delay_in_samples_ + static_cast<int>(std::ceil(modulation_amplitude_in_samples_)), 0);
			// resizes the buffer at least to 1, the smallest length possible for the algorithm to work
		writes_[channel] %= buffers_[channel].size();
	}
	assert(0 <= modulation_amplitude_in_samples_);
	assert(modulation_amplitude_in_samples_ <= delay_in_samples_);
}

void Flanger::process(Sample l[], Sample r[], int samples, int)
{
	process(sin_sequences_[left], buffers_[left] , writes_[left] , l, samples, (*this)(left_feedback));
	process(sin_sequences_[right], buffers_[right], writes_[right], r, samples, (*this)(right_feedback));
	modulation_phase_ = math::remainder(modulation_phase_ + modulation_radians_per_sample_ * samples, math::pi * 2);
}

inline void Flanger::process(math::sinus_sequence & sinus_sequence, std::vector<Real> & buffer, int & write, Sample input [], const int & samples, const Real & feedback) throw()
{
	const int size(static_cast<int>(buffer.size()));
	switch((*this)[interpolation])
	{
	case yes:
		{
			for(int sample(0) ; sample < samples ; ++sample)
			{
				const Real sin(sinus_sequence()); // <bohan> this uses 64-bit floating point numbers or else accuracy is not sufficient
				Real fraction_part = modulation_amplitude_in_samples_ * sin;
				int integral_part = static_cast<int>(fraction_part);
				fraction_part = fraction_part-integral_part;
				if (fraction_part < 0) { fraction_part += 1; integral_part -= 1; }
//				Real integral_part(0);
//				Real fraction_part = std::modf(modulation_amplitude_in_samples_ * sin,&integral_part);
//				if (fraction_part < 0) fraction_part = 1.0+fraction_part;
				int read = write - delay_in_samples_ + integral_part;
				int nextvalue = read+1;
				if(read < 0)
				{
					read += size;
					if ( nextvalue < 0 )
						nextvalue += size;
				}
				else if(nextvalue >= size)
				{
					nextvalue -= size;
					if ( read >= size)
						read -= size;
				}

				const Real buffer_read = buffer[read]*(1.0-fraction_part) + buffer[nextvalue]*fraction_part;
				
				Sample & input_sample = input[sample];
				Sample buffer_write = input_sample + feedback * buffer_read + 1.0e-9;
				buffer_write-= static_cast<Sample>(1.0e-9);
//				Sample buffer_write = input_sample + feedback * buffer_read;
//				math::erase_all_nans_infinities_and_denormals(buffer_write);
				buffer[write] = buffer_write;
				++write %= size;
				input_sample *= (*this)(dry);
				input_sample += (*this)(wet) * buffer_read;
//				math::erase_all_nans_infinities_and_denormals(input_sample);
			}
		}
		break;
	case no:
	default:
		{
			for(int sample(0) ; sample < samples ; ++sample)
			{
				const Real sin(sinus_sequence()); // <bohan> this uses 64-bit floating point numbers or else accuracy is not sufficient
				/* test without optimized sinus sequence...
				Real sin;
				if(&sin_sequence == &sin_sequences_[left])
					sin = std::sin(modulation_phase_);
				else
					sin = std::sin(modulation_phase_ + (*this)(modulation_stereo_dephase));
				*/

				assert(-1 <= sin);
				assert(sin <= 1);
				assert(0 <= write);
				assert(write < static_cast<int>(buffer.size()));
				assert(0 <= modulation_amplitude_in_samples_);
				assert(modulation_amplitude_in_samples_ <= delay_in_samples_);

				int read;
				//if(sin < 0) read = write - delay_in_samples_ + static_cast<int>(modulation_amplitude_in_samples_ * sin);
				//else read = write - delay_in_samples_ + static_cast<int>(modulation_amplitude_in_samples_ * sin) - 1;
//				read = write - delay_in_samples_ + std::floor(modulation_amplitude_in_samples_ * sin);
				int integral_part = static_cast<int>(modulation_amplitude_in_samples_ * sin);
				read = write - delay_in_samples_ + integral_part;

				if(read < 0) read += size; else if(read >= size) read -= size;

				assert(0 <= read);
				assert(read < static_cast<int>(buffer.size()));

				const Real buffer_read(buffer[read]);
				Sample & input_sample = input[sample];
				Sample buffer_write = input_sample + feedback * buffer_read + 1.0e-9;
				buffer_write -= static_cast<Sample>(1.0e-9);
//				Sample buffer_write = input_sample + feedback * buffer_read;
//				math::erase_all_nans_infinities_and_denormals(buffer_write);
				buffer[write] = buffer_write;
				++write %= size;
				input_sample *= (*this)(dry);
				input_sample += (*this)(wet) * buffer_read;
//				math::erase_all_nans_infinities_and_denormals(input_sample);
			}
		}
		break;
	}
}

}}
