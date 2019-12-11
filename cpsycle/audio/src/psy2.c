// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"
#include "../../detail/os.h"

#include "psy2.h"
#include "songio.h"
#include "pattern.h"
#include "constants.h"
#include <datacompression.h>
#include <dir.h>
#include "machinefactory.h"
#include <stdlib.h>
#include <string.h>


#if !defined DIVERSALIS__OS__MICROSOFT
#define _MAX_PATH 4096
#endif

typedef struct {
	int32_t x;
	int32_t y;
} POINT;

static void makeplugincatchername(const char* psy3dllname,
	char* catchername);
static void psy2machineload(SongFile*, int32_t slot);
static void psy2pluginload(SongFile*, int32_t slot);
static void psy2samplerload(SongFile*, int32_t slot);
static void psy2readmachineconnections(SongFile*, int32_t slot);
static char* replace_char(char* str, char c, char r)
{
	char* p;
	
	for (p = strchr(str, c); p != 0; p = strchr(p + 1, c)) *p = r;
	return p;
}
void makeplugincatchername(const char* modulename, char* catchername)
{
	char prefix[_MAX_PATH];
	char ext[_MAX_PATH];

	extract_path(modulename, prefix, catchername, ext);
	_strlwr(catchername);
	replace_char(catchername, ' ', '-');
	if (strstr(catchername, "blitz")) {
		strcpy(catchername, "blitzn");
	}
}

