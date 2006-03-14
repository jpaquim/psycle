#ifndef _VSTPLUGIN_HOST
#define _VSTPLUGIN_HOST


/** @file
 *  @brief implementation file
 *  $Date: 2005/01/15 22:44:20 $
 *  $Revision: 1.6 $
 */
#include "Machine.h"
#include "Vst\AEffectx.h"
#include "constants.h"
#include "Helpers.h"
#include "configuration.h"

#if !defined(_WINAMP_PLUGIN_)
	#include "NewMachine.h"
#endif

#define MAX_INOUTS		16

#define VST_QUANTIZATION 65535 // Dialog max ticks for parameters.
//define VST_QUANTIZATION 32767 // Dialog max ticks for parameters.


#define VSTINSTANCE_ERR_NO_VALID_FILE -1
#define VSTINSTANCE_ERR_NO_VST_PLUGIN -2
#define VSTINSTANCE_ERR_REJECTED -3
#define VSTINSTANCE_ERR_EXCEPTION -4
#define VSTINSTANCE_NO_ERROR 0

typedef AEffect* (*PVSTMAIN)(audioMasterCallback audioMaster);

typedef struct {
	unsigned char note;
	unsigned char midichan;
} VSTinote;

class VSTPlugin : public Machine
{
public:
	static const int NAME_LENGTH = 64;
	VSTPlugin();
	~VSTPlugin();

	void Free();
	int Instance(const TCHAR *dllname,bool overwriteName=true);
//	void Create(VSTPlugin *plug);
//	virtual void Init(void);
	virtual bool Load(RiffFile& riffFile);

	bool LoadChunk(RiffFile& riffFile);	// To be removed when changing the fileformat.
	virtual bool LoadSpecificFileChunk(RiffFile& riffFile, int version)
	{

		UINT size;
		riffFile.Read(&size,sizeof(size));

		if (size)
		{
			if (version > CURRENT_FILE_VERSION_MACD)
			{
				// data is from a newer format of psycle, it might be unsafe to load.
				riffFile.Skip(size);
				return FALSE;
			}

			riffFile.Read(&_program,sizeof(_program));
			// set the program
			SetCurrentProgram(_program);

			UINT count;

			riffFile.Read(&count,sizeof(count));  // size of vars

			for (UINT i = 0; i < count; i++)
			{
				float temp;
				riffFile.Read(&temp,sizeof(temp));
				SetParameter(i,temp);
			}

			size -= sizeof(_program)+sizeof(count)+(sizeof(float)*count);

			if (size)
			{
				if(_pEffect->flags & effFlagsProgramChunks)
				{
					char* pData = new char[size];
					riffFile.Read(pData, size); // Number of parameters
					try 
					{
						Dispatch(effSetChunk,0, size, pData,0.0f);
						delete pData;
					}
					catch (...)
					{
						MessageBox(NULL,SF::CResourceString(IDS_ERR_MSG0105)
							,GetDllName(),NULL);
						return FALSE;
					}
				}
				else
				{
					// there is a data chunk, but this machine does not want one.
					riffFile.Skip(size);
					return FALSE;
				}
			}
		}
		return TRUE;
	};

	bool LoadDll(TCHAR* psFileName)
	{
		_tcslwr(psFileName);

		TCHAR sPath2[_MAX_PATH];
		CString sPath;
#if defined(_WINAMP_PLUGIN_)
		sPath = Global::pConfig->GetVstDir();
		if ( FindFileinDir(psFileName,sPath) )
		{
			strcpy(sPath2,sPath);
			if (Instance(sPath2,false) != VSTINSTANCE_NO_ERROR)
			{
				::MessageBox(NULL,
					(SF::tformat(SF::CResourceString(IDS_ERR_MSG0015)) % sPath2).str().c_Str(),
					SF::CResourceString(IDS_ERR_MSG0016), MB_OK);
				return FALSE;
			}
		}
		else
		{
			::MessageBox(NULL,
				(SF::tformat(SF::CResourceString(IDS_ERR_MSG0015)) % sPath2).str().c_Str(),
				SF::CResourceString(IDS_ERR_MSG0016), MB_OK);
			return FALSE;
		}
#else // if !_WINAMP_PLUGIN_
		if ( CNewMachine::dllNames.Lookup(psFileName,sPath) )
		{
			_tcscpy(sPath2,sPath);
			if ( !CNewMachine::TestFilename(sPath2) ) 
			{
				return false;
			}
			if (Instance(sPath2,false) != VSTINSTANCE_NO_ERROR)
			{
				::MessageBox(NULL,
					(SF::tformat(SF::CResourceString(IDS_ERR_MSG0015)) % sPath2).str().data(),
					SF::CResourceString(IDS_ERR_MSG0016), MB_OK);
				return FALSE;
			}
		}
		else
		{
			::MessageBox(NULL,
				(SF::tformat(SF::CResourceString(IDS_ERR_MSG0015)) % sPath2).str().data(),
				SF::CResourceString(IDS_ERR_MSG0016), MB_OK);
			return FALSE;
		}
#endif // _WINAMP_PLUGIN_
		return TRUE;
	};



