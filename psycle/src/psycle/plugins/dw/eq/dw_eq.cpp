#include <packageneric/pre-compiled.private.hpp>
#include <psycle/plugin.hpp>

#include "../dw_filter.hpp"

#ifndef M_PI
 #define M_PI 3.14159265358979323846
#endif

namespace psycle { namespace plugin {

///////////////////////////////////
// dw eq


class dw_eq : public Plugin
{
public:
	virtual void help(std::ostream & out) const throw()
	{
		out << "dw eq v0.1" << std::endl;
		out << "parametric eq plugin by d.w.aley" << std::endl;
	}

	enum
	{
		prm_freq1=0,prm_freq2,	prm_freq3,	prm_freq4,
		prm_gain1,	prm_gain2,	prm_gain3,	prm_gain4,
		prm_slope1,	prm_slope2,	prm_slope3,	prm_slope4,
		num_params
	};

	enum
	{
		chan_left=0, chan_right
	};


	static const Information & information() throw()
	{
		static const Information::Parameter parameters [] =
		{
			Information::Parameter::exponential("Freq 1", 20, 120,		20000),
			Information::Parameter::exponential("Freq 2", 20, 800,		20000),
			Information::Parameter::exponential("Freq 3", 20, 2500,		20000),
			Information::Parameter::exponential("Freq 4", 20, 6000,		20000),
			Information::Parameter::exponential("Gain 1", 0.125,	1,		8.0),
			Information::Parameter::exponential("Gain 2", 0.125,	1,		8.0),
			Information::Parameter::exponential("Gain 3", 0.125,	1,		8.0),
			Information::Parameter::exponential("Gain 4", 0.125,	1,		8.0),
			Information::Parameter::linear("Bandwidth 1", 0,		0,			M_PI/3.0f),
			Information::Parameter::linear("Bandwidth 2", .01,		M_PI/6.0f,	M_PI/3.0f),
			Information::Parameter::linear("Bandwidth 3", .01,		M_PI/6.0f,	M_PI/3.0f),
			Information::Parameter::linear("Bandwidth 4", 0,		0,			M_PI/3.0f)
		};

		static const Information information(
			Information::Types::effect
#ifndef NDEBUG
			, "dw eq (debug build)"
#else
			, "dw eq"
#endif
			, "eq"
			, "dw"
			, 3
			, parameters
			, sizeof parameters / sizeof *parameters);
		return information;
	}

	virtual void describe(std::ostream & out, const int & parameter) const
	{
		out.setf(std::ios::fixed);
		out<<std::setw(6)<<std::setprecision(3);
		switch(parameter)
		{
		case prm_freq1:		out<<(*this)(prm_freq1)<<" Hz";
							break;
		case prm_freq2:		out<<(*this)(prm_freq2)<<" Hz";
							break;
		case prm_freq3:		out<<(*this)(prm_freq3)<<" Hz";
							break;
		case prm_freq4:		out<<(*this)(prm_freq4)<<" Hz";
							break;
		case prm_slope1:	if((*this)(prm_slope1)==0)
								out<<"Low Shelf";
							else
								out<<(*this)(prm_slope1)<< " octaves";
							break;
		case prm_slope4:	if((*this)(prm_slope4)==0 )
								out<<"Hi Shelf";
							else
								out<<(*this)(prm_slope4)<< " octaves";
							break;
		case prm_slope2:	out<<(*this)(prm_slope2)<< " octaves";
							break;
		case prm_slope3:	out<<(*this)(prm_slope3)<< " octaves";
							break;

		//todo: gain display oddity: "-0.000"
		//round to 0 when within (-0.001, +0.001) (the channel will be effectively bypassed anyways)
		case prm_gain1:		out<< 20 * log10( (*this)(prm_gain1) ) << " dB";
							break;
		case prm_gain2:		out<< 20 * log10( (*this)(prm_gain2) ) << " dB";
							break;
		case prm_gain3:		out<< 20 * log10( (*this)(prm_gain3) ) << " dB";
							break;
		case prm_gain4:		out<< 20 * log10( (*this)(prm_gain4) ) << " dB";
							break;
		default:
			Plugin::describe(out, parameter);
		}
	}