void psy2_load(struct SongFile* songfile)
{	
	int32_t i;	
	int32_t num,sampR;
	unsigned char _machineActive[128];
//	unsigned char busEffect[64];
	unsigned char busMachine[64];
	unsigned char playorder[MAX_SONG_POSITIONS];
	int32_t patternLines[MAX_PATTERNS];
	char patternName[MAX_PATTERNS][32];
	char name_[129];
	char author_[65];
	char comments_[65536];
	SongProperties songproperties;
		
//	int32_t temp;
	int32_t songtracks;
	int32_t m_beatspermin;
	int32_t m_linesperbeat;
	int32_t currentoctave;
//	int32_t solo;
//	int32_t _tracksoloed;
//	int32_t seqbus;
//	int32_t paramselected;
//	int32_t auxcolselected;
	int32_t instselected;
//	unsigned char _trackmuted[64];
///	int32_t _trackarmedcount;
//	unsigned char _trackarmed[64];
	int32_t m_ticksperbeat;
	int32_t m_extraticksperline;
//	unsigned char sharetracknames;
	int32_t playlength;

	int32_t pans[OLD_MAX_INSTRUMENTS];
	char names[OLD_MAX_INSTRUMENTS][32];	


	//progress.SetWindowText("Loading old format...");
	// DoNew();
	
	psyfile_read(songfile->file, name_, 32);
	psyfile_read(songfile->file, author_, 32);
	psyfile_read(songfile->file, comments_,128);
	songproperties_init(&songproperties, name_, author_, comments_);
	song_setproperties(songfile->song, &songproperties);

	psyfile_read(songfile->file, &m_beatspermin, sizeof m_beatspermin);
	psyfile_read(songfile->file, &sampR, sizeof sampR);
	songfile->song->properties.bpm = (psy_dsp_beat_t) m_beatspermin;
	if( sampR <= 0)
	{
		// Shouldn't happen but has happened.
		m_linesperbeat = 4;
	}
	// The old format assumes we output at 44100 samples/sec, so...
	else m_linesperbeat = (int32_t) (44100 * 60 / (sampR * m_beatspermin));
	m_ticksperbeat= 24;
	m_extraticksperline= 0;

	songfile->song->properties.lpb = m_linesperbeat;

	//if (fullopen)
	{
		///\todo: Warning! This is done here, because the plugins, when loading, need an up-to-date information.
		/// It should be coded in some way to get this information from the loading song, since doing it here
		/// is bad for the Winamp plugin (or any other multi-document situation).
		/// Global::player().SetBPM(BeatsPerMin(), LinesPerBeat(), ExtraTicksPerLine());
	}
	psyfile_read(songfile->file, &currentoctave, sizeof(char));
	psyfile_read(songfile->file, busMachine, 64);
	psyfile_read(songfile->file, playorder, 128);
	psyfile_read(songfile->file, &playlength, sizeof(int32_t));
	psyfile_read(songfile->file, &songtracks, sizeof(int32_t));
	{ // Patterns
		psyfile_read(songfile->file, &num, sizeof num);
		for(i =0 ; i < num; ++i)
		{
			psyfile_read(songfile->file, &patternLines[i], sizeof *patternLines);
			psyfile_read(songfile->file, &patternName[i][0], sizeof *patternName);
			if(patternLines[i] > 0)
			{				
				///\todo: tweak_effect should be converted to normal tweaks!				
				PatternNode* node = 0;
				float offset = 0.f;
				unsigned char* pData;
				int32_t c;
				Pattern* pattern;
				
				pattern = pattern_allocinit();
				patterns_insert(&songfile->song->patterns, i, pattern);
				pData = malloc(OLD_MAX_TRACKS * sizeof(PatternEntry));
				for(c = 0; c < patternLines[i] ; ++c)
				{					
					int32_t track;
					unsigned char* ptrack;

					psyfile_read(songfile->file, pData, OLD_MAX_TRACKS * sizeof(PatternEvent));
					ptrack = pData;
					for (track = 0; track < songtracks; ++track) {
						PatternEvent* event = (PatternEvent*)(ptrack);
						if (event->note != 255) {
							node = pattern_insert(pattern, node, track, offset, event);
						}
						ptrack += sizeof(PatternEvent);
					}					
					offset += 0.25;					
				}
				pattern->length = patternLines[i] * 0.25f;
				free(pData);
				pData = 0;	
			}
			else
			{
				Pattern* pattern;
				
				pattern = pattern_allocinit();
				patterns_insert(&songfile->song->patterns, i, pattern);
				pattern->length = 64 * 0.25f;				
			}			
		}
		{
			int32_t i;			
			SequencePosition sequenceposition;

			sequenceposition.track =
				sequence_appendtrack(&songfile->song->sequence,
				sequencetrack_allocinit());		
			for (i = 0; i < playlength; ++i) {			
				sequenceposition.trackposition =
					sequence_last(&songfile->song->sequence,
					sequenceposition.track);
				sequence_insert(&songfile->song->sequence,
					sequenceposition, playorder[i]);
			}
		}
	}
	// progress.m_Progress.SetPos(2048);
	// ::Sleep(1); ///< ???
	{ // Instruments			;
		unsigned char _NNA[OLD_MAX_INSTRUMENTS];
		///\name Amplitude Envelope overview:
		///\{
		/// Attack Time [in Samples at 44.1Khz, independently of the real samplerate]
		int32_t ENV_AT[OLD_MAX_INSTRUMENTS];
		/// Decay Time [in Samples at 44.1Khz, independently of the real samplerate]
		int32_t ENV_DT[OLD_MAX_INSTRUMENTS];
		/// Sustain Level [in %]
		int32_t ENV_SL[OLD_MAX_INSTRUMENTS];
		/// Release Time [in Samples at 44.1Khz, independently of the real samplerate]
		int32_t ENV_RT[OLD_MAX_INSTRUMENTS];
		///\}

			///\name Filter 
		///\{
		/// Attack Time [in Samples at 44.1Khz]
		int32_t ENV_F_AT[OLD_MAX_INSTRUMENTS];
		/// Decay Time [in Samples at 44.1Khz]
		int32_t ENV_F_DT[OLD_MAX_INSTRUMENTS];
		/// Sustain Level [0..128]
		int32_t ENV_F_SL[OLD_MAX_INSTRUMENTS];
		/// Release Time [in Samples at 44.1Khz]
		int32_t ENV_F_RT[OLD_MAX_INSTRUMENTS];

		/// Cutoff Frequency [0-127]
		int32_t ENV_F_CO[OLD_MAX_INSTRUMENTS];
		/// Resonance [0-127]
		int32_t ENV_F_RQ[OLD_MAX_INSTRUMENTS];
		/// EnvAmount [-128,128]
		int32_t ENV_F_EA[OLD_MAX_INSTRUMENTS];
		/// Filter Type. See psycle::helpers::dsp::FilterType. [0..6]
		int32_t ENV_F_TP[OLD_MAX_INSTRUMENTS];
		///\}

		unsigned char _RPAN[OLD_MAX_INSTRUMENTS];
		unsigned char _RCUT[OLD_MAX_INSTRUMENTS];
		unsigned char _RRES[OLD_MAX_INSTRUMENTS];


		int32_t i;
		int32_t tmpwvsl;

		psyfile_read(songfile->file, &instselected, sizeof instselected);
		
		for(i=0 ; i < OLD_MAX_INSTRUMENTS ; ++i)
		{
			psyfile_read(songfile->file, names[i], sizeof(names[0]));
		}
		for (i=0; i<OLD_MAX_INSTRUMENTS; i++)
		{
			psyfile_read(songfile->file, &_NNA[i], sizeof(_NNA[0]));
		}
		for (i=0; i<OLD_MAX_INSTRUMENTS; i++)
		{
			int32_t tmp;
			psyfile_read(songfile->file, &tmp, sizeof tmp);
			//Truncate to 220 samples boundaries, and ensure it is not zero.
			tmp = (tmp/220)*220; if (tmp <=0) tmp=1;
			ENV_AT[i] = tmp;
		}
		for (i=0; i<OLD_MAX_INSTRUMENTS; i++)
		{
			int32_t tmp;
			psyfile_read(songfile->file, &tmp, sizeof tmp);
			//Truncate to 220 samples boundaries, and ensure it is not zero.
			tmp = (tmp/220)*220; if (tmp <=0) tmp=1;
			ENV_DT[i] = tmp;
		}
		for (i=0; i<OLD_MAX_INSTRUMENTS; i++)
		{			
			psyfile_read(songfile->file, &ENV_SL[i], sizeof ENV_SL[0]);			
		}
		for (i=0; i<OLD_MAX_INSTRUMENTS; i++)
		{
			int32_t tmp;
			psyfile_read(songfile->file, &tmp, sizeof tmp);
			//Truncate to 220 samples boundaries, and ensure it is not zero. (also change default value)
			if (tmp == 16) tmp = 220;
			else { tmp = (tmp/220)*220; if (tmp <=0) tmp=1; }
			ENV_RT[i] = tmp;
		}
		for (i=0; i<OLD_MAX_INSTRUMENTS; i++)
		{
			int32_t tmp;
			psyfile_read(songfile->file, &tmp, sizeof tmp);
			//Truncate to 220 samples boundaries, and ensure it is not zero.
			tmp = (tmp/220)*220; if (tmp <=0) tmp=1;
			ENV_F_AT[i] = tmp;
		}
		for (i=0; i<OLD_MAX_INSTRUMENTS; i++)
		{
			int32_t tmp;
			psyfile_read(songfile->file, &tmp, sizeof tmp);
			//Truncate to 220 samples boundaries, and ensure it is not zero.
			tmp = (tmp/220)*220; if (tmp <=0) tmp=1;
			ENV_F_DT[i] = tmp;
		}
		for (i=0; i<OLD_MAX_INSTRUMENTS; i++)
		{			
			psyfile_read(songfile->file, &ENV_F_SL, sizeof ENV_F_SL[0]);			
		}
		for (i=0; i<OLD_MAX_INSTRUMENTS; i++)
		{
			int32_t tmp;
			psyfile_read(songfile->file, &tmp, sizeof tmp);
			//Truncate to 220 samples boundaries, and ensure it is not zero.
			tmp = (tmp/220)*220; if (tmp <=0) tmp=1;
			ENV_F_RT[i] = tmp;
		}
		for (i=0; i<OLD_MAX_INSTRUMENTS; i++)
		{			
			psyfile_read(songfile->file, &ENV_F_CO[i], sizeof ENV_F_CO[0]);
		}
		for (i=0; i<OLD_MAX_INSTRUMENTS; i++)
		{						
			psyfile_read(songfile->file, &ENV_F_RQ[i], sizeof ENV_F_RQ[0]);
		}
		for (i=0; i<OLD_MAX_INSTRUMENTS; i++)
		{			
			psyfile_read(songfile->file, &ENV_F_EA[i], sizeof ENV_F_EA[0]);
		}
		for (i=0; i<OLD_MAX_INSTRUMENTS; i++)
		{						
			psyfile_read(songfile->file, &ENV_F_TP[i], sizeof ENV_F_TP[0]);
		}
		for (i=0; i<OLD_MAX_INSTRUMENTS; i++)
		{			
			psyfile_read(songfile->file, &pans[i], sizeof(pans[0]));
		}
		for (i=0; i<OLD_MAX_INSTRUMENTS; i++)
		{			
			psyfile_read(songfile->file, &_RPAN[i], sizeof(_RPAN[0]));
		}
		for (i=0; i<OLD_MAX_INSTRUMENTS; i++)
		{
			psyfile_read(songfile->file, &_RCUT[i], sizeof(_RCUT[0]));
		}
		for (i=0; i<OLD_MAX_INSTRUMENTS; i++)
		{			
			psyfile_read(songfile->file, &_RRES[i], sizeof(_RRES[0]));
		}
	

	
		// progress.m_Progress.SetPos(4096);
		// ::Sleep(1);
		// Waves
		//
				
		psyfile_read(songfile->file, &tmpwvsl, sizeof(int32_t));

		for (i=0; i<OLD_MAX_INSTRUMENTS; i++)
		{
			int32_t w;

			for (w=0; w<OLD_MAX_WAVES; w++)
			{
				int32_t wltemp;
				psyfile_read(songfile->file, &wltemp, sizeof(wltemp));
				if (wltemp > 0)
				{
					if ( w == 0 )
					{
						Sample* wave;
						short tmpFineTune;
						char dummy[33];
						unsigned short volume = 0;
						unsigned char stereo = 0;
						unsigned char doloop = 0;
						short* pData;
						uint32_t f;

						wave = sample_allocinit();						
						//Old format assumed 44Khz
						wave->samplerate = 44100;
						wave->panfactor = (float) pans[i] / 256.f ; //(value_mapper::map_256_1(pan));
						//Old wavename, not really used anyway.
						psyfile_read(songfile->file, dummy, 32);
						wave->name = strdup(names[i]);						
						psyfile_read(songfile->file, &volume, sizeof volume);
						wave->globalvolume = volume*0.01f;
						psyfile_read(songfile->file, &tmpFineTune, sizeof(short));
						//Current sample uses 100 cents. Older used +-256
						tmpFineTune = (int32_t)((float)tmpFineTune/2.56f);
						wave->finetune = tmpFineTune;						

						psyfile_read(songfile->file, &wave->loopstart, sizeof wave->loopstart);
						psyfile_read(songfile->file, &wave->loopend, sizeof wave->loopend);

						psyfile_read(songfile->file, &doloop, sizeof(doloop));
						wave->looptype = doloop ? LOOP_NORMAL : LOOP_DO_NOT;
						psyfile_read(songfile->file, &wave->stereo, sizeof(wave->stereo));
																	
						pData = malloc(wltemp*sizeof(short)+4);// +4 to avoid any attempt at buffer overflow by the code
						psyfile_read(songfile->file, pData, wltemp*sizeof(short));
						wave->numframes = wltemp;			
						wave->channels.samples[0] = malloc(sizeof(float)*wave->numframes);
						for (f = 0; f < wave->numframes; ++f) {
							short val = (short) pData[f];
							wave->channels.samples[0][f] = (float) val;				
						}
						free(pData);
						pData = 0;
						wave->channels.numchannels = 1;		
						if (wave->stereo)
						{
							uint32_t f;
							pData = malloc(wltemp*sizeof(short)+4);// +4 to avoid any attempt at buffer overflow by the code
							psyfile_read(songfile->file, pData, wltemp*sizeof(short));				
							wave->channels.samples[1] = 
								malloc(sizeof(float)*wave->numframes);
							for (f = 0; f < wave->numframes; ++f) {
								short val = pData[f];
								wave->channels.samples[1][f] = (float) val;					
							}
							free(pData);
							pData = 0;
							wave->channels.numchannels = 2;
						}			
						samples_insert(&songfile->song->samples, wave, i);
						{
							Instrument* instrument;

							instrument = instrument_allocinit();
							instrument_setname(instrument, names[i]);
							instruments_insert(&songfile->song->instruments,
								instrument, i);
						}

					}
					else 
					{
						unsigned char stereo;
						psyfile_skip(songfile->file, 42+sizeof(unsigned char));
						psyfile_read(songfile->file, &stereo,sizeof(unsigned char));
						psyfile_skip(songfile->file, wltemp);
						if(stereo) {
							psyfile_skip(songfile->file, wltemp);
						}
					}
				}
			}
		}
	} 
	
	// progress.m_Progress.SetPos(4096+2048);
	// ::Sleep(1);
	{ // VST DLLs
		int32_t i;
		
		// VSTLoader vstL[OLD_MAX_PLUGINS]; 
		for (i=0; i<OLD_MAX_PLUGINS; i++)
		{
			unsigned char valid;
			psyfile_read(songfile->file, &valid, sizeof(valid));
			if( valid )
			{
				/*pFile->Read(vstL[i].dllName,sizeof(vstL[i].dllName));
				_strlwr(vstL[i].dllName);
				pFile->Read(&(vstL[i].numpars), sizeof(int32_t));
				vstL[i].pars = new float[vstL[i].numpars];

				for (int32_t c=0; c<vstL[i].numpars; c++)
				{
					pFile->Read(&(vstL[i].pars[c]), sizeof(float));
				}*/
			}
		}
	}

	

//	progress.m_Progress.SetPos(8192);
//	::Sleep(1);
	{  
		Machine* pMac[128];
		Properties* machinesproperties;	
		int32_t i;
		// Machines
		//
		psyfile_read(songfile->file, &_machineActive[0], sizeof(_machineActive));		
		memset(pMac,0,sizeof(pMac));		
		machinesproperties = properties_createsection(
			songfile->workspaceproperties, "machines");
	//	convert_internal_machines::Converter converter;								
		for (i=0; i<128; i++)
		{
			int32_t x,y,type;
			char sDllName[256];
			char plugincatchername[256];

			plugincatchername[0] = '\0';

			if (_machineActive[i])
			{
				Machine* machine;
				int32_t slot;
				Properties* machineproperties;
				char plugincatchername[256];

				if (i == 0) {
					slot = MASTER_INDEX;
				} else
				if (i == MASTER_INDEX) {
					slot = 0;
				} else {
					slot = i;
				}
				// progress.m_Progress.SetPos(8192+i*(4096/128));
				// ::Sleep(1);
				machineproperties = properties_createsection(machinesproperties, "machine");
				properties_append_int(machineproperties, "index", slot, 0, MAX_MACHINES);
				
				psyfile_read(songfile->file, &x, sizeof(x));
				psyfile_read(songfile->file, &y, sizeof(y));

				properties_append_int(machineproperties, "x", x, 0, 0);
				properties_append_int(machineproperties, "y", y, 0, 0);


				psyfile_read(songfile->file, &type, sizeof(type));

				if (type == MACH_PLUGIN) {					
					psyfile_read(songfile->file, sDllName, sizeof(sDllName)); // Plugin dll name
					makeplugincatchername(sDllName, plugincatchername);
				}				
				machine = machinefactory_makemachine(
					songfile->song->machinefactory,
					type,
					plugincatchername);
				if (!machine) {
					machine = machinefactory_makemachine(
						songfile->song->machinefactory, 
						MACH_DUMMY,
						plugincatchername);
					type = MACH_DUMMY;
				}				
				if (type == MACH_DUMMY) {
					char txt[40];
					strcpy(txt, "X!");
					//strcat(txt, sDllName);
					properties_append_string(machineproperties, "editname", "dummy");
				} else {
					properties_append_string(machineproperties, "editname", sDllName);
				}
				
				if (type == MACH_SAMPLER) {
					psy2samplerload(songfile, slot);
				} else
				if (type == MACH_PLUGIN) {
					psy2pluginload(songfile, slot);
				} else {
					psy2machineload(songfile, slot);
				}
				machines_insert(&songfile->song->machines, slot, machine);
			}
		}
	}

	/*
	// Patch 0: Some extra data added around the 1.0 release.
	for (int32_t i=0; i<OLD_MAX_INSTRUMENTS; i++)
	{
		pFile->Read(&_pInstrument[i]->_loop, sizeof(_pInstrument[0]->_loop));
	}
	for (int32_t i=0; i<OLD_MAX_INSTRUMENTS; i++)
	{
		pFile->Read(&_pInstrument[i]->_lines, sizeof(_pInstrument[0]->_lines));
	}

	// Validate the machine arrays. 
	for (int32_t i=0; i<128; i++) // First, we add the output volumes to a Matrix for reference later
	{
		if (!_machineActive[i])
		{
			zapObject(pMac[i]);
		}
		else if (!pMac[i])
		{
			_machineActive[i] = false;
		}
	}

	// Patch 1: BusEffects (twf). Try to read it, and if it doesn't exist, generate it.
	progress.m_Progress.SetPos(8192+4096);
	::Sleep(1);
	if ( pFile->Read(&busEffect[0],sizeof(busEffect)) == false )
	{
		int32_t j=0;
		unsigned char invmach[128];
		memset(invmach,255,sizeof(invmach));
		// The guessing procedure does not rely on the machmode because if a plugin
		// is missing, then it is always tagged as a generator.
		for (int32_t i = 0; i < 64; i++)
		{
			if (busMachine[i] != 255) invmach[busMachine[i]]=i;
		}
		for ( int32_t i=1;i<128;i++ ) // machine 0 is the Master machine.
		{
			if (_machineActive[i])
			{
				if (invmach[i] == 255)
				{
					busEffect[j]=i;	
					j++;
				}
			}
		}
		while(j < 64)
		{
			busEffect[j] = 255;
			j++;
		}
	}
	// Validate that there isn't any duplicated machine.
	for ( int32_t i=0;i<64;i++ ) 
	{
		for ( int32_t j=i+1;j<64;j++ ) 
		{
			if  (busMachine[i] == busMachine[j]) busMachine[j]=255;
			if  (busEffect[i] == busEffect[j]) busEffect[j]=255;
		}
		for (int32_t j=0;j<64;j++)
		{
			if  (busMachine[i] == busEffect[j]) busEffect[j]=255;
		}
	}

	// Patch 1.2: Fixes erroneous machine mode when a dummy replaces a bad plugin
	// (missing dll, or when the load process failed).
	// At the same time, we validate the indexes of the busMachine and busEffects arrays.
	for ( int32_t i=0;i<64;i++ ) 
	{
		if (busEffect[i] != 255)
		{
			if ( busEffect[i] > 128 || !_machineActive[busEffect[i]] )
				busEffect[i] = 255;
			// If there's a dummy, force it to be an effect
			else if (pMac[busEffect[i]]->_type == MACH_DUMMY ) 
			{
				pMac[busEffect[i]]->_mode = MACHMODE_FX;
			}
			// Else if the machine is a generator, move it to gens bus.
			// This can't happen, but it is here for completeness
			else if (pMac[busEffect[i]]->_mode == MACHMODE_GENERATOR)
			{
				int32_t k=0;
				while (busEffect[k] != 255 && k<MAX_BUSES) 
				{
					k++;
				}
				busMachine[k]=busEffect[i];
				busEffect[i]=255;
			}
		}
		if (busMachine[i] != 255)
		{
			if (busMachine[i] > 128 || !_machineActive[busMachine[i]])
				busMachine[i] = 255;
			 // If there's a dummy, force it to be a Generator
			else if (pMac[busMachine[i]]->_type == MACH_DUMMY ) 
			{
				pMac[busMachine[i]]->_mode = MACHMODE_GENERATOR;
			}
			// Else if the machine is an fx, move it to FXs bus.
			// This can't happen, but it is here for completeness
			else if ( pMac[busMachine[i]]->_mode != MACHMODE_GENERATOR)
			{
				int32_t j=0;
				while (busEffect[j] != 255 && j<MAX_BUSES) 
				{
					j++;
				}
				busEffect[j]=busMachine[i];
				busMachine[i]=255;
			}
		}
	}

	 // Patch 2: VST Chunks.
	progress.m_Progress.SetPos(8192+4096+1024);
	::Sleep(1);
	bool chunkpresent=false;
	pFile->Read(&chunkpresent,sizeof(chunkpresent));

	if ( fullopen && chunkpresent ) for ( int32_t i=0;i<128;i++ ) 
	{
		if (_machineActive[i])
		{
			if ( pMac[i]->_type == MACH_DUMMY ) 
			{
				if (((Dummy*)pMac[i])->wasVST && chunkpresent )
				{
					// Since we don't know if the plugin saved it or not, 
					// we're stuck on letting the loading crash/behave incorrectly.
					// There should be a flag, like in the VST loading Section to be correct.
					MessageBox(NULL,"Missing or Corrupted VST plug-in has chunk, trying not to crash.", "Loading Error", MB_OK);
				}
			}
			else if (( pMac[i]->_type == MACH_VST ) || ( pMac[i]->_type == MACH_VSTFX))
			{
				bool chunkread = false;
				try
				{
					vst::Plugin & plugin(*static_cast<vst::Plugin*>(pMac[i]));
					if(chunkpresent) chunkread = plugin.LoadChunk(pFile);
				}
				catch(const std::exception &)
				{
					// o_O`
				}
			}
		}
	}

	*/
	//////////////////////////////////////////////////////////////////////////
	//Finished all the songfile->file loading. Now Process the data to the current structures

	// The old fileformat stored the volumes on each output, 
	// so what we have in inputConVol is really outputConVol
	// and we have to convert while recreating them.
	
	/*progress.m_Progress.SetPos(8192+4096+2048);
	::Sleep(1);
	for (int32_t i=0; i<128; i++) // we go to fix this for each
	{
		if (_machineActive[i])		// valid machine (important, since we have to navigate!)
		{
			for (int32_t c=0; c<MAX_CONNECTIONS; c++) // all for each of its input connections.
			{
				LegacyWire& wire = pMac[i]->legacyWires[c];
				if (wire._inputCon && wire._inputMachine > -1 && wire._inputMachine < 128
					&& pMac[wire._inputMachine])	// If there's a valid machine in this inputconnection,
				{
					Machine* pSourceMac = pMac[wire._inputMachine];
					int32_t d = Machine::FindLegacyOutput(pSourceMac, i); // We get that machine and wire
					if ( d != -1 )
					{
						float val = pSourceMac->legacyWires[d]._inputConVol;
						if( val > 4.1f )
						{
							val*=0.000030517578125f; // BugFix
						}
						else if ( val < 0.00004f) 
						{
							val*=32768.0f; // BugFix
						}
						// and set the volume.
						if (wire.pinMapping.size() > 0) {
							pMac[i]->inWires[c].ConnectSource(*pSourceMac,0,d,&wire.pinMapping);
						}
						else {
							pMac[i]->inWires[c].ConnectSource(*pSourceMac,0,d);
						}
						pMac[i]->inWires[c].SetVolume(val*wire._wireMultiplier);
					}
				}
			}
		}
	}

	// Psycle no longer uses busMachine and busEffect, since the pMachine Array directly maps
	// to the real machine.
	// Due to this, we have to move machines to where they really are, 
	// and remap the inputs and outputs indexes again... ouch
	// At the same time, we validate each wire.
	progress.m_Progress.SetPos(8192+4096+2048+1024);
	::Sleep(1);
	unsigned char invmach[128];
	memset(invmach,255,sizeof(invmach));
	for (int32_t i = 0; i < 64; i++)
	{
		if (busMachine[i] != 255) invmach[busMachine[i]]=i;
		if (busEffect[i] != 255) invmach[busEffect[i]]=i+64;
	}
	invmach[0]=MASTER_INDEX;

	for (int32_t i = 0; i < 128; i++)
	{
		if (invmach[i] != 255)
		{
			Machine *cMac = _pMachine[invmach[i]] = pMac[i];
			cMac->_macIndex = invmach[i];
			_machineActive[i] = false; // mark as "converted"
		}
	}
	// verify that there isn't any machine that hasn't been copied into _pMachine
	// Shouldn't happen. It would mean a damaged songfile->file.
	int32_t j=0;
	int32_t k=64;
	for (int32_t i=0;i < 128; i++)
	{
		if (_machineActive[i])
		{
			if ( pMac[i]->_mode == MACHMODE_GENERATOR)
			{
				while (_pMachine[j] && j<64) j++;
				_pMachine[j]=pMac[i];
			}
			else
			{
				while (_pMachine[k] && k<128) k++;
				_pMachine[k]=pMac[i];
			}
		}
	}

	progress.m_Progress.SetPos(16384);
	::Sleep(1);
	if(fullopen) converter.retweak(*this);
	for (int32_t i(0); i < MAX_MACHINES;++i) if ( _pMachine[i]) _pMachine[i]->PostLoad(_pMachine);
	seqBus=0;
	// Clean memory.
	for(int32_t i(0) ; i < MAX_MACHINES ; ++i) if(_pMachine[i])	{
		_pMachine[i]->legacyWires.clear();
	}
	// Clean the vst loader helpers.
	for (int32_t i=0; i<OLD_MAX_PLUGINS; i++)
	{
		if( vstL[i].valid )
		{
			zapObject(vstL[i].pars);
		}
	}
	*/
//	_saved=true;
//	return true;	
}

