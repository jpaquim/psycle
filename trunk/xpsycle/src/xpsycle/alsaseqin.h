//
// File: alsaseqin.h
// Created by: User <Email>
// Created on: Thu Jul 20 13:35:01 2006
//

#ifndef _ALSASEQIN_H_
#define _ALSASEQIN_H_

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


#endif	//_ALSASEQIN_H_