	dw_eq() : Plugin(information())
	{
		band1.SetSampleRate(samprate);
		band2.SetSampleRate(samprate);
		band3.SetSampleRate(samprate);
		band4.SetSampleRate(samprate);

		band1.SetMode(dwfilter::eq_loshelf);
		band2.SetMode(dwfilter::eq_parametric);
		band3.SetMode(dwfilter::eq_parametric);
		band4.SetMode(dwfilter::eq_hishelf);
	}

	virtual void process(Sample l[], Sample r[], int samples, int);
	virtual void parameter(const int &);
protected:
	virtual void samples_per_second_changed() { band1.SetSampleRate(samples_per_second());
												band2.SetSampleRate(samples_per_second());
												band3.SetSampleRate(samples_per_second());
												band4.SetSampleRate(samples_per_second());	}
	virtual void sequencer_ticks_per_second_changed() {}

	dwfilter band1, band2, band3, band4; //this is silly, i should use an array
	int samprate;
	bool active[4];	//used to bypass eq channels with gain==0
};

PSYCLE__PLUGIN__INSTANCIATOR(dw_eq)



void dw_eq::parameter(const int & param)
{
	switch(param)
	{
	case prm_freq1:	band1.SetFreq((*this)(param));
					break;
	case prm_freq2:	band2.SetFreq((*this)(param));
					break;
	case prm_freq3:	band3.SetFreq((*this)(param));
					break;
	case prm_freq4:	band4.SetFreq((*this)(param));
					break;

	case prm_slope2:band2.SetBW((*this)(param));
					break;
	case prm_slope3:band3.SetBW((*this)(param));
					break;

	case prm_gain1: band1.SetGain((*this)(param));
					if(abs((*this)(param) - 1.0) >=.001)	//parameter never makes it to exactly 1.0
						active[0]=true;
					else
						active[0]=false;
					break;
	case prm_gain2: band2.SetGain((*this)(param));
					if(abs((*this)(param) - 1.0) >=.001)	//parameter never makes it to exactly 1.0
						active[1]=true;
					else
						active[1]=false;
					break;
	case prm_gain3: band3.SetGain((*this)(param));
					if(abs((*this)(param) - 1.0) >=.001)	//parameter never makes it to exactly 1.0
						active[2]=true;
					else
						active[2]=false;
					break;
	case prm_gain4: band4.SetGain((*this)(param));
					if(abs((*this)(param) - 1.0) >=.001)	//parameter never makes it to exactly 1.0
						active[3]=true;
					else
						active[3]=false;
					break;

	case prm_slope1:if((*this)(param)==0)
						band1.SetMode(dwfilter::eq_loshelf);
					else
					{
						band1.SetMode(dwfilter::eq_parametric);
						band1.SetBW((*this)(param));
					}
					break;
	case prm_slope4:if((*this)(param)==0)
						band4.SetMode(dwfilter::eq_hishelf);
					else
					{
						band4.SetMode(dwfilter::eq_parametric);
						band4.SetBW((*this)(param));
					}
					break;
	}
}

void dw_eq::process(Sample l[], Sample r[], int samples, int)
{
	do
	{
		if(active[0])
		{
			*l = band1.Process(*l, chan_left);
			*r = band1.Process(*r, chan_right);
		}
		if(active[1])
		{
			*l = band2.Process(*l, chan_left);
			*r = band2.Process(*r, chan_right);
		}
		if(active[2])
		{
			*l = band3.Process(*l, chan_left);
			*r = band3.Process(*r, chan_right);
		}
		if(active[3])
		{
			*l = band4.Process(*l, chan_left);
			*r = band4.Process(*r, chan_right);
		}
		++l;
		++r;
	} while(--samples);
}


} }