void psy2machineload(SongFile* songfile, int32_t slot)
{
	char junk[256];

	char _editName[16];
	int32_t _panning;
	int32_t _outDry;
	int32_t _outWet;

	memset(&junk, 0, sizeof(junk));

	psyfile_read(songfile->file, &_editName, sizeof(_editName));
	psy2readmachineconnections(songfile, slot);
	psyfile_read(songfile->file, &_panning, sizeof(_panning));
	// Machine::SetPan(_panning);
	psyfile_read(songfile->file, &junk[0], 8*sizeof(int32_t)); // SubTrack[]
	psyfile_read(songfile->file, &junk[0], sizeof(int32_t)); // numSubtracks
	psyfile_read(songfile->file, &junk[0], sizeof(int32_t)); // interpol

	psyfile_read(songfile->file, &_outDry, sizeof(_outDry));
	psyfile_read(songfile->file, &_outWet, sizeof(_outWet));

	psyfile_read(songfile->file, &junk[0], sizeof(int32_t)); // distPosThreshold
	psyfile_read(songfile->file, &junk[0], sizeof(int32_t)); // distPosClamp
	psyfile_read(songfile->file, &junk[0], sizeof(int32_t)); // distNegThreshold
	psyfile_read(songfile->file, &junk[0], sizeof(int32_t)); // distNegClamp

	psyfile_read(songfile->file, &junk[0], sizeof(char)); // sinespeed
	psyfile_read(songfile->file, &junk[0], sizeof(char)); // sineglide
	psyfile_read(songfile->file, &junk[0], sizeof(char)); // sinevolume
	psyfile_read(songfile->file, &junk[0], sizeof(char)); // sinelfospeed
	psyfile_read(songfile->file, &junk[0], sizeof(char)); // sinelfoamp

	psyfile_read(songfile->file, &junk[0], sizeof(int32_t)); // delayTimeL
	psyfile_read(songfile->file, &junk[0], sizeof(int32_t)); // delayTimeR
	psyfile_read(songfile->file, &junk[0], sizeof(int32_t)); // delayFeedbackL
	psyfile_read(songfile->file, &junk[0], sizeof(int32_t)); // delayFeedbackR

	psyfile_read(songfile->file, &junk[0], sizeof(int32_t)); // filterCutoff
	psyfile_read(songfile->file, &junk[0], sizeof(int32_t)); // filterResonance
	psyfile_read(songfile->file, &junk[0], sizeof(int32_t)); // filterLfospeed
	psyfile_read(songfile->file, &junk[0], sizeof(int32_t)); // filterLfoamp
	psyfile_read(songfile->file, &junk[0], sizeof(int32_t)); // filterLfophase
	psyfile_read(songfile->file, &junk[0], sizeof(int32_t)); // filterMode	
}

