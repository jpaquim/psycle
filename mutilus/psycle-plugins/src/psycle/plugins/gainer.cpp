// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2003-2007 johan boule <bohan@jabber.org>
// copyright 2003-2007 psycledelics http://psycle.pastnotecut.org

/// \file
/// \brief just a multiplier
#include <packageneric/pre-compiled.private.hpp>
#include "plugin.hpp"
namespace psycle { namespace plugin {

class Gainer : public Plugin
{
	public:

		virtual void help(std::ostream & out) const throw()
		{
			out << "just a multiplier" << std::endl;
		}

		enum Parameters { gain };

		static const Information & information() throw()
		{
			static const Information::Parameter parameters [] =
			{
				Information::Parameter::exponential("gain", std::exp(-4.), 1, std::exp(+4.))
			};
			static const Information information(Information::Types::effect, "ayeternal Gainer", "Gainer", "bohan", 1, parameters, sizeof parameters / sizeof *parameters);
			return information;
		}

		virtual void describe(std::ostream & out, const int & parameter) const
		{
			switch(parameter)
			{
				case gain:
					out << std::setprecision(3) << std::setw(6) << gain_;
					if(gain_) out << " (" << std::setw(6) << 20 * std::log10(gain_) << " dB)";
					break;
				default:
					Plugin::describe(out, parameter);
			}
		};

		Gainer() : Plugin(information()) {}

		virtual void parameter(const int & parameter)
		{
			switch(parameter)
			{
				case gain:
					gain_ = (*this)[gain] ? (*this)(gain) : 0;
					break;
			}
		}

		virtual void process(Sample l[], Sample r[], int sample, int)
		{
			while(sample--)
			{
				process(l[sample]);
				process(r[sample]);
			}
		}

	protected:

		inline void process(Sample & sample)
		{
			sample *= gain_;
		}

		Real gain_;
};

PSYCLE__PLUGIN__INSTANCIATOR(Gainer)

}}
