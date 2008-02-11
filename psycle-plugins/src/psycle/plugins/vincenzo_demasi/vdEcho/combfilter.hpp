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

// 500 millis. at 44100 Hz
#define COMB_FILTER_BUFFER_SIZE 22051
#define COMB_FILTER_MAX_DELAY   (COMB_FILTER_BUFFER_SIZE - 1)

class CombFilter
{

private:
	float buffer[COMB_FILTER_BUFFER_SIZE];
	unsigned int writein, readout;

public:

	CombFilter()
	{
		readout = writein = 0;
		for(register int i = 0; i < COMB_FILTER_BUFFER_SIZE; ++i)
			buffer[i] = 0.0f;
	}

	~CombFilter()
	{
	}

	inline void setDelay(unsigned int samples)
	{
		if((writein = readout + samples) >= COMB_FILTER_BUFFER_SIZE)
			writein -= COMB_FILTER_BUFFER_SIZE;
	}

	inline float process(float sample)
	{
		buffer[writein] = sample;
		sample = buffer[readout];
		if(++writein == COMB_FILTER_BUFFER_SIZE) writein = 0;
		if(++readout == COMB_FILTER_BUFFER_SIZE) readout = 0;
		return sample;
	};
};
