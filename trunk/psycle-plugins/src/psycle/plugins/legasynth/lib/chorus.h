/* -*- mode:c++, indent-tabs-mode:t -*- */
/////////////////////////////////////////////////////////////////////
// Dmitry "Sartorius" Kulikov LegaSynth plugins for PSYCLE

/***************************************************************************
							chorus.h  -  description
								-------------------
	begin                : Wed Jul 17 2002
	copyright            : (C) 2002 by red
	email                : red@server
	***************************************************************************/

/***************************************************************************
	*                                                                         *
	*   This program is free software; you can redistribute it and/or modify  *
	*   it under the terms of the GNU General Public License as published by  *
	*   the Free Software Foundation; either version 2 of the License, or     *
	*   (at your option) any later version.                                   *
	*                                                                         *
	***************************************************************************/

#ifndef CHORUS_H
#define CHORUS_H

#include <vector>
#define BUFFER_SIZE 30000
#define M_PI 3.14159265359f
/**
	*@author red
	*/

class Chorus {

		float delay;
		float lfo_speed;
	float lfo_depth;
	float feedback;
		float width;
	
		int index; //osc index
	int ring_buffer_index;

	//std::vector<float> ringbuffer_l;
	//std::vector<float> ringbuffer_r;
	std::vector<int> ringbuffer_l;
	std::vector<int> ringbuffer_r;

	int mixfreq;
	
public:

	void set_mixfreq(int p_mixfreq);

	void process(float *p_lest,float *p_right,int p_amount);
	//void process(int *p_lest,int *p_right,int p_amount);

	//void set_chorus_params(float p_delay, float p_lfo_speed, float p_lfo_depth, float p_feedback, float p_width);
	void set_delay(float p_delay);
	void set_lfo_speed(float p_lfo_speed);
	void set_lfo_depth(float p_lfo_depth);
	void set_feedback(float p_feedback);
	void set_width(float p_width);

	//void get_chorus_params(float &p_delay, float &p_lfo_speed, float &p_lfo_depth, float &p_feedback, float &p_width);
		
	Chorus();
	~Chorus();
};

#endif
