/////////////////////////////////////////////////////////////////////
// Dmitry "Sartorius" Kulikov LegaSynth plugins for PSYCLE

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
#include "chorus.h"
#include <cmath>

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

//void Chorus::set_chorus_params(float p_delay, float p_lfo_speed, float p_lfo_depth, float p_feedback, float p_width) {
//
//				delay=p_delay;
//				lfo_speed=p_lfo_speed;
// 				lfo_depth=p_lfo_depth;
//  				feedback=p_feedback;
//   				width=p_width;
//
//}
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

	//--p_s_left;
	//--p_s_right;

	int delay_samples = (delay/1000.0) * (float)mixfreq;
	int depth_samples = (lfo_depth/10000.0) * (float)mixfreq;

	if (delay_samples+depth_samples*2>=BUFFER_SIZE){
		delay_samples = BUFFER_SIZE - (depth_samples + 10);
		if (delay_samples<0) {

						delay_samples=0;
					depth_samples=BUFFER_SIZE-10;
					}
	}

		int copy_amount=p_amount;
//        int aux_idx=0;

		float real_feedback=feedback/*127.0*//100.0;
		float real_width=width/127.0/*/100.0*/;

		//++copy_amount;
	//     while (copy_amount--) {

	//     				ringbuffer_l[(ring_buffer_index+aux_idx) % BUFFER_SIZE] = (int)*++p_s_left;
			//ringbuffer_r[(ring_buffer_index+aux_idx) % BUFFER_SIZE] = (int)*++p_s_right;
	//     				aux_idx++;				
	//     }
	
		
//				copy_amount=p_amount;
	//   --p_left;
	//--p_right;

		while (copy_amount--) {
				
				float weight_l = delay_samples+depth_samples*(1.0+std::sin( index*lfo_speed*(M_PI)/mixfreq ));
				float weight_r = delay_samples+depth_samples*(1.0+std::sin( index*lfo_speed*(M_PI+M_PI/4)/mixfreq ));
				
				//int final_index_l=(int)weight_l;
				//int final_index_r=(int)weight_r;


				//int final_index_l=intdelay_samples+depth_samples*(1.0+std::sin( index*lfo_speed*(M_PI)/mixfreq ));
				//int final_index_r=delay_samples+depth_samples*(1.0+std::sin( index*lfo_speed*(M_PI+M_PI/4)/mixfreq ));

				int final_index_l = (BUFFER_SIZE+ring_buffer_index-(int)weight_l) % BUFFER_SIZE;
				int final_index_r = (BUFFER_SIZE+ring_buffer_index-(int)weight_r) % BUFFER_SIZE;

				weight_l = weight_l - std::floor(weight_l);
				weight_r = weight_r - std::floor(weight_r);


				/*float*/ int val = ringbuffer_l[final_index_l]*(1.0-weight_l) + ringbuffer_l[(final_index_l+1==BUFFER_SIZE ? 0 : final_index_l+1)] * weight_l;//.at(final_index_l); //take left val
				
				/*float*/ int left=val*real_feedback+(int)*p_s_left; //apply the feedback thing
				
				val = ringbuffer_r[final_index_r]*(1.0-weight_r) + ringbuffer_r[(final_index_r+1==BUFFER_SIZE ? 0 : final_index_r+1)] * weight_l;//.at(final_index_l); //.at(final_index_r); //take right val
				
				/*float*/ int right=val*real_feedback+(int)*p_s_right; //apply the feedback thing

				ringbuffer_l[ring_buffer_index]=left;
				ringbuffer_r[ring_buffer_index]=right;
				
				val=right;
				right=(right*(1/*27*/-real_width)/*>>7*/) + (left*(real_width)/*>>7*/);
				left=(val*(real_width)/*>>7*/) + (left*(1/*27*/-real_width)/*>>7*/);
				//if (left>32767) left=32767;
				//if (left<-32767) left=-32767;
				//if (right>32767) right=32767;
				//if (right<-32767) right=-32767;


			*p_s_left =left;
			*p_s_right =right;

				//if (index++==44100)index=0;
			index++;

		
			ring_buffer_index++;
			ring_buffer_index%=BUFFER_SIZE;
			++p_s_left;
			++p_s_right;

		}



}



Chorus::~Chorus(){
}
