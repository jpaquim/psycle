/// \file
/// \brief delay
#include <packageneric/pre-compiled.private.hpp>
#include <psycle/plugin.hpp>
#include <cassert>
#include <vector>
namespace psycle { namespace plugin {


class Delay : public Plugin
{
public:
	virtual void help(std::ostream & out) const throw()
	{
		out << "Delay." << std::endl;
		out << "Compatible with original psycle 1 arguru's dala delay." << std::endl;
		out << std::endl;
		out << "Beware if you tweak the delay length with a factor > 2 that the memory buffer gets resized." << std::endl;
	}
	enum Parameters
	{
		dry, wet,
		left_delay, left_feedback,
		right_delay, right_feedback,
		snap
	};

	static const Information & information() throw()
	{
		const int factors(2 * 3 * 4 * 5 * 7);
		const Real delay_maximum(Information::Parameter::input_maximum_value / factors);
		static const Information::Parameter parameters [] =
		{
			Information::Parameter::linear("dry", -1, 1, 1),
			Information::Parameter::linear("wet", -1, 0, 1),
			Information::Parameter::linear("delay left", 0, 0, delay_maximum),
			Information::Parameter::linear("feedback left", -1, 0, 1),
			Information::Parameter::linear("delay right", 0, 0, delay_maximum),
			Information::Parameter::linear("feedback right", -1, 0, 1),
			Information::Parameter::discrete("snap to", 0, factors - 1)
		};
		static const Information information(Information::Type::effect, "ayeternal Dalay Delay", "Dalay Delay", "bohan", 4, parameters, sizeof parameters / sizeof *parameters);
		return information;
	}

	virtual void describe(std::ostream & out, const int & parameter) const
	{
		switch(parameter)
		{
		case left_delay:
		case right_delay:
			out << (*this)(parameter) << " ticks (lines)";
			break;
		case snap:
			if((*this)[parameter] == information().parameter(parameter).maximum_value) out << "off ";
			out << "1 / " << 1 + (*this)[parameter] << " ticks (lines)";
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

	Delay() : Plugin(information()) {}
	virtual void init();
	virtual void process(Sample l [], Sample r [], int samples, int);
	virtual void parameter(const int &);
protected:
	virtual void samples_per_second_changed()
	{
		parameter(left_delay);
		parameter(right_delay);
	}
	virtual void sequencer_ticks_per_second_changed()
	{
		parameter(left_delay);
		parameter(right_delay);
	}
	enum Channels { left, right, channels };
	std::vector<Real> buffers_ [channels];
	std::vector<Real>::iterator buffer_iterators_ [channels];
	inline void process(std::vector<Real> & buffer, std::vector<Real>::iterator & buffer_iterator, Sample & input, const Sample & feedback);
	inline void resize(const int & channel, const int & parameter);
	inline void resize(const int & channel, const Real & delay);
};

PSYCLE__PLUGIN__INSTANCIATOR(Delay)

void Delay::init()
{
	for(int channel(0) ; channel < channels ; ++channel)
		resize(channel, Real(0)); // resizes the buffers not to 0, but to 1, the smallest length possible for the algorithm to work
}

void Delay::parameter(const int & parameter)
{
	switch(parameter)
	{
	case left_delay:
		resize(left, left_delay);
		break;
	case right_delay:
		resize(right, right_delay);
		break;
	}
}

inline void Delay::resize(const int & channel, const int & parameter)
{
	const int snap1((*this)[snap] + 1);
	const Real snap_delay(static_cast<int>((*this)(parameter) * snap1) / static_cast<Real>(snap1));
	this->scaled_parameters_[parameter] = snap_delay;
	this->parameters_[parameter] = information().parameter(parameter).scale.apply_inverse(snap_delay);
	resize(channel, snap_delay);
}

inline void Delay::resize(const int & channel, const Real & delay)
{
	buffers_[channel].resize(1 + static_cast<int>(delay * samples_per_sequencer_tick()), 0);
		 // resizes the buffer at least to 1, the smallest length possible for the algorithm to work
	buffer_iterators_[channel] = buffers_[channel].begin();
}

void Delay::process(Sample l [], Sample r [], int samples, int)
{
	for(int sample(0) ; sample < samples ; ++sample)
	{
		process(buffers_[left] , buffer_iterators_[left] , l[sample], (*this)(left_feedback));
		process(buffers_[right], buffer_iterators_[right], r[sample], (*this)(right_feedback));
	}
}

inline void Delay::process(std::vector<Real> & buffer, std::vector<Real>::iterator & buffer_iterator, Sample & input, const Sample & feedback)
{
	const Real read(*buffer_iterator);
	*buffer_iterator = /*math::renormalized*/(input + feedback * read);
	if(++buffer_iterator == buffer.end()) buffer_iterator = buffer.begin();
	input = static_cast<Sample>((*this)(dry) * input + (*this)(wet) * read);
}

}}
