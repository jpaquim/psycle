#include <project.h>
#include <psycle/plugin/plugin.h>
#include <cassert>
#include <vector>
/// \file
/// \brief Haas stereo time delay spatial localization
namespace psycle { namespace plugin {
/// Haas stereo time delay spatial localization.
class Haas : public Plugin
{
public:
	/// <bohan> use 64-bit floating point numbers or else accuracy is not sufficient
	typedef double Real;

	virtual void help(std::ostream & out) const throw()
	{
		out << "Haas stereo time delay spatial localization" << std::endl;
		out << std::endl;
		out << "What we're doing here is essentially creating two Haas effects:" << std::endl;
		out << "\t The first gives your ears direct spacialization between the sound source and the listener," << std::endl;
		out << "\t and the second gives your brain indirect spacialization between the listener and the source via some reference point of which the sound reflected." << std::endl;
		out << std::endl;
		out << "A third delay can be used for late reflection, giving your brain a hint about the openess of the environement." << std::endl;
		out << std::endl;
		out << "Note: The stereo input signal left and right channels are first summed to form a mono input signal." << std::endl;
		out << std::endl;
		out << "Beware if you tweak any delay length with a factor > 2 that the memory buffer gets resized." << std::endl;
	}

	enum Parameters
	{
		separator_direct,
		direct_gain, direct_delay_stereo_delta, direct_pan_amount,
		separator_early_reflection,
		early_reflection_gain, early_reflection_delay, early_reflection_delay_stereo_delta, early_reflection_pan_amount,
		separator_late_reflection,
		late_reflection_gain, late_reflection_delay, late_reflection_pan,
		separator,
		channel_mix
	};

	enum Channel_Mix { normal, swapped, mono };

	static const Information & information() throw()
	{
		static const Information::Parameter parameters [] =
		{
			Information::Parameter("direct"),
			Information::Parameter::exponential("gain", std::pow(10., -60. / 20), 1, std::pow(10., +24. / 20)),
			Information::Parameter::linear("delay stereo delta", -.006, 0, +.006),
			Information::Parameter::linear("pan amount", 0, 0, 1),
			Information::Parameter("early reflection"),
			Information::Parameter::exponential("gain", std::pow(10., -60. / 20), 0, std::pow(10., +24. / 20)),
			Information::Parameter::exponential("delay", .0005, .01, .045),
			Information::Parameter::linear("delay stereo delta", -.006, 0, +.006),
			Information::Parameter::linear("pan amount", 0, 0, 1),
			Information::Parameter("late reflection"),
			Information::Parameter::exponential("gain", std::pow(10., -60. / 20), 0, std::pow(10., +24. / 20)),
			Information::Parameter::exponential("delay", .015, .04, .100),
			Information::Parameter::linear("pan", -1, 0, 1),
			Information::Parameter(),
			Information::Parameter::discrete("channel mix", normal, mono)
		};
		static const Information information(Information::Type::effect, "Haas stereo time delay spatial localization", "Haas", "bohan/dilvie collaboration", 1, parameters, sizeof parameters / sizeof *parameters);
		return information;
	}

	virtual void describe(std::ostream & out, const int & parameter) const
	{
		switch(parameter)
		{
		case direct_delay_stereo_delta:
		case early_reflection_delay_stereo_delta:
			if(std::fabs((*this)(parameter)) < 1e-6)
			{
				out << 0;
				break;
			}
		case early_reflection_delay:
		case late_reflection_delay:
			out << (*this)(parameter) * 1000 << " ms";
			break;
		case direct_gain:
		case early_reflection_gain:
		case late_reflection_gain:
			out << std::setprecision(3) << std::setw(6) << (*this)(parameter);
			out << " (" << std::setw(6) << 20 * std::log10((*this)(parameter)) << " dB)";
			break;
		case channel_mix:
			switch((*this)[parameter])
			{
			case normal:
				out << "normal";
				break;
			case swapped:
				out << "swapped";
				break;
			case mono:
				out << "mono";
				break;
			default:
				out << "???";
			}
			break;
		case late_reflection_pan:
			if(std::fabs((*this)(parameter)) < 2e-5)
			{
				out << 0;
				break;
			}
		case direct_pan_amount:
		case early_reflection_pan_amount:
		default:
			Plugin::describe(out, parameter);
		}
	};

