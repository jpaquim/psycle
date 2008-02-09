/* -*- mode:c++, indent-tabs-mode:t -*- */
// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2003-2007 johan boule <bohan@jabber.org>
// copyright 2003-2007 psycledelics http://psycle.pastnotecut.org

/// \file
/// \brief distortion
#include <packageneric/pre-compiled.private.hpp>
#include "plugin.hpp"

namespace psycle { namespace plugin {

class Distortion : public Plugin
{
public:
	virtual void help(std::ostream & out) const throw()
	{
		out << "distortion ..." << std::endl;
		out << "compatible with psycle 1 arguru's original distortion" << std::endl;
	}

	enum Parameters
	{
		input_gain, output_gain,
		positive_threshold, positive_clamp,
		negative_threshold, negative_clamp,
		symmetric
	};

	enum Symmetric { no, yes };
	
	/* static const Real */ enum { amplitude = 0x7fff };

	static const Information & information() throw()
	{
		static const Information::Parameter parameters [] =
		{
			Information::Parameter::exponential("input gain", ::exp(-4.), 1, ::exp(+4.)),
			Information::Parameter::exponential("output gain", ::exp(-4.), 1, ::exp(+4.)),
			Information::Parameter::linear("positive threshold", 0, +amplitude, +amplitude),
			Information::Parameter::linear("positive clamp", 0, +amplitude, +amplitude),
			Information::Parameter::linear("negative threshold", 0, +amplitude, +amplitude),
			Information::Parameter::linear("negative clamp", 0, +amplitude, +amplitude),
			Information::Parameter::discrete("symmetric", no, yes)
		};
		static const Information information(Information::Types::effect, "ayeternal Dist! Distortion", "Dist!ortion", "bohan", 4, parameters, sizeof parameters / sizeof *parameters);
		return information;
	}

	virtual void describe(std::ostream & out, const int & parameter) const
	{
		switch(parameter)
		{
		case symmetric:
			switch((*this)[symmetric])
			{
			case no:
				out << "no";
				break;
			case yes:
				out << "yes (use positive)";
				break;
			default:
				throw std::string("yes or no ?");
			}
			break;
		case positive_threshold:
		case positive_clamp:
			out << (*this)(parameter) / amplitude;
			break;
		case negative_threshold:
		case negative_clamp:
			out << -(*this)(parameter) / amplitude;
			break;
		case input_gain:
		case output_gain:
			out << std::setprecision(3) << std::setw(6) << (*this)(parameter);
			out << " (" << std::setw(6) << 20 * ::log10((*this)(parameter)) << " dB)";
			break;
		default:
			Plugin::describe(out, parameter);
		}
	}

	Distortion() : Plugin(information()) {}
	virtual void process(Sample l[], Sample r[], int samples, int);
protected:
	inline void process
	(
		Sample & l, Sample & r,
		const Sample & positive_threshold, const Sample & positive_clamp,
		const Sample & negative_threshold, const Sample & negative_clamp
	);
	inline void process
	(
		Sample & sample,
		const Sample & positive_threshold, const Sample & positive_clamp,
		const Sample & negative_threshold, const Sample & negative_clamp
	);
};

PSYCLE__PLUGIN__INSTANCIATOR(Distortion)

void Distortion::process(Sample l[], Sample r[], int sample, int)
{
	switch((*this)[symmetric])
	{
	case no:
		while(sample--)
		{
			process
				(
					l[sample], r[sample],
					(*this)(positive_threshold), (*this)(positive_clamp),
					-(*this)(negative_threshold), -(*this)(negative_clamp)
				);
		}
		break;
	case yes:
		while(sample--)
		{
			process
				(
					l[sample], r[sample],
					(*this)(positive_threshold), (*this)(positive_clamp),
					-(*this)(positive_threshold), -(*this)(positive_clamp)
				);
		}
		break;
	default:
		throw std::string("yes or no ?");
	}
}

inline void Distortion::process
(
	Sample & l, Sample & r,
	const Sample & positive_threshold, const Sample & positive_clamp,
	const Sample & negative_threshold, const Sample & negative_clamp
)
{
	process(l, positive_threshold, positive_clamp, negative_threshold, negative_clamp);
	process(r, positive_threshold, positive_clamp, negative_threshold, negative_clamp);
}

inline void Distortion::process
(
	Sample & sample,
	const Sample & positive_threshold, const Sample & positive_clamp,
	const Sample & negative_threshold, const Sample & negative_clamp
)
{
	sample *= (*this)(input_gain);
	if(sample > positive_threshold) sample = positive_clamp;
	else if(sample < negative_threshold) sample = negative_clamp;
	sample *= (*this)(output_gain);
}

}}
