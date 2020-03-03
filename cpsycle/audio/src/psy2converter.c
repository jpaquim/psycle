// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "psy2converter.h"
#include "machinefactory.h"
#include "song.h"
#include "songio.h"
#include <string.h>
#include <stdlib.h>
#include <dir.h>
#include "constants.h"
#include "../../detail/portable.h"

#if !defined DIVERSALIS__OS__MICROSOFT
#define _MAX_PATH 4096
#endif

typedef struct {
	int32_t x;
	int32_t y;
} POINT;

static void psy2pluginload(psy_audio_SongFile* songfile, int32_t slot);
static void psy2samplerload(psy_audio_SongFile* songfile, int32_t slot);
static void psy2machineload(psy_audio_SongFile* songfile, int32_t slot);
static void psy2readmachineconnections(psy_audio_SongFile* songfile, int32_t slot);
static int convertindex(int index);
static void ReadPlugin(psy_audio_SongFile* songfile, int machtype, int psy2type);

enum {
	master,
	ring_modulator,
	distortion, 
	sampler, 
	delay,
	filter_2_poles,
	gainer,
	flanger,
	nativeplug,
	vsti,
	vstfx,
	scope,
	dummy = 255
};

enum {
	abass,
	asynth,
	asynth2,
	asynth21,
	synth21,
	asynth22
};

psy_audio_Machine* psy_audio_psy2converter_load(
	psy_audio_SongFile* songfile, int index,
	int* newindex, int* x, int* y)
{
	psy_audio_Machine* rv = 0;
	int32_t type;	
	int32_t temp32;
	char dllname[256];
	char editname[16];
	psy_audio_MachineFactory* factory;
	
	factory = songfile->song->machinefactory;
	*newindex = convertindex(index);	
	psyfile_read(songfile->file, &temp32, sizeof(temp32));
	*x = temp32;
	psyfile_read(songfile->file, &temp32, sizeof(temp32));
	*y = temp32;
	psyfile_read(songfile->file, &temp32, sizeof(temp32));
	type = temp32;	

	if (type == master) {
		psyfile_read(songfile->file, &editname, sizeof(editname));
		rv = machinefactory_makemachine(factory, MACH_MASTER, "");		
		psy2machineload(songfile, *newindex);
	} else
	if (type == delay) {
		psyfile_read(songfile->file, &editname, sizeof(editname));
		rv = machinefactory_makemachine(factory, MACH_PLUGIN, "delay:0");		
		psy2machineload(songfile, *newindex);
	} else
	if (type == filter_2_poles) {
		psyfile_read(songfile->file, &editname, sizeof(editname));
		rv = machinefactory_makemachine(factory, MACH_PLUGIN, "filter-2-poles:0");		
		psy2machineload(songfile, *newindex);
	} else
	if (type == sampler) {
		psyfile_read(songfile->file, &editname, sizeof(editname));
		rv = machinefactory_makemachine(factory, MACH_SAMPLER, "");		
		psy2samplerload(songfile, *newindex);
	} else
	if (type == nativeplug) {		
		psyfile_read(songfile->file, dllname, sizeof(dllname));
		psy_strlwr(dllname);
		psyfile_read(songfile->file, &editname, sizeof(editname));
		if (strcmp(dllname, "arguru bass.dll") == 0) {
			rv = machinefactory_makemachine(factory, MACH_PLUGIN, "arguru-synth-2f:0");
			if (rv) {
				ReadPlugin(songfile, MACH_PLUGIN, abass);
				psy2machineload(songfile, *newindex);
			}
		} else	
		if (strcmp(dllname, "arguru synth.dll") == 0) {
			rv = machinefactory_makemachine(factory, MACH_PLUGIN, "arguru-synth-2f:0");
		} else
		if (strcmp(dllname, "arguru synth 2.dll") == 0) {
			rv = machinefactory_makemachine(factory, MACH_PLUGIN, "arguru-synth-2f:0");
		} else
		if (strcmp(dllname, "synth21.dll") == 0) {
			rv = machinefactory_makemachine(factory, MACH_PLUGIN, "arguru-synth-2f:0");
		} else
		if (strcmp(dllname, "synth22.dll") == 0) {
			rv = machinefactory_makemachine(factory, MACH_PLUGIN, "arguru-synth-2f:0");
		} else {
			char plugincatchername[256];
			
			plugincatcher_catchername(songfile->song->machinefactory->catcher,
				dllname, plugincatchername, 0);			
			rv = machinefactory_makemachine(factory, MACH_PLUGIN,
				plugincatchername);
			if (!rv) {
				rv = machinefactory_makemachine(factory, MACH_DUMMY,
					plugincatchername);
			}
			psy2pluginload(songfile, *newindex);
		}
	} else {
		editname[0] = '\0';
	}
	if (rv) {
		psy_audio_machine_seteditname(rv, editname);
	}
	return rv;
}

