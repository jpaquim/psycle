//////////////////////////////////////////////////////////////////////
// LADSPA GVerb plugin for PSYCLE by Sartorius
//
//   Original
/*

GVerb algorithm designed and implemented by Juhana Sadeharju.
LADSPA implementation and GVerb speeds ups by Steve Harris.

Comments and suggestions should be mailed to Juhana Sadeharju
(kouhia at nic funet fi).

*/

#include <psycle/plugin_interface.hpp>
#include "gverb.h"
#include <cstdlib>
#include <cstring>
#include <cmath>

using namespace psycle::plugin_interface;

CMachineParameter const paraRoomSize = { "Room size", "Room size", 1, 300, MPF_STATE, 144 };
CMachineParameter const paraRevTime = { "Reverb time", "Reverb time", 10, 3000, MPF_STATE, 1800 };
CMachineParameter const paraDamping = { "Damping", "Damping", 0, 1000, MPF_STATE, 1000 };
CMachineParameter const paraBandwidth = { "Input bandwidth", "Input bandwidth", 0, 1000, MPF_STATE, 0 };
CMachineParameter const paraDry = { "Dry signal level", "Dry", -70000, 0, MPF_STATE, -70000 };
CMachineParameter const paraEarly = { "Early reflection level", "Early", -70000, 0, MPF_STATE, 0 };
CMachineParameter const paraTail = { "Tail level", "Tail level", -70000, 0, MPF_STATE, -17500 };
CMachineParameter const paraMonoStereo = { "Input", "Input", 0, 1, MPF_STATE, 0 };

CMachineParameter const *pParameters[] = {
	&paraRoomSize,
	&paraRevTime,
	&paraDamping,
	&paraBandwidth,
	&paraDry,
	&paraEarly,
	&paraTail,
	&paraMonoStereo
};

CMachineInfo const MacInfo (
	MI_VERSION,				
	EFFECT,
	sizeof pParameters / sizeof *pParameters,
	pParameters,
	#ifndef NDEBUG
		"LADSPA GVerb (Debug build)",
	#else
		"LADSPA GVerb",
	#endif
	"GVerb",
	"Juhana Sadeharju/Steve Harris/Sartorius",
	"About",
	1
);

class mi : public CMachineInterface {
	public:
		mi();
		virtual ~mi();

		virtual void Init();
		virtual void SequencerTick();
		virtual void Work(float *psamplesleft, float *psamplesright, int numsamples, int tracks);
		virtual bool DescribeValue(char* txt, int const param, int const value);
		virtual void Command();
		virtual void ParameterTweak(int par, int val);
	private:
		ty_gverb *gv_l,*gv_r;
		int samplerate;
};

PSYCLE__PLUGIN__INSTANTIATOR(mi, MacInfo)

template<typename Real> inline Real DB_CO(Real g) {
	return g > Real(-90) ? std::pow(Real(10), g * Real(0.05)) : Real(0);
}

template<typename Real> inline Real CO_DB(Real v) {
	return Real(20) * std::log10(v);
}

mi::mi() {
	Vals = new int[sizeof pParameters / sizeof *pParameters];
}

mi::~mi() {
	delete[] Vals;
	gverb_free(gv_l);
	gverb_free(gv_r);
}

void mi::Init() {
	samplerate = pCB->GetSamplingRate();
	gv_l = gverb_new(samplerate, 300.0f, 50.0f, 7.0f, 0.5f, 15.0f, 0.5f, 0.5f, 0.5f);
	gv_r = gverb_new(samplerate, 300.0f, 50.0f, 7.0f, 0.5f, 15.0f, 0.5f, 0.5f, 0.5f);
}

