#include "project.h"
#include "../plugin.h"
///\file
///\brief arguru simple distortion/saturator plugin
namespace psycle { namespace plugin {

class Goaslicer : public Plugin
{
public:
	virtual void help(std::ostream & out) const throw()
	{
		out << "-=<([aRgUrU's G-o-a-s-l-i-e-r])>=-" << std::endl;
		out << "Made 18/5/2000 by Juan Antonio Arguelles Rius for Psycl3!" << std::endl;
		out << "(note: too much 1337)" << std::endl;
	}

	enum Parameters { length, slope };

	static const Information & information() throw()
	{
		static const Information::Parameter parameters [] =
		{
			Information::Parameter::linear("length", 1, 2048, 8192),
			Information::Parameter::linear("slope", 1, 512, 2048)
		};
		static const Information information(Information::Type::effect, "arguru goaslicer", "j arguelles & the psycledelics community", 2, parameters, sizeof parameters / sizeof parameters[0]);
		return information;
	}

	Goaslicer() : Plugin(information()), timer_(0)
	{
		for(int volume(0) ; volume < volumes ; ++volume)
			volumes_[volume] = 0;
	}

	virtual void process(Sample l[], Sample r[], int samples, int);
	virtual void sequencer_tick_event();
protected:
	inline const Real process(const Real & sample) const;
	enum Volumes { current, target, volumes };
	Real volumes_[volumes];
	int timer_;
};

PLUGIN_INSTANCIATOR(Goaslicer);

void Goaslicer::sequencer_tick_event()
{
	timer_ = 0;
}

void Goaslicer::process(Sample l[], Sample r[], int samples, int)
{
	const Real samples_per_sequencer_tick(samples_per_sequencer_tick());
	for(int sample(0) ; sample < samples ; ++sample)
	{
		if(timer_ < (*this)(length)) volumes_[target] = 1;
		else volumes_[target] = 0;
		if(++timer_ > (*this)(slope)) timer_ = 0;
		if(volumes_[current] < volumes_[target])
		{
			volumes_[current] += (*this)(length);
			if(volumes_[current] > volumes_[target]) volumes_[current] = volumes_[target];
		}
		if(volumes_[current] > volumes_[target])
		{
			volumes_[current] -= (*this)(length);
			if(volumes_[current] < volumes_[target]) volumes_[current] = volumes_[target];
		}
		l[sample] = static_cast<Sample>(process(l[sample]));
		r[sample] = static_cast<Sample>(process(r[sample]));
	}
};

inline const Real Goaslicer::process(const Real & sample) const
{
	return sample * volumes_[current];
}

}}
