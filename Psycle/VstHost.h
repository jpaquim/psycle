#ifndef _VSTPLUGIN_HOST
#define _VSTPLUGIN_HOST

#include "Machine.h"
#include "Vst\AEffectx.h"
#include "Song.h"
#include "Helpers.h"

#define MAX_EVENTS		64
#define MAX_INOUTS		8

#define NUMTICKS 1000 // Dialog max ticks for parameters.


#define VSTINSTANCE_ERR_NO_VALID_FILE -1
#define VSTINSTANCE_ERR_NO_VST_PLUGIN -2
#define VSTINSTANCE_ERR_REJECTED -3
#define VSTINSTANCE_NO_ERROR 0

typedef AEffect* (*PVSTMAIN)(audioMasterCallback audioMaster);

typedef struct {
	unsigned char note;
	unsigned char midichan;
} VSTinote;

class VSTPlugin : public Machine
{
public:
	VSTPlugin();
	~VSTPlugin();

	void Free();
	int Instance(char *dllname,bool overwriteName=true);
//	void Create(VSTPlugin *plug);
	virtual void Init(void);
	virtual bool Load(RiffFile* pFile);
#if !defined(_WINAMP_PLUGIN_)
	virtual bool Save(RiffFile* pFile);
#endif // ndef _WINAMP_PLUGIN_
	bool LoadChunk(RiffFile* pFile);	// To be removed when changing the fileformat.
#if !defined(_WINAMP_PLUGIN_)
	bool SaveChunk(RiffFile* pFile,bool &isfirst);	// "    "   "   " 
#endif // ndef _WINAMP_PLUGIN_
	virtual int GetNumParams(void) { return _pEffect->numParams; }
	virtual void GetParamName(int numparam,char* name)
	{
		if ( numparam < _pEffect->numParams ) Dispatch(effGetParamName,numparam,0,name,0.0f);
		else strcpy(name,"Out of Range");

	}
	virtual void GetParamValue(int numparam,char* parval)
	{
		if ( numparam < _pEffect->numParams ) DescribeValue(numparam,parval);
		else strcpy(parval,"Out of Range");
	}
	virtual int GetParamValue(int numparam)
	{
		if ( numparam < _pEffect->numParams ) return f2i(_pEffect->getParameter(_pEffect, numparam)*65535);
		else return -1;
	}

	virtual char* GetName(void) { return _sProductName; }
	long GetVersion() { return _version; }
	char* GetVendorName(void) { return _sVendorName; }
	char* GetDllName(void) { return _sDllName; }

	long NumParameters(void) { return _pEffect->numParams; }
	float GetParameter(long parameter) { return _pEffect->getParameter(_pEffect, parameter); }
	bool DescribeValue(int parameter,char* psTxt);
	bool SetParameter(int parameter, float value);
	bool SetParameter(int parameter, int value);
	void SetCurrentProgram(int prg);
	int GetCurrentProgram();
	int NumPrograms() { return _pEffect->numPrograms; }
	bool IsSynth() { return _isSynth; }

	bool AddMIDI(unsigned char data0,unsigned char data1=0,unsigned char data2=0);
	void SendMidi();

	AEffect *_pEffect;
	long Dispatch(long opCode, long index, long value, void *ptr, float opt)
	{
		return _pEffect->dispatcher(_pEffect, opCode, index, value, ptr, opt);
	}
	static long Master(AEffect *effect, long opcode, long index, long value, void *ptr, float opt);

	unsigned char _program;
	bool instantiated;
	int _instance;		// Remove when Changing the FileFormat.

	bool requiresProcess;	// It needs to use Process
	bool requiresRepl;		// It needs to use ProcessRepl
	int macindex;
	CFrameWnd* editorWnd;
	
protected:

	HMODULE h_dll;
	char *_sDllName;	// Contains dll name

	char _sProductName[64];
	char _sVendorName[64];
	long _version;
	bool _isSynth;
	bool wantidle;

	float * inputs[MAX_INOUTS];
	float * outputs[MAX_INOUTS];
	float junk[STREAM_SIZE];

	static VstTimeInfo _timeInfo;
	VstMidiEvent midievent[MAX_EVENTS];
	VstEvents events;
	int	queue_size;

};

class VSTInstrument : public VSTPlugin
{
public:
	VSTInstrument();
	virtual void Tick(int channel, PatternEntry* pEntry);
	virtual void Work(int numSamples);
	virtual void Stop(void);

	bool AddNoteOn(unsigned char channel, unsigned char note,unsigned char speed,unsigned char midichannel=0);
	bool AddNoteOff(unsigned char channel,unsigned char midichannel=0,bool addatStart=false);

protected:
	VSTinote trackNote[MAX_TRACKS];
};

class VSTFX : public VSTPlugin
{
public:
	VSTFX();
	~VSTFX();
	virtual void Tick(int channel, PatternEntry* pEntry);
	virtual void Work(int numSamples);

protected:
	float *_pOutSamplesL;
	float *_pOutSamplesR;
};

#endif // _VSTPLUGIN_HOST