void psy2pluginload(SongFile* songfile, int32_t slot)
{
	char junk[256];
	char _editName[16];
	int32_t _panning;
	int32_t _outDry;
	int32_t _outWet;
	int32_t i;
	int32_t numParameters;

	psyfile_read(songfile->file, &_editName, sizeof(_editName));
	psyfile_read(songfile->file, &numParameters, sizeof(numParameters));
	for (i=0; i<numParameters; i++)
	{
		psyfile_read(songfile->file, &junk[0], sizeof(int32_t));			
	}
	psy2readmachineconnections(songfile, slot);
	psyfile_read(songfile->file, &_panning, sizeof(_panning));
//	Machine::SetPan(_panning);
	psyfile_read(songfile->file, &junk[0], 8*sizeof(int32_t)); // SubTrack[]
	psyfile_read(songfile->file, &junk[0], sizeof(int32_t)); // numSubtracks
	psyfile_read(songfile->file, &junk[0], sizeof(int32_t)); // interpol

	psyfile_read(songfile->file, &_outDry, sizeof(_outDry));
	psyfile_read(songfile->file, &_outWet, sizeof(_outWet));

	psyfile_read(songfile->file, &junk[0], sizeof(int32_t)); // distPosThreshold
	psyfile_read(songfile->file, &junk[0], sizeof(int32_t)); // distPosClamp
	psyfile_read(songfile->file, &junk[0], sizeof(int32_t)); // distNegThreshold
	psyfile_read(songfile->file, &junk[0], sizeof(int32_t)); // distNegClamp

	psyfile_read(songfile->file, &junk[0], sizeof(char)); // sinespeed
	psyfile_read(songfile->file, &junk[0], sizeof(char)); // sineglide
	psyfile_read(songfile->file, &junk[0], sizeof(char)); // sinevolume
	psyfile_read(songfile->file, &junk[0], sizeof(char)); // sinelfospeed
	psyfile_read(songfile->file, &junk[0], sizeof(char)); // sinelfoamp

	psyfile_read(songfile->file, &junk[0], sizeof(int32_t)); // delayTimeL
	psyfile_read(songfile->file, &junk[0], sizeof(int32_t)); // delayTimeR
	psyfile_read(songfile->file, &junk[0], sizeof(int32_t)); // delayFeedbackL
	psyfile_read(songfile->file, &junk[0], sizeof(int32_t)); // delayFeedbackR

	psyfile_read(songfile->file, &junk[0], sizeof(int32_t)); // filterCutoff
	psyfile_read(songfile->file, &junk[0], sizeof(int32_t)); // filterResonance
	psyfile_read(songfile->file, &junk[0], sizeof(int32_t)); // filterLfospeed
	psyfile_read(songfile->file, &junk[0], sizeof(int32_t)); // filterLfoamp
	psyfile_read(songfile->file, &junk[0], sizeof(int32_t)); // filterLfophase
	psyfile_read(songfile->file, &junk[0], sizeof(int32_t)); // filterMode
}

