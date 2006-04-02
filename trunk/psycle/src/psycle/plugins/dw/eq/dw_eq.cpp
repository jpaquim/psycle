#include <packageneric/pre-compiled.private.hpp>
#include <psycle/plugin_gui.hpp>

#include "../dw_filter.hpp"


///////////////////////////////////
// dw eq

// while i have tested the eq filters, i haven't been able to actually listen to this plugin yet,
// since the current state of the svn sources doesn't allow machine connections

//todo : currently, i have it set such that one display node can't pass another on the x axis by dragging.. but
//			there's no such check on the frequency knobs.  i need to either let people put their bands anywhere
//			without restriction, or restrict the knobs as well.
//		i also need to come up with a way to convert the linear parameter values into logarithmic scale, for both freq and gain.
//		also! it needs to be made quite a bit prettier :)  fix the display grid on init, arrange and size things better, make the eq
//			curve actually look like an eq curve, etc



#ifndef M_PI
 #define M_PI 3.14159265358979323846
#endif
#define GRID_SIZE_X 270
#define GRID_SIZE_Y 110

#define SLOPE_SCALER	1000
#define GAIN_SCALER	100

#define MIN_FREQ	20
#define MIN_GAIN	0.125 * GAIN_SCALER
#define MIN_SLOPE	.01 * SLOPE_SCALER

#define MAX_FREQ	20000
#define MAX_GAIN	8 * GAIN_SCALER
#define MAX_SLOPE	M_PI/3.0f * SLOPE_SCALER


CMachineParameter const Gain =	{ "Gain",	"Gain",				MIN_GAIN,		MAX_GAIN,		MPF_STATE,		GAIN_SCALER			};
CMachineParameter const Freq1 =	{ "Freq",	"Frequency",		MIN_FREQ,		MAX_FREQ,		MPF_STATE,		120					};
CMachineParameter const Freq2 =	{ "Freq",	"Frequency",		MIN_FREQ,		MAX_FREQ,		MPF_STATE,		1200				};
CMachineParameter const Freq3 =	{ "Freq",	"Frequency",		MIN_FREQ,		MAX_FREQ,		MPF_STATE,		6000				};
CMachineParameter const Freq4 =	{ "Freq",	"Frequency",		MIN_FREQ,		MAX_FREQ,		MPF_STATE,		15000				};
CMachineParameter const EndSlope={ "Slope",	"Slope",			0,				MAX_SLOPE,		MPF_STATE,		0					};	//0 slope = shelf
CMachineParameter const Slope = { "Slope",	"Slope",			MIN_SLOPE,		MAX_SLOPE,		MPF_STATE,		MAX_SLOPE/2			};
CMachineParameter const Null	={	"",				"",				0,				0,			MPF_LABEL,		0					};


enum
{
	prm_freq1=0,prm_freq2,	prm_freq3,	prm_freq4,
	prm_gain1,	prm_gain2,	prm_gain3,	prm_gain4,
	prm_slope1,	prm_slope2,	prm_slope3,	prm_slope4,
	prm_grid,
	prm_bgr,
	num_params
};

enum
{
	chan_left=0, chan_right
};

std::map<int, CMachineGuiParameter**> InstMap;


CMachineParameter const *pParameters[]=
{
	&Freq1,&Freq2,&Freq3,&Freq4,
	&Gain, &Gain, &Gain, &Gain,
	&EndSlope, &Slope, &Slope, &EndSlope,
	&Null,
	&Null
};

CMachineInfo const MacInfo = 
{
	MI_VERSION,	
	USEGUI,								// flags
	num_params,								// numParameters
	pParameters,					// Pointer to parameters
#ifndef NDEBUG
	"dw eq (Debug build)",		// name
#else
	"dw eq",						// name
#endif
	"eq",						// short name
	"dw",							// author
	"About",						// A command, that could be use for open an editor, etc...
	2
};



class mi : public CMachineInterface
{
public:
	mi();
	mi(int index);
	virtual ~mi();
	virtual void Init();
	virtual void SequencerTick();
	virtual void Work(float *psamplesleft, float *psamplesright , int numsamples, int tracks);
	virtual bool DescribeValue(char* txt,int const param, int const value);
	virtual void Command();
	virtual void ParameterTweak(int par, int val);

private:
	CMachineGuiParameter * Params[num_params];
	CMachineGuiParameter  *yParams[4], *xParams[4];
	int xParamNums[4], yParamNums[4];
	CMachineXYMultiNode * prmGrid;
	CBitmap *bgr;

	int machNum;
	dwfilter band1, band2, band3, band4;
	int samprate;


};

PSYCLE__GUIPLUGIN__INSTANCIATOR(mi, MacInfo, InstMap)

