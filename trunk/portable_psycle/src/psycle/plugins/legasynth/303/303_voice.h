/////////////////////////////////////////////////////////////////////
// Dmitry "Sartorius" Kulikov LegaSynth plugins for PSYCLE

/***************************************************************************
                          303_voice.h  -  description
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

#ifndef TB303_VOICE_H
#define TB303_VOICE_H
#include "../lib/voice.h"

/**
  *@author red
  * CONTAINS PORTIONS FOUND IN GSYNTH
  */

class TB303_Voice : public Voice  {
public:

	struct Data { //the bank will need this data to configure us

        int coarse; //coarse -24 to +24
        int fine; //coarse -65535 to 65535
		int envelope_cutoff; //"Cut", "Filter envelope cutoff 0 - 0xFFFF"
		int resonance; //"Res", "Filter resonance - 0 - 0xFFFF 
		int envelope_mod; //"Env", "Filter envelope modulation"
		int envelope_decay; // "Filter envelope decay" - 0 - 0xFFFF

		int cutoff_sweep; // -127 to 127
		int cutoff_lfo_speed; // 0 - 0xFF
		int cutoff_lfo_depth; // 0 - 0xFF
		
		Data() {

			memset(this,0,sizeof(Data)); //zero it
			envelope_cutoff=0x7FFF;
  		}
	};

private:
        enum {

        	ENVINC=64
        };
        
	float base_freq; //well cache the base freq
    bool noteoff_was_pressed;
	Data used_data; //cache of data
/*GSYN STUFF!*/

	struct DynamicParams {

		int cutoff;
		int resonance;
	} params;


	float vco_inc, vco_k;

	float vcf_cutoff, vcf_envmod, vcf_envdecay, vcf_reso, vcf_rescoeff;
	float vcf_e0, vcf_e1;
	float vcf_c0; // c0=e1 on retrigger; c0*=ed every sample; cutoff=e0+c0
	float vcf_d1, vcf_d2;
	float vcf_a, vcf_b, vcf_c;
	int vcf_envpos;

	float vca_attack, vca_decay, vca_a0, vca_a;
	int vca_mode; // attack/decay mode

/*ENDOF GSYN STUFF*/

	

	void recalculate_increment();
	void recalculate_filters();
	virtual void recalculate_pitch_internal(); //this will get called when the pitch of the note changes
public:

	void set_data(Data *p_data);


	Status get_status(); //bit obvious :)

	void poll_for_dead_staus(); //call in case we try to figure out if this note voice is dead, but i guess we dont

    void set_note_internal(char p_note,char p_velocity); //this gets called on noteon
	void set_note_off_internal(char p_velocity); //and this on noteoff
	
	void mix_modifier_call(); //this gets called, i think, 50 times per second in case we want to periodically change some modifier
	//bool set_controller(char p_ctrl,char p_param);

public: /* MIXING! */

	void set_mix_frequency_internal(int p_mixfreq); //this will be called to configure the frequency
	//void mix_internal(int p_amount,float *p_where_l,float *p_where_r); //and this will ask us to mix the voice
	void mix_internal(int p_amount,int *p_where_l,int *p_where_r);

	TB303_Voice();
	~TB303_Voice();
};

#endif