	Haas() :
		Plugin(information()),
		direct_delay_stereo_delta_abs(0),
		direct_delay_stereo_delta_positive(false),
		direct_delay_stereo_delta_negative(false),
		direct_left(direct_first),
		direct_right(direct_first),
		direct_gain_left(1),
		direct_gain_right(1),
		early_reflection_delay_stereo_delta_abs(0),
		early_reflection_delay_stereo_delta_positive(false),
		early_reflection_delay_stereo_delta_negative(false),
		early_reflection_left(early_reflection_first),
		early_reflection_right(early_reflection_first),
		early_reflection_gain_left(1),
		early_reflection_gain_right(1),
		late_reflection_gain_left(1),
		late_reflection_gain_right(1)
	{
	}
	virtual void init();
	virtual void process(Sample l [], Sample r [], int samples, int);
	virtual void parameter(const int &);
protected:
	virtual void samples_per_second_changed()
	{
		parameter(direct_delay_stereo_delta);
	}
	virtual void sequencer_ticks_per_second_changed()
	{
	}
	enum Channels { left, right, channels };
	enum Stages { direct_first, direct_last, early_reflection_first, early_reflection_last, late_reflection, stages };
	int direct_left, direct_right, early_reflection_left, early_reflection_right;
	std::vector<Real> buffer_;
	std::vector<Real>::iterator buffer_iterators_ [stages];
	bool direct_delay_stereo_delta_positive, early_reflection_delay_stereo_delta_positive;
	bool direct_delay_stereo_delta_negative, early_reflection_delay_stereo_delta_negative;
	Real direct_delay_stereo_delta_abs, early_reflection_delay_stereo_delta_abs;
	Real direct_gain_left, direct_gain_right;
	Real early_reflection_gain_left, early_reflection_gain_right;
	Real late_reflection_gain_left, late_reflection_gain_right;
	inline void process(Sample & left, Sample & right);
	inline void resize(const Real & delay);
};

PSYCLE__PLUGIN__INSTANCIATOR(Haas);

void Haas::init()
{
	resize(Real(0)); // resizes the buffer not to 0, but to 1, the smallest length possible for the algorithm to work
}

void Haas::parameter(const int & parameter)
{
	switch(parameter)
	{
	case direct_delay_stereo_delta:
		direct_delay_stereo_delta_positive = (*this)(direct_delay_stereo_delta) > 0;
		direct_delay_stereo_delta_negative = !direct_delay_stereo_delta_positive && (*this)(direct_delay_stereo_delta) < 0;
		direct_delay_stereo_delta_abs = std::fabs((*this)(direct_delay_stereo_delta));
		goto resize_max;
	case early_reflection_delay_stereo_delta:
		early_reflection_delay_stereo_delta_positive = (*this)(direct_delay_stereo_delta) > 0;
		early_reflection_delay_stereo_delta_negative = !early_reflection_delay_stereo_delta_positive && (*this)(direct_delay_stereo_delta) < 0;
		early_reflection_delay_stereo_delta_abs = std::fabs((*this)(direct_delay_stereo_delta));
		goto resize_max;
	case early_reflection_delay:
	case late_reflection_delay:
	resize_max:
		{
			Real max;
			max = std::max(direct_delay_stereo_delta_abs, (*this)(early_reflection_delay) + early_reflection_delay_stereo_delta_abs);
			max = std::max(max, (*this)(late_reflection_delay));
			resize(max);
		}
		break;
	case direct_gain:
	case direct_pan_amount:
		if(direct_delay_stereo_delta_positive)
		{
			direct_left = direct_last;
			direct_right = direct_first;
			direct_gain_left = (*this)(direct_gain) * (1 - (*this)(direct_pan_amount));
			direct_gain_right = (*this)(direct_gain);
		}
		else if(direct_delay_stereo_delta_negative)
		{
			direct_left = direct_first;
			direct_right = direct_last;
			direct_gain_left = (*this)(direct_gain);
			direct_gain_right = (*this)(direct_gain) * (1 - (*this)(direct_pan_amount));
		}
		else
		{
			direct_left = direct_first;
			direct_right = direct_first;
			direct_gain_left = (*this)(direct_gain);
			direct_gain_right = (*this)(direct_gain);
		}
		break;
	case early_reflection_gain:
	case early_reflection_pan_amount:
		if(early_reflection_delay_stereo_delta_positive)
		{
			early_reflection_left = direct_last;
			early_reflection_right = direct_first;
			early_reflection_gain_left = (*this)(early_reflection_gain) * (1 - (*this)(early_reflection_pan_amount));
			early_reflection_gain_right = (*this)(early_reflection_gain);
		}
		else if(early_reflection_delay_stereo_delta_negative)
		{
			early_reflection_left = direct_first;
			early_reflection_right = direct_last;
			early_reflection_gain_left = (*this)(early_reflection_gain);
			early_reflection_gain_right = (*this)(early_reflection_gain) * (1 - (*this)(early_reflection_pan_amount));
		}
		else
		{
			early_reflection_left = direct_first;
			early_reflection_right = direct_first;
			early_reflection_gain_left = (*this)(early_reflection_gain);
			early_reflection_gain_right = (*this)(early_reflection_gain);
		}
		break;
	case late_reflection_gain:
	case late_reflection_pan:
		late_reflection_gain_left = (*this)(late_reflection_gain) * std::min(Real(1), (1 - (*this)(late_reflection_pan)));
		late_reflection_gain_right = (*this)(late_reflection_gain) * std::min(Real(1), (1 + (*this)(late_reflection_pan)));
	}
}

inline void Haas::resize(const Real & delay)
{
	buffer_.resize(1 + static_cast<int>(delay * samples_per_second()), 0);
		 // resizes the buffer at least to 1, the smallest length possible for the algorithm to work
	buffer_iterators_[direct_first] = buffer_.begin();
	buffer_iterators_[direct_last] = buffer_.begin() + static_cast<int>(direct_delay_stereo_delta_abs * samples_per_second());
	buffer_iterators_[early_reflection_first]  = buffer_.begin() + static_cast<int>((*this)(early_reflection_delay) * samples_per_second());
	buffer_iterators_[early_reflection_last]  = buffer_.begin() + static_cast<int>(((*this)(early_reflection_delay) + early_reflection_delay_stereo_delta_abs) * samples_per_second());
	buffer_iterators_[late_reflection] = buffer_.end() - 1;
}

void Haas::process(Sample l [], Sample r [], int samples, int)
{
	switch((*this)[channel_mix])
	{
	case normal:
		for(int sample(0) ; sample < samples ; ++sample)
			process(l[sample], r[sample]);
		break;
	case swapped:
		for(int sample(0) ; sample < samples ; ++sample)
			process(r[sample], l[sample]);
		break;
	case mono:
		for(int sample(0) ; sample < samples ; ++sample)
		{
			process(l[sample], r[sample]);
			l[sample] = r[sample] = l[sample] + r[sample];
		}
		break;
	default:
		throw Exception("bad setting for the channel mix parameter");
	}
}

inline void Haas::process(Sample & left, Sample & right)
{
	const Real mono_input(left + right);
	*buffer_iterators_[direct_first] = mono_input;
	left = static_cast<Sample>
	(
		direct_gain_left * *buffer_iterators_[direct_left] +
		early_reflection_gain_left * *buffer_iterators_[early_reflection_left] +
		late_reflection_gain_left * *buffer_iterators_[late_reflection]
	);
	right = static_cast<Sample>
	(
		direct_gain_right * *buffer_iterators_[direct_right] +
		early_reflection_gain_right * *buffer_iterators_[early_reflection_right] +
		late_reflection_gain_right * *buffer_iterators_[late_reflection]
	);
	for(int stage(0) ; stage < stages ; ++stage)
		if(++buffer_iterators_[stage] == buffer_.end())
			buffer_iterators_[stage] = buffer_.begin();
}

}}
