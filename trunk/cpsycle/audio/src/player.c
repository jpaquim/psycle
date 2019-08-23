// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net
#include "player.h"
#include "math.h"
#include "plugin.h"
#include "vstplugin.h"
#include <operations.h>

static float buffer[65535];
static float left[65535];
static float right[65535];

static float* Work(Player* player, int* numsamples);
static void Interleave(float* dst, float* left, float* right, int num);
static void OnEnumerateSequencer(Machine* machine, int slot, PatternNode* events);

extern HANDLE hGuiEvent;
extern HANDLE hWorkDoneEvent;

#define STREAM_SIZE 256

void player_init(Player* self, Song* song)
{
	Master* master;		
	
	self->song = song;	
	self->pos = 0.0f;
	self->playing = FALSE;	
	
	sequencer_init(&self->sequencer);
	self->sequencer.sequence = &song->sequence;
	self->driver = (Driver*) malloc(sizeof(Driver));
	self->t = 125 / (44100 * 60.0f);
	driver_init(self->driver);
	driver_connect(self->driver, self, Work);	

	master = malloc(sizeof(Master));
	master_init(master);
	machines_insert(&self->song->machines, 0, (Machine*)master);	
	
	driver_open(self->driver);		
}

void player_dispose(Player* self)
{		
	ResetEvent(hGuiEvent);
	WaitForSingleObject(hWorkDoneEvent, INFINITE);
	driver_close(self->driver);
	SetEvent(hGuiEvent);	
	driver_dispose(self->driver);
	free(self->driver);
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

				machine = machines_at(&self->song->machines, (int)pathptr->node);
				connections = machines_connections(&self->song->machines, (int)pathptr->node);				

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
				if ((int)(pathptr->node) != 0) {
					if (self->playing) {												
						self->sequencer.curr = curr;
						sequencer_enumerate(&self->sequencer, machine, (int)(pathptr->node), OnEnumerateSequencer);						
					}
					machine->work(machine, amount, 16);				
				}
				pathptr = pathptr->next;
			}
							
		}
		master = machines_at(&self->song->machines, 0);
		if (master && master->outputs[0] && master->outputs[1]) {				
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
	PatternEvent* event = &((PatternEntry*)(node->node))->event;
	if (event->mach == slot) {
		machine->seqtick(machine, 0, event->note, event->inst, event->cmd,
			event->parameter);	
	}
}