void psy2machineload(psy_audio_SongFile* songfile, int32_t slot)
{
	char junk[256];

	int32_t _panning;
	int32_t _outDry;
	int32_t _outWet;

	memset(&junk, 0, sizeof(junk));

	psy2readmachineconnections(songfile, slot);
	psyfile_read(songfile->file, &_panning, sizeof(_panning));
	// psy_audio_Machine::SetPan(_panning);
	psyfile_read(songfile->file, &junk[0], 8*sizeof(int32_t)); // SubTrack[]
	psyfile_read(songfile->file, &junk[0], sizeof(int32_t)); // numSubtracks
	psyfile_read(songfile->file, &junk[0], sizeof(int32_t)); // interpol

	psyfile_read(songfile->file, &_outDry, sizeof(_outDry));
	psyfile_read(songfile->file, &_outWet, sizeof(_outWet));

	psyfile_read(songfile->file, &junk[0], sizeof(int32_t)); // distPosThreshold
	psyfile_read(songfile->file, &junk[0], sizeof(int32_t)); // distPosClamp
	psyfile_read(songfile->file, &junk[0], sizeof(int32_t)); // distNegThreshold
	psyfile_read(songfile->file, &junk[0], sizeof(int32_t)); // distNegClamp

	psyfile_read(songfile->file, &junk[0], sizeof(char)); // sinespeed
	psyfile_read(songfile->file, &junk[0], sizeof(char)); // sineglide
	psyfile_read(songfile->file, &junk[0], sizeof(char)); // sinevolume
	psyfile_read(songfile->file, &junk[0], sizeof(char)); // sinelfospeed
	psyfile_read(songfile->file, &junk[0], sizeof(char)); // sinelfoamp

	psyfile_read(songfile->file, &junk[0], sizeof(int32_t)); // delayTimeL
	psyfile_read(songfile->file, &junk[0], sizeof(int32_t)); // delayTimeR
	psyfile_read(songfile->file, &junk[0], sizeof(int32_t)); // delayFeedbackL
	psyfile_read(songfile->file, &junk[0], sizeof(int32_t)); // delayFeedbackR

	psyfile_read(songfile->file, &junk[0], sizeof(int32_t)); // filterCutoff
	psyfile_read(songfile->file, &junk[0], sizeof(int32_t)); // filterResonance
	psyfile_read(songfile->file, &junk[0], sizeof(int32_t)); // filterLfospeed
	psyfile_read(songfile->file, &junk[0], sizeof(int32_t)); // filterLfoamp
	psyfile_read(songfile->file, &junk[0], sizeof(int32_t)); // filterLfophase
	psyfile_read(songfile->file, &junk[0], sizeof(int32_t)); // filterMode	
}

void psy2pluginload(psy_audio_SongFile* songfile, int32_t slot)
{
	char junk[256];
	int32_t _panning;
	int32_t _outDry;
	int32_t _outWet;
	int32_t i;
	int32_t numParameters;
	
	psyfile_read(songfile->file, &numParameters, sizeof(numParameters));
	for (i=0; i<numParameters; i++)
	{
		psyfile_read(songfile->file, &junk[0], sizeof(int32_t));			
	}
	psy2readmachineconnections(songfile, slot);
	psyfile_read(songfile->file, &_panning, sizeof(_panning));
//	psy_audio_Machine::SetPan(_panning);
	psyfile_read(songfile->file, &junk[0], 8*sizeof(int32_t)); // SubTrack[]
	psyfile_read(songfile->file, &junk[0], sizeof(int32_t)); // numSubtracks
	psyfile_read(songfile->file, &junk[0], sizeof(int32_t)); // interpol

	psyfile_read(songfile->file, &_outDry, sizeof(_outDry));
	psyfile_read(songfile->file, &_outWet, sizeof(_outWet));

	psyfile_read(songfile->file, &junk[0], sizeof(int32_t)); // distPosThreshold
	psyfile_read(songfile->file, &junk[0], sizeof(int32_t)); // distPosClamp
	psyfile_read(songfile->file, &junk[0], sizeof(int32_t)); // distNegThreshold
	psyfile_read(songfile->file, &junk[0], sizeof(int32_t)); // distNegClamp

	psyfile_read(songfile->file, &junk[0], sizeof(char)); // sinespeed
	psyfile_read(songfile->file, &junk[0], sizeof(char)); // sineglide
	psyfile_read(songfile->file, &junk[0], sizeof(char)); // sinevolume
	psyfile_read(songfile->file, &junk[0], sizeof(char)); // sinelfospeed
	psyfile_read(songfile->file, &junk[0], sizeof(char)); // sinelfoamp

	psyfile_read(songfile->file, &junk[0], sizeof(int32_t)); // delayTimeL
	psyfile_read(songfile->file, &junk[0], sizeof(int32_t)); // delayTimeR
	psyfile_read(songfile->file, &junk[0], sizeof(int32_t)); // delayFeedbackL
	psyfile_read(songfile->file, &junk[0], sizeof(int32_t)); // delayFeedbackR

	psyfile_read(songfile->file, &junk[0], sizeof(int32_t)); // filterCutoff
	psyfile_read(songfile->file, &junk[0], sizeof(int32_t)); // filterResonance
	psyfile_read(songfile->file, &junk[0], sizeof(int32_t)); // filterLfospeed
	psyfile_read(songfile->file, &junk[0], sizeof(int32_t)); // filterLfoamp
	psyfile_read(songfile->file, &junk[0], sizeof(int32_t)); // filterLfophase
	psyfile_read(songfile->file, &junk[0], sizeof(int32_t)); // filterMode
}

