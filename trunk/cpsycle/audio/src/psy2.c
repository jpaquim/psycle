// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"
#include "../../detail/os.h"

#include "psy2.h"
#include "psy2converter.h"
#include "song.h"
#include "songio.h"
#include "pattern.h"
#include "constants.h"
#include <datacompression.h>
#include <operations.h>
#include <dir.h>
#include "machinefactory.h"
#include <stdlib.h>
#include <string.h>
#include "../../detail/portable.h"

void psy_audio_psy2_load(struct psy_audio_SongFile* songfile)
{
	int32_t i;
	int32_t num, sampR;
	//	unsigned char busEffect[64];
	unsigned char busMachine[64];
	unsigned char playorder[MAX_SONG_POSITIONS];
	char patternName[32];
	char name_[129];
	char author_[65];
	char comments_[65536];
	SongProperties songproperties;

	//	int32_t temp;
	int32_t songtracks;
	int32_t m_beatspermin;
	int32_t m_linesperbeat;
	uint8_t currentoctave;
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
	int32_t numlines;

	int32_t pans[OLD_MAX_INSTRUMENTS];
	char names[OLD_MAX_INSTRUMENTS][32];


	//progress.SetWindowText("Loading old format...");
	// DoNew();

	psyfile_read(songfile->file, name_, 32);
	name_[32] = '\0';
	psyfile_read(songfile->file, author_, 32);
	author_[32] = '\0';
	psyfile_read(songfile->file, comments_, 128);
	comments_[128] = '\0';
	songproperties_init(&songproperties, name_, author_, comments_);
	psy_audio_song_setproperties(songfile->song, &songproperties);

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
	{ // psy_audio_Patterns
		psyfile_read(songfile->file, &num, sizeof num);
		for(i =0 ; i < num; ++i)
		{
			psyfile_read(songfile->file, &numlines, sizeof(numlines));
			psyfile_read(songfile->file, patternName, sizeof(patternName));
			if(numlines > 0)
			{				
				///\todo: tweak_effect should be converted to normal tweaks!				
				PatternNode* node = 0;
				float offset = 0.f;
				unsigned char* pData;
				int32_t c;
				psy_audio_Pattern* pattern;
				
				pattern = pattern_allocinit();
				patterns_insert(&songfile->song->patterns, i, pattern);
				pData = malloc(OLD_MAX_TRACKS * sizeof(psy_audio_PatternEntry));
				for (c = 0; c < numlines; ++c) {
					int32_t track;
					unsigned char* ptrack;

					psyfile_read(songfile->file, pData, OLD_MAX_TRACKS * EVENT_SIZE);
					ptrack = pData;
					for (track = 0; track < songtracks; ++track) {
						psy_audio_PatternEvent event;
						// Psy3 PatternEntry format
						// type				offset
						// uint8_t note;		0
						// uint8_t inst;		1
						// uint8_t mach;		2
						// uint8_t cmd;			3
						// uint8_t parameter;	4												
						patternevent_clear(&event);
						event.note = ptrack[0];
						event.inst = (ptrack[1] == 0xFF)
							? event.inst = NOTECOMMANDS_INST_EMPTY
							: ptrack[1];
						event.mach = (ptrack[2] == 0xFF)
							? event.mach = NOTECOMMANDS_MACH_EMPTY
							: ptrack[2];
						event.cmd = ptrack[3];
						event.parameter = ptrack[4];
						if (!patternevent_empty(&event)) {							
							node = pattern_insert(pattern, node, track, offset,
								&event);
						}
						ptrack += EVENT_SIZE;
					}
					offset += 0.25;					
				}
				pattern->length = numlines * 0.25f;
				free(pData);
				pData = 0;	
			}
			else
			{
				psy_audio_Pattern* pattern;
				
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
	{ // psy_audio_Instruments			;
		unsigned char _NNA[OLD_MAX_INSTRUMENTS];
		///\name Amplitude Envelope overview:
		///\{
		/// Attack Time [in psy_audio_Samples at 44.1Khz, independently of the real samplerate]
		int32_t ENV_AT[OLD_MAX_INSTRUMENTS];
		/// Decay Time [in psy_audio_Samples at 44.1Khz, independently of the real samplerate]
		int32_t ENV_DT[OLD_MAX_INSTRUMENTS];
		/// Sustain Level [in %]
		int32_t ENV_SL[OLD_MAX_INSTRUMENTS];
		/// Release Time [in psy_audio_Samples at 44.1Khz, independently of the real samplerate]
		int32_t ENV_RT[OLD_MAX_INSTRUMENTS];
		///\}

			///\name psy_dsp_Filter 
		///\{
		/// Attack Time [in psy_audio_Samples at 44.1Khz]
		int32_t ENV_F_AT[OLD_MAX_INSTRUMENTS];
		/// Decay Time [in psy_audio_Samples at 44.1Khz]
		int32_t ENV_F_DT[OLD_MAX_INSTRUMENTS];
		/// Sustain Level [0..128]
		int32_t ENV_F_SL[OLD_MAX_INSTRUMENTS];
		/// Release Time [in psy_audio_Samples at 44.1Khz]
		int32_t ENV_F_RT[OLD_MAX_INSTRUMENTS];

		/// Cutoff Frequency [0-127]
		int32_t ENV_F_CO[OLD_MAX_INSTRUMENTS];
		/// Resonance [0-127]
		int32_t ENV_F_RQ[OLD_MAX_INSTRUMENTS];
		/// EnvAmount [-128,128]
		int32_t ENV_F_EA[OLD_MAX_INSTRUMENTS];
		/// psy_dsp_Filter Type. See psycle::helpers::dsp::FilterType. [0..6]
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
						psy_audio_Sample* wave;
						short tmpFineTune;
						char dummy[33];
						unsigned short volume = 0;
						unsigned char stereo = 0;
						unsigned char doloop = 0;
						short* pData;
						uint32_t f;

						wave = sample_allocinit(2);						
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
						wave->channels.samples[0] =
							dsp.memory_alloc(wave->numframes, sizeof(float));
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
								dsp.memory_alloc(wave->numframes, sizeof(float));
							for (f = 0; f < wave->numframes; ++f) {
								short val = (short) pData[f];
								wave->channels.samples[1][f] = (float) val;					
							}
							free(pData);
							pData = 0;
							wave->channels.numchannels = 2;
						}			
						psy_audio_samples_insert(&songfile->song->samples, wave,
							sampleindex_make(i, 0));
						{
							psy_audio_Instrument* instrument;

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
				psy_strlwr(vstL[i].dllName);
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

	{  	// psy_audio_Machines			
		unsigned char active[128];
		int32_t i;

		psyfile_read(songfile->file, &active[0], sizeof(active));
		for (i=0; i<128; ++i) {
			if (active[i]) {
				psy_audio_Machine* machine;
				psy_audio_MachineUi* machineui;
				int32_t index;
				int32_t x;
				int32_t y;				
				machine = psy_audio_psy2converter_load(songfile, i, &index,
					&x, &y);
				if (machine) {				
					machineui = psy_audio_songfile_machineui(songfile, index);
					machineui->x = x;
					machineui->y = y;
					machines_insert(&songfile->song->machines, index, machine);
				}
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
		for ( int32_t i=1;i<128;i++ ) // machine 0 is the psy_audio_Master machine.
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
					vst::psy_audio_Plugin & plugin(*static_cast<vst::psy_audio_Plugin*>(pMac[i]));
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
					psy_audio_Machine* pSourceMac = pMac[wire._inputMachine];
					int32_t d = psy_audio_Machine::FindLegacyOutput(pSourceMac, i); // We get that machine and wire
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
			psy_audio_Machine *cMac = _pMachine[invmach[i]] = pMac[i];
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

