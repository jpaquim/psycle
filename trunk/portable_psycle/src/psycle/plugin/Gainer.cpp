#include <project.h>
#include <psycle/plugin/plugin.h>
/// \file
/// \brief just a multiplier
namespace psycle { namespace plugin {

class Gainer : public Plugin
{
public:
	virtual void help(std::ostream & out) const throw()
	{
		out << "just a multiplier" << std::endl;
		out << "it's of course compatible with original psycle 1 arguru's gainer" << std::endl;
	}

	enum Parameters { gain };

	static const Information & information() throw()
	{
		static const Information::Parameter parameters [] =
		{
			Information::Parameter::exponential("gain", std::exp(-4.), 1, std::exp(+4.))
		};
		static const Information information(Information::Type::effect, "ayeternal Gainer", "Gainer", "bohan", 1, parameters, sizeof parameters / sizeof *parameters);
		return information;
	}

	virtual void describe(std::ostream & out, const int & parameter) const
	{
		switch(parameter)
		{
		case gain:
			out << std::setprecision(3) << std::setw(6) << (*this)(gain);
			out << " (" << std::setw(6) << 20 * ::log10((*this)(gain)) << " dB)";
			break;
		default:
			Plugin::describe(out, parameter);
		}
	};

	Gainer() : Plugin(information()) {}
	virtual void process(Sample l[], Sample r[], int samples, int);
protected:
	inline void process(Sample &);
};

PSYCLE__PLUGIN__INSTANCIATOR(Gainer);

void Gainer::process(Sample l[], Sample r[], int sample, int)
{
	while(sample--)
	{
		process(l[sample]);
		process(r[sample]);
	}
}

inline void Gainer::process(Sample & sample)
{
	sample *= (*this)(gain);
}

}}