void psy2samplerload(psy_audio_SongFile* songfile, int32_t slot)
{	
	char junk[256];	
	int32_t _panning;
	int32_t _outDry;
	int32_t _outWet;
	int32_t i;
//	int32_t numParameters;
	int32_t _numVoices;


	memset(&junk, 0, sizeof(junk)); 	
	psy2readmachineconnections(songfile, slot);
	psyfile_read(songfile->file, &_panning, sizeof(_panning));
// 	psy_audio_Machine::SetPan(_panning);
	psyfile_read(songfile->file, &junk[0], 8*sizeof(int32_t)); // SubTrack[]
	psyfile_read(songfile->file, &_numVoices, sizeof(_numVoices)); // numSubtracks

/*	if (_numVoices < 4)  // No more need for this code.
	{
		// Most likely an old polyphony
		_numVoices = 8;
	}
*/
	psyfile_read(songfile->file, &i, sizeof(int32_t)); // interpol
/*	switch (i)
	{
	case 2:
		_resampler.SetQuality(RESAMPLE_SPLINE);
		break;
	case 0:
		_resampler.SetQuality(RESAMPLE_NONE);
		break;
	default:
	case 1:
		_resampler.SetQuality(RESAMPLE_LINEAR);
		break;
	}*/

	psyfile_read(songfile->file, &_outDry, sizeof(_outDry));
	psyfile_read(songfile->file, &_outWet, sizeof(_outWet));

	psyfile_read(songfile->file, &junk[0], sizeof(int32_t)); // distPosThreshold
	psyfile_read(songfile->file, &junk[0], sizeof(int32_t)); // distPosClamp
	psyfile_read(songfile->file, &junk[0], sizeof(int32_t)); // distNegThreshold
	psyfile_read(songfile->file, &junk[0], sizeof(int32_t)); // distNegClamp

	psyfile_read(songfile->file, &junk[0], sizeof(char)); // sinespeed
	psyfile_read(songfile->file, &junk[0], sizeof(char)); // sineglide
	psyfile_read(songfile->file, &junk[0], sizeof(char)); // sinevolume
	psyfile_read(songfile->file, &junk[0], sizeof(char)); // sinelfospeed
	psyfile_read(songfile->file, &junk[0], sizeof(char)); // sinelfoamp

	psyfile_read(songfile->file, &junk[0], sizeof(int32_t)); // delayTimeL
	psyfile_read(songfile->file, &junk[0], sizeof(int32_t)); // delayTimeR
	psyfile_read(songfile->file, &junk[0], sizeof(int32_t)); // delayFeedbackL
	psyfile_read(songfile->file, &junk[0], sizeof(int32_t)); // delayFeedbackR

	psyfile_read(songfile->file, &junk[0], sizeof(int32_t)); // filterCutoff
	psyfile_read(songfile->file, &junk[0], sizeof(int32_t)); // filterResonance
	psyfile_read(songfile->file, &junk[0], sizeof(int32_t)); // filterLfospeed
	psyfile_read(songfile->file, &junk[0], sizeof(int32_t)); // filterLfoamp
	psyfile_read(songfile->file, &junk[0], sizeof(int32_t)); // filterLfophase
	psyfile_read(songfile->file, &junk[0], sizeof(int32_t)); // filterMode	
}