void mi::SequencerTick() {
	if(samplerate != pCB->GetSamplingRate()) {
		samplerate = pCB->GetSamplingRate();

		gverb_free(gv_l);
		gverb_free(gv_r);
		
		gv_l = gverb_new(samplerate, 300.0f, 50.0f, 7.0f, 0.5f, 15.0f, 0.5f, 0.5f, 0.5f);
		gv_r = gverb_new(samplerate, 300.0f, 50.0f, 7.0f, 0.5f, 15.0f, 0.5f, 0.5f, 0.5f);

		gverb_set_roomsize(gv_l,(float)Vals[0]);
		gverb_set_roomsize(gv_r,(float)Vals[0]);

		gverb_set_revtime(gv_l,(float)Vals[1]*.01f);
		gverb_set_revtime(gv_r,(float)Vals[1]*.01f);

		gverb_set_damping(gv_l,(float)Vals[2] * .001f);
		gverb_set_damping(gv_r,(float)Vals[2] * .001f);

		gverb_set_inputbandwidth(gv_l,(float)Vals[3] * .001f);
		gverb_set_inputbandwidth(gv_r,(float)Vals[3] * .001f);

		gverb_set_earlylevel(gv_l,DB_CO((float)Vals[5]*.001f));
		gverb_set_earlylevel(gv_r,DB_CO((float)Vals[5]*.001f));

		gverb_set_taillevel(gv_l,DB_CO((float)Vals[6]*.001f));
		gverb_set_taillevel(gv_r,DB_CO((float)Vals[6]*.001f));
	}
}

void mi::Command() {
	pCB->MessBox("LADSPA GVerb","GVerb",0);
}

void mi::ParameterTweak(int par, int val) {
	Vals[par]=val;
	switch(par) {
		case 0:
			gverb_set_roomsize(gv_l,(float)val);
			gverb_set_roomsize(gv_r,(float)val);
			break;
		case 1:
			gverb_set_revtime(gv_l,(float)val*.01f);
			gverb_set_revtime(gv_r,(float)val*.01f);
			break;
		case 2:
			gverb_set_damping(gv_l,(float)val * .001f);
			gverb_set_damping(gv_r,(float)val * .001f);
			break;
		case 3:
			gverb_set_inputbandwidth(gv_l,(float)val * .001f);
			gverb_set_inputbandwidth(gv_r,(float)val * .001f);
			break;
		case 4: break;
		case 5:
			gverb_set_earlylevel(gv_l,DB_CO((float)val*.001f));
			gverb_set_earlylevel(gv_r,DB_CO((float)val*.001f));
			break;
		case 6:
			gverb_set_taillevel(gv_l,DB_CO((float)val*.001f));
			gverb_set_taillevel(gv_r,DB_CO((float)val*.001f));
			break;
		case 7:
			gverb_flush(gv_l);
			gverb_flush(gv_r);
			break;
		default:
			break;
	}
}

void mi::Work(float *psamplesleft, float *psamplesright , int numsamples, int tracks) {
	float const dry = DB_CO((float)Vals[4]*.001f);
	
	float outl = 0;
	float outr = 0;

	if(Vals[7]) {
		do {
			float inl = *psamplesleft;// * 0.000030517578125f;
			float inr = *psamplesright;// * 0.000030517578125f;
			gverb_do(gv_l,inl,&outl,&outr);
			*psamplesleft = inl*dry + outl;//*32767.f;
			*psamplesright = inr*dry + outr;//*32767.f;
			gverb_do(gv_r,inr,&outl,&outr);
			*psamplesleft += outl;//*32767.f;
			*psamplesright += outr;//*32767.f;

			++psamplesleft;
			++psamplesright;
		} while(--numsamples);
	} else {
		do {
			float sm = (*psamplesleft + *psamplesright) * 0.5f;// * 0.000030517578125f;
			gverb_do(gv_l,sm,&outl,&outr);

			*psamplesleft = *psamplesleft * dry + outl;//*32767.f;
			*psamplesright = *psamplesright * dry + outr;//*32767.f;

			++psamplesleft;
			++psamplesright;
		} while(--numsamples);
	}
}

// Function that describes value on client's displaying
bool mi::DescribeValue(char* txt,int const param, int const value) {
	switch(param) {
		case 0:
			std::sprintf(txt,"%i m",value);
			return true;
		case 1:
			std::sprintf(txt,"%.2f s",(float)value*.01f);
			return true;
		case 2: case 3:
			std::sprintf(txt,"%.01f",(float)value*.001f);
			return true;
		case 4: case 5: case 6:
			std::sprintf(txt,"%i dB",value/1000);
			return true;
		case 7:
			std::sprintf(txt,value?"stereo":"mono");
			return true;
		default:
			return false;
	}
}
