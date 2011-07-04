// Vst.cpp: implementation of the CVst class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
// #include "d:\ventana\ventana.h"
#include "Vst.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CVst::CVst()
{
	editorOpen=false;
	sprintf(m_dir,"Vst");

	MaxBufferLength=256;
	SamplesPerSec=44100;

	numwods=0;

	for(int c=0;c<MAX_VST_DLLS;c++)
	{
		m_bDll[c]=false;
		sprintf(plugFile[c],"");
	}

	for(c=0;c<MAX_VST_INSTANCES;c++)
	Instanced[c]=false;


}

CVst::~CVst()
{
//	FreeDlls();
	FreeAllVstInstances();
}


CVst::ReadDlls(char *newdir)
{
	FreeDlls();
	sprintf(m_dir,newdir);

	CFileFind finder;
	BOOL bWorking = finder.FindFile(CString(m_dir)+"/*.dll");

	while (bWorking)
	{
    bWorking = finder.FindNextFile();
   	dllHandle[numwods]=LoadLibrary(CString(m_dir)+"/"+finder.GetFileName());
	sprintf(plugFile[numwods],finder.GetFileName());
	m_bDll[numwods]=true;
	++numwods;
	}
}

CVst::FreeDlls()
{
	for(int c=0;c<numwods;c++)
	{
		if(m_bDll[c])
		{
		FreeLibrary(dllHandle[c]);
		m_bDll[c]=false;
		}
	}

	numwods=0;
}

int CVst::GetNumDrivers()
{
	return numwods;
}

char * CVst::GetDllName(int index)
{
	return plugFile[index];
}


char * CVst::GetPlugName(int index)
{
	return plugName[index];
}

bool CVst::InstanciateDll(char *name, int const index)
{
	bool canass=true;
	bool dllFound=false;
	int id=-1;
	for(int c=0;c<GetNumDrivers();c++)
	{
		if(strcmp(name,plugFile[c])==0)
		{
			if(canass)
			{
			id=c;
			dllFound=true;
			canass=false;
			}
		}
	}

	// DLL Found
	if(id>-1 && dllFound)
	{
	
	//	AfxMessageBox(GetDllName(id),MB_OK,0);
		//access to main function
			FreeVstInstance(index);
	
			AEffect *(*main)(audioMasterCallback audioMaster);

			main=(AEffect *(*)(audioMasterCallback audioMaster))GetProcAddress(dllHandle[id],"main");
			if(!main)
			{
				MessageBox(NULL,"This is not a VST plugin",NULL,MB_OK);
				effect[index]=NULL;
				Instanced[index]=false;
				return false;
			}

	
			//obtain AEffect structure
			effect[index]=main((audioMasterCallback)&Master);
			if(!effect[index] || effect[index]->magic!=kEffectMagic)
			{
				MessageBox(NULL,"This is not a VST plugin",NULL,MB_OK);
				effect[index]=NULL;
				Instanced[index]=false;
				return false;
			}

			//init plugin 
			effect[index]->dispatcher(effect[index], effOpen        ,  0, 0, NULL, 0.f);
			effect[index]->dispatcher(effect[index], effSetProgram  ,  0, 0, NULL, 0.f);
			effect[index]->dispatcher(effect[index], effMainsChanged,  0, 1, NULL, 0.f);
			effect[index]->dispatcher(effect[index], effSetSampleRate, 0, 0, 0, 44100);
			effect[index]->dispatcher(effect[index], effSetBlockSize,  0, 256, NULL, 0.f);

			sprintf(plugName[index],GetDllName(id));

			//keep plugin name

		Instanced[index]=true;
		}
	
	return dllFound;
}

CVst::FreeVstInstance(int index)
{
	if(Instanced[index])
	{
	effect[index]->dispatcher(effect[index], effMainsChanged, 0, 0, 0, 0.f);
	effect[index]->dispatcher(effect[index], effClose,        0, 0, 0, 0.f);
	effect[index]=NULL;
		
	Instanced[index]=false;
	}
}