void psy2readmachineconnections(psy_audio_SongFile* songfile, int32_t slot)
{	
	int32_t _inputMachines[MAX_CONNECTIONS];	// Incoming connections psy_audio_Machine number
	int32_t _outputMachines[MAX_CONNECTIONS];	// Outgoing connections psy_audio_Machine number
	float _inputConVol[MAX_CONNECTIONS];	// Incoming connections psy_audio_Machine vol
//	float _wireMultiplier[MAX_CONNECTIONS];	// Value to multiply _inputConVol[] to have a 0.0...1.0 range
	unsigned char _connection[MAX_CONNECTIONS];      // Outgoing connections activated
	unsigned char _inputCon[MAX_CONNECTIONS];		// Incoming connections activated
	int32_t _numInputs;							// number of Incoming connections
	int32_t _numOutputs;						// number of Outgoing connections
	POINT _connectionPoint[MAX_CONNECTIONS];			
	int c;
	int32_t index;

	index = slot;

	psyfile_read(songfile->file, &_inputMachines[0], sizeof(_inputMachines));
	psyfile_read(songfile->file, &_outputMachines[0], sizeof(_outputMachines));
	psyfile_read(songfile->file, &_inputConVol[0], sizeof(_inputConVol));
	psyfile_read(songfile->file, &_connection[0], sizeof(_connection));
	psyfile_read(songfile->file, &_inputCon[0], sizeof(_inputCon));
	psyfile_read(songfile->file, &_connectionPoint[0], sizeof(_connectionPoint));
	psyfile_read(songfile->file, &_numInputs, sizeof(_numInputs));
	psyfile_read(songfile->file, &_numOutputs, sizeof(_numOutputs));

	for (c = 0; c < MAX_CONNECTIONS; ++c) { // all for each of its input connections.		
		if (_connection[c]) {
			int32_t output;
			int32_t input;
			unsigned char connection;
			unsigned char incon;
			float inconvol;
			float wiremultiplier = 1.f;

			connection = _connection[c];
			incon = _inputCon[c];
			inconvol = _inputConVol[c];
			input = convertindex(_inputMachines[c]);
			output = convertindex(_outputMachines[c]);	
			if (connection && output != -1) {
				machines_connect(&songfile->song->machines, index, output);
			}
			if (incon && input != -1) {
				machines_connect(&songfile->song->machines, input, index);
				connections_setwirevolume(&songfile->song->machines.connections,
					input, index, inconvol * wiremultiplier);
			}		
		}		
	}
}

int convertindex(int index)
{
	int rv;

	if (index == 0) {
		rv = MASTER_INDEX;
	} else
	if (index == MASTER_INDEX) {
		rv = 0;
	} else {
		rv = index;
	}
	return rv;
}

void psy_audio_psy2converter_retweak(int type, int parameter, int* integral_value)
{
}

void ReadPlugin(psy_audio_SongFile* songfile, int machtype, int psy2type) //const std::pair<int, std::string> type, Machine& machine, RiffFile& riff)
{
	int32_t numParameters;
	int* Vals;

	psyfile_read(songfile->file, &numParameters, sizeof(numParameters));
	Vals = malloc(numParameters * sizeof(int32_t));
	psyfile_read(songfile->file, Vals, numParameters * sizeof(int));

	if (machtype == MACH_DUMMY) {
		//do nothing.
	}
	else if (psy2type == abass) {
		// retweak(machine, type, Vals, 15, 0);
		// machine.SetParameter(19, 0);
		// retweak(machine, type, Vals + 15, 1, 15);
		// if (numParameters > 16) {
			//retweak(machine, type, Vals + 16, 2, 16);
		// }
		// else {
		//	machine.SetParameter(24, 0);
		//	machine.SetParameter(25, 0);
		//}
	}
	else if (psy2type == asynth) {
		// retweak(machine, type, Vals, numParameters, 0);
		// machine.SetParameter(24, 0);
		// machine.SetParameter(25, 0);
		// machine.SetParameter(27, 1);
	}
	else if (psy2type == asynth2) {
		// retweak(machine, type, Vals, numParameters, 0);
		// machine.SetParameter(24, 0);
		// machine.SetParameter(25, 0);
	}
	else if (psy2type == asynth21) {
		//I am unsure which was the diference between asynth2 and asynth21 (need to chech sources in the cvs)
		// retweak(machine, type, Vals, numParameters, 0);
		// machine.SetParameter(24, 0);
		// machine.SetParameter(25, 0);
	}
	else if (psy2type == asynth22) {
		// retweak(machine, type, Vals, numParameters, 0);
	}
	free(Vals);
}