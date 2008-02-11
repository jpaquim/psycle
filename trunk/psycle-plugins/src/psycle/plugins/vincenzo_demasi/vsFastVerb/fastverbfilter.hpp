// -*- mode:c++; indent-tabs-mode:t -*-
/*      Copyright (C) 2002 Vincenzo Demasi.

		This plugin is free software; you can redistribute it and/or modify
		it under the terms of the GNU General Public License as published by
		the Free Software Foundation; either version 2 of the License, or
		(at your option) any later version.\n"\

		This plugin is distributed in the hope that it will be useful,
		but WITHOUT ANY WARRANTY; without even the implied warranty of
		MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
		GNU General Public License for more details.

		You should have received a copy of the GNU General Public License
		along with this program; if not, write to the Free Software
		Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

		Vincenzo Demasi. E-Mail: <v.demasi@tiscali.it>
*/

#pragma once

// 250 millis. at 44100 Hz
#define FASTVERB_FILTER_BUFFER_SIZE 22051
#define FASTVERB_FILTER_MAX_DELAY   (FASTVERB_FILTER_BUFFER_SIZE - 1)

class FastverbFilter
{

private:
	float buffer[FASTVERB_FILTER_BUFFER_SIZE];
	unsigned int writein, readout;
	float feedback;

public:

	FastverbFilter()
	{
		readout = writein = 0;
		for(register int i = 0; i < FASTVERB_FILTER_BUFFER_SIZE; ++i)
			buffer[i] = 0.0f;
	}

	~FastverbFilter()
	{
	}

	inline void setDelay(unsigned int samples)
	{
		if((writein = readout + samples) >= FASTVERB_FILTER_BUFFER_SIZE)
			writein -= FASTVERB_FILTER_BUFFER_SIZE;
	}

	inline void setFeedback(float gain)
	{
		feedback = gain;
	}

	inline float process(float sample)
	{
		/* WARNING: not properly work when delay=0 because readout==writein
			(ignored for efficiency) */
		float output = sample + feedback * buffer[readout];
		buffer[writein] = output;
		if(++writein == FASTVERB_FILTER_BUFFER_SIZE) writein = 0;
		if(++readout == FASTVERB_FILTER_BUFFER_SIZE) readout = 0;
		return output;
	};
};
