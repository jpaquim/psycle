//																dw tremolo, with the new gui

// since this is the first plugin to use the new gui code, i'm commenting it pretty heavily to help other people convert their own
// plugins or make new ones.

//one important note: since the plugin-gui uses mfc, the .pch file for all gui plugin projects needs to be pre_compiled_headers.mfc.pch, 
// -not- pre_compiled_headers.pch.  also, the pdb should be changed to pre_compiled_headers.mfc.pdb
//it won't compile if you don't do this!!

#include <packageneric/pre-compiled.private.hpp>
//#include <psycle/plugin_interface.hpp> //plugin_gui.hpp includes plugin_interface, so it's no longer necessary
#include <psycle/plugin_gui.hpp>
#include "resources.hpp"


#define MAGIC_COLOR 0x00859296

#ifndef M_PI
	#define M_PI 3.14159265358979323846
#endif

#define LFO_SIZE				720

#define DEPTH_DIV				1000
#define MIN_SPEED   16
#define MAX_DEPTH				2000
#define MAX_SPEED				10000
#define MAX_WAVES				3
#define MAX_SKEW				LFO_SIZE
#define MAX_PHASE				LFO_SIZE
#define MAX_GRAVITY 200												// this can't be changed without other updates, be careful

#define MAPSIZE								4096
#define DISPLAY_REFRESH				882

#define DISP_SIZE_X 180
#define DISP_SIZE_Y 60



//every CMachineGuiParameter is built off of a CMachineParameter, even bitmaps and backgrounds. also, an array of pointers to
//these classes is still needed in the plugin's CMachineInfo class.
CMachineParameter const paramDepth =				{ "Depth",								"Tremolo Depth",				0,								MAX_DEPTH,								MPF_STATE,								MAX_DEPTH/2				};
CMachineParameter const paramSpeed =				{ "Speed",								"Tremolo Speed",				MIN_SPEED,				MAX_SPEED,				MPF_STATE,								MAX_SPEED/6				};
CMachineParameter const paramWaveform =				{ "Waveform",				"lfo waveform",								0,								MAX_WAVES-1,				MPF_STATE,								0				};
CMachineParameter const paramSkew =								{ "P.Width/Skew",				"lopsidedness",				10,								MAX_SKEW-10,				MPF_STATE,								MAX_SKEW/2				};
CMachineParameter const paramGravity =				{ "Gravity",				"Gravity",												0,								MAX_GRAVITY,				MPF_STATE,								100				};
CMachineParameter const paramGravMode = { "Gravity Mode","Gravity Mode",				0,								1,																MPF_STATE,								0				};
CMachineParameter const paramStereoPhase={ "Stereo Phase",				"Stereo Phase",				0,								MAX_PHASE,								MPF_STATE,								MAX_PHASE*3/4				};
CMachineParameter const paramSync				=				{"Restart LFO",				"Restart LFO",								0,								1,																MPF_STATE,								0				};
CMachineParameter const paramNull				=				{"",												"",																				0,								0,																MPF_LABEL,								0				};


enum
{
	prm_depth=0,				prm_waveform,								prm_gravity,								prm_skew,
	prm_speed,								prm_stereophase,				prm_gravmode,								prm_sync,
	prm_display, 
	prm_bgr,
	num_params
};

enum
{
	lfo_sine=0,
	lfo_tri,
	lfo_square
};

enum
{				
	grav_updown=0,
	grav_inout
};

std::map<int, CMachineGuiParameter**> InstMap;				//this map keeps track of all the parameter pointers for every instance of the plugin.
												//the key value is the index of the machine, which is sent to the constructor of the
												//CMachineInterface class.  the constructor should save the index also, to erase it
												//from the map when an instance is destroyed.  (this is necessary because gui
												//parameter classes contain instance-specific pointers and values, which the old
												//parameters didn't have-- therefore, plugin instances can't share the same parameters with
												//eachother)


