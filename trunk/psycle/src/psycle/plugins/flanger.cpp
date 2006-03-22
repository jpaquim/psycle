/// \file
/// \brief delay modulated by a sine
#include <packageneric/pre-compiled.private.hpp>
#include <psycle/plugin.hpp>
namespace psycle { namespace plugin {

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
			Information::Parameter::discrete("interpolation", no, yes),
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
			else if((*this)(modulation_stereo_dephase) == math::sample::pi) out << "pi";
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
	inline void process(math::Sin_Sequence & sin_sequence, std::vector<Real> & buffer, int & write, Sample input [], const int & samples, const Real & feedback) throw();
	inline void resize(const Real & delay);
	enum Channels { left, right, channels };
	std::vector<Real> buffers_[channels];
	int delay_in_samples_, writes_[channels];
	Real weight; //<Sartorius> for interpolation
	Real modulation_amplitude_in_samples_, modulation_radians_per_sample_, modulation_phase_;
	math::Sin_Sequence sin_sequences_[channels];
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
	weight = delay * samples_per_second() - floor(delay * samples_per_second()); //<Sartorius> for interpolation
	modulation_amplitude_in_samples_ = (*this)(modulation_amplitude) * delay_in_samples_;
	for(int channel(0) ; channel < channels ; ++channel)
	{
		buffers_[channel].resize(1 + delay_in_samples_ + static_cast<int>(::ceil(modulation_amplitude_in_samples_)), 0);
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

inline void Flanger::process(math::Sin_Sequence & sin_sequence, std::vector<Real> & buffer, int & write, Sample input [], const int & samples, const Real & feedback) throw()
{
	#if 0
		//<Sartorius> linear interpolation

		const int size(buffer.size());
		for(int sample(0) ; sample < samples ; ++sample)
		{
			const Real sin(sin_sequence()); // <bohan> this uses 64-bit floating point numbers or else accuracy is not sufficient


			assert(-1 <= sin);
			assert(sin <= 1);
			assert(0 <= write);
			assert(write < static_cast<int>(buffer.size()));
			assert(0 <= modulation_amplitude_in_samples_);
			assert(modulation_amplitude_in_samples_ <= delay_in_samples_);

			int read;
			//if(sin < 0) read = write - delay_in_samples_ + static_cast<int>(modulation_amplitude_in_samples_ * sin);
			//else read = write - delay_in_samples_ + static_cast<int>(modulation_amplitude_in_samples_ * sin) - 1;
			read = write - delay_in_samples_ + std::floor(modulation_amplitude_in_samples_ * sin);
			if(read < 0) read += size; else if(read >= size) read -= size;

			assert(0 <= read);
			assert(read < static_cast<int>(buffer.size()));
			
			/*const */ Real buffer_read(0);
			if ((*this)[interpolation]==yes) 
			{
				///\todo [bohan] this assertion is failing
				assert(read+1 < static_cast<int>(buffer.size()));
				buffer_read = buffer[read] * (1.0 - weight) + buffer[read+1] * weight;
			} else {
				buffer_read = buffer[read];
			}
			Sample & input_sample = input[sample];
			buffer[write] = /*math::renormalized*/(input_sample + feedback * buffer_read);
			++write %= size;
			input_sample *= (*this)(dry);
			input_sample += (*this)(wet) * buffer_read;
			// NaN and Den remover :
			unsigned int corrected_sample = *((unsigned int*)&input_sample);
			unsigned int exponent = corrected_sample & 0x7F800000;
			corrected_sample *= ((exponent < 0x7F800000) & (exponent > 0));
			input_sample = *((float*)&corrected_sample);
		}

	#else
		// no interpolation

		switch((*this)[interpolation])
		{
			case yes: /// \todo interpolation not done for now
	//			ULARGE_INTEGER tmposcL;
	//			tmposcL.QuadPart = (__int64)((y0L*_fLfoAmp) *0x100000000);
	//			int _delayedCounterL = _counter - _time + tmposcL.HighPart;
	//
	//			if (_delayedCounterL < 0) _delayedCounterL += 2048;
	//			int c = (_delayedCounterL==2047) ? 0 : _delayedCounterL+1;
	//
	//			float y_l = pResamplerWork( 0 ,
	//				_pBufferL[_delayedCounterL],
	//				_pBufferL[c], 0,
	//				tmposcL.LowPart, _delayedCounterL, 2050); // Since we already take care or buffer overrun, we set the length bigger.
	//			if (IS_DENORMAL(y_l) ) y_l=0.0f;
	//
	//			ULARGE_INTEGER tmposcR;
	//			tmposcR.QuadPart = (__int64)((y0R*_fLfoAmp) *0x100000000);
	//			int _delayedCounterR = _counter - _time + tmposcR.HighPart;
	//
	//			if (_delayedCounterR < 0) _delayedCounterR += 2048;
	//			c = (_delayedCounterR==2047) ? 0 : _delayedCounterR+1;
	//
	//			float y_r = pResamplerWork(	0,
	//				_pBufferR[_delayedCounterR],
	//				_pBufferR[c], 0,
	//				tmposcR.LowPart, _delayedCounterR, 2050); // Since we already take care or buffer overrun, we set the length bigger.
	//			if (IS_DENORMAL(y_r) ) y_r=0.0f;
			
			case no:
			default:
				{
					const int size(buffer.size());
					for(int sample(0) ; sample < samples ; ++sample)
					{
						const Real sin(sin_sequence()); // <bohan> this uses 64-bit floating point numbers or else accuracy is not sufficient
						/* test without optimized sin sequence...
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
						read = write - delay_in_samples_ + std::floor(modulation_amplitude_in_samples_ * sin);
						if(read < 0) read += size; else if(read >= size) read -= size;

						assert(0 <= read);
						assert(read < static_cast<int>(buffer.size()));

						const Real buffer_read(buffer[read]);
						Sample & input_sample = input[sample];
						buffer[write] = /*math::renormalized*/(input_sample + feedback * buffer_read);
						++write %= size;
						input_sample *= (*this)(dry);
						input_sample += (*this)(wet) * buffer_read;
						// NaN and Den remover :
						unsigned int corrected_sample = *((unsigned int*)&input_sample);
						unsigned int exponent = corrected_sample & 0x7F800000;
						corrected_sample *= ((exponent < 0x7F800000) & (exponent > 0));
						input_sample = *((float*)&corrected_sample);
					}
				}
				break;
		}

	#endif
}

}}
