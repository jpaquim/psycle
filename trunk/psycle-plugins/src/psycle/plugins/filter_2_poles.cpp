// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2003-2007 johan boule <bohan@jabber.org>
// copyright 2003-2007 psycledelics http://psycle.pastnotecut.org

/// \file
/// \brief filter in the frequency domain using 2 poles
#include "plugin.hpp"
#include <psycle/helpers/math.hpp>
namespace psycle { namespace plugin {

using namespace helpers::math;

class Filter_2_Poles : public Plugin
{
public:
	/*override*/ void help(std::ostream & out) throw()
	{
		out << "filter in the frequency domain using 2 poles" << std::endl;
		out << "compatible with original psycle 1 arguru's 2 poles filter" << std::endl;
		out << "no more sample rate bug like in the original one" << std::endl;
		out << "the cutoff frequency is 'modulated' in an unsusal way via an oscillator (not a sine)" << std::endl;
		out << "(contact arguru if you want to know why the modulation is done like this)" << std::endl;
		out << std::endl;
		out << "commands:" << std::endl;
		out << "0x01 0x00-0xff: cutoff frequency modulation phase" << std::endl;
	}

	enum Parameters
	{
		response, cutoff_frequency, resonance,
		modulation_sequencer_ticks, modulation_amplitude, modulation_stereo_dephase
	};

	enum Response_Type { low, high };

	static const Information & information() throw()
	{
		static bool initialized = false;
		static Information *info = NULL;
		if (!initialized) {
			static const Information::Parameter parameters [] =
			{
				Information::Parameter::discrete("response", low, high),
				Information::Parameter::exponential("cutoff frequency", 73, 7276, 7276),
				Information::Parameter::linear("resonance", 0, 0, 1),
				Information::Parameter::exponential("mod. frequency", pi * 2 / 10000, 0, pi * 2 * 2 * 3 * 4 * 5 * 7),
				Information::Parameter::linear("mod. amplitude", 0, 0, 1),
				Information::Parameter::linear("mod. stereodephase", 0, 0, pi)
			};
			static Information information(0x0110, Information::Types::effect, "ayeternal 2-Pole Filter", "2-Pole Filter", "bohan", 2, parameters, sizeof parameters / sizeof *parameters);
			info = &information;
			initialized = true;
		}
		return *info;
	}

	/*override*/ void describe(std::ostream & out, const int & parameter) const
	{
		switch(parameter)
		{
		case response:
			switch((*this)[response])
			{
			case low:
				out << "low";
				break;
			case high:
				out << "high";
				break;
			default:
				throw std::string("unknown response type");
			}
			break;
		case cutoff_frequency:
			out << (*this)(cutoff_frequency) << " hertz";
			break;
		case modulation_sequencer_ticks:
			out << pi * 2 / (*this)(modulation_sequencer_ticks) << " ticks (lines)";
			break;
		case modulation_amplitude:
			//information.parameter(cutoff_frequency).scale.output_maximum()  <-- The Exponential scale mangles min and max
			out << "(+/-)" << 7276  * (*this)(modulation_amplitude) << " hertz";
			break;
		case modulation_stereo_dephase:
			if((*this)(modulation_stereo_dephase) == 0) out << 0;
			else if((*this)(modulation_stereo_dephase) == Sample(pi)) out << "pi";
			else out << "pi / " << pi / (*this)(modulation_stereo_dephase);
			break;
		default:
			Plugin::describe(out, parameter);
		}
	}

	Filter_2_Poles() : Plugin(information()), modulation_phase_(0), cutoff_sin_(0)
	{
		::memset(buffers_, 0, sizeof buffers_);
	}

