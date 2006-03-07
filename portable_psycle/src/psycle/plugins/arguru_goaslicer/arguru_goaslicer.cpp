#include <packageneric/pre-compiled.private.hpp>
#include <psycle/plugin_interface.hpp>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>

//////////////////////////////////////////////////////////////////////
// Arguru simple distortion/saturator plugin for PSYCLE

CMachineParameter const paraLength = 
{ 
	"Length",
	"Length",										// description
	1,												// MinValue	
	8192,											// MaxValue
	MPF_STATE,										// Flags
	2048,
};

CMachineParameter const paraSlope = 
{ 
	"Slope",
	"Slope",											// description
	1,												// MinValue	
	2048,											// MaxValue
	MPF_STATE,										// Flags
	512,
};

CMachineParameter const *pParameters[] = 
{ 
	// global
	&paraLength,
	&paraSlope
};

CMachineInfo const MacInfo = 
{
	MI_VERSION,	
	0,										// flags
	2,										// numParameters
	pParameters,							// Pointer to parameters
#ifdef _DEBUG
	"Arguru Goaslicer (Debug build)",		// name
#else
	"Arguru Goaslicer",						// name
#endif
	"Goaslicer",							// short name
	"J. Arguelles",							// author
	"About",								// A command, that could be use for open an editor, etc...
	1
};


class mi : public CMachineInterface
{
public:
	mi();
	virtual ~mi();

	virtual void Init();
	virtual void SequencerTick();
	virtual void Work(float *psamplesleft, float *psamplesright , int numsamples, int tracks);
	virtual bool DescribeValue(char* txt,int const param, int const value);
	virtual void Command();
	virtual void ParameterTweak(int par, int val);

private:

	float m_CurrentVolume;
	float m_TargetVolume;

	int m_Timer;
};

PSYCLE__PLUGIN__INSTANCIATOR(mi, MacInfo)

mi::mi()
{
	m_CurrentVolume=0;
	m_TargetVolume=0;
	m_Timer=0;

	// The constructor zone
	Vals = new int[2];
}

mi::~mi()
{
	delete Vals;
// Destroy dinamically allocated objects/memory here
}

void mi::Init()
{
// Initialize your stuff here
}

void mi::SequencerTick()
{
// Called on each tick while sequencer is playing
m_Timer=0;
}

void mi::Command()
{
// Called when user presses editor button
// Probably you want to show your custom window here
// or an about button
pCB->MessBox("Made 18/5/2000 by Juan Antonio Arguelles Rius for Psycl3!","-=<([aRgUrU's G-o-a-s-l-i-e-r])>=-",0);
}

void mi::ParameterTweak(int par, int val)
{
	Vals[par]=val;
}

// Work... where all is cooked 
void mi::Work(float *psamplesleft, float *psamplesright , int numsamples, int tracks)
{
		
		float const ts=(float)Vals[1]/8192.0f;
		int const barrier=Vals[0];
		int const hik=pCB->GetTickLength();

		do
		{
		float sl = *psamplesleft;
		float sr = *psamplesright;

		if(m_Timer<barrier)
			m_TargetVolume=1.0f;
		else
			m_TargetVolume=0.0f;

		if(++m_Timer>hik)
		m_Timer=0;

		if(m_CurrentVolume<m_TargetVolume)
		{
			m_CurrentVolume+=ts;
			
			if(m_CurrentVolume>m_TargetVolume)
			m_CurrentVolume=m_TargetVolume;
		}

		
		if(m_CurrentVolume>m_TargetVolume)
		{
			m_CurrentVolume-=ts;
			
			if(m_CurrentVolume<m_TargetVolume)
			m_CurrentVolume=m_TargetVolume;
		}

		*psamplesleft=sl*m_CurrentVolume;
		*psamplesright=sr*m_CurrentVolume;

		++psamplesleft;
		++psamplesright;
			
		} while(--numsamples);
}

// Function that describes value on client's displaying
bool mi::DescribeValue(char* txt,int const param, int const value)
{
	return false;
}








/////////////////////////////////////////////////////////////////////////////////////////////////:
/////////////////////////////////////////////////////////////////////////////////////////////////:
/////////////////////////////////////////////////////////////////////////////////////////////////:
// following is an unfinished port to the new plugin interface
#if 0

#include <packageneric/pre-compiled.private.hpp>
#include <psycle/plugin.hpp>
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

#endif // 0