long CVst::Master(AEffect *effect, long opcode, long index, long value, void *ptr, float opt)
{
	char txt[20];
	switch(opcode)
	{
	case	audioMasterWantMidi:sprintf(txt,"Want Midi");break;
		case 	audioMasterGetTime:sprintf(txt,"Get Time");break;
						
		case 	audioMasterProcessEvents:sprintf(txt,"Prec. Events");break;
		case 	audioMasterSetTime:sprintf(txt,"Set Time");break;
		case 	audioMasterTempoAt:sprintf(txt,"Temp At");break;

		case 	audioMasterGetNumAutomatableParameters:sprintf(txt,"GetNumAutPar");break;
		case 	audioMasterGetParameterQuantization:sprintf(txt,"ParamQuant");break;
										
										
		case 	audioMasterIOChanged:sprintf(txt,"IOchanged");break;
		case 	audioMasterNeedIdle	:sprintf(txt,"Need Idle");break;
		case 	audioMasterSizeWindow:sprintf(txt,"Size Window");break;
		case 	audioMasterGetSampleRate:sprintf(txt,"GetSampleRate");break;
		case 	audioMasterGetBlockSize:sprintf(txt,"GetBlockSize");break;
		case 	audioMasterGetInputLatency:sprintf(txt,"GetInLatency");break;
		case 	audioMasterGetOutputLatency:sprintf(txt,"GetOutLatency");break;
		case 	audioMasterGetPreviousPlug	:sprintf(txt,"PrevPlug");break;
		case 	audioMasterGetNextPlug:sprintf(txt,"NextPlug");break;

		case 	audioMasterWillReplaceOrAccumulate:sprintf(txt,"WillReplace");break;
		case 	audioMasterGetCurrentProcessLevel:sprintf(txt,"GetProcessLevel");break;
		case 	audioMasterGetAutomationState:sprintf(txt,"GetAutState");break;

		case 	audioMasterOfflineStart:sprintf(txt,"Offlinestart");break;
		case 	audioMasterOfflineRead:sprintf(txt,"Offlineread");break;
		case 	audioMasterOfflineWrite:sprintf(txt,"Offlinewrite");break;
		case 	audioMasterOfflineGetCurrentPass:sprintf(txt,"OfflineGetcurrentpass");break;
		case 	audioMasterOfflineGetCurrentMetaPass:sprintf(txt,"GetGetCurrentMetapass");break;

		case 	audioMasterSetOutputSampleRate:sprintf(txt,"Setsamplerate");break;
		case 	audioMasterGetSpeakerArrangement:sprintf(txt,"Getspeaker");break;
		case 	audioMasterGetVendorString:sprintf(txt,"GetVendor");break;
		case 	audioMasterGetProductString:sprintf(txt,"GetProduct");break;
		case 	audioMasterGetVendorVersion:sprintf(txt,"GetVendorver.");break;
		case 	audioMasterVendorSpecific:sprintf(txt,"VendorSpecific");break;
		case 	audioMasterSetIcon:sprintf(txt,"seticon");break;
		case 	audioMasterCanDo:sprintf(txt,"Can Do");break;
		case 	audioMasterGetLanguage:sprintf(txt,"GetLanguage");break;
		case 	audioMasterOpenWindow:sprintf(txt,"OpenWindow");break;
		case 	audioMasterCloseWindow:sprintf(txt,"CloseWindow");break;
		case 	audioMasterGetDirectory:sprintf(txt,"GetDirectory");break;
		case 	audioMasterUpdateDisplay:sprintf(txt,"UpdateDisplay");break;

		case	audioMasterAutomate:		sprintf(txt,"automat")	;break;
		case	audioMasterVersion:			sprintf(txt,"version")	;break;
		case	audioMasterCurrentId:		sprintf(txt,"currid.")	;break;
		case	audioMasterIdle: 			sprintf(txt,"idle   ")	;break;		
		case	audioMasterPinConnected:	sprintf(txt,"pincnct")	;break;
		default:							sprintf(txt,"undefed")	;break;
	}

	switch(opcode)
	{
	case	audioMasterAutomate: return 0;			// index, value, returns 0
	case	audioMasterVersion: return 2;				// vst version, currently 2 (0 for older)
	case	audioMasterCurrentId: return 'Argu';	// returns the unique id of a plug that's currently loading
	case	audioMasterIdle: 	return 0;					// call application idle routine (this will call effEditIdle for all open editors too) 
	case	audioMasterPinConnected: return true;	// inquire if an input or output is beeing connected;
																	// index enumerates input or output counting from zero,
																	// value is 0 for input and != 0 otherwise. note: the
																	// return value is 0 for <true> such that older versions
																	// will always return true.
	case audioMasterUpdateDisplay:
	redraw=TRUE;
	return 0;
	
	case audioMasterGetTime: 
	return 0;

//	case audioMasterTempoAt: return pMasterInfo->BeatsPerMin*10000;
	default: 
	return 0;
	}
}

CVst::FreeAllVstInstances()
{
	for(int c=0;c<MAX_VST_INSTANCES;c++)
	FreeVstInstance(c);
}
