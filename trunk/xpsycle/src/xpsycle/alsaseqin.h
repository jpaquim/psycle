//
// File: alsaseqin.h
// Created by: User <Email>
// Created on: Thu Jul 20 13:35:01 2006
//

#ifndef XPSYCLE__ALSASEQIN
#define XPSYCLE__ALSASEQIN
#if defined XPSYCLE__CONFIGURATION
	#include <xpsycle/alsa_conditional_build.h>
#endif
#if !defined XPSYCLE__NO_ALSA

#include <alsa/asoundlib.h>
#include "eventdriver.h"

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

#endif // !defined XPSYCLE__NO_ALSA
#endif // !defined XPSYCLE__ALSASEQIN
