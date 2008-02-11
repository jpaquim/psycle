// -*- mode:c++; indent-tabs-mode:t -*-
/////////////////////////////////////////////////////////////////////
// Dmitry "Sartorius" Kulikov LegaSynth plugins for PSYCLE

/***************************************************************************
								-------------------
	begin                : Wed Jul 10 2002
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


#include <packageneric/pre-compiled.private.hpp> 
#include "303_voice.h"

#include <cmath>

Voice::Status TB303_Voice::get_status(){ //obvious function

	return (vca_mode==2) ? DEAD : ATTACKING; //i think i should change this to SUSTAINING.. ohwell..
	//dont return DEAD unless the voice finished, when you return DEAD, the voice is _killed_.				
}
void TB303_Voice::poll_for_dead_staus(){ //call in case we try to figure out if this note voice is dead

	//but we wont. i think i'll take this function out, it's just for compatibility with DX7
}
void TB303_Voice::set_note_internal(char p_note,char p_velocity) {
	//there is not really need to take the parameters unless you _really_ want them
		//the best is to just use get_fnote() and get_total_volume() later on
	noteoff_was_pressed=false;
	vca_mode = 0;
	recalculate_increment(); //we'll recalculate the increment

}

void TB303_Voice::set_note_off_internal(char p_velocity){

	noteoff_was_pressed=true; //noteoff was pressed!
	vca_mode=1;
}


void TB303_Voice::recalculate_increment() {

	float basenote;
	float frequency;

	basenote=get_fnote(); //fnote is the note, but given as FLOAT (0-127 with decimals), so it allows perfect control for vibratos/sweeps/etc
	basenote+=used_data.coarse; //apply coarse tunning
	basenote+=(float)used_data.fine*0.000015259021896696421759365224689097f; /* /65535.0; */
	frequency=8.1757989156*std::pow(2.0,basenote/12.0); //note -> frequency conversion  8.1757989156 is the midi C-0
	//float const offset = -36.3763165623f; // 6 * 12 - 3 - 12 * ln(440) / ln(2)
	//frequency = std::pow(2.f, (basenote - offset) / 12.f);
//				vco_inc = (440.0/44100.0)*std::pow(2, (patdata[0]-57)*(1.0/12.0));

	vco_inc=frequency/base_freq;
	
	//vca_mode = 0;
	vcf_c0 = vcf_e1;
	vcf_envpos = ENVINC;

}
/*
bool TB303_Voice::set_controller(char p_ctrl,char p_param) {

	switch (p_ctrl) {

		case 80: {

			used_data.envelope_cutoff=(int)p_param << 9;
		} break;
		case 81: {

			used_data.resonance=(int)p_param << 9;
		} break;
		case 82: {

			used_data.envelope_mod=(int)p_param << 9;
		} break;				                
		default: {

			return false; //not found!
		}																																
	}

	return true; //recognized
}
*/

void TB303_Voice::set_data(Data *p_data) {

	used_data=*p_data;
	params.cutoff=p_data->envelope_cutoff;
	params.resonance=p_data->resonance;
}

void TB303_Voice::mix_modifier_call() {
	//remember, this function is called 50 times per second

	//int cutoff_sweep; // -127 to 127
	//int cutoff_lfo_speed; // 0 - 0xFF
	//int cutoff_lfo_depth; // 0 - 0xFF
	
	used_data.envelope_cutoff+=used_data.cutoff_sweep<<4;
	
	if (used_data.envelope_cutoff<0)
		used_data.envelope_cutoff=0;
	if (used_data.envelope_cutoff>0xFFFF)
		used_data.envelope_cutoff=0xFFFF;

		params.cutoff=used_data.envelope_cutoff;


		params.cutoff+=used_data.cutoff_lfo_depth*0x7F*std::sin( ( (float)get_update_count()*(M_PI*2)/(float)0x100)*((float)(used_data.cutoff_lfo_speed+1)/10.0));

	if (params.cutoff<0) params.cutoff=0;
	if (params.cutoff>0xFFFF) params.cutoff=0xFFFF;


	params.resonance=used_data.resonance;
	recalculate_filters();        
	
}

void TB303_Voice::recalculate_pitch_internal() {

	recalculate_increment(); // let's just recalculate the increment
			
}


void TB303_Voice::recalculate_filters() {

	vcf_cutoff = (float)params.cutoff*0.000015259021896696421759365224689097f; /*/65535.0;*/
//				do_recalc = 1;

	vcf_reso = (float)params.resonance*0.000015259021896696421759365224689097f; /*/65535.0;*/
	vcf_rescoeff = std::exp(-1.20 + 3.455*vcf_reso);
//				do_recalc = 1;

	vcf_envmod = (float)used_data.envelope_mod*0.000015259021896696421759365224689097f; /*/65535.0;*/
//				do_recalc = 1;

	float d = (float)used_data.envelope_decay*0.000015259021896696421759365224689097f; /*/65535.0;*/
	d = 0.2 + (2.3*d);
	d*=base_freq;
	vcf_envdecay = std::pow(0.1, 1.0/d * ENVINC);

	// patdata[7] // accent

	vcf_e1 = std::exp(6.109 + 1.5876*vcf_envmod + 2.1553*vcf_cutoff - 1.2*(1.0-vcf_reso));
	vcf_e0 = std::exp(5.613 - 0.8*vcf_envmod + 2.1553*vcf_cutoff - 0.7696*(1.0-vcf_reso));
	vcf_e0*=M_PI/base_freq;
	vcf_e1*=M_PI/base_freq;
	vcf_e1 -= vcf_e0;
	//undenormalise(vcf_e1);
	vcf_envpos = ENVINC;
}


