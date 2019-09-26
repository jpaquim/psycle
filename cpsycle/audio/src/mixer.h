// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(MIXER_H)
#define MIXER_H

#include "machine.h"


#define MAX_CONNECTIONS		12

typedef struct {
	// std::vector<float> sendVols_;
	float volume_;
	float panning_;
	float drymix_;
	unsigned char mute_;
	unsigned char dryonly_;
	unsigned char wetonly_;
} MixerInputChannel;

void mixerinputchannel_init(MixerInputChannel*);

typedef struct {
	// Wire wire_;
	// std::vector<bool> sendsTo_;
	unsigned char mastersend_;
	float volume_;
	float panning_;
	unsigned char mute_;
} MixerReturnChannel;

void mixerreturnchannel_init(MixerReturnChannel*);

typedef struct {
	float volume_;
	float drywetmix_;
	float gain_;
} MixerMasterChannel;

void mixermasterchannel_init(MixerMasterChannel*);


typedef struct {
	Machine machine;	
	/// helper variable for scheduled processing: returns can be routed to other returns
	int sched_returns_processed_curr;
	int sched_returns_processed_prev;
	unsigned char mixed;

	int solocolumn_;
	// std::vector<InputChannel> inputs_;
	// std::vector<ReturnChannel> returns_;
	// std::vector<Wire*> sends_;
	MixerMasterChannel master_;
	// std::vector<LegacyWire> legacyReturn_;
	// std::vector<LegacyWire> legacySend_;

	// Arrays of precalculated values for FxSend and Mix functions
	// _sendvol = vol from ins to sends
	// _mixvol = vol from ins to master
	// mixvolret = vol from returns to sends (if redirected)
	// last mixvolret = vol from returns to master.
	float _sendvolpl[MAX_CONNECTIONS][MAX_CONNECTIONS];
	float _sendvolpr[MAX_CONNECTIONS][MAX_CONNECTIONS];
	float mixvolpl[MAX_CONNECTIONS];
	float mixvolpr[MAX_CONNECTIONS];
	float mixvolretpl[MAX_CONNECTIONS][MAX_CONNECTIONS+1]; // +1 is Master
	float mixvolretpr[MAX_CONNECTIONS][MAX_CONNECTIONS+1];
	// Wire::Mapping sendMapping[2*MAX_CONNECTIONS][MAX_CONNECTIONS];//ins and returns to sends.

	/// tweaks:
	/// [0x]:
	///  0 -> Master volume
	///  1..C -> Input volumes
	///  D -> master drywetmix.
	///  E -> master gain.
	///  F -> master pan.
	/// [1x..Cx]:
	///  0 -> input wet mix.
	///  1..C -> input send amout to the send x.
	///  D -> input drywetmix. ( 0 normal, 1 dryonly, 2 wetonly  3 mute)
	///  E -> input gain.
	///  F -> input panning.
	/// [Dx]:
	///  0 -> Solo channel.
	///  1..C -> return grid. // the return grid array grid represents: bit0 -> mute, bit 1..12 routing to send. bit 13 -> route to master
	/// [Ex]: 
	///  1..C -> return volumes
	/// [Fx]:
	///  1..C -> return panning
} Mixer;

void mixer_init(Mixer*, MachineCallback);


#endif