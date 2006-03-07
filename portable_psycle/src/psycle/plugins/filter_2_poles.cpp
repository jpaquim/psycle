/// \file
/// \brief filter in the frequency domain using 2 poles
#include <packageneric/pre-compiled.private.hpp>
#include <psycle/plugin.hpp>
namespace psycle { namespace plugin {

class Filter_2_Poles : public Plugin
{
public:
	virtual void help(std::ostream & out) const throw()
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
		static const Information::Parameter parameters [] =
		{
			Information::Parameter::discrete("response", low, high),
			Information::Parameter::exponential("cutoff frequency", 15 * math::pi, 22050 * math::pi, 22050 * math::pi),
			Information::Parameter::linear("resonance", 0, 0, 1),
			Information::Parameter::exponential("mod. frequency", math::pi * 2 / 10000, 0, math::pi * 2 * 2 * 3 * 4 * 5 * 7),
			Information::Parameter::linear("mod. amplitude", 0, 0, 1),
			Information::Parameter::linear("mod. stereodephase", 0, 0, math::pi)
		};
		static const Information information(Information::Type::effect, "ayeternal 2-Pole Filter", "2-Pole Filter", "bohan", 2, parameters, sizeof parameters / sizeof *parameters);
		return information;
	}

	virtual void describe(std::ostream & out, const int & parameter) const
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
			out << (*this)(cutoff_frequency) / math::pi << " hertz";
			break;
		case modulation_sequencer_ticks:
			out << math::pi * 2 / (*this)(modulation_sequencer_ticks) << " ticks (lines)";
			break;
		case modulation_stereo_dephase:
			if((*this)(modulation_stereo_dephase) == 0) out << 0;
			else if((*this)(modulation_stereo_dephase) == math::sample::pi) out << "pi";
			else out << "pi / " << math::pi / (*this)(modulation_stereo_dephase);
			break;
		default:
			Plugin::describe(out, parameter);
		}
	}

	Filter_2_Poles() : Plugin(information()), modulation_phase_(0)
	{
		::memset(buffers_, 0, sizeof buffers_);
	}

	virtual void process(Sample l[], Sample r[], int samples, int);
	virtual void parameter(const int &);
protected:
	virtual void sequencer_note_event(const int, const int, const int, const int command, const int value);
	virtual void samples_per_second_changed() { parameter(cutoff_frequency); }
	virtual void sequencer_ticks_per_second_changed() { parameter(modulation_sequencer_ticks); }
	static const int poles = 2;
	inline void update_coefficients();
	inline void update_coefficients(Real coefficients[poles + 1], const Real & modulation_stereo_dephase = 0);
	enum Channels { left, right, channels };
    void erase_NaNs_Infinities_And_Denormals( float* inSample );
    inline const Real process(const Real & input, Real buffer[channels], const Real coefficients[poles + 1]);
	Real cutoff_sin_, modulation_radians_per_sample_, modulation_phase_, buffers_ [channels][poles], coefficients_ [channels][poles + 1];
};

PSYCLE__PLUGIN__INSTANCIATOR(Filter_2_Poles)

void Filter_2_Poles::sequencer_note_event(const int note, const int, const int, const int command, const int value)
{
	switch(command)
	{
	case 1:
		modulation_phase_ = math::pi * 2 * value / 0x100;
		break;
	}
	if ( note < 120 )
	{

	}
}

void Filter_2_Poles::parameter(const int & parameter)
{
	switch(parameter)
	{
	case cutoff_frequency:
		cutoff_sin_ = static_cast<Sample>(sin((*this)(cutoff_frequency) * seconds_per_sample()));
		break;
	case modulation_sequencer_ticks:
		modulation_radians_per_sample_ = (*this)(modulation_sequencer_ticks) / samples_per_sequencer_tick();
	}
	update_coefficients();
}

/***********************************************************************
Cure for malicious samples
Type : Filters Denormals, NaNs, Infinities
References : Posted by urs[AT]u-he[DOT]com
***********************************************************************/
void Filter_2_Poles::erase_NaNs_Infinities_And_Denormals( float* inSample )
{
	unsigned int* inFloat = (unsigned int*) inSample;

	unsigned int sample = *inFloat;
	unsigned int exponent = sample & 0x7F800000;
	// exponent < 0x7F800000 is 0 if NaN or Infinity, otherwise 1
	// exponent > 0 is 0 if denormalized, otherwise 1
	int aNaN = exponent < 0x7F800000;
	int aDen = exponent > 0;
	*inFloat = sample * ( aNaN & aDen );
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
	coefficients[0] = math::clipped(minimum, static_cast<Real>(cutoff_sin_ + (*this)(modulation_amplitude) * sin(modulation_phase_ + modulation_stereo_dephase)), maximum);
	float tmp = static_cast<Sample>(coefficients[0]);
	erase_NaNs_Infinities_And_Denormals(&tmp);
	coefficients[0] = tmp;
	coefficients[1] = 1 - coefficients[0];
	tmp = static_cast<Sample>(coefficients[1]);
	erase_NaNs_Infinities_And_Denormals(&tmp);
	coefficients[1] = tmp;
	coefficients[2] = (*this)(resonance) * (1 + 1 / coefficients[1]);
	tmp = static_cast<Sample>(coefficients[2]);
	erase_NaNs_Infinities_And_Denormals(&tmp);
	coefficients[2] = tmp;
}

void Filter_2_Poles::process(Sample l[], Sample r[], int samples, int)
{
	for(int sample(0) ; sample < samples ; ++sample)
	{
		l[sample] = static_cast<Sample>(process(l[sample], buffers_[left] , coefficients_[left]));
		r[sample] = static_cast<Sample>(process(r[sample], buffers_[right], coefficients_[right]));
	}

	if((*this)(modulation_amplitude)) // note: this would be done each sample for perfect quality
	{
		modulation_phase_ = math::remainder(modulation_phase_ + modulation_radians_per_sample_ * samples, math::pi * 2);
		update_coefficients();
	}
}

inline const Real Filter_2_Poles::process(const Real & input, Real buffer[poles], const Real coefficients[poles + 1])
{
	buffer[0] = coefficients[1] * buffer[0] + coefficients[0] * (input + coefficients[2] * (buffer[0] - buffer[1]));
	float tmp = static_cast<Sample>(buffer[0]);
	erase_NaNs_Infinities_And_Denormals(&tmp);
	buffer[0] = tmp;
	buffer[1] = coefficients[1] * buffer[1] + coefficients[0] * buffer[0];
	tmp = static_cast<Sample>(buffer[1]);
	erase_NaNs_Infinities_And_Denormals(&tmp);
	buffer[1] = tmp;
	switch((*this)[response])
	{
	case low:
		return buffer[1];
		break;
	case high:
		return input - buffer[1];
		break;
	default:
		throw Exception("unknown response type");
	}
}

}}