mi::mi()
{
}

mi::mi(int index)
{

		
	Params[prm_freq1]= new CMachineKnob(Freq1,		60,		152);
	Params[prm_freq2]= new CMachineKnob(Freq2,		120,	152);
	Params[prm_freq3]= new CMachineKnob(Freq3,		180,	152);
	Params[prm_freq4]= new CMachineKnob(Freq4,		240,	152);
	Params[prm_gain1]= new CMachineKnob(Gain,		60,		203);
	Params[prm_gain2]= new CMachineKnob(Gain,		120,	203);
	Params[prm_gain3]= new CMachineKnob(Gain,		180,	203);
	Params[prm_gain4]= new CMachineKnob(Gain,		240,	203);
	Params[prm_slope1]=new CMachineKnob(EndSlope,	60,		254);
	Params[prm_slope2]=new CMachineKnob(Slope,		120,	254);
	Params[prm_slope3]=new CMachineKnob(Slope,		180,	254);
	Params[prm_slope4]=new CMachineKnob(EndSlope,	240,	254);

	for(int i(1);i<4;++i)
	{
		Params[i]->showLabel=off;		//turn off labels for all parameters except first column
		Params[i+4]->showLabel=off;
		Params[i+8]->showLabel=off;
	}
	for(int i=0;i<=8;i+=4)				//set labels for first column to left side of control
		Params[i]->showLabel=left;

	for(int i=0;i<4;i++)
	{
		xParams[i]=Params[i];
		yParams[i]=Params[i+4];
		xParamNums[i]=i;
		yParamNums[i]=i+4;
	}

	Params[prm_grid]=new CMachineXYMultiNode(	Null, 10, 10, 
												10+GRID_SIZE_X, 10+GRID_SIZE_Y,
												xParams, yParams, 
												xParamNums, yParamNums, 4);

	prmGrid=(CMachineXYMultiNode*)Params[prm_grid];

	bgr = new CBitmap;
	bgr->LoadBitmap(IDB_TREMOLOBG); //same size as guitremolo..
	Params[prm_bgr]=new CMachineBackground(Null, bgr,	300,	308);

	Vals = new int[sizeof(pParameters)];

	for(int i(0);i<num_params;++i)
	{
		Params[i]->value=&(Vals[i]);

	}

	machNum=index;
	InstMap[machNum]=Params;
}


mi::~mi()
{
	delete Vals;
	bgr->DeleteObject();
	delete bgr;
	for(int i(0);i<num_params;++i)
		delete Params[i];
	InstMap.erase(machNum);
}

void mi::Init()
{
	prmGrid->ClearData(0x00000000);
/*
	prmGrid->Line(CPoint(0,GRID_SIZE_Y/2),					prmGrid->GetNodeLocation(0), 0x00EEEEEE, 3);
	prmGrid->Line(prmGrid->GetNodeLocation(0),				prmGrid->GetNodeLocation(1), 0x00EEEEEE, 3);
	prmGrid->Line(prmGrid->GetNodeLocation(1),				prmGrid->GetNodeLocation(2), 0x00EEEEEE, 3);
	prmGrid->Line(prmGrid->GetNodeLocation(2),				CPoint(GRID_SIZE_X-1,GRID_SIZE_Y/2), 0x00EEEEEE, 3);

	prmGrid->nodes[0].SetMaxX(prmGrid->GetNodeLocation(1).x);
	prmGrid->nodes[1].SetMinX(prmGrid->GetNodeLocation(0).x);
	prmGrid->nodes[1].SetMaxX(prmGrid->GetNodeLocation(2).x);
	prmGrid->nodes[2].SetMinX(prmGrid->GetNodeLocation(1).x);
*/
//	samprate = pCB->GetSamplingRate();
	band1.SetSampleRate(samprate);
	band2.SetSampleRate(samprate);
	band3.SetSampleRate(samprate);
	band4.SetSampleRate(samprate);

	band1.SetMode(3);
	band2.SetMode(1);
	band3.SetMode(1);
	band4.SetMode(2);


}


void mi::SequencerTick()
{
// Called on each tick while sequencer is playing
}

void mi::Command()
{
// Called when user presses editor button
	pCB->MessBox("dw eq v0.0alpha\n\nby d.w. aley","uNF",0);
}



