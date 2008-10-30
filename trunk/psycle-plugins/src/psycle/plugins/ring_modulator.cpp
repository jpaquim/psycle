// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2003-2007 johan boule <bohan@jabber.org>
// copyright 2003-2007 psycledelics http://psycle.pastnotecut.org

/// \file
/// \brief ring modulator with frequency modulation of the amplitude modulation
#include "plugin.hpp"
#include <psycle/helpers/math/pi.hpp>
#include <psycle/helpers/math/remainder.hpp>
namespace psycle { namespace plugin {

namespace math = helpers::math;

class Ring_Modulator : public Plugin
{
public:
	virtual void help(std::ostream & out) const throw()
	{
		out << "ring modulator with frequency modulation of the amplitude modulation" << std::endl;
		out << "compatible with original psycle 1 arguru's psych-osc" << std::endl;
		out << "but i removed the volume attenuation that followed the am frequency decrease" << std::endl;
		out << "use a high pass frequency filter to achieve the same effect" << std::endl;
		out << "beware: this is a silly machine !" << std::endl;
		out << std::endl;
		out << "commands:" << std::endl;
		out << "0x01 0x00-0xff: am phase" << std::endl;
		out << "0x02 0x00-0xff: fm phase" << std::endl;
	}

	enum Parameters { am_radians_per_second, am_glide, fm_radians_per_second, fm_bandwidth };
	
	static const Information & information() throw()
	{
		static const Information::Parameter parameters [] =
		{
			Information::Parameter::exponential("am frequency", 0.0001 * math::pi * 2, 0, 22050 * math::pi * 2),
			Information::Parameter::exponential("am frequency glide", 0.0001 * math::pi * 2, 0, 15 * 22050 * math::pi * 2),
			Information::Parameter::exponential("fm frequency", 0.0001 * math::pi * 2, 0, 100 * math::pi * 2),
			Information::Parameter::exponential("fm bandwidth", 0.0001 * math::pi * 2, 0, 22050 * math::pi * 2)
		};
		static const Information information(Information::Types::effect, "ayeternal PsychOsc AM", "PsychOsc AM", "bohan and the psycledelics community", 2, parameters, sizeof parameters / sizeof *parameters);
		return information;
	}

	virtual void describe(std::ostream & out, const int & parameter) const
	{
		switch(parameter)
		{
		case am_radians_per_second:
		case fm_radians_per_second:
		case fm_bandwidth:
			out << (*this)(parameter) / math::pi / 2 << " hertz";
			break;
		case am_glide:
			out << (*this)(am_glide) / math::pi / 2 << " hertz/second";
			break;
		default:
			Plugin::describe(out, parameter);
		}
	}

	Ring_Modulator() : Plugin(information()), glided_am_radians_per_samples_(0)
	{
		for(int oscillator(0) ; oscillator < oscillators ; ++oscillator)
			phases_[oscillator] = 0;
	}

	virtual void process(Sample l[], Sample r[], int samples, int);
	virtual void parameter(const int &);
protected:
	virtual void sequencer_note_event(const int, const int, const int, const int command, const int value);
	virtual void samples_per_second_changed()
	{
		parameter(am_radians_per_second);
		parameter(fm_radians_per_second);
		parameter(fm_bandwidth);
		parameter(am_glide);
	}
	inline const Real process(const Real & sample, const Real & modulation) const;
	enum Oscillators { am, fm, oscillators };
	Real
		phases_[oscillators], radians_per_sample_[oscillators],
		am_glide_, glided_am_radians_per_samples_, fm_bandwidth_;
};

PSYCLE__PLUGIN__INSTANCIATOR(Ring_Modulator)

void Ring_Modulator::sequencer_note_event(const int, const int, const int, const int command, const int value)
{
	switch(command)
	{
	case 1:
		phases_[am] = math::pi * 2 * value / 0x100;
		break;
	case 2:
		phases_[fm] = math::pi * 2 * value / 0x100;
		break;
	}
}

void Ring_Modulator::parameter(const int & parameter)
{
	switch(parameter)
	{
	case am_radians_per_second:
		radians_per_sample_[am] = (*this)(am_radians_per_second) * seconds_per_sample();
		break;
	case fm_radians_per_second:
		radians_per_sample_[fm] = (*this)(fm_radians_per_second) * seconds_per_sample();
		break;
	case fm_bandwidth:
		fm_bandwidth_ = (*this)(fm_bandwidth) * seconds_per_sample();
	case am_glide:
		am_glide_ = (*this)(am_glide) * seconds_per_sample() * seconds_per_sample();
	}
}

void Ring_Modulator::process(Sample l[], Sample r[], int samples, int)
{
	for(int sample(0) ; sample < samples ; ++sample)
	{
		const Real modulation(static_cast<Real>(::sin(phases_[am]))); // * glided_am_radians_per_samples_);
		l[sample] = static_cast<Sample>(process(l[sample], modulation));
		r[sample] = static_cast<Sample>(process(r[sample], modulation));;
		phases_[am] += static_cast<Real>(glided_am_radians_per_samples_ + ::sin(phases_[fm]) * fm_bandwidth_);
		phases_[fm] += radians_per_sample_[fm];
		if(radians_per_sample_[am] > glided_am_radians_per_samples_)
		{
			glided_am_radians_per_samples_ += am_glide_;
			if(radians_per_sample_[am] < glided_am_radians_per_samples_)
				glided_am_radians_per_samples_ = radians_per_sample_[am];
		}
		else if(radians_per_sample_[am] < glided_am_radians_per_samples_)
		{
			glided_am_radians_per_samples_ -= am_glide_;
			if(radians_per_sample_[am] > glided_am_radians_per_samples_)
				glided_am_radians_per_samples_ = radians_per_sample_[am];
		}
	}
	for(int oscillator(0) ; oscillator < oscillators ; ++oscillator)
		phases_[oscillator] = math::remainder(phases_[oscillator], math::pi * 2);
}

inline const Ring_Modulator::Real Ring_Modulator::process(const Real & sample, const Real & modulation) const
{
	return sample * modulation;
}

}}