CMachineParameter const *pParameters[] =								//this array should be filled with the CMachineParameters that the guiparameters are
{																																																//based off of.  the guiparameters in the map are used for most things, but for now there
	&paramDepth,																																//are still some functions that use these pointers to get parameter ranges and names
	&paramWaveform,																																//and such.
	&paramGravity,
	&paramSkew,
	&paramSpeed,
	&paramStereoPhase,
	&paramGravMode,
	&paramSync,
	&paramNull,
	&paramNull
};

CMachineInfo const MacInfo = 
{
	MI_VERSION,				

	USEGUI,																																//<-- make sure you set the USEGUI flag here, or else everything will fall apart.

	num_params,																																// numParameters
	pParameters,																				// Pointer to parameters
#ifndef NDEBUG
	"dw GuiTremolo (Debug build)",								// name
#else
	"dw GuiTremolo",																				// name
#endif
	"GuiTremolo",																								// short name
	"dw",																												// author
	"About",																								// A command, that could be use for open an editor, etc...
	2
};
//a note on the numcols variable and sizing in general:  the size of the window is automatically calculated based on the positions of the
//gui parameters.  the smallest size possible that will fit every parameter is chosen, in increments of 28x150 (the size of the old parameters).
//currently, the numcols variable in the CMachineInfo class provides a starting point for the calculations, but that will probably be deemed
//unnecessary and may change.


class mi : public CMachineInterface
{
public:
	mi(){};
	virtual ~mi();
	virtual void Init();
	virtual void SequencerTick();
	virtual void Work(float *psamplesleft, float *psamplesright , int numsamples, int tracks);
	virtual bool DescribeValue(char* txt,int const param, int const value);
	virtual void Command();
	virtual void ParameterTweak(int par, int val);
	void fillLfo();


protected:
	float lfo[LFO_SIZE];
	float lfopos;

	float srMultiplier;

	float inCurve[MAPSIZE+2];
	float outCurve[MAPSIZE+2];
	float lowCurve[MAPSIZE+2];
	float highCurve[MAPSIZE+2];												// later we scale from 0 (not 1) to MAPSIZE, which is why there's +1.. we make it +2 just 
										//  in case (we'll init it to 1.0f later)
	int disp_counter;

/*
	CMachineGuiParameter *Params[num_params];				//this is the array of params we'll copy into InstMap in the constructor.
	int machineIndex;
	CMachineDisplay *Display;																				//since we're using functions the display class has that aren't in
												//CMachineGuiParameter, this pointer is initialized as a casted alias for
												//Params[prm_display].  we could've just casted it every time we call one
*/																																																//of those functions, also.
};


//PSYCLE__GUIPLUGIN__INSTANCIATOR(mi, MacInfo, InstMap)				//note that it's GUIPLUGIN, not PLUGIN, and our instance map is the third parameter
PSYCLE__PLUGIN__INSTANCIATOR(mi, MacInfo)

