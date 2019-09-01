// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net
#include "player.h"
#include "math.h"
#include "plugin.h"
#include "vstplugin.h"
#include "silentdriver.h"
#include <operations.h>

static float buffer[65535];
static float left[65535];
static float right[65535];

static float* Work(Player* player, int* numsamples);
static void Interleave(float* dst, float* left, float* right, int num);
static void OnEnumerateSequencer(Machine* machine, int slot, PatternNode* events);
static float Offset(Player* self, int numsamples);
static void player_loaddriver(Player* self, const char* path);
static void player_unloaddriver(Player* self);

extern HANDLE hGuiEvent;
extern HANDLE hWorkDoneEvent;

#define STREAM_SIZE 256

typedef EXPORT void (CALLBACK  *ptest)(void);

void player_init(Player* self, Song* song, const char* driverpath)
{
	char path[MAX_PATH];
	Master* master;
		
	self->song = song;	
	self->pos = 0.0f;
	self->playing = FALSE;
	self->lpb = 4;
	
	sequencer_init(&self->sequencer);
	self->sequencer.sequence = &song->sequence;
	self->silentdriver = create_silent_driver();
	self->silentdriver->init(self->silentdriver);	
	
	strcpy(path, driverpath);
	strcat(path, "mme.dll");
	player_loaddriver(self, path);
	
	self->t = 125 / (44100 * 60.0f);
	master = malloc(sizeof(Master));
	master_init(master);
	machines_insert(&self->song->machines, 0, (Machine*)master);
		
	self->driver->open(self->driver);
}

void player_loaddriver(Player* self, const char* path)
{
	Driver* driver = 0;	
	self->module = LoadLibrary(path);
	if (self->module) {
		pfndriver_create pdrivercreate;			
		pdrivercreate = (pfndriver_create) GetProcAddress (self->module, "driver_create");
		if (pdrivercreate) {
			driver = pdrivercreate();					
			driver->init(driver);
			driver->connect(driver, self, Work);
		}
	}
	if (!driver) {
		driver = self->silentdriver;
	}
	self->driver = driver;
}

void player_unloaddriver(Player* self)
{
	if (self->driver && self->driver != self->silentdriver && self->module) {
		self->driver->dispose(self->driver);
		self->driver->free(self->driver);
		FreeLibrary(self->module);
		self->module = 0;
	}
}

void player_dispose(Player* self)
{		
	if (self->driver != self->silentdriver) {
		ResetEvent(hGuiEvent);
		WaitForSingleObject(hWorkDoneEvent, INFINITE);
		self->driver->close(self->driver);
		SetEvent(hGuiEvent);		
		player_unloaddriver(self);
	}	
	self->silentdriver->dispose(self->silentdriver);
	self->silentdriver->free(self->silentdriver);
	self->silentdriver = 0;	
}

void player_start(Player* self)
{
	self->pos = 0.0f;
	sequencer_setposition(&self->sequencer, 0.0f);
	self->playing = TRUE;
}

void player_stop(Player* self)
{
	self->playing = FALSE;
}

float player_position(Player* self)
{
	return self->sequencer.pos;
}

void player_setbpm(Player* self, float bpm)
{
	if (bpm < 32) {
		self->sequencer.bpm = 32;
	} else
	if (bpm > 999) {
		self->sequencer.bpm = 999;
	} else {
		self->sequencer.bpm = bpm;
	}
	self->t = self->sequencer.bpm / (44100 * 60.0f);
}

void player_setlpb(Player* self, unsigned int lpb)
{
	self->lpb = lpb;
}

float Offset(Player* self, int numsamples)
{
	return numsamples * self->t;
}

float* Work(Player* self, int* numsamples)
{		
	int amount;
	int numSamplex = *numsamples;
	float* pSamples = buffer;
	Machine* master;
		
	if (numSamplex > STREAM_SIZE)
	{
		amount = STREAM_SIZE;
	}
	else
	{
		amount = numSamplex;
	}
	do
	{							
		MachinePath* path = self->song->machines.path;
		if (self->playing) {
			sequencer_tick(&self->sequencer, Offset(self, amount));			
		}		
		if (path) {
			MachinePath* pathptr;						
			SequencePtr curr = self->sequencer.curr;
			pathptr = path;
			while (pathptr != NULL) {
				Machine* machine;
				MachineConnections* connections;				
				int i;

				machine = machines_at(&self->song->machines, (int)pathptr->entry);
				connections = machines_connections(&self->song->machines, (int)pathptr->entry);				

				for (i = 0; i < 2; ++i) {
					machine->outputs[i] = machines_nextbuffer(&self->song->machines);				
					dsp_clear(machine->outputs[i], amount);					
				}

				if (connections) {
					MachineConnection* connectionptr;
					connectionptr = &connections->inputs;
					while (connectionptr) {
						if (connectionptr->slot != -1) {
							Machine* source = machines_at(&self->song->machines, connectionptr->slot);
							for (i = 0; i < 2; ++i) {
								dsp_add(source->outputs[i], machine->outputs[i], amount, 1.0f);							
							}
						}
						connectionptr = connectionptr->next;
					}								
				}				
				if ((int)(pathptr->entry) != 0) {
					if (self->playing) {												
						self->sequencer.curr = curr;
						sequencer_enumerate(&self->sequencer, machine, (int)(pathptr->entry), OnEnumerateSequencer);						
					}
					machine->work(machine, amount, 16);
					signal_emit(&machine->signal_worked, machine, 1, amount);
				}
				pathptr = pathptr->next;
			}
							
		}
		master = machines_at(&self->song->machines, 0);
		if (master && master->outputs[0] && master->outputs[1]) {	
			signal_emit(&master->signal_worked, master, 1, amount);
			dsp_interleave(pSamples, master->outputs[0], master->outputs[1], amount);			
		}
		numSamplex -= amount;		
		pSamples  += 2*amount;
	
	} 
	while (numSamplex);
	SetEvent(hWorkDoneEvent);
	WaitForSingleObject(hGuiEvent, INFINITE);	
	
//	dsp_mul(buffer, *numsamples, 0.5);
	return buffer;
}

void OnEnumerateSequencer(Machine* machine, int slot, PatternNode* node)
{
	PatternEvent* event = &((PatternEntry*)(node->entry))->event;
	if (event->mach == slot) {
		machine->seqtick(machine, 0, event->note, event->inst, event->cmd,
			event->parameter);	
	}
}





