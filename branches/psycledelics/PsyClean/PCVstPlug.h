// PCVstPlug.h: interface for the PCVstPlug class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PCVSTPLUG_H__4AACC0D7_7CDF_4257_AA25_26C1E5B27F90__INCLUDED_)
#define AFX_PCVSTPLUG_H__4AACC0D7_7CDF_4257_AA25_26C1E5B27F90__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "vst/aeffect.h"
#include "PCSignalProcessor.h"

#define VSTINSTANCE_ERR_NO_VALID_FILE -1
#define VSTINSTANCE_ERR_NO_VST_PLUGIN -2
#define VSTINSTANCE_ERR_REJECTED -3
#define VSTINSTANCE_NO_ERROR 0


class PCVSTHost
{
public:
	PCVSTHost();
	virtual ~PCVSTHost();

	PCSignalProcessor* Create(CString& dllpath,int& error);
	static long Master(AEffect *effect, long opcode, long index, long value, void *ptr, float opt);

};



class PCVstPlug : public PCSignalProcessor
{
public:
	PCVstPlug();
	virtual ~PCVstPlug();
	virtual void Create(HMODULE hdll, AEffect* aeffect);
	virtual void Destroy(void);

	virtual void AddMidiEvent(PCMidiEvent& midievent);
	virtual void ProcessMidi(void);


protected:
	inline long PCVstPlug::Dispatch(long opCode, long index, long value, void *ptr, float opt)
	{
		return _pEffect->dispatcher(_pEffect, opCode, index, value, ptr, opt);
	}
	void Init();

	static VstTimeInfo _timeInfo;
	VstMidiEvent vstmidievent[MAX_EVENTS];
	VstEvents events;
	bool wantMidi;
	
	bool initialized;
	AEffect *_pEffect;
	HMODULE h_dll;


};


class PCVSTi : public PCVstPlug  
{
public:
	PCVSTi();
	virtual ~PCVSTi();

	virtual bool ProcessSoundAdding(sBufferArray inputbuffers,sBufferArray outputbuffers);
	virtual bool ProcessSoundReplacing(sBufferArray inputbuffers,sBufferArray outputbuffers);

	void Create();

};


class PCVSTFx : public PCVstPlug  
{
public:
	PCVSTFx();
	virtual ~PCVSTFx();

	virtual void Ceate(HMODULE hdll, AEffect* aeffect);

	virtual bool ProcessSoundAdding(sBufferArray inputbuffers,sBufferArray outputbuffers);
	virtual bool ProcessSoundReplacing(sBufferArray inputbuffers,sBufferArray outputbuffers);

	bool canBypass;

};

#endif // !defined(AFX_PCVSTPLUG_H__4AACC0D7_7CDF_4257_AA25_26C1E5B27F90__INCLUDED_)
