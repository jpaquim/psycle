// -*- mode:c++; indent-tabs-mode:t -*-
/////////////////////////////////////////////////////////////////////
// Dmitry "Sartorius" Kulikov LegaSynth plugins for PSYCLE

/***************************************************************************
							voice.cpp  -  description
								-------------------
	begin                : Fri Jul 5 2002
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
#include "voice.h"

#include <cmath>

void Voice::mix_modifier() {

	if (update_count>sweep_delay*4) {
		sweep_level+=(sweep_value*sweep_value*0.0025)*((sweep_value<0)?-1.0:1.0); //exponential curve for sweeping
		recalculate_pitch_internal();								
	}

	if (update_count>LFO_delay*4) {

		int phase=(LFO_speed*update_count*10)&0xFFF;
		
		switch (LFO_type) {

			//phase is 0-1024              				
		
			case 0: {//sine
				LFO_level=std::sin((float)phase*(M_PI*2.0)/(float)0x1000);
			} break;
			case 1: {//saw
				LFO_level=(float)phase/(float)0x1000;
				LFO_level*=2;
				LFO_level-=1;
			} break;
			case 2: {//pulse
				LFO_level=(phase<0x800)?-1:1;
			} break;

		}
		
		LFO_level*=LFO_depth;
		LFO_level/=10;
		
		recalculate_pitch_internal();
	}
	
	mix_modifier_call();
}

float Voice::get_fnote() {

		float fnote = (float)note + (float)(pitch_bend-0x2000)*(float)pitch_bend_depth/(float)0x2000;

		fnote+=sweep_level;
		fnote+=LFO_level;

		if (fnote<0) fnote=0;
		if (fnote>127) fnote=127;

	return fnote;
}
float Voice::get_total_volume() {

	float totalvol;

	totalvol=main_volume;
	totalvol*=expression;
	totalvol*=velocity;
	totalvol*=0.00000048818995275785827162205505513373f; // /(127.0*127.0*127.0);
	totalvol*=preamp;

	return totalvol;
}

int Voice::get_update_count() {

	return update_count;
}

void Voice::mix(int p_amount,int *p_where_l,int *p_where_r) {

	if (update_count==0) mix_modifier();

			int amount=(update_ofs<p_amount)?update_ofs:p_amount;
				update_ofs-=amount;


			poll_for_dead_staus();
			if (get_status()==DEAD) return;

			mix_internal(p_amount,p_where_l,p_where_r);
			//mix_internal(amount,&buffer_l.front(),&buffer_r.front());

			//poll_for_dead_staus();
			//if (get_status()==DEAD) return;


			if (update_ofs<=0) {
					update_ofs=update_interval_samples;
					update_count++;
				mix_modifier();
				}

}

void Voice::set_sustain(bool p_sustain) {

			sustain=p_sustain;

	if (!sustain && sustaining) {
		set_note_off(sustain_noteoff_velocity);
		sustaining=false;
	}
}

void Voice::set_note(char p_note,char p_velocity) {

	note=p_note;
	velocity=p_velocity;
	set_note_internal(p_note,p_velocity);
	recalculate_pitch_internal();
}

void Voice::set_note_off(char p_velocity) {

	if (sustain) {

		sustaining=true;
		sustain_noteoff_velocity=p_velocity;
		return;
	}

	set_note_off_internal(p_velocity);								
}


void Voice::set_pan(char p_pan) {

	pan=p_pan;
}
void Voice::set_pitch_bend(int p_bend) {

	pitch_bend=p_bend;
	recalculate_pitch_internal();
}
void Voice::set_pitch_depth(int p_bend_depth) {

	pitch_bend_depth=p_bend_depth;
	recalculate_pitch_internal();
}
void Voice::set_expression(char p_vol) {

	expression=p_vol;
}

void Voice::set_velocity(char p_vol) {

	velocity=p_vol;
}

void Voice::set_main_volume(char p_vol) {

	main_volume=p_vol;
}


int Voice::get_pitch_bend() {

	return pitch_bend;
}

int Voice::get_pitch_depth() {

	return pitch_bend_depth;
}

char Voice::get_pan() {

	return pan;
}

int Voice::get_total_pan() {

	int aux_pan=pan+pan_relative;
	if (aux_pan<0) aux_pan=0;
	if (aux_pan>127) aux_pan=127;

	return aux_pan;
}
char Voice::get_note() {

	return note;
}

char Voice::get_expression() {

	return expression;
}

char Voice::get_main_volume() {

	return main_volume;
}

void Voice::set_mix_frequency(int p_mixfreq) {

	update_interval_samples=p_mixfreq/UPDATES_PER_SECOND;
	update_ofs=update_interval_samples;

	set_mix_frequency_internal(p_mixfreq);
}

void Voice::mix_modifier_call() {


}
//void Voice::set_channel(char p_channel) {
//
//				channel=p_channel;
//
//}
//char Voice::get_channel() {
//
//				return channel;
//}
void Voice::set_default_data(Default_Data *p_data) {

	pan_relative=p_data->relative_pan;
	sweep_delay=p_data->sweep_delay;
	sweep_value=p_data->sweep_value;

	LFO_speed=p_data->LFO_speed;
	LFO_depth=p_data->LFO_depth;
	LFO_type=p_data->LFO_type;
	LFO_delay=p_data->LFO_delay;

	//last_val_l=0;
	//last_val_r=0;

}


Voice::Voice(){
/*
	buffer_l.assign(MIX_BUFFER_SIZE,0);
	buffer_r.assign(MIX_BUFFER_SIZE,0);
*/
	update_ofs =0;
	update_count =0;
	
	note=0;
	pitch_bend_depth=2;
	pitch_bend=0x2000;
	pan=64;

	expression=127;
	main_volume=127;
	velocity=127;

	channel=0;
	mixing_frequency=0;

		update_interval_samples=0;
		update_ofs=0;
		update_count=0;

		sweep_level=0;
	LFO_level=0;

	preamp=1;
		//sustain=true;
		sustain=false;
		sustaining=false;
	//last_val_l = 0;
	//last_val_r = 0;
}

//float Voice::get_last_value_L() {
//
//				return last_val_l;
//}
//
//float Voice::get_last_value_R() {
//
//				return last_val_r;
//
//}

void Voice::set_preamp(float p_preamp) {

	preamp=p_preamp;
}
bool Voice::set_controller(char p_ctrl,char p_param) { //true if recognized

	return false; //not recognized by default
	
}
/*
void Voice::process_declicker() {

	int l,r;
	unsigned int index=0;
	while(index<declicker_l.size()) {

		l=(int)declicker_l[index];
		r=(int)declicker_r[index];

		if (l>DECLICKER_THRESHOLD) {

			for (int i=(DECLICKER_BUFFER_SIZE-1);i>=0;i--) {

				buffer_l[(DECLICKER_BUFFER_SIZE-i)]+=i*l/DECLICKER_BUFFER_SIZE;

			}
		}
		if (r>DECLICKER_THRESHOLD) {

			for (int i=(DECLICKER_BUFFER_SIZE-1);i>=0;i--) {

				buffer_r[(DECLICKER_BUFFER_SIZE-i)]+=i*r/DECLICKER_BUFFER_SIZE;

			}
		}
		index++;
	}
	declicker_l.clear();
	declicker_r.clear();
}
*/

Voice::~Voice(){

}
