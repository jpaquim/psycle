// -*- mode:c++; indent-tabs-mode:t -*-
//
// File: alsaseqin.h
// Created by: User <Email>
// Created on: Thu Jul 20 13:35:01 2007
//

#ifndef PSYCLE__ALSASEQIN
#define PSYCLE__ALSASEQIN
#if defined PSYCLE__ALSA_AVAILABLE
#include <alsa/asoundlib.h>
#include <psycle/core/eventdriver.h>

class AlsaSeqIn : public EventDriver
{
	public:
		AlsaSeqIn();
			~AlsaSeqIn();
	
		// AlsaSeqIn interface
		virtual bool Open();
		virtual bool Close();
	
		virtual void MidiCallback(int numSamples);
		// TODO: add member function declarations...
	
	protected:
		// AlsaSeqIn variables
	snd_seq_t  *seq_handle;
		int npolldesc;
		struct pollfd *ppolldesc;
	
	
		// TODO: add member variables...
	virtual void ProcessEvent();
	
};

#endif // defined PSYCLE__ALSA_AVAILABLE
#endif
