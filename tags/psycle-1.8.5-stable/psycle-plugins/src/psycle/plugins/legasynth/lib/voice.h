/////////////////////////////////////////////////////////////////////
// Dmitry "Sartorius" Kulikov LegaSynth plugins for PSYCLE

/***************************************************************************
							voice.h  -  description
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

#ifndef VOICE_H
#define VOICE_H

#include <vector>

#define M_PI 3.14159265359f


class Voice {


	int note;
	int pitch_bend_depth;
	int pitch_bend;
	int pan;

	int pan_relative;

	int LFO_speed;
	int LFO_depth;
	int LFO_type; // 0 - sine / 1 - saw / 2- pulse
	int LFO_delay;
	
	int sweep_delay;
	int sweep_value;
	
	int velocity;
	int expression;
	int main_volume;
	
	char channel;
	int mixing_frequency;
	
		int update_interval_samples; // update interfal for effects (samples)
		int update_ofs;
		int update_count;

		float sweep_level;
		float LFO_level;
	float preamp;

		bool sustain;
		bool sustaining;
	char sustain_noteoff_velocity;

	//float last_val_l;
	//float last_val_r;
/*
	std::vector<float> buffer_l;
	std::vector<float> buffer_r;

	std::vector<float> declicker_l;
	std::vector<float> declicker_r;
*/
		void mix_modifier();


	
	
public: /* STATUS */


	struct Default_Data {

			/* Voice Stuff */
	
		int relative_pan;

		int LFO_speed;
		int LFO_depth;
		int LFO_type; // 0 - sine / 1 - saw / 2- pulse
		int LFO_delay; 
		
		int sweep_delay;
		int sweep_value;
		
			/* Channel Default Stuff */
		
		//int vibrato_depth;
		//int vibrato_speed;

		int channel_pan;

		int main_volume;
	
		//int default_chorus;
		//int default_reverb;

		//int portamento_time_coarse;
		//int portamento_time_fine;
		//bool do_portamento;

				Default_Data() {

					memset(this,0,sizeof(Default_Data));
						channel_pan=0;
						relative_pan=0;
						main_volume=127;
						//vibrato_speed=30;
				}

	};

	enum {
		UPDATES_PER_SECOND=60,
		REQUESTED_MAX_SAMPLE_VALUE=(1<<16) //(1<<29)
	};


		enum Status {
		DEAD,
		ATTACKING,
		RELEASING,
		FINISHED_ON_VOLUME
	}; //knowing the phase status is very important, since that's what the vchannel kill algorithm uses to kill notes when out of vchannels

	virtual Status get_status() =0; //obvious function

	virtual void poll_for_dead_staus() =0; //call in case we try to figure out if this note voice is dead


protected:
	//virtual void mix_internal(int p_amount,float* p_where_l,float* p_where_r)=0; //set where to mix
	virtual void mix_internal(int p_amount,int *p_where_l,int *p_where_r)=0; //set where to mix
	virtual void mix_modifier_call(); //mixer modifier call, if you want to something between mixblocks
	virtual void set_note_internal(char p_note,char p_velocity)=0; //noteon
	virtual void set_note_off_internal(char p_velocity)=0;
	virtual void set_mix_frequency_internal(int p_mixfreq) =0;
	virtual void recalculate_pitch_internal()=0;


protected: /* API INTERNAL - Voice may need these */
		/*void add_buffer(float* p_dst, float *p_src, int p_amount);*/

		float get_fnote(); //This gives back the note, format is of a common midi note, but not bound to integers, so pitch bend/vibrato/etc can take place
		float get_total_volume(); //This gives back the volume in a 0-1 range
		int get_total_pan(); //This gives back the volume in a 0-1 range
		int get_update_count();


		void store_last_values(float p_left,float p_right); //this is needed for the declicker, so USE IT
	
public: /* COMMANDS */

	/* set */
	void set_note(char p_note,char p_velocity); //noteon
	void set_note_off(char p_velocity); //noteoff
//        virtual void set_data(Data *p_data) =0; //pan

		void set_sustain(bool p_sustain);

		void set_pan(char p_pan); //pitch
		void set_pitch_bend(int p_bend); //pitch bend
		void set_pitch_depth(int p_bend_depth); //pitch depth
	void set_expression(char p_vol); //expression (CC 11)
	void set_velocity(char p_vol); //expression (CC 11)
	void set_main_volume(char p_vol); //mainvol (CC 7)
	void set_default_data(Default_Data *p_data);
	void set_preamp(float p_preamp);


	virtual bool set_controller(char p_ctrl,char p_param);
	
	/* get */
		int get_pitch_bend();
	int get_pitch_depth();
		char get_pan();
	char get_note(); //check purposes only
	char get_expression();
	char get_main_volume();


//public: /* METADATA */

	//void set_channel(char p_channel);
	//char get_channel();

public: /* MIXING! */

	void set_mix_frequency(int p_mixfreq);
	//void mix(int p_amount,float *p_where_l,float *p_where_r); //set where to mix
	void mix(int p_amount,int *p_where_l,int *p_where_r);
	float get_last_value_L(); //used for declicker				
	float get_last_value_R(); //used for declicker
	//void process_declicker();
public: /* DELETING */

	Voice();
	virtual ~Voice();
};

#endif
