#ifndef _PLUGIN_H
#define _PLUGIN_H

#include "Machine.h"
#include "Song.h"
#include "Configuration.h"
#include "MachineInterface.h"


class PluginFxCallback : public CFxCallback
{
public:
	HWND hWnd;

	virtual void MessBox(char* ptxt,char *caption,unsigned int type)
	{
		MessageBox(hWnd,ptxt,caption,type);
	}
	virtual int GetTickLength(void)
	{
		return Global::_pSong->SamplesPerTick;
	}
	virtual int GetSamplingRate(void)
	{
#if defined(_WINAMP_PLUGIN_)
		return Global::pConfig->_samplesPerSec;
#else
		return Global::pConfig->_pOutputDriver->_samplesPerSec;
#endif // _WINAMP_PLUGIN_
	}
	virtual int GetBPM(void)
	{
		return Global::_pSong->BeatsPerMin;
	}
	virtual int GetTPB(void)
	{
		return Global::_pSong->_ticksPerBeat;
	}
};

class Plugin : public Machine
{
public:

	Plugin();
	virtual ~Plugin();

	virtual void Init(void);
	virtual void Work(int numSamples);
	virtual void Stop(void);
	void Tick(void);
	virtual void Tick(int channel, PatternEntry* pEntry);
	virtual char* GetName(void) { return _psName; };
	virtual int GetNumParams(void) { return _pInfo->numParameters; }
	virtual void GetParamName(int numparam,char* name)
	{
		if ( numparam < _pInfo->numParameters )
			strcpy(name,_pInfo->Parameters[numparam]->Name);
		else strcpy(name,"Out of Range");

	}
	virtual void GetParamValue(int numparam,char* parval)
	{
		if ( numparam < _pInfo->numParameters )
		{
			if ( _pInterface->DescribeValue(parval,numparam,_pInterface->Vals[numparam]) == false )
			{
				sprintf(parval,"%i",_pInterface->Vals[numparam]);
			}
		}
		else strcpy(parval,"Out of Range");
	}
	virtual int GetParamValue(int numparam)
	{
		if ( numparam < _pInfo->numParameters )
			return _pInterface->Vals[numparam];
		else return -1;
	}
	virtual bool SetParameter(int numparam,int value)
	{
		if ( numparam < _pInfo->numParameters )
		{
			_pInterface->ParameterTweak(numparam,value);
			return true;
		}
		else return false;
	}
	virtual bool Load(RiffFile* pFile);
#if !defined(_WINAMP_PLUGIN_)
	virtual bool Save(RiffFile* pFile);
#endif // ndef _WINAMP_PLUGIN_

	bool Instance(char* psFileName);
	void Free(void);
//	bool Create(Plugin *plug);

	bool IsSynth(void) { return _isSynth; }
	char* GetDllName(void) { return _psDllName; }
	char* GetShortName(void) { return _psShortName; }
	char* GetAuthor(void) { return _psAuthor; }

	CMachineInfo* GetInfo(void) { return _pInfo; };
	CMachineInterface* GetInterface(void) { return _pInterface; };
	PluginFxCallback* GetCallback(void) { return &_callback; };

protected:
	HINSTANCE _dll;
	char _psShortName[16];
	char* _psAuthor;
	char* _psDllName;
	char* _psName;
	bool _isSynth;
	static PluginFxCallback _callback;
	CMachineInfo* _pInfo;
	CMachineInterface* _pInterface;
};

#endif