#include <project.h>
#include <psycle/plugin/plugin.h>
#include <exception>
/// \file
/// \brief a crash on purpose plugin
namespace psycle { namespace plugin {

class Crasher : public Plugin
{
public:
	virtual void help(std::ostream & out) const throw()
	{
		out << "This is a crash on purpose plugin for helping devers make the host crash proof." << std::endl;
	}

	static const Information & information() throw()
	{
		static const Information information(Information::Type::effect, "crasher", "crasher", "bohan", 1, 0, 0);
		return information;
	}

	Crasher() : Plugin(information())
	{
	}

	virtual void init()
	{
	}

	virtual void process(Sample l[], Sample r[], int samples, int);
protected:
	inline void process(Sample &);
	void crash() throw(...)
	{
		// c++ exception:
		//throw "crash on purpose!";
		throw std::runtime_error("crash on purpose!");

		// division by 0:
		//volatile int i(0); i = 0 / i; // trick so that the compiler does not remove the code when optimizing

		// infinite loop so that we can test interruption signal:
		//while(true);
	}
};

PSYCLE__PLUGIN__INSTANCIATOR(Crasher);

void Crasher::process(Sample l[], Sample r[], int sample, int)
{
	while(sample--)
	{
		process(l[sample]);
		process(r[sample]);
	}
	crash(); ///////////////////////// <--- crash !!!
}

inline void Crasher::process(Sample & sample)
{
	sample = -sample;
}

}}
