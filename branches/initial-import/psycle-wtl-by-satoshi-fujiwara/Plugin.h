#ifndef _PLUGIN_H
#define _PLUGIN_H
/** @file
 *  @brief implementation file
 *  $Date: 2005/01/15 22:44:20 $
 *  $Revision: 1.5 $
 */
#include "Machine.h"
#include "Song.h"
#include "Instrument.h"
#include "configuration.h"
#include "MachineInterface.h"
#include "Player.h"

#if !defined(_WINAMP_PLUGIN_)
	#include "NewMachine.h"
#endif


class PluginFxCallback : public CFxCallback
{
public:
	HWND hWnd;

	virtual void MessBox(TCHAR* ptxt,TCHAR *caption,unsigned int type)
	{
		MessageBox(hWnd,ptxt,caption,type);
	}
	virtual int GetTickLength(void)
	{
		return Global::_pSong->SamplesPerTick();
	}
	virtual int GetSamplingRate(void)
	{
		return Global::pConfig->_pOutputDriver->_samplesPerSec;
	}
	virtual int GetBPM(void)
	{
		return Global::pPlayer->bpm;
	}
	virtual int GetTPB(void)
	{
		return Global::pPlayer->tpb;
	}
};

class Plugin : public Machine
{
public:

	Plugin(int index);
	virtual ~Plugin();

	virtual void Init(void);
	virtual void Work(int numSamples);
	virtual void Stop(void);
	void Tick(void);
	virtual void Tick(int channel, PatternEntry* pEntry);
	virtual TCHAR* GetName(void) { return _psName; };
	virtual int GetNumParams(void) { return _pInfo->numParameters; }
	virtual void GetParamName(int numparam,TCHAR* name)
	{
		if ( numparam < _pInfo->numParameters )
			_tcscpy(name,CA2T(_pInfo->Parameters[numparam]->Name));
		else _tcscpy(name,_T("Out of Range"));

	}
	virtual void GetParamValue(int numparam,TCHAR* parval)
	{
		if ( numparam < _pInfo->numParameters )
		{
			if ( _pInterface->DescribeValue(CT2A(parval),numparam,_pInterface->Vals[numparam]) == false )
			{
				_stprintf(parval,_T("%i"),_pInterface->Vals[numparam]);
			}
		}
		else _tcscpy(parval,_T("Out of Range"));
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
	virtual bool Load(RiffFile& riffFile);
	virtual bool LoadSpecificFileChunk(RiffFile& riffFile, int version)
	{
		UINT size;
		riffFile.Read(&size,sizeof(size)); // size of whole structure

		if (size)
		{
			if (version > CURRENT_FILE_VERSION_MACD)
			{
				// data is from a newer format of psycle, it might be unsafe to load.
				riffFile.Skip(size);
				return FALSE;
			}
			else
			{
				UINT count;
				riffFile.Read(&count,sizeof(count));  // size of vars
				/*
				if (count)
				{
					riffFile.Read(_pInterface->Vals,sizeof(_pInterface->Vals[0])*count);
				}
				*/
				for (UINT i = 0; i < count; i++)
				{
					int temp;
					riffFile.Read(&temp,sizeof(temp));
					SetParameter(i,temp);
				}

				size -= sizeof(count) + sizeof(int)*count;

				if (size)
				{
					byte* pData = new byte[size];
					riffFile.Read(pData, size); // Number of parameters
					try 
					{
						_pInterface->PutData(pData); // Internal load
						delete pData;
					}
					catch (...)
					{
						MessageBox(NULL,SF::CResourceString(IDS_ERR_MSG0104),GetDllName(),NULL);
						return FALSE;
					}
					return TRUE;
				}
			}
		}
		return TRUE;
	};

	virtual void SaveSpecificChunk(RiffFile& riffFile) 
	{
		UINT count = GetNumParams();
		UINT size2 = _pInterface->GetDataSize();
		UINT size = size2 + sizeof(count) + sizeof(int)*count;

		riffFile.Write(&size,sizeof(size));
		riffFile.Write(&count,sizeof(count));
//		riffFile.Write(_pInterface->Vals,sizeof(_pInterface->Vals[0])*count);

		for (UINT i = 0; i < count; i++)
		{
			int temp = GetParamValue(i);
			riffFile.Write(&temp,sizeof(temp));
		}

		if (size2)
		{
			byte* pData = new byte[size2];
			try 
			{
				_pInterface->GetData(pData); // Internal save
			}
			catch (...)
			{
				MessageBox(NULL,
					SF::CResourceString(IDS_ERR_MSG0017),GetDllName(),NULL);
			}
			riffFile.Write(pData, size2); // Number of parameters
			delete pData;
		}
	};
	
	virtual void SaveDllName(RiffFile& riffFile) 
	{
		CString str = _psDllName;
		TCHAR str2[256];
		_tcscpy(str2,str.Mid(str.ReverseFind(_T('\\'))+1));
		riffFile.Write(CT2A(str2),_tcslen(str2)+1);
	};


	bool Instance(TCHAR* psFileName);
	void Free(void);
//	bool Create(Plugin *plug);
	bool LoadDll(TCHAR* psFileName)
	{
		_tcslwr(psFileName);
		TCHAR sPath2[_MAX_PATH];
		CString sPath;
#if defined(_WINAMP_PLUGIN_)
		sPath = Global::pConfig->GetPluginDir();

		if ( FindFileinDir(psFileName,sPath) )
		{
			_tcscpy(sPath2,sPath);
			return Instance(sPath2);
		}
#else
		if ( !CNewMachine::dllNames.Lookup(psFileName,sPath) ) 
		{
//			Check Compatibility Table.
//			Probably could be done with the dllNames lockup.
//
//			GetCompatible(psFileName,sPath2) // If no one found, it will return a null string.
			_tcscpy(sPath2,psFileName);
		}
		else 
		{ 
			_tcscpy(sPath2,sPath); 
		}

		if ( !CNewMachine::TestFilename(sPath2) ) 
		{
			return false;
		}
		if (!Instance(sPath2))
		{
			::MessageBox(NULL,
				(SF::tformat(SF::CResourceString(IDS_ERR_MSG0103)) % psFileName).str().data(),
				SF::CResourceString(IDS_ERR_MSG0043), MB_OK);
			return FALSE;
		}
		else
		{
			return TRUE;
		}
#endif // _WINAMP_PLUGIN_	};
		return FALSE;
	};

	bool IsSynth(void) { return _isSynth; }
	TCHAR* GetDllName(void) { return _psDllName; }
	TCHAR* GetShortName(void) { return _psShortName; }
	TCHAR* GetAuthor(void) { return _psAuthor; }

	CMachineInfo* GetInfo(void) { return _pInfo; };
	CMachineInterface* GetInterface(void) { return _pInterface; };
	PluginFxCallback* GetCallback(void) { return &_callback; };


protected:
	HINSTANCE _dll;
	TCHAR _psShortName[16];
	TCHAR* _psAuthor;
	TCHAR* _psDllName;
	TCHAR* _psName;
	bool _isSynth;
	static PluginFxCallback _callback;
	CMachineInfo* _pInfo;
	CMachineInterface* _pInterface;

};


#endif