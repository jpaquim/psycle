/***************************************************************************
                          cpp  -  description
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
#include <project.private.hpp>
#include "chorus.h"
#include <math.h>

Chorus::Chorus(){

	ringbuffer_l.assign(BUFFER_SIZE,0);
	ringbuffer_r.assign(BUFFER_SIZE,0);

 	delay=3;
   	lfo_speed=2;
    lfo_depth=1;
    feedback=64;
 	width=-10;
  	index=0;
	ring_buffer_index=0;
}

void Chorus::set_chorus_params(float p_delay, float p_lfo_speed, float p_lfo_depth, float p_feedback, float p_width) {

	delay=p_delay;
	lfo_speed=p_lfo_speed;
 	lfo_depth=p_lfo_depth;
  	feedback=p_feedback;
   	width=p_width;

}
/*
void Chorus::get_chorus_params(float &p_delay, float &p_lfo_speed, float &p_lfo_depth, float &p_feedback, float &p_width) {

	p_delay=delay;
	p_lfo_speed=lfo_speed;
 	p_lfo_depth=lfo_depth;
  	p_feedback=feedback;
   	p_width=width;

}
*/
void Chorus::set_mixfreq(int p_mixfreq) {

	mixfreq=p_mixfreq;

}

void Chorus::set_delay(float p_delay) {

	delay=p_delay;

}
void Chorus::set_lfo_speed(float p_lfo_speed) {

	lfo_speed=p_lfo_speed;

}
void Chorus::set_lfo_depth(float p_lfo_depth) {

	lfo_depth=p_lfo_depth;

}
void Chorus::set_feedback(float p_feedback) {

	feedback=p_feedback;

}
void Chorus::set_width(float p_width) {

	width=p_width;

}

void Chorus::process(float *p_left,float *p_right,int p_amount) {

    float *p_s_left = p_left;
	float *p_s_right = p_right;
	--p_s_left;
	--p_s_right;

	float delay_samples = (delay/1000.0) * (float)mixfreq;
	float depth_samples = (lfo_depth/1000.0) * (float)mixfreq;

        int copy_amount=p_amount;
        int aux_idx=0;

        float real_feedback=feedback/*127.0*//100.0;
        float real_width=width/127.0/*/100.0*/;


        while (copy_amount--) {

        	ringbuffer_l[(ring_buffer_index+aux_idx) % BUFFER_SIZE] = *++p_s_left;
			ringbuffer_r[(ring_buffer_index+aux_idx) % BUFFER_SIZE] = *++p_s_right;
        	aux_idx++;	
        }
	
        
	copy_amount=p_amount;
    --p_left;
	--p_right;

        while (copy_amount--) {

                float read_index=((float)index*lfo_speed*M_PI)/*256.0*//(float)mixfreq;
				int final_index=delay_samples+(depth_samples+std::sin(read_index/**M_PI/256.0*/)*depth_samples);

                
                float val = ringbuffer_l[ (BUFFER_SIZE+(ring_buffer_index-final_index)) % BUFFER_SIZE ];//take left val
                
                float left=(val*real_feedback); //apply the feedback thing
                
                val = ringbuffer_r[ (BUFFER_SIZE+(ring_buffer_index-final_index)) % BUFFER_SIZE  ] ; //take right val
                
                float right=(val*real_feedback); //apply the feedback thing

                val=right;
                right=(right*(1/*27*/-real_width)/*>>7*/) + (left*(real_width)/*>>7*/);
                left=(val*(real_width)/*>>7*/) + (left*(1/*27*/-real_width)/*>>7*/);


		*++p_left +=left;
		*++p_right +=right;

  		index++;
    
		ring_buffer_index++;
		ring_buffer_index%=ringbuffer_l.size();
        }



}



Chorus::~Chorus(){
}