mi::mi(int index)
{
	Vals = new int[sizeof pParameters];
	Vals[prm_skew] = MAX_SKEW/2;
	Vals[prm_gravity] = 100;

	//load the background bitmap..
	bgr = new CBitmap;
	bgr->LoadBitmap(IDB_TREMOLOBGR);
	//initialize our parameters..
	Params[0] = new CMachineVSlider								(paramDepth,								187,				30,								84);
	Params[1] = new CMachineComboBox				(paramWaveform,								55,								135,				40);
	Params[2] = new CMachineVSlider								(paramGravity,								82,								218,				42);
	Params[3] = new CMachineHSlider								(paramSkew,												112,				186,				158);
	Params[4] = new CMachineVSlider								(paramSpeed,								241,				30,								84);
	Params[5]=  new CMachineKnob								(paramStereoPhase,				35,								55				);
	Params[6] = new CMachineVSwitchBank				(paramGravMode,								20,								233,				2);
	Params[7] = new CMachineButton								(paramSync,												110,				60				);
	Params[8] = new CMachineDisplay								(paramNull,												112,				218,				DISP_SIZE_X, DISP_SIZE_Y);
	Params[9] = new CMachineBackground				(paramNull, bgr,				300,				308);

	
	//in order to know how to draw itself, every parameter has a pointer called 'value' that points
	//to the corresponding Vals[] index.
	for(int i(0);i<num_params;++i)
		Params[i]->value=&(Vals[i]);



	//initialize stuff to values we want
	Params[prm_gravmode]->showValue=top;				//showValue and showLabel can be off, top, right, left, or bottom.
	Params[prm_gravmode]->showLabel=off;				//the default for most controls is showLabel=top, showValue=bottom.
	Params[prm_gravmode]->lblTextColor				=				0x00000000;								//change the label text colors to match the
	Params[prm_gravmode]->lblBkColor =								MAGIC_COLOR;				//background. default colors for both label and value
	Params[prm_stereophase]->lblTextColor=				0x00000000;								//are black bg, white text.
	Params[prm_stereophase]->lblBkColor =				MAGIC_COLOR;
	Params[prm_speed]->lblTextColor =								0x00000000;
	Params[prm_speed]->lblBkColor =								MAGIC_COLOR;
	Params[prm_depth]->lblTextColor =								0x00000000;
	Params[prm_depth]->lblBkColor =								MAGIC_COLOR;
	Params[prm_sync]->lblTextColor =								0x00000000;
	Params[prm_sync]->lblBkColor =												MAGIC_COLOR;
	Params[prm_gravity]->lblTextColor =				0x00000000;
	Params[prm_gravity]->lblBkColor =								MAGIC_COLOR;
	Params[prm_waveform]->lblTextColor =				0x00000000;
	Params[prm_waveform]->lblBkColor =								MAGIC_COLOR;
	Params[prm_skew]->lblTextColor =								0x00000000;
	Params[prm_skew]->lblBkColor =												MAGIC_COLOR; 
	Params[prm_sync]->showValue=top;
	static_cast<CMachineVSlider*>(Params[prm_gravity])->bReversed=true;

	Display = static_cast<CMachineDisplay*>(Params[prm_display]);

	//add this instance to the map
	InstMap[index]=Params;
	machineIndex=index;
}

mi::~mi()
{
	delete Vals;
	bgr->DeleteObject();
	delete bgr;

	//remove this instance from the map, and delete our parameters
	InstMap.erase(machineIndex);
	for(int i(0);i<num_params;++i)
		delete Params[i];
}

void mi::Init()
{
	lfopos=0;
	srMultiplier = 44100/(float)pCB->GetSamplingRate();								// this coefficient changes the increment to lfopos..  timing is defined by samples,
															// so without it playing the same song in 22050 will halve the lfo speed.
	float j;
	for(int i=0;i<=MAPSIZE;++i)
	{
		j = i / (float)MAPSIZE;
		highCurve[i] = sqrt((float)(-j*j + 2*j));								//sqrt is ambiguous without the cast
		lowCurve[MAPSIZE - i] = 1.0f - highCurve[i];
	}
	for(int i=0;i<=MAPSIZE/2;++i)
	{
		j = i / (float)(MAPSIZE / 2);
		inCurve[i] = sqrt((float)(-j*j + 2*j)) / 2.0f;
		inCurve[MAPSIZE + 1 - i]				= 1.0f - inCurve[i];
		outCurve[MAPSIZE/2-i]				= .5f - inCurve[i]; 
		outCurve[MAPSIZE/2+i+1]				= .5f + inCurve[i];
	}
	highCurve[MAPSIZE+1] = lowCurve[MAPSIZE+1] = inCurve[MAPSIZE+1] = outCurve[MAPSIZE+1] = 1.0f;

}

