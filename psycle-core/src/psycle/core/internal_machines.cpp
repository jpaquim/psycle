
/**********************************************************************************************
	Copyright 2007-2008 members of the psycle project http://psycle.sourceforge.net

	This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.
	This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
	You should have received a copy of the GNU General Public License along with this program; if not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
**********************************************************************************************/

#include "internal_machines.h"

///\todo: These two includes need to be replaced by a "host" callback which gives such information.
#include "commands.h"
#include "player.h"

#include "song.h"
#include "dsp.h"
#include "fileio.h"

namespace psy { namespace core {

/****************************************************************************************************/
// Dummy

std::string Dummy::_psName = "Dummy";

Dummy::Dummy(MachineCallbacks* callbacks, Machine::id_type id)
	: Machine(callbacks, id)
{
	_type = MACH_DUMMY;
	defineInputAsStereo();
	defineOutputAsStereo();
	SetEditName(_psName);
	//DefineStereoInput(1);
	//DefineStereoOutput(1);
	SetAudioRange(32768.0f);
}

Dummy::~Dummy() throw() {
	//DestroyInputs();
	//DestroyOutputs();
}

int Dummy::GenerateAudio(int numSamples) {
	//cpu::cycles_type cost(cpu::cycles());
	Machine::UpdateVuAndStanbyFlag(numSamples);
	//cost = cpu::cycles() - cost;
	//work_cpu_cost(work_cpu_cost() + cost);
	_worked = true;
	return numSamples;
}

// Since Dummy is used by the loader to load broken/missing plugins, 
// its "LoadSpecificChunk" skips the data of the chunk so that the
// song loader can continue the sequence.
bool Dummy::LoadSpecificChunk(RiffFile* pFile, int version) {
	std::uint32_t size=0;
	pFile->Read(size); // size of this part params to load
	pFile->Skip(size);
	return true;
}

void Dummy::CopyFrom(Machine* mac) {
	_bypass = mac->_bypass;
	_mute = mac->_mute;
	SetAudioRange(mac->GetAudioRange());
	SetPan(mac->Pan());
	SetEditName(mac->GetEditName());
	_connectedInputs =  mac->_connectedInputs;
	_connectedOutputs = mac->_connectedOutputs;
	for(int i(0); i < MAX_CONNECTIONS; ++i) {
		_inputMachines[i]=mac->_inputMachines[i];
		_outputMachines[i]=mac->_outputMachines[i];
		_inputConVol[i]=mac->_inputConVol[i];
		_wireMultiplier[i]=mac->_wireMultiplier[i];
		_connection[i]=mac->_connection[i];
		_inputCon[i]=mac->_inputCon[i];
	}
}

/****************************************************************************************************/
// NoteDuplicator

std::string DuplicatorMac::_psName = "Dupe it!";

DuplicatorMac::DuplicatorMac(MachineCallbacks* callbacks, Machine::id_type id)
:
	Machine(callbacks, id)
{
	_type = MACH_DUPLICATOR;
	SetEditName(_psName);
	_numPars = NUM_MACHINES*2;
	_nCols = 2;
	bisTicking = false;
	for(int i(0); i < NUM_MACHINES; ++i) {
		macOutput[i] = -1;
		noteOffset[i] = 0;
		for(int j(0); j < MAX_TRACKS; ++j)
			allocatedchans[j][i] = -1;
	}
	for(int i(0); i < MAX_MACHINES; ++i)
		for(int j(0); j < MAX_TRACKS; ++j)
			availablechans[i][j] = true;
}

DuplicatorMac::~DuplicatorMac() throw() {}

void DuplicatorMac::Init() {
	Machine::Init();
	for(int i(0); i < NUM_MACHINES; ++i) {
		macOutput[i]=-1;
		noteOffset[i]=0;
		for(int j(0); j < MAX_TRACKS; ++j)
			allocatedchans[j][i] = -1;
	}
	for(int i(0); i < MAX_MACHINES; ++i)
		for(int j(0); j < MAX_TRACKS; ++j)
			availablechans[i][j] = true;
}

void DuplicatorMac::Stop() {
	for(int j(0); j < MAX_TRACKS; ++j) {
		for(int i(0); i < NUM_MACHINES; ++i)
			allocatedchans[j][i] = -1;
		for(int i(0); i < MAX_MACHINES; ++i)
			availablechans[i][j] = true;
	}
}

void DuplicatorMac::Tick( int /*channel*/, const PatternEvent & /*pData*/ ) {
	//const PlayerTimeInfo & timeInfo = callbacks->timeInfo();
}

void DuplicatorMac::PreWork(int numSamples) {
	Machine::PreWork(numSamples);
	for(; !workEvents.empty(); workEvents.pop_front()) {
		WorkEvent & workEvent = workEvents.front();
		if(!_mute && !bisTicking) {
			bisTicking = true;
			for(int i(0); i < NUM_MACHINES; ++i) {
				if(macOutput[i] != -1 && callbacks->song().machine(macOutput[i])) {
					AllocateVoice(workEvent.track(),i);
					#if 0
						PatternEvent temp = workEvent.event();
						//\todo: Multiple notes.
						NoteteEvent thenote = temp.note(0);
					#else
						PatternEvent thenote = workEvent.event();
					#endif
					if(thenote.note() < notetypes::release) {
						int note = thenote.note()+noteOffset[i];
						if(note >= notetypes::release) note = 119;
						else if(note < 0) note = 0;
						thenote.setNote(note);
						#if 0
							temp.SetNote(0, thenote);
						#endif
					}
					if(callbacks->song().machine(macOutput[i]) != this) {
						#if 0
							callbacks->song().machine(macOutput[i])->AddEvent(workEvent.beatOffset(), workEvent.track(), temp);
							if(temp.note(0).note() >= notetypes::release)
								DeallocateVoice(workEvent.track(), i);
						#else
							callbacks->song().machine(macOutput[i])->AddEvent(workEvent.beatOffset(), workEvent.track(), thenote);
							if(thenote.note() >= notetypes::release)
								DeallocateVoice(workEvent.track(), i);
						#endif
					}
				}
			}
		}
		bisTicking = false;
	}
}

void DuplicatorMac::AllocateVoice(int channel,int machine) {
	// If this channel already has allocated channels, use them.
	if(allocatedchans[channel][machine] != -1) return;
	// If not, search an available channel
	int j = channel;
	while(j < MAX_TRACKS && !availablechans[macOutput[machine]][j]) ++j;
	if(j == MAX_TRACKS) {
		j = 0;
		while(j < MAX_TRACKS && !availablechans[macOutput[machine]][j]) ++j;
		if(j == MAX_TRACKS)
			j = (unsigned int) (  (double)rand() * MAX_TRACKS /(((double)RAND_MAX) + 1.0 ));
	}
	allocatedchans[channel][machine] = j;
	availablechans[macOutput[machine]][j] = false;
}
void DuplicatorMac::DeallocateVoice(int channel, int machine) {
	if(allocatedchans[channel][machine] == -1) return;
	availablechans[macOutput[machine]][allocatedchans[channel][machine]] = true;
	allocatedchans[channel][machine] =- 1;
}

void DuplicatorMac::GetParamName(int numparam, char * name) const {
	if(numparam >= 0 && numparam < NUM_MACHINES)
		std::sprintf(name, "Output Machine %d", numparam);
	else if(numparam >= NUM_MACHINES && numparam < NUM_MACHINES * 2)
		std::sprintf(name, "Note Offset %d", numparam - NUM_MACHINES);
	else name[0] = '\0';
}

void DuplicatorMac::GetParamRange(int numparam, int & minval, int & maxval) const {
	if(numparam < NUM_MACHINES) { minval = -1; maxval = MAX_BUSES * 2 - 1; }
	else if(numparam < NUM_MACHINES * 2) { minval = -48; maxval = 48; }
}

int DuplicatorMac::GetParamValue(int numparam) const {
	if(numparam >=0 && numparam < NUM_MACHINES)
		return macOutput[numparam];
	else if(numparam >=NUM_MACHINES && numparam < NUM_MACHINES * 2)
		return noteOffset[numparam-NUM_MACHINES];
	else return 0;
}

void DuplicatorMac::GetParamValue(int numparam, char * parVal) const {
	if(numparam >= 0 && numparam < NUM_MACHINES) {
		if((macOutput[numparam] != -1) && callbacks->song().machine(macOutput[numparam]))
			std::sprintf(parVal, "%X -%s", macOutput[numparam], callbacks->song().machine(macOutput[numparam])->GetEditName().c_str());
		else if(macOutput[numparam] != -1) std::sprintf(parVal, "%X (none)", macOutput[numparam]);
		else std::sprintf(parVal, "(disabled)");
	} else if(numparam >= NUM_MACHINES && numparam < NUM_MACHINES * 2) {
		char notes[12][3] = { "C-", "C#", "D-", "D#", "E-", "F-", "F#", "G-", "G#", "A-", "A#", "B-" };
		std::sprintf(parVal, "%s%d",
			notes[(noteOffset[numparam - NUM_MACHINES] + 60) % 12],
			(noteOffset[numparam - NUM_MACHINES] + 60) / 12
		);
	}
	else parVal[0] = '\0';
}

bool DuplicatorMac::SetParameter(int numparam, int value) {
	if (numparam >=0 && numparam<NUM_MACHINES) {
		macOutput[numparam]=value;
		return true;
	} else if (numparam >=NUM_MACHINES && numparam<NUM_MACHINES*2) {
		noteOffset[numparam-NUM_MACHINES]=value;
		return true;
	}
	else return false;
}

int DuplicatorMac::GenerateAudio( int numSamples ) {
	_worked = true;
	Standby(true);
	return numSamples;
}

bool DuplicatorMac::LoadSpecificChunk(RiffFile* pFile, int /*version*/) {
	std::uint32_t size;
	pFile->Read(size);
	pFile->ReadArray(macOutput,NUM_MACHINES);
	pFile->ReadArray(noteOffset,NUM_MACHINES);
	return true;
}

void DuplicatorMac::SaveSpecificChunk(RiffFile* pFile) const {
	std::uint32_t const size(sizeof macOutput + sizeof noteOffset);
	pFile->Write(size);
	pFile->WriteArray(macOutput,NUM_MACHINES);
	pFile->WriteArray(noteOffset,NUM_MACHINES);
}


/****************************************************************************************************/
// Master

std::string Master::_psName = "Master";

float * Master::_pMasterSamples = 0;

Master::Master(MachineCallbacks* callbacks, Machine::id_type id)
:
	Machine(callbacks, id),
	sampleCount(0),
	decreaseOnClip(false),
	_lMax(0),
	_rMax(0),
	_outDry(256)
{
	_type = MACH_MASTER;
	defineInputAsStereo();
	SetEditName(_psName);
	_outDry = 256;
	SetAudioRange(32768.0f);
	//DefineStereoInput(1);
}

Master::~Master() throw() {}

void Master::Stop() {
	_clip = false;
	sampleCount = 0;
}

void Master::Init() {
	Machine::Init();
	sampleCount = 0;
	//_LMAX = 1; // Min value should NOT be zero, because we use a log10() to calculate the vu-meter's value.
	//_RMAX = 1;
	currentpeak=0.0f;
	peaktime=1;
	_lMax = 1;
	_rMax = 1;
	vuupdated = false;
	_clip = false;
}

void Master::Tick(int /*channel*/, const PatternEvent & data ) {
	#if 0
		///\todo: multiple commands
		if(data.command(0).command() == commandtypes::SET_VOLUME)
			_outDry = data.command(0).param();
	#else
		if(data.command() == commandtypes::SET_VOLUME)
			_outDry = data.parameter();
	#endif
}

int Master::GenerateAudio( int numSamples ) {
	#if PSYCLE__CONFIGURATION__FPU_EXCEPTIONS
		universalis::processor::exceptions::fpu::mask fpu_exception_mask(this->fpu_exception_mask()); // (un)masks fpu exceptions in the current scope
	#endif

	//cpu::cycles_type cost(cpu::cycles());
	
	//if(!_mute) {
	
	float mv = CValueMapper::Map_255_1(_outDry);
	float *pSamples = _pMasterSamples;
	float *pSamplesL = _pSamplesL;
	float *pSamplesR = _pSamplesR;

	//_lMax -= numSamples*8;
	//_rMax -= numSamples*8;
	//_lMax *= 0.5;
	//_rMax *= 0.5;
	if(vuupdated)  { 
		_lMax *= 0.5; 
		_rMax *= 0.5; 
	}
	
	int i = numSamples;
	if(decreaseOnClip) {
		do {
			// Left channel
			if(std::fabs(*pSamples = *pSamplesL = *pSamplesL * mv) > _lMax)
				_lMax = fabsf(*pSamplesL);
			if(*pSamples > 32767.0f) {
				_outDry = f2i((float)_outDry * 32767.0f / (*pSamples));
				mv = CValueMapper::Map_255_1(_outDry);
				*pSamples = *pSamplesL = 32767.0f; 
			} else if (*pSamples < -32767.0f) {
				_outDry = f2i((float)_outDry * -32767.0f / (*pSamples));
				mv = CValueMapper::Map_255_1(_outDry);
				*pSamples = *pSamplesL = -32767.0f; 
			}
			++pSamples;
			++pSamplesL;
			// Right channel
			if(std::fabs(*pSamples = *pSamplesR = *pSamplesR * mv) > _rMax)
				_rMax = fabsf(*pSamplesR);
			if(*pSamples > 32767.0f) {
				_outDry = f2i((float)_outDry * 32767.0f / (*pSamples));
				mv = CValueMapper::Map_255_1(_outDry);
				*pSamples = *pSamplesR = 32767.0f; 
			} else if (*pSamples < -32767.0f) {
				_outDry = f2i((float)_outDry * -32767.0f / (*pSamples));
				mv = CValueMapper::Map_255_1(_outDry);
				*pSamples = *pSamplesR = -32767.0f; 
			}
			++pSamples;
			++pSamplesR;
		} while(--i);
	} else {
		do {
			// Left channel
			if(std::fabs( *pSamples = (*pSamplesL = *pSamplesL * mv)) > _lMax)
				_lMax = fabsf(*pSamplesL);
			++pSamples;
			++pSamplesL;
			// Right channel
			if(std::fabs(*pSamples = (*pSamplesR = *pSamplesR * mv)) > _rMax)
				_rMax = fabsf(*pSamplesR);
			++pSamples;
			++pSamplesR;
		} while(--i);
	}

	if(_lMax > 32767.0f) {
		_clip = true;
		_lMax = 32767.0f; //_LMAX = 32768;
	} else if(_lMax < 1.0f) { _lMax = 1.0f; /*_LMAX = 1;*/ }
	//else _LMAX = Dsp::F2I(_lMax);

	if(_rMax > 32767.0f) {
		_clip = true;
		_rMax = 32767.0f; //_RMAX = 32768;
	} else if(_rMax < 1.0f) { _rMax = 1.0f; /*_RMAX = 1;*/ }
	//else _RMAX = Dsp::F2I(_rMax);
	
	if(_lMax > currentpeak) currentpeak = _lMax;
	if(_rMax > currentpeak) currentpeak = _rMax;
	
	//} // if(!_mute)
	
	sampleCount += numSamples;
	
	//cost = cpu::cycles() - cost;
	//work_cpu_cost(work_cpu_cost() + cost);
	
	_worked = true;
	return numSamples;
}

bool Master::LoadSpecificChunk(RiffFile* pFile, int /*version*/) {
	std::uint32_t size;
	pFile->Read(size);
	pFile->Read(_outDry);
	pFile->Read(decreaseOnClip);
	return true;
}

void Master::SaveSpecificChunk(RiffFile* pFile) const {
	std::uint32_t const size(sizeof _outDry + sizeof decreaseOnClip);
	pFile->Write(size);
	pFile->Write(_outDry);
	pFile->Write(decreaseOnClip);
}


/****************************************************************************************************/
// LFO

// todo:
// - as is, control rate is proportional to MAX_BUFFER_LENGTH.. we update in work, which (at the moment) means once every 256
//   samples. at 44k, this means a cr of 142hz.  this is probably good enough for most purposes, but i believe it also
//   means that the lfo can and likely will be phased by 5.8ms depending on where it is placed in the machine view..
//   if we want to take the idea of modulation machines much further, we should probably put together some kind of
//   standard place in the processing chain where these machines will work, preferably -before- any audio
//   <JosepMa> About the "before any audio", the player can support this right now in two different ways:
//   One is in the "Machine::preWork" function, currently only used for buffer cleanup and generation of the wire visual data.
//   The second one is in the "Player::NotifyNewLine" function or in "Player::ExecuteGlobalCommands"
//   processing.  this should also eliminate the need for the lfo to be connected to something to work.
//   Also, note that currently, work does NOT mean 256 samples. It means *at much* 256, and quite frequently, it is a smaller
//   value (each line). This will change with the event based player.
//  - respond to pulse width knob.. consider using it as a 'skew' control for sine/tri waves as in dw-tremolo?
//  - now that we have a gui, keeping the 'position' display knob as an un-controllable control is just silly
//  - prettify gui
//  - vst support??

std::string LFO::_psName = "LFO";

#if 0 // don't worry, msvc is the weird
int const LFO::LFO_SIZE;
int const LFO::MAX_PHASE;
int const LFO::MAX_SPEED;
int const LFO::MAX_DEPTH;
int const LFO::NUM_CHANS;
#endif


LFO::LFO(MachineCallbacks* callbacks, Machine::id_type id)
:
	Machine(callbacks, id)
{
	SetEditName(_psName);
	_numPars = prms::num_params;
	_nCols = 3;
	bisTicking = false;
}

LFO::~LFO() throw()
{
}

void LFO::Init()
{
	Machine::Init();
	for (int i=0;i<NUM_CHANS;i++)
	{
		macOutput[i]=-1;
		paramOutput[i]=-1;
		level[i]=100;
		phase[i]= static_cast<int>( MAX_PHASE / 2.0f );
		prevVal[i]   = 0;
						centerVal[i] = 0;
	}
	lfoPos = 0.0;
	lSpeed= MAX_SPEED / 10;
	waveform = lfo_types::sine;
	FillTable();
}

void LFO::Tick( int /*channel*/, const PatternEvent & pData )
{
	if(!bisTicking) {
		bisTicking = true;
		// 0x01.. seems appropriate for a machine with exactly one command, but if this goes
		// against any established practices or something, let me know
		#if 0
		if(pData.command(0).command() == 0x01) lfoPos = 0;
		#else
		if(pData.command() == 0x01) lfoPos = 0;
		#endif
	}
	bisTicking = false;
}

void LFO::GetParamName(int numparam,char *name) const
{
	if(numparam==prms::wave)
		sprintf(name,"Waveform");
	else if(numparam==prms::speed)
		sprintf(name,"Speed");
	else if (numparam<prms::prm0)
		sprintf(name,"Output Machine %d",numparam-prms::mac0);
	else if (numparam<prms::level0)
		sprintf(name,"Output Param %d",numparam-prms::prm0);
	else if (numparam<prms::phase0)
		sprintf(name,"Output Level %d",numparam-prms::level0);
	else if (numparam<prms::num_params)
		sprintf(name,"Output Phase %d",numparam-prms::phase0);
	else name[0] = '\0';
}

void LFO::GetParamRange(int numparam,int &minval,int &maxval) const
{
	if(numparam==prms::wave) { minval = 0; maxval = lfo_types::num_lfos-1;}
	else if (numparam==prms::speed) {minval = 0; maxval = MAX_SPEED;}
	else if (numparam <prms::prm0) {minval = -1; maxval = (MAX_BUSES*2)-1;}
	else if (numparam <prms::level0)
	{
		minval = -1;
		if(macOutput[numparam-prms::prm0]==-1 || callbacks->song().machine(macOutput[numparam-prms::prm0]) == NULL)
			maxval = -1;
		else
			maxval =  callbacks->song().machine(macOutput[numparam-prms::prm0])->GetNumParams()-1;
	}

	else if (numparam <prms::phase0){minval = 0; maxval = MAX_DEPTH*2;}
	else if (numparam <prms::num_params){minval = 0; maxval = MAX_PHASE; }
	else {minval=0;maxval=0; }

}

int LFO::GetParamValue(int numparam) const
{
	if(numparam==prms::wave) return waveform;
	else if(numparam==prms::speed) return lSpeed;
	else if(numparam <prms::prm0) return macOutput[numparam-prms::mac0];
	else if(numparam <prms::level0) return paramOutput[numparam-prms::prm0];
	else if(numparam <prms::phase0) return level[numparam-prms::level0];
	else if(numparam <prms::num_params) return phase[numparam-prms::phase0];
	else return 0;
}

void LFO::GetParamValue(int numparam, char *parVal) const
{
	if(numparam==prms::wave)
	{
		switch(waveform)
		{
		case lfo_types::sine: sprintf(parVal, "sine"); break;
		case lfo_types::tri: sprintf(parVal, "triangle"); break;
		case lfo_types::saw: sprintf(parVal, "saw"); break;
		case lfo_types::square: sprintf(parVal, "square"); break;
		default: throw;
		}
	} 
	else if(numparam==prms::speed)
	{
		if(lSpeed==0)
			sprintf(parVal, "inf.");
		else
		{
			float speedInMs = 100.0f / (float)(lSpeed/(float)(MAX_SPEED));
			if(speedInMs<1000.0f)
				sprintf(  parVal, "%.1f ms", speedInMs);
			else
				sprintf( parVal, "%.3f secs", speedInMs/1000.0f);
		}
	} 
	else if(numparam<prms::prm0)
	{
		if ((macOutput[numparam-prms::mac0] != -1 ) &&( callbacks->song().machine(macOutput[numparam-prms::mac0]) != NULL))
			sprintf(parVal,"%X -%s",macOutput[numparam-prms::mac0],callbacks->song().machine(macOutput[numparam-prms::mac0])->GetEditName().c_str());
		else if (macOutput[numparam-prms::mac0] != -1)
			sprintf(parVal,"%X (none)",macOutput[numparam-prms::mac0]);
		else 
			sprintf(parVal,"(disabled)");
	}
	else if(numparam<prms::level0)
	{
		if((macOutput[numparam-prms::prm0] != -1) 
			&& (callbacks->song().machine(macOutput[numparam-prms::prm0]) != NULL)
			&& (paramOutput[numparam-prms::prm0] >= 0))
		{
			if(paramOutput[numparam-prms::prm0] < callbacks->song().machine(macOutput[numparam-prms::prm0])->GetNumParams())
			{
				char name[128];
				callbacks->song().machine(macOutput[numparam-prms::prm0])->GetParamName(paramOutput[numparam-prms::prm0], name);
				sprintf(parVal,"%X -%s", paramOutput[numparam-prms::prm0], name);
			}
			else
				sprintf(parVal,"%X -none", paramOutput[numparam-prms::prm0]);
		} 
		else 
			sprintf(parVal,"(disabled)");

	}
	else if(numparam<prms::phase0)
		sprintf(parVal,"%i%%", level[numparam-prms::level0]-MAX_DEPTH);
	else if(numparam<prms::num_params)
		sprintf(parVal,"%.1f deg.", (phase[numparam-prms::phase0]-MAX_PHASE/2.0f)/float(MAX_PHASE/2.0f) * 180.0f);
	else
		parVal[0] = '\0';
}

bool LFO::SetParameter(int numparam, int value)
{
	if(numparam==prms::wave)
	{
		waveform = value;
		FillTable();
		return true;
	}
	else if(numparam==prms::speed)
	{
		lSpeed = value;
		return true;
	}
	else if(numparam <prms::prm0)
	{
		if(value!=macOutput[numparam-prms::mac0])
		{
			int newMac(0);

			//if we're increasing, increase until we hit an active machine
			if(value>macOutput[numparam-prms::mac0])
			{
				for(newMac=value; newMac<MAX_MACHINES && callbacks->song().machine(newMac)==NULL; ++newMac)
					;
			}
			//if we're decreasing, or if we're increasing but didn't find anything, decrease until we find an active machine
			if(value<macOutput[numparam-prms::mac0] || newMac>=MAX_MACHINES)
			{
				for(newMac=value;newMac>-1 && callbacks->song().machine(newMac)==NULL; --newMac)
					;
			}

			ParamEnd(numparam-prms::mac0);
			macOutput[numparam-prms::mac0] = newMac;
			paramOutput[numparam-prms::mac0] = -1;
		}
		return true;
	}
	else if(numparam <prms::level0)
	{
		if( macOutput[numparam-prms::prm0]>-1 && callbacks->song().machine(macOutput[numparam-prms::prm0]) )
		{
			if(value<callbacks->song().machine(macOutput[numparam-prms::prm0])->GetNumParams())
			{
				ParamEnd(numparam-prms::prm0);
				paramOutput[numparam-prms::prm0] = value;
				ParamStart(numparam-prms::prm0);
			}
			else
			{
				ParamEnd(numparam-prms::prm0);
				paramOutput[numparam-prms::prm0] = callbacks->song().machine(macOutput[numparam-prms::prm0])->GetNumParams()-1;
				ParamStart(numparam-prms::prm0);
			}
		}
		else
			paramOutput[numparam-prms::prm0] = -1;

		return true;
	}
	else if(numparam <prms::phase0)
	{
		level[numparam-prms::level0] = value;
		return true;
	}
	else if(numparam <prms::num_params)
	{
		phase[numparam-prms::phase0] = value;
		return true;
	}
	else return false;
}

void LFO::PreWork(int numSamples)
{
	Machine::PreWork(numSamples);
	//cpu::cycles_type cost(cpu::cycles());

	int maxVal=0, minVal=0;
	int curVal=0, newVal=0;
	float curLFO=0.0;
	float lfoAmt=0.0;
	bool bRedraw=false;

	for(int j(0);j<NUM_CHANS;++j)
	{
		if( macOutput[j] != -1 && callbacks->song().machine(macOutput[j]) != NULL &&
			paramOutput[j] != -1 && paramOutput[j] < callbacks->song().machine(macOutput[j])->GetNumParams() )
		{
			callbacks->song().machine(macOutput[j])->GetParamRange(paramOutput[j], minVal, maxVal);
			curVal = callbacks->song().machine(macOutput[j])->GetParamValue(paramOutput[j]);
			curLFO = waveTable[int(lfoPos+phase[j]+(MAX_PHASE/2.0f)) % LFO_SIZE];
			lfoAmt = (level[j]-MAX_DEPTH)/(float)MAX_DEPTH;

			centerVal[j] -= prevVal[j] - curVal; // compensate for external movement

			newVal = (int) (curLFO * ((maxVal-minVal)/2.0f) * lfoAmt + centerVal[j]);

			if(newVal>maxVal) newVal=maxVal;
			else if(newVal<minVal) newVal=minVal;

			callbacks->song().machine(macOutput[j])->SetParameter(paramOutput[j], newVal); // make it happen!
			bRedraw=true;
			prevVal[j] = newVal;
		}
	}

	if(bRedraw)
		Player::singleton().Tweaker=true;
	bRedraw=false;

	float minms = callbacks->timeInfo().sampleRate() /1000.0f * 100.0f; // 100ms in samples
	lfoPos += (lSpeed/ float(MAX_SPEED)) * (LFO_SIZE/float(minms/float(numSamples)));
	if(lfoPos>LFO_SIZE) lfoPos-=LFO_SIZE;

	//cost = cpu::cycles() - cost;
	//work_cpu_cost(work_cpu_cost() + cost);
}

int LFO::GenerateAudio( int numSamples )
{
	_worked=true;
	return numSamples;
}

bool LFO::LoadSpecificChunk(RiffFile* pFile, int /*version*/)
{
	std::uint32_t size;
	pFile->Read(size);
	pFile->Read(waveform);
	pFile->Read(lSpeed);
	pFile->ReadArray(macOutput,NUM_CHANS);
	pFile->ReadArray(paramOutput,NUM_CHANS);
	pFile->ReadArray(level,NUM_CHANS);
	pFile->ReadArray(phase,NUM_CHANS);
	return true;
}

void LFO::SaveSpecificChunk(RiffFile* pFile) const
{
	std::uint32_t const size(sizeof waveform + sizeof lSpeed + sizeof macOutput + sizeof paramOutput + sizeof level + sizeof phase);
	pFile->Write(size);
	pFile->Write(waveform);
	pFile->Write(lSpeed);
	pFile->WriteArray(macOutput,NUM_CHANS);
	pFile->WriteArray(paramOutput,NUM_CHANS);
	pFile->WriteArray(level,NUM_CHANS);
	pFile->WriteArray(phase,NUM_CHANS);
}

void LFO::FillTable()
{
	switch(waveform)
	{
	case lfo_types::sine:
		for (int i(0);i<LFO_SIZE;++i)
		{
			waveTable[i]=sin(i/float(LFO_SIZE-1) * 6.2831853071795864769252);
		}
		break;
	case lfo_types::tri:
		for (int i(0);i<LFO_SIZE/2;++i)
		{
			waveTable[i] = waveTable[LFO_SIZE-i-1] = i/float(LFO_SIZE/4) - 1;
		}
		break;
	case lfo_types::saw:
		for (int i(0);i<LFO_SIZE;++i)
		{
			waveTable[i] = i/float((LFO_SIZE-1)/2) - 1;
		}
		break;
	case lfo_types::square:
		for (int i(0);i<LFO_SIZE/2;++i)
		{
			waveTable[i] = 1;
			waveTable[LFO_SIZE-1-i]=-1;
		}
		break;
	default:
		for(int i(0);i<LFO_SIZE;++i) //????
		{
			waveTable[i] = 0;
		}
		break;
	}
}

void LFO::ParamStart( int which )
{
	if(which<0 || which>=NUM_CHANS) return; // jic
	int destMac = macOutput[which];
	int destParam = paramOutput[which];

	if(destMac != -1 && callbacks->song().machine(destMac) != NULL
		&& destParam != -1 && destParam < callbacks->song().machine(destMac)->GetNumParams())
	{
		int minVal, maxVal;
		float curLFO, lfoAmt;

		callbacks->song().machine(destMac)->GetParamRange(destParam, minVal, maxVal);
		curLFO = waveTable[int(lfoPos+phase[which]+(MAX_PHASE/2.0f)) % LFO_SIZE];
		lfoAmt = (level[which]-MAX_DEPTH)/(float)MAX_DEPTH;

		//bad! bad!
		//prevVal[which] = Gloxxxxxxxxxxxxxxxxxbal::callbacks->song().machine(macOutput[which])->GetParamValue(paramOutput[which]);
		//centerVal[which] = prevVal[which] - (curLFO * ((maxVal-minVal)/2.0f) * lfoAmt);

		centerVal[which] = callbacks->song().machine(destMac)->GetParamValue(destParam);
		prevVal[which] = centerVal[which];

		// the way i've set this up, a control will 'jump' if the lfo is at a peak or dip when a control is first selected.
		// that may seem like a bad thing, but it means that the center of the lfo does -not- depend on the lfo position
		// when a control is selected.. and i think that this would be the preferred behavior.

	}
	else
	{
		prevVal[which]=0;
		centerVal[which]=0;
	}
}


//currently, this function resets the value of an output parameter to where it would be at lfo==0.  this behavior deserves
// some consideration, because it is conceivable that some people would want turning off an output to leave the parameter where it is
// instead of jerking it back to its original position..  on the other hand, without this code, sweeping through a list of parameters
// carelessly can wreak havoc on an entire plugin's settings.  i may decide just to let the user choose which s/he prefers..
void LFO::ParamEnd(int which)
{
	if(which<0 || which>=NUM_CHANS) return;
	id_type destMac(macOutput[which]);
	int destParam = paramOutput[which];

	if(destMac != -1 && callbacks->song().machine(destMac) != NULL
		&& destParam != -1 && destParam < callbacks->song().machine(destMac)->GetNumParams())
	{
		int minVal, maxVal;
		int newVal;
		callbacks->song().machine(destMac)->GetParamRange(destParam, minVal, maxVal);
		newVal = centerVal[which];
		if(newVal<minVal) newVal=minVal;
		else if(newVal>maxVal) newVal=maxVal;

		if(destMac != this->id()) // craziness may ensue without this check.. folks routing the lfo to itself are on their own
			callbacks->song().machine(destMac)->SetParameter(destParam, newVal); //set to value at lfo==0
	}
}

}}