void psy2samplerload(SongFile* songfile, int32_t slot)
{	
	char junk[256];	
	char _editName[16];
	int32_t _panning;
	int32_t _outDry;
	int32_t _outWet;
	int32_t i;
//	int32_t numParameters;
	int32_t _numVoices;


	memset(&junk, 0, sizeof(junk));

	psyfile_read(songfile->file, &_editName, sizeof(_editName));
	psy2readmachineconnections(songfile, slot);
	psyfile_read(songfile->file, &_panning, sizeof(_panning));
// 	Machine::SetPan(_panning);
	psyfile_read(songfile->file, &junk[0], 8*sizeof(int32_t)); // SubTrack[]
	psyfile_read(songfile->file, &_numVoices, sizeof(_numVoices)); // numSubtracks

/*	if (_numVoices < 4)  // No more need for this code.
	{
		// Most likely an old polyphony
		_numVoices = 8;
	}
*/
	psyfile_read(songfile->file, &i, sizeof(int32_t)); // interpol
/*	switch (i)
	{
	case 2:
		_resampler.SetQuality(RESAMPLE_SPLINE);
		break;
	case 0:
		_resampler.SetQuality(RESAMPLE_NONE);
		break;
	default:
	case 1:
		_resampler.SetQuality(RESAMPLE_LINEAR);
		break;
	}*/

	psyfile_read(songfile->file, &_outDry, sizeof(_outDry));
	psyfile_read(songfile->file, &_outWet, sizeof(_outWet));

	psyfile_read(songfile->file, &junk[0], sizeof(int32_t)); // distPosThreshold
	psyfile_read(songfile->file, &junk[0], sizeof(int32_t)); // distPosClamp
	psyfile_read(songfile->file, &junk[0], sizeof(int32_t)); // distNegThreshold
	psyfile_read(songfile->file, &junk[0], sizeof(int32_t)); // distNegClamp

	psyfile_read(songfile->file, &junk[0], sizeof(char)); // sinespeed
	psyfile_read(songfile->file, &junk[0], sizeof(char)); // sineglide
	psyfile_read(songfile->file, &junk[0], sizeof(char)); // sinevolume
	psyfile_read(songfile->file, &junk[0], sizeof(char)); // sinelfospeed
	psyfile_read(songfile->file, &junk[0], sizeof(char)); // sinelfoamp

	psyfile_read(songfile->file, &junk[0], sizeof(int32_t)); // delayTimeL
	psyfile_read(songfile->file, &junk[0], sizeof(int32_t)); // delayTimeR
	psyfile_read(songfile->file, &junk[0], sizeof(int32_t)); // delayFeedbackL
	psyfile_read(songfile->file, &junk[0], sizeof(int32_t)); // delayFeedbackR

	psyfile_read(songfile->file, &junk[0], sizeof(int32_t)); // filterCutoff
	psyfile_read(songfile->file, &junk[0], sizeof(int32_t)); // filterResonance
	psyfile_read(songfile->file, &junk[0], sizeof(int32_t)); // filterLfospeed
	psyfile_read(songfile->file, &junk[0], sizeof(int32_t)); // filterLfoamp
	psyfile_read(songfile->file, &junk[0], sizeof(int32_t)); // filterLfophase
	psyfile_read(songfile->file, &junk[0], sizeof(int32_t)); // filterMode	
}