void mi::SequencerTick()
{
// Called on each tick while sequencer is playing

	float newsr = 44100/(float)pCB->GetSamplingRate();								// in case samprate is changed.. but this should probably go somewhere else.
	if(newsr!=srMultiplier)																																				// it doesn't need to happen this frequently-- plus, it won't register until
		srMultiplier=newsr;																																				// an actual song is playing

}

void mi::Command()
{
// Called when user presses editor button
	pCB->MessBox("dw Tremolo v. 0.1g\n\nby d.w. aley","uNF",0);
}



void mi::ParameterTweak(int par, int val)
{

	Vals[par]=val;
	if								(par == prm_skew 
			|| par == prm_waveform
			|| par == prm_gravity
			|| par == prm_gravmode)
		fillLfo();

	if (par == prm_sync && val==1)
	{
		disp_counter=DISPLAY_REFRESH;
		lfopos=0;
		sprintf(Params[prm_sync]->lblString, "LFO Restarted!");				//any guiparameter's label can be changed on the fly like this
	}

}

void mi::Work(float *psamplesleft, float *psamplesright , int numsamples, int tracks)
{
	int rtpos, ltpos;
	int ltSign, rtSign;

	float volMult = Vals[prm_depth] / (float)DEPTH_DIV;
	do
	{
		ltpos=(int)floor(lfopos);
		rtpos=(int)(ltpos+Vals[prm_stereophase]+(MAX_PHASE/2.0f))%LFO_SIZE;				

		ltSign = (*psamplesleft  < 0? -1: +1);
		rtSign = (*psamplesright < 0? -1: +1);
		*psamplesleft  -= lfo[ltpos] * *psamplesleft  * volMult;
		*psamplesright -= lfo[rtpos] * *psamplesright * volMult;

		//when the depth is > 100%, the above multiplication can pass 0.. this next bit makes sure nothing does.
		//if the original sign multiplied by the new value is negative, it means the sign has changed, and the value is zeroed out.
		if(*psamplesleft * ltSign < 0) *psamplesleft = 0.0;								// i'm not sure how cpu-intensive it is to multiply by 1 or -1..
		if(*psamplesright* rtSign < 0) *psamplesright= 0.0;								// but this seems the most terse way to get the result i want.

		


		lfopos += (Vals[prm_speed]) / (float) (MAX_SPEED*(LFO_SIZE/360.0f));// * srMultiplier;								//todo: srMultiplier broke my plugin!# it worked great, until
																									// one fateful build when it just stopped. so for now, changing 
																									// the samprate means changing the tremolo speed..
		if(lfopos>=LFO_SIZE) 												lfopos-=LFO_SIZE;

		if(Vals[prm_sync]==1)
			if(--disp_counter < 0)
			{
				Vals[prm_sync]=0;
				sprintf(Params[prm_sync]->lblString, "Restart LFO");
			}
		

		++psamplesleft;
		++psamplesright;

	} while(--numsamples);
}


