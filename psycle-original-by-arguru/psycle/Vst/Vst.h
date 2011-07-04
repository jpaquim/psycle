// Vst.h: interface for the CVst class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_VST_H__AEA9F7CF_5BE5_11D4_A87E_00C026101D9A__INCLUDED_)
#define AFX_VST_H__AEA9F7CF_5BE5_11D4_A87E_00C026101D9A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "aeffectx.h"
#include "AEffEditor.hpp"

#define MAX_VST_DLLS		256
#define MAX_VST_INSTANCES	256
#define WM_ROUT (WM_USER+'R')
#define WM_ROUT_BUZZ_CHANGED_PROGRAM (WM_ROUT+1)

static bool redraw;

class CVst  
{
public:
	EditPlug(int index);
	FreeAllVstInstances();
	FreeVstInstance(int index);
	bool InstanciateDll(char* name, int const index);

	char * GetDllName(int index);
	char * GetPlugName(int index);
	
	CVst();
	virtual ~CVst();
	AEffect * effect[MAX_VST_INSTANCES];
	bool Instanced[MAX_VST_INSTANCES];

	HMODULE dllHandle[MAX_VST_DLLS];

	int GetNumDrivers();
	FreeDlls();
	ReadDlls(char *newdir);
	static long Master(AEffect *effect, long opcode, long index, long value, void *ptr, float opt);

private:
	bool editorOpen;

	int numwods;
	char m_dir[_MAX_PATH];

	bool m_bDll[MAX_VST_DLLS];
	char plugFile[MAX_VST_DLLS][256];
	char plugName[MAX_VST_INSTANCES][256];

	int MaxBufferLength;
	int SamplesPerSec;


};

#endif // !defined(AFX_VST_H__AEA9F7CF_5BE5_11D4_A87E_00C026101D9A__INCLUDED_)