	/*override*/ void Work(Sample l[], Sample r[], int samples, int);
	/*override*/ void parameter(const int &);
protected:
	/*override*/ void SeqTick(int, int, int, int command, int value);
	/*override*/ void samples_per_second_changed() { parameter(cutoff_frequency); }
	/*override*/ void sequencer_ticks_per_second_changed() { parameter(modulation_sequencer_ticks); }
	static const int poles = 2;
	inline void update_coefficients();
	inline void update_coefficients(Real coefficients[poles + 1], const Real & modulation_stereo_dephase = 0);
	enum Channels { left, right, channels };
	void erase_NaNs_Infinities_And_Denormals( float* inSample );
	inline const Real WorkLow(const Real & input, Real buffer[channels], const Real coefficients[poles + 1]);
	inline const Real WorkHigh(const Real & input, Real buffer[channels], const Real coefficients[poles + 1]);
	Real cutoff_sin_, modulation_radians_per_sample_, modulation_phase_, buffers_ [channels][poles], coefficients_ [channels][poles + 1];
};

PSYCLE__PLUGIN__INSTANTIATOR(Filter_2_Poles)

void Filter_2_Poles::SeqTick(const int note, const int, const int, const int command, const int value)
{
	switch(command)
	{
	case 1:
		modulation_phase_ = pi * 2 * value / 0x100;
		break;
	}
	if ( note <= psycle::plugin_interface::NOTE_MAX )
	{
		///\todo: set cutoff_frequency by note.
		//parameter(cutoff_frequency,pow(2.0, (float)(note-18)/12.0));
	}
}

void Filter_2_Poles::parameter(const int & parameter)
{
	switch(parameter)
	{
	case cutoff_frequency:
		cutoff_sin_ = static_cast<Sample>((*this)(cutoff_frequency)* 6.283 * seconds_per_sample());
		break;
	case modulation_sequencer_ticks:
		modulation_radians_per_sample_ = (*this)(modulation_sequencer_ticks) / samples_per_sequencer_tick();
	}
	update_coefficients();
}

void Filter_2_Poles::update_coefficients()
{
	update_coefficients(coefficients_[left]);
	update_coefficients(coefficients_[right], (*this)(modulation_stereo_dephase));
}

inline void Filter_2_Poles::update_coefficients(Real coefficients[poles + 1], const Real & modulation_stereo_dephase)
{
	const Real minimum(static_cast<Real>(1e-2));
	const Real maximum(1 - minimum);
	coefficients[0] = clipped(minimum, static_cast<Real>(cutoff_sin_ + (*this)(modulation_amplitude) * std::sin(modulation_phase_ + modulation_stereo_dephase)), maximum);
	coefficients[1] = 1 - coefficients[0];
	coefficients[2] = (*this)(resonance) * (1 + 1 / coefficients[1]);
	erase_all_nans_infinities_and_denormals(coefficients, poles + 1);
}

void Filter_2_Poles::Work(Sample l[], Sample r[], int samples, int)
{
	switch((*this)[response]) {
	case low:
		for(int sample(0) ; sample < samples ; ++sample)
		{
			l[sample] = static_cast<Sample>(WorkLow(l[sample], buffers_[left] , coefficients_[left]));
			r[sample] = static_cast<Sample>(WorkLow(r[sample], buffers_[right], coefficients_[right]));
		}
		break;
	case high:
		for(int sample(0) ; sample < samples ; ++sample)
		{
			l[sample] = static_cast<Sample>(WorkHigh(l[sample], buffers_[left] , coefficients_[left]));
			r[sample] = static_cast<Sample>(WorkHigh(r[sample], buffers_[right], coefficients_[right]));
		}
		break;
	default:
		throw Exception("unknown response type");
	}

	if((*this)(modulation_amplitude)) // note: this would be done each sample for perfect quality
	{
		//\fixme: lowpass at half (~800Hz), ressonance 0.9,  mod freq 6Ticks, mod amp 0.5. Use arguru synth with the white noise setting.
		// The sound is not sinusoidal, because the modulation is not exponential
		//(i.e. since the parameter is exponential, this change has to be exponential too)
		modulation_phase_ = std::fmod(modulation_phase_ + modulation_radians_per_sample_ * samples, pi * 2);
		update_coefficients();
	}
}

inline const Filter_2_Poles::Real Filter_2_Poles::WorkLow(const Real & input, Real buffer[poles], const Real coefficients[poles + 1])
{
	buffer[0] = coefficients[1] * buffer[0] + coefficients[0] * (input + coefficients[2] * (buffer[0] - buffer[1]));
	buffer[1] = coefficients[1] * buffer[1] + coefficients[0] * buffer[0];
	erase_all_nans_infinities_and_denormals(buffer, channels);
	return buffer[1];
}

inline const Filter_2_Poles::Real Filter_2_Poles::WorkHigh(const Real & input, Real buffer[poles], const Real coefficients[poles + 1])
{
	buffer[0] = coefficients[1] * buffer[0] + coefficients[0] * (input + coefficients[2] * (buffer[0] - buffer[1]));
	buffer[1] = coefficients[1] * buffer[1] + coefficients[0] * buffer[0];
	erase_all_nans_infinities_and_denormals(buffer, channels);
	return input - buffer[1];
}
}}