void psy2readmachineconnections(SongFile* songfile, int32_t slot)
{	
	int32_t _inputMachines[MAX_CONNECTIONS];	// Incoming connections Machine number
	int32_t _outputMachines[MAX_CONNECTIONS];	// Outgoing connections Machine number
	float _inputConVol[MAX_CONNECTIONS];	// Incoming connections Machine vol
//	float _wireMultiplier[MAX_CONNECTIONS];	// Value to multiply _inputConVol[] to have a 0.0...1.0 range
	unsigned char _connection[MAX_CONNECTIONS];      // Outgoing connections activated
	unsigned char _inputCon[MAX_CONNECTIONS];		// Incoming connections activated
	int32_t _numInputs;							// number of Incoming connections
	int32_t _numOutputs;						// number of Outgoing connections
	POINT _connectionPoint[MAX_CONNECTIONS];			

	psyfile_read(songfile->file, &_inputMachines[0], sizeof(_inputMachines));
	psyfile_read(songfile->file, &_outputMachines[0], sizeof(_outputMachines));
	psyfile_read(songfile->file, &_inputConVol[0], sizeof(_inputConVol));
	psyfile_read(songfile->file, &_connection[0], sizeof(_connection));
	psyfile_read(songfile->file, &_inputCon[0], sizeof(_inputCon));
	psyfile_read(songfile->file, &_connectionPoint[0], sizeof(_connectionPoint));
	psyfile_read(songfile->file, &_numInputs, sizeof(_numInputs));
	psyfile_read(songfile->file, &_numOutputs, sizeof(_numOutputs));


	/*for (c = 0; c < MAX_CONNECTIONS; ++c) { // all for each of its input connections.		
		if (_connection[c]) {
			int32_t invertslot;

			if (_inputMachines[c] == 0) {
				invertslot = MASTER_INDEX;
			} else
			if (_inputMachines[c] == MASTER_INDEX) {
				invertslot = 0;
			} else {
				invertslot = _inputMachines[c];
			}
			connections_connect(&songfile->song->machines.connections,
				invertslot, slot);
			if (_outputMachines[c] == 0) {
				invertslot = MASTER_INDEX;
			} else
			if (_outputMachines[c] == MASTER_INDEX) {
				invertslot = 0;
			} else {
				invertslot = _outputMachines[c];
			}
			connections_connect(&songfile->song->machines.connections,
				slot, invertslot);
		}		
	}*/
}
