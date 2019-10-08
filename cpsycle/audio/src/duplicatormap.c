// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "duplicatormap.h"
#include <stdlib.h>

#define NUMMACHINES 8

void duplicatormap_init(DuplicatorMap* self)
{
	int i;
	int j;

	for (i=0; i< NUMMACHINES;i++)
	{		
		for (j=0;j<MAX_TRACKS;j++)
		{
			self->allocatedchans[j][i] = -1;
		}
	}
	for (i=0;i<MAX_VIRTUALINSTS;i++)
	{
		for (j=0;j<MAX_TRACKS;j++)
		{
			self->availablechans[i][j] = 1;
		}
	}
}

void duplicatormap_dispose(DuplicatorMap* self)
{
}

void duplicatormap_allocate(DuplicatorMap* self, int channel, int machine, int outputmachine)
{
	int j=channel;
	// If this channel already has allocated channels, use them.
	if (self->allocatedchans[channel][machine] != -1 )
		return;
	// If not, search an available channel	
	while (j<MAX_TRACKS && !self->availablechans[outputmachine][j]) j++;
	if ( j == MAX_TRACKS)
	{
		j=0;
		while (j<MAX_TRACKS && !self->availablechans[outputmachine][j]) j++;
		if (j == MAX_TRACKS)
		{
			j = (unsigned int) (  (double)rand() * MAX_TRACKS /(((double)RAND_MAX) + 1.0 ));
		}
	}
	self->allocatedchans[channel][machine]=j;
	self->availablechans[outputmachine][j]=0;
}

int duplicatormap_at(DuplicatorMap* self, int channel, int machine)
{
	return self->allocatedchans[channel][machine];
}

void duplicatormap_remove(DuplicatorMap* self, int channel, int machine, int outputmachine)
{
	if (self->allocatedchans[channel][machine] == -1 )
			return;
	self->availablechans[outputmachine][self->allocatedchans[channel][machine]]= 1;
	self->allocatedchans[channel][machine]=-1;
}