bool mi::DescribeValue(char* txt,int const param, int const value)
{
	switch(param)
	{
	case prm_depth:												sprintf(txt," %.1f%% ", value/(float)(DEPTH_DIV/100.0f));
							return true;

	case prm_stereophase:
	case prm_skew:												sprintf(txt, " %i deg. ", (int)((value - MAX_SKEW/2.0f) / (MAX_SKEW/360.0f)));
							return true;

	case prm_speed:												sprintf(txt, " %.1f ms ", LFO_SIZE/(float)(value/(float)MAX_SPEED)/44.1f * LFO_SIZE/360.0f);								// please, don't ask me to explain how i derived
							return true;																																																																																				// this mess :) (i hate these conversions)

	case prm_gravity:								if(Vals[prm_gravmode]==grav_updown)
							{
								if(value<100)
									sprintf(txt, " %i%% (upwards) ", value-100);
								else if(value>100)
									sprintf(txt, " %i%% (downwards) ", value-100);
								else
									sprintf(txt, " 0%% ");
							}				else				{
								if(value<100)
									sprintf(txt, " %i%% (outwards) ", value-100);
								else if(value>100)
									sprintf(txt, " %i%% (inwards) ", value-100);
								else
									sprintf(txt, " 0%% ");
							}
							return true;

	case prm_gravmode:								if(value == grav_updown)
							{
								sprintf(txt, " Up/Down ");
							} else if(value == grav_inout) 
							{
								sprintf(txt, " In/Out ");
							} else {
								return false;
							}
							return true;

	case prm_sync:												//if(value==1)
							//				sprintf(txt, "lfo restarted!");
							//else																//the old tremolo printed this as a value, but the guitremolo changes the actual label instead.
								sprintf(txt, "");
							return true;

	case prm_waveform:								switch(value)
							{
							case lfo_sine:								sprintf(txt, "sine");
												break;
							case lfo_tri:								sprintf(txt, "triangle");
												break;
							case lfo_square:				sprintf(txt, "square");
												break;
							}
							return true;
	}
	return false;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  fillLfo()

// called whenever a parameter is changed that warrants rebuffering the lfo
// just so we're clear, this actually makes -inverted- sine/tri/square waves,
// since these values determine how much is -subtracted- from the signal.
// everything is scaled between 0 and 1

// since there may be a possibility that the user will want to sweep the skew variable, it may be
// in our best interest, efficiency-wise, to reimplement this such that skew controls the increment speed of the
// lfopos variable in Work(), and the buffer keeps an even midpoint. (we could also do something similar with the gravity curves..)

void mi::fillLfo()
{

	int skew = Vals[prm_skew];
	if(skew>LFO_SIZE||skew<1) skew = LFO_SIZE/2;								// crashes psycle without this! on creation of the class, this function gets called before
														// Vals[prm_skew] is ever initialized.  i'm sure there are more efficient ways to do this..

	int sqSlope; 
	float xlop, ylop, temp, x;

	switch(Vals[prm_waveform])
	{
	case lfo_tri:								
						for(int i=0;i<skew;++i)
							lfo[i]= 1.0f - (i/(float)skew);
						for(int i=skew;i<LFO_SIZE;++i)
							lfo[i]= ((float)(i-skew) / (float)(LFO_SIZE-skew));
						break;

	case lfo_sine:								for(int i=0;i<skew;++i)
							lfo[i] =				0.5f * cos(i/(float)skew * M_PI) + 0.5f;
						for(int i=skew;i<LFO_SIZE;++i)
							lfo[i] =   -0.5f * cos( (i-skew)/(float)(LFO_SIZE-skew) * M_PI) + 0.5f;

						break;

	case lfo_square:				sqSlope = (int)(LFO_SIZE/80.0f);								// 80.0f is a completely arbitrary constant, feel free to adjust to taste
						if(sqSlope==0) sqSlope=1;																//better safe than sorry..
						if(skew<sqSlope)				skew=sqSlope;								// more of a trapezoid wave, i guess.. perfect square makes clicks

						for(int i=0;i<sqSlope;++i)
						{
							lfo[i]=1.0f - (i/(float)sqSlope);
							lfo[i+skew] = (i/(float)sqSlope);
						}
						for(int i=sqSlope;i<skew;++i)
							lfo[i]=0.0f;
												
						for(int i=skew+sqSlope;i<LFO_SIZE;++i)
							lfo[i]=1.0f;
						break;
	}
	

	if(Vals[prm_gravity]!=100 && Vals[prm_waveform]!=lfo_square)  //we -could- do all this with 0 gravity or a square wave, it just wouldn't change anything..
	{
		ylop=std::fabs(Vals[prm_gravity]-100.0f) / 100.0f;								// for weighted average of linear value and curved value

		for(int i=0;i<LFO_SIZE;++i)
		{
			x = lfo[i] * MAPSIZE;

			if(x<0) x=0;																																//okay, this really should not be happening, but until i've checked out my equations a bit more,
			if(x>MAPSIZE) x=MAPSIZE;																				// i don't want to risk it

			xlop=x-floor(x);																																// for linear interpolation between points in the curve mapping array
			

			if(Vals[prm_gravmode]==grav_updown)
			{
				if(Vals[prm_gravity]>100)																								// positive gravity, use highcurve (remember! this is being subtracted, so our lfos are umop apisdn)
				{
					temp = (highCurve[(int)(floor(x))] * (1.0f-xlop))								//xlop==0 means 100% floor, xlop==.5 means 50% floor and 50% ceil, etc..
							+ (highCurve[(int)(ceil(x))]  * xlop);
				} else {
					temp = (lowCurve[(int)(floor(x))] * (1.0f-xlop))								//now, x should never be more than 1.0f, which means ceil(x) * MAPSIZE should never
							+ (lowCurve[(int)(ceil(x))]  * xlop);												// be more than MAPSIZE, but just for safety, the mapping array has one extra element
				}																																																												// at [MAPSIZE+1], initialized to 1.0f.
			} else {
				if(Vals[prm_gravity]>100)
				{
					temp = (inCurve[(int)(floor(x))] * (1.0f-xlop))
							+ (inCurve[(int)(ceil(x))]  *				xlop);
				} else {
					temp = (outCurve[(int)(floor(x))] * (1.0f-xlop))
							+ (outCurve[(int)(ceil(x))]  *  xlop);
				}
			}
			lfo[i] = (lfo[i] * (1.0f-ylop))																				//ylop==1 means 100% curve map, ylop==.5 means 50% linear and 50% curved, etc.
					+ (temp   *    ylop);

		}
		
	}
	Display->ClearData(); //wipe the display clean.  you can also provide a COLORREF value to clear to a color besides black.
	int xp, yp, xpPrev, ypPrev;


	//i wrote these loops before making CMachineDisplay.Line(), so this kind of thing isn't necessary anymore, but you get the idea..
	for(int i(0);i<DISP_SIZE_X;++i)
	{
		Display->PutPixel(CPoint(i,				DISP_SIZE_Y/4),								0x00550000);
		Display->PutPixel(CPoint(i,				DISP_SIZE_Y*3/4),				0x00550000);
		for(int j(1);j<8;j+=2)
			Display->PutPixel(CPoint(i,				DISP_SIZE_Y*j/8),				0x00440000);
	}
	for(int i(0);i<DISP_SIZE_Y;++i)
	{
		Display->PutPixel(CPoint(DISP_SIZE_X/2,				i),				0x00663300);
		Display->PutPixel(CPoint(DISP_SIZE_X/4,				i),				0x00550000);
		Display->PutPixel(CPoint(DISP_SIZE_X*3/4,				i),				0x00550000);
		for(int j(1);j<8;j+=2)
			Display->PutPixel(CPoint(DISP_SIZE_X*j/8,				i),				0x00440000);
	}
	for(int i(0);i<DISP_SIZE_X;++i)
		Display->PutPixel(CPoint(i,				DISP_SIZE_Y/2),								0x00663300);				//i don't want the vertical gridlines to overdraw this


	//here we go through the lfo array, connecting each point with a line to the next.
	xpPrev = 0;
	ypPrev = lfo[0] * (DISP_SIZE_Y-1);
	for(int i(1);i<LFO_SIZE-1;++i)
	{
		xp = int(i*(DISP_SIZE_X-1)/float(LFO_SIZE));
		yp = lfo[i] * (DISP_SIZE_Y-1);

		Display->Line(CPoint(xpPrev, ypPrev), CPoint(xp, yp), 0x00EEEEEE, 3);
		xpPrev=xp;
		ypPrev=yp;
	}
}