	bool SaveChunk(RiffFile& riffFile,bool &isfirst);	// "    "   "   " 
	virtual void SaveSpecificChunk(RiffFile& riffFile) 
	{
		UINT count = GetNumParams();
		UINT size = sizeof(_program)+sizeof(count)+(sizeof(float)*count);
		char* pData = NULL;

		if(_pEffect->flags & effFlagsProgramChunks)
		{
			try 
			{
				size += Dispatch( effGetChunk,0,0, &pData,0.0f);
			}
			catch (...)
			{
				::MessageBox(NULL,
					SF::CResourceString(IDS_ERR_MSG0017),
					GetDllName(), MB_OK);
				size = sizeof(_program) + sizeof(count) + (sizeof(float) * count);
			}
		}

		riffFile.Write(&size,sizeof(size));

		riffFile.Write(&_program,sizeof(_program));

		riffFile.Write(&count,sizeof(count));
		for (UINT i = 0; i < count; i++)
		{
			float temp = GetParameter(i);
			riffFile.Write(&temp,sizeof(temp));
		}

		size -= sizeof(_program) + sizeof(count) + (sizeof(float)*count);
		if (size > 0)
		{
			riffFile.Write(pData,size);
		}

	};

	virtual void SaveDllName(RiffFile& riffFile) 
	{
		CString str = GetDllName();
		TCHAR str2[256];
		_tcscpy(str2,str.Mid(str.ReverseFind(L'\\') + 1 ));
		CT2A _str2(str2);
		riffFile.Write(_str2,strlen(_str2) + 1);
	};

	virtual int GetNumParams(void) { return _pEffect->numParams; }
	virtual void GetParamName(int numparam,char* name)
	{
		if ( numparam < _pEffect->numParams ) Dispatch(effGetParamName,numparam,0,name,0.0f);
		else strcpy(name,"Out of Range");

	}

	virtual void GetParamValue(int numparam,TCHAR* parval)
	{
		if ( numparam < _pEffect->numParams ){
			DescribeValue(numparam,parval);
		} else {
			_tcscpy(parval,SF::CResourceString(IDS_MSG0002));
		}
	}

	virtual int GetParamValue(int numparam)
	{
		if ( numparam < _pEffect->numParams ) return f2i(_pEffect->getParameter(_pEffect, numparam)*65535);
		else return -1;
	}

	virtual TCHAR* GetName(void) { return _sProductName; }
	long GetVersion() { return _version; }
	TCHAR* GetVendorName(void) { return _sVendorName; }
	TCHAR* GetDllName(void) { return _sDllName; }

	long NumParameters(void) { return _pEffect->numParams; }
	float GetParameter(long parameter) { return _pEffect->getParameter(_pEffect, parameter); }
	bool DescribeValue(int parameter,TCHAR* psTxt);
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
	static long AudioMaster(AEffect *effect, long opcode, long index, long value, void *ptr, float opt);

	unsigned char _program;
	bool instantiated;
	int _instance;		// Remove when Changing the FileFormat.

	bool requiresProcess;	// It needs to use Process
	bool requiresRepl;		// It needs to use ProcessRepl

#if !defined(_WINAMP_PLUGIN_)
	CWindow* editorWnd;
#endif // _WINAMP_PLUGIN_
	
protected:

	HMODULE h_dll;
	TCHAR *_sDllName;	// Contains dll name

	TCHAR _sProductName[NAME_LENGTH];
	TCHAR _sVendorName[NAME_LENGTH];
	long _version;
	bool _isSynth;
	bool wantidle;

	float * inputs[MAX_INOUTS];
	float * outputs[MAX_INOUTS];
	float junk[STREAM_SIZE];

	static VstTimeInfo _timeInfo;
	VstMidiEvent midievent[MAX_VST_EVENTS];
	VstEvents events;
	int	queue_size;

};

class VSTInstrument : public VSTPlugin
{
public:
	VSTInstrument(int index);
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
	VSTFX(int index);
	~VSTFX();
	virtual void Tick(int channel, PatternEntry* pEntry);
	virtual void Work(int numSamples);

protected:
	float *_pOutSamplesL;
	float *_pOutSamplesR;
};

#endif // _VSTPLUGIN_HOST

