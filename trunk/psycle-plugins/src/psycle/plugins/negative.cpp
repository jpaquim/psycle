/* -*- mode:c++, indent-tabs-mode:t -*- */
// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2003-2007 dan peddle <dazzled@retropaganda.info>
// copyright 2003-2007 johan boule <bohan@jabber.org>
// copyright 2003-2007 psycledelics http://psycle.pastnotecut.org

/// \file
/// \brief just a negative
#include <packageneric/pre-compiled.private.hpp>
#include "plugin.hpp"
namespace psycle { namespace plugin {

class Negative : public Plugin
{
public:
	virtual void help(std::ostream & out) const throw()
	{
		out << "just a Negative (out = -in)" << std::endl;
	}
	static const Information & information() throw()
	{
		static const Information information(Information::Types::effect, "Negative", "Negative", "who cares", 1, 0, 0);
		return information;
	}
	Negative() : Plugin(information()) {}
	virtual void process(Sample l[], Sample r[], int samples, int);
protected:
	inline void process(Sample &);
};

PSYCLE__PLUGIN__INSTANCIATOR(Negative)

void Negative::process(Sample l[], Sample r[], int sample, int)
{
	while(sample--)
	{
		process(l[sample]);
		process(r[sample]);
	}
}

inline void Negative::process(Sample & sample)
{
	sample = -sample;
}

}}