void TB303_Voice::set_mix_frequency_internal(int p_mixfreq){ //stuff we need to recalculate when having the frequency

	base_freq=p_mixfreq;

	recalculate_filters();
	recalculate_increment();

}

//void TB303_Voice::mix_internal(int p_amount,float *p_where_l,float *p_where_r){
void TB303_Voice::mix_internal(int p_amount,int *p_where_l,int *p_where_r){

	float mix_volume_left;
	float mix_volume_right;
	float mix_volume;
	float val;//,left_c,right_c;
	int left_c,right_c;
	left_c=right_c=0;

	mix_volume=get_total_volume()*0.007874015748031496062992125984252f; // (/ 127.0) get_total_volume returns the TOTAL volume in a range from 0 to 1, it inclues main volume, preamp, expression, velocity, tremolo, etc
		
	mix_volume_right=(float)get_total_pan() * mix_volume; //get_total_pan returns the pan to use, it goes from 0 to 127, so we'll do multipliers from there
	mix_volume_left=(float)( 127 - get_total_pan() ) * mix_volume ;

	--p_where_l;
	--p_where_r;

		while (p_amount--) { //ok, the amount of SAMPLES to mix (remember, buffer is STEREO


		float w,k; //,s;

			// update vcf
				if(vcf_envpos >= ENVINC) {
					w = vcf_e0 + vcf_c0;
					k = std::exp(-w/vcf_rescoeff);
					vcf_c0 *= vcf_envdecay;
					vcf_a = 2.0*cos(2.0*w) * k;
					vcf_b = -k*k;
					vcf_c = 1.0 - vcf_a - vcf_b;
					vcf_envpos = 0;
				}

				// compute sample
				val=vcf_a*vcf_d1 + vcf_b*vcf_d2 + vcf_c*vco_k*vca_a;
			//undenormalise(val);
			//anti-denormal code from Jazz
			//unsigned int corrected_sample = *((unsigned int*)&val);
				//corrected_sample *= ((corrected_sample < 0x7F800000) && ((corrected_sample & 0x7F800000) > 0));
				//val = *((float*)&corrected_sample);
			unsigned int corrected_sample = *((unsigned int*)&val);
				unsigned int exponent = corrected_sample & 0x7F800000;
				corrected_sample *= ((exponent < 0x7F800000) & (exponent > 0));
				val = *((float*)&corrected_sample);
			left_c = val * mix_volume_left*REQUESTED_MAX_SAMPLE_VALUE;
			right_c = val * mix_volume_right*REQUESTED_MAX_SAMPLE_VALUE;
			//left_c*=REQUESTED_MAX_SAMPLE_VALUE; //this is the max value for the voice, it's actually (1<<29)
			//right_c*=REQUESTED_MAX_SAMPLE_VALUE;
			*++p_where_l += left_c; //mix to channel buffer, left sample
			*++p_where_r += right_c; //mix to channel buffer, right sample

				vcf_d2=vcf_d1;
				vcf_envpos++;
				vcf_d1=val; //outbuf[i]
				// update vco
				vco_k += vco_inc;
				if(vco_k > 0.5) vco_k -= 1.0;
				// update vca
			if(!vca_mode){
				vca_a+=(vca_a0-vca_a)*vca_attack;
			}
				else if(vca_mode == 1) {
					vca_a *= vca_decay;
					// the following line actually speeds up processing on SGIs
					if(vca_a < (1/65536.0)) { vca_a = 0; vca_mode = 2; }
				}
	}
	//store_last_values(left_c,right_c); //this goes to the declicker, to try avoid clicking if the voice is abruptly cut
	
}

TB303_Voice::TB303_Voice():Voice(){

	noteoff_was_pressed=false;
	base_freq=1;
	//base_freq=44100.f;


	vco_inc=vco_k=vcf_rescoeff=0;
	
	
	vcf_cutoff = 0; vcf_envmod = 0;
	vcf_reso = 0; vcf_envdecay = 0;
	vcf_envpos = ENVINC;
	vcf_a = vcf_b = 0;
	vcf_d1 = vcf_d2 = 0;
	vcf_c0 = 0; vcf_e0 = 0; vcf_e1 = 0;

	vca_mode = 0;  vca_a = vcf_b = vcf_c =0;
	vca_attack = 1.0f - 0.94406088f;
	vca_decay = 0.99897516f;
	vca_a0 = 0.5;
}
TB303_Voice::~TB303_Voice(){

}