void mi::ParameterTweak(int par, int val)
{
	switch(par)
	{
	case prm_freq1:	band1.SetFreq(val);
					break;
	case prm_freq2:	band2.SetFreq(val);
					break;
	case prm_freq3:	band3.SetFreq(val);
					break;
	case prm_freq4:	band4.SetFreq(val);
					break;

	case prm_slope2:band2.SetBW(val/(float)SLOPE_SCALER);
					break;
	case prm_slope3:band3.SetBW(val/(float)SLOPE_SCALER);
					break;

	case prm_gain1: band1.SetGain(val/(float)GAIN_SCALER);
					break;
	case prm_gain2: band2.SetGain(val/(float)GAIN_SCALER);
					break;
	case prm_gain3: band3.SetGain(val/(float)GAIN_SCALER);
					break;
	case prm_gain4: band4.SetGain(val/(float)GAIN_SCALER);
					break;

	case prm_slope1:if(val<MIN_SLOPE)
						band2.SetMode(eq_loshelf);
					else
						band1.SetMode(eq_parametric);
					band1.SetBW(val/(float)SLOPE_SCALER);
					break;
	case prm_slope4:if(val<MIN_SLOPE)
						band4.SetMode(eq_hishelf);
					else
						band4.SetMode(eq_parametric);
					band4.SetBW(val/(float)SLOPE_SCALER);
					break;
	}

	Vals[par]=val;

	prmGrid->ClearData(0x00000000);
	prmGrid->Line(CPoint(0,GRID_SIZE_Y/2),					prmGrid->GetNodeLocation(0), 0x00EEEEEE, 3);
	prmGrid->Line(prmGrid->GetNodeLocation(0),				prmGrid->GetNodeLocation(1), 0x00EEEEEE, 3);
	prmGrid->Line(prmGrid->GetNodeLocation(1),				prmGrid->GetNodeLocation(2), 0x00EEEEEE, 3);
	prmGrid->Line(prmGrid->GetNodeLocation(2),				prmGrid->GetNodeLocation(3), 0x00EEEEEE, 3);
	prmGrid->Line(prmGrid->GetNodeLocation(3),				CPoint(GRID_SIZE_X-1,GRID_SIZE_Y/2), 0x00EEEEEE, 3);

	prmGrid->nodes[0].SetMaxX(prmGrid->GetNodeLocation(1).x);
	prmGrid->nodes[1].SetMinX(prmGrid->GetNodeLocation(0).x);
	prmGrid->nodes[1].SetMaxX(prmGrid->GetNodeLocation(2).x);
	prmGrid->nodes[2].SetMinX(prmGrid->GetNodeLocation(1).x);
	prmGrid->nodes[2].SetMaxX(prmGrid->GetNodeLocation(3).x);
	prmGrid->nodes[3].SetMinX(prmGrid->GetNodeLocation(2).x);

}

void mi::Work(float *psamplesleft, float *psamplesright , int numsamples, int tracks)
{

	do
	{

		if(prm_gain1!=GAIN_SCALER) //actually '1 * GAIN_SCALER'-- i.e. unity gain
		{
			*psamplesleft  = band1.Process(*psamplesleft, chan_left);
			*psamplesright = band1.Process(*psamplesright, chan_right);
		}
		if(prm_gain2!=GAIN_SCALER)
		{
			*psamplesleft  = band2.Process(*psamplesleft, chan_left);
			*psamplesright = band2.Process(*psamplesright, chan_right);
		}
		if(prm_gain3!=GAIN_SCALER)
		{
			*psamplesleft  = band3.Process(*psamplesleft, chan_left);
			*psamplesright = band3.Process(*psamplesright, chan_right);
		}
		if(prm_gain4!=GAIN_SCALER)
		{
			*psamplesleft  = band4.Process(*psamplesleft, chan_left);
			*psamplesright = band4.Process(*psamplesright, chan_right);
		}


		++psamplesleft;
		++psamplesright;

	} while(--numsamples);

}
bool mi::DescribeValue(char* txt,int const param, int const value)
{
	switch(param)
	{
	case prm_freq1:
	case prm_freq2:
	case prm_freq3:
	case prm_freq4:		sprintf(txt, "%i Hz", value);
						return true;
	case prm_slope1:	if(value<MIN_SLOPE)
							sprintf(txt, "low shelf");
						else
							sprintf(txt, "%.2f octaves", value/(float)SLOPE_SCALER);
						return true;
	case prm_slope4:	if(value<MIN_SLOPE)
							sprintf(txt, "high shelf");
						else
							sprintf(txt, "%.2f octaves", value/(float)SLOPE_SCALER);
						return true;
	case prm_slope2:
	case prm_slope3:		sprintf(txt, "%.2f octaves", value/(float)SLOPE_SCALER);
						return true;
	case prm_gain1:
	case prm_gain2:
	case prm_gain3:
	case prm_gain4:		sprintf(txt, "%.2f dB", 20 * log10((float)value / (float)GAIN_SCALER));
						return true;
	}
	return false;
}
