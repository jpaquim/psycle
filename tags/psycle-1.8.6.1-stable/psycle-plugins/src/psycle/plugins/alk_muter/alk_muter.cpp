///\file
///\brief Alk muter plugin for PSYCLE
#include <psycle/plugin_interface.hpp>
#include <cstdio> // for std::sprintf
#include <stdexcept>
namespace psycle { namespace plugins { namespace alk_muter {

using namespace plugin_interface;

CMachineParameter const static mute_parameter = { 
	"Mute",
	"Mute off/on", // description
	0, // min value				
	1, // max value
	MPF_STATE, // flags
	0
};

CMachineParameter const static * const parameters[] = { 
	&mute_parameter
};

CMachineInfo const static machine_info = {
	MI_VERSION,				
	0, // flags
	sizeof parameters / sizeof *parameters, // number of parameters
	parameters, // pointer to parameters

	"Alk Muter 1.1" // name
	#ifndef NDEBUG
		" (debug build)"
	#endif
	,
	
	"Muter", // short name
	"Alk", // author
	"About", // a command, that could be use to open an editor, etc...
	1 // number of columns
};

class machine : public CMachineInterface {
	public:
		machine();
		~machine();
		void Init();
		void SequencerTick();
		void Work(float * left_samples, float * right_samples, int sample_count, int tracks);
		bool DescribeValue(char* text, int const parameter, int const value);
		void Command();
		void ParameterTweak(int parameter, int value);
private:
	float volume;
	bool change;
};

PSYCLE__PLUGIN__INSTANTIATOR(machine, machine_info)

machine::machine():volume(1.0f), change(false) {
	Vals = new int[sizeof parameters];
}

machine::~machine() {
	delete Vals;
}

void machine::Init() {
}

void machine::SequencerTick() {
	// called on each tick while sequencer is playing
}

void machine::Command() {
	// called when user presses editor button
	char text[] = "Made by alk\nThis machine is an alternative way to mute the audio of a path.\0";
	char caption[] = "Alk's Muter\0";
	pCB->MessBox(text, caption, 0);
}

void machine::ParameterTweak(int parameter, int value) {
	switch(parameter) {
		case 0:
			if (value != Vals[0]) {
				change=true;
			}
			Vals[0] = value;
			break;
		default:
			break;
	}
}

void machine::Work(float * left_samples, float * right_samples, int sample_count, int /*tracks*/) {
	try {
	if (!change) {
		if(!Vals[0]) return;
		if (sample_count < 0 ) {
			int i = 1;
		}
		while(sample_count--) *left_samples++ = *right_samples++ = 0;
	} else if (!Vals[0]){ // mute disabled
		while(volume<0.99f && sample_count--) {
			(*left_samples)*=volume; (*right_samples)*=volume;
			left_samples++; right_samples++;
			volume+=0.01f;
		}
		if (volume>=0.99f) {
			change=false;
		}
	} else if (Vals[0] == 1){ // mute enabled
		while(volume>0.01f && sample_count--) {
			(*left_samples)*=volume; (*right_samples)*=volume;
			left_samples++; right_samples++;
			volume-=0.01f;
		}
		sample_count++;
		while(sample_count--) *left_samples++ = *right_samples++ = 0;
		if (volume<=0.01f) {
			change=false;
		}
	}
	}catch(std::exception e) {
		int i =1;
	}
}

bool machine::DescribeValue(char * text, int const parameter, int const value) {
	switch(parameter) {
		case 0:
			switch(value) {
				case 0: std::sprintf(text, "off"); return true;
				case 1: std::sprintf(text, "on"); return true;
				default: throw std::runtime_error("illegal value for parameter");
			}
		default: return false; // returning false will simply show the value as a raw integral number
	}
}
}}}

