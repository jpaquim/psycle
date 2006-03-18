#include "VST2Plugin.h"
#include "dsplib.h"
DLL_EXPORTS

static bool redraw;

void CTrack::Stop()
{
	if (noteactive) pmi->Endnote(Note,channel);
	noteactive=FALSE;
}

void CTrack::Init()
{
	volume=100;
}

#define midinote '\x091'
#define midioff '\x081'

void mi::SendVSTnoteevent(int note,int volume,byte channel)
{
	if(effect)
	{
	if (!events) events=new(VstEvents);
	events->numEvents=1;
	VstEvent* a;
	a=new(VstEvent);
	VstMidiEvent* b=(VstMidiEvent*)a;
	b->type=kVstMidiType;
	b->byteSize=24;
	b->deltaFrames=0;
	b->flags=0;
	b->detune=0;
	b->noteLength=0;
	b->noteOffset=0;
	b->reserved1=0;
	b->reserved2=0;
	if (volume>0) b->midiData[0]=midinote+channel; else b->midiData[0]=midioff+channel;
	b->midiData[1]=note;
	b->midiData[2]=volume;
	events->events[0]=a;
	effect->dispatcher(effect,effProcessEvents,0,0,events,0);
	delete(a);
	}
}	


void mi::Newnote(int note,int volume,byte channel)
{
	SendVSTnoteevent(note,volume,channel);
}
void mi::Endnote(int note,byte channel)
{
	SendVSTnoteevent(note,0,channel);
}

void CTrack::Tick( tvals const &tv)
{
    if( (tv.note != paraNote.NoValue) && (tv.note != NOTE_OFF)) 
	{
		if (noteactive==TRUE) pmi->Endnote(Note,channel);
		Note=((tv.note>>4)*12)+(tv.note&0x0f)-1; 
		pmi->Newnote(Note,volume,channel);	
		noteactive=TRUE;
	}
	if( tv.note == NOTE_OFF)
	{
		pmi->Endnote(Note,channel);
		noteactive=FALSE;
	}
    if( tv.volume != paraVolume.NoValue) 
	{
		volume=tv.volume;
	}
	if (tv.channel!=paraChannel.NoValue)
	{
		channel=tv.channel;
	}
	if (tv.effect!=paraEffect.NoValue && tv.effectvalue!=paraEffectvalue.NoValue)
	{
		//pmi->effect
	}
	
}

void mi::Command(int const i)
{
	OPENFILENAME ofn;
	switch(i)
	{
	case 0: //"Load..."
		
		while(directoryString[0]==0)
		{
			SelectNewPluginFolder();

			if(directoryString[0]==0)
				MessageBox(NULL,"You must select a folder !!!",NULL,MB_OK);
		}
		
		ZeroMemory(&ofn,sizeof(OPENFILENAME));
		ofn.lStructSize=sizeof(OPENFILENAME);
		ofn.hInstance=hInst;
		ofn.lpstrFilter="VST Plugins (*.dll)\0*.dll\0\0";
		ofn.lpstrFile=(char*)malloc(256);
		ofn.lpstrFileTitle=(char*)malloc(256);
		ofn.lpstrInitialDir=directoryString;
		ofn.nMaxFile=MAX_PATH;
		ofn.nMaxFileTitle=MAX_PATH;
		ofn.Flags=OFN_NOCHANGEDIR|OFN_HIDEREADONLY;
		strcpy(ofn.lpstrFile,"");

		if(GetOpenFileName(&ofn))
		{
			if(effect && plugin)
			{	
				effect->dispatcher(effect, effMainsChanged, 0, 0, 0, 0.f);
				effect->dispatcher(effect, effClose,        0, 0, 0, 0.f);
				effect=NULL;

				FreeLibrary(plugin);
				plugin=NULL;
			}

			AEffect *(*main)(audioMasterCallback audioMaster);

			//load dll
			char str[256];
			strcpy(str,directoryString);
			strcat(str,"\\");
			strcat(str,ofn.lpstrFileTitle);

			plugin=LoadLibrary(str);
			if(!plugin)
			{
				char msg[500];

				sprintf(msg,"This file could not be opened.\nThe vst plugin you want to use MUST be placed in the plugin folder.  This folder can be changed in the 'Options/About' dialog box.\n\nMake sure the file \"%s\" exists and is a dll.",str);
				MessageBox(NULL,msg,NULL,MB_OK);
				return;
			}

			//access to main function
			main=(AEffect *(*)(audioMasterCallback audioMaster))GetProcAddress(plugin,"main");
			if(!main)
			{
				MessageBox(NULL,"This is not a VST plugin",NULL,MB_OK);
				plugin=NULL;
				effect=NULL;
				return;
			}

			//obtain AEffect structure
			effect=main((audioMasterCallback)&Master);
			if(!effect || effect->magic!=kEffectMagic)
			{
				MessageBox(NULL,"This is not a VST plugin",NULL,MB_OK);
				plugin=NULL;
				effect=NULL;
				return;
			}

			//init plugin 
			effect->dispatcher(effect, effOpen        ,  0, 0, NULL, 0.f);
			effect->dispatcher(effect, effSetProgram  ,  0, 0, NULL, 0.f);
			effect->dispatcher(effect, effMainsChanged,  0, 1, NULL, 0.f);
			effect->dispatcher(effect, effSetSampleRate, 0, 0, 0, (float)pMasterInfo->SamplesPerSec);
			effect->dispatcher(effect, effSetBlockSize,  0, MAX_BUFFER_LENGTH, NULL, 0.f);

			//keep plugin name
			strcpy(plugname, ofn.lpstrFileTitle);

		}
		free(ofn.lpstrFile);
		free(ofn.lpstrFileTitle);
		break;

	case 1: //"Edit..."
		if(!plugin)
		{
			MessageBox(NULL,"Please load a plugin first",NULL,MB_OK);
			break;
		}
		if(effect->flags & effFlagsHasEditor)
		{
			//pass effect ptr
			long success=DlgEditor(NULL,WM_ROUT,(long)effect,(long)this);

			if(!success)
				MessageBox(NULL,"Please close the previous parameter editor",NULL,MB_OK);
			else
			{
				//Load plugin editor
				char str[256];
				getPlugname(str);

				HMENU menu=LoadMenu(hInst,(char*)IDR_EDITORMENU);

				HWND hwnd=CreateWindowEx(/*WS_EX_TOOLWINDOW|*/WS_EX_TOPMOST,"VST2pluginwrapper",str,WS_OVERLAPPEDWINDOW-WS_MINIMIZEBOX-WS_MAXIMIZEBOX-WS_THICKFRAME,CW_USEDEFAULT,CW_USEDEFAULT,
					CW_USEDEFAULT,CW_USEDEFAULT,HWND_DESKTOP,menu,hInst,NULL);
				
				if(!hwnd)
					MessageBox(NULL,"Can't create editor window",NULL,MB_OK);

				ShowWindow(hwnd,SW_SHOW);
				UpdateWindow(hwnd);
			}
		}
		else
		{
			//pass editor address
			long success=EditDlgFunc(NULL,WM_ROUT,(long)effect,(long)this);
			if(!success)
				MessageBox(NULL,"Please close the previous parameter editor",NULL,MB_OK);
			else
				//load my own multi-effect editor
				DialogBox(hInst,(LPCSTR)IDD_EDITDIALOG,NULL,(DLGPROC)EditDlgFunc);
		}
		break;
	case 2: //"About..."
			//pass editor address
			long success=AboutFunc(NULL,WM_ROUT,(long)effect,(long)this);
			if(!success)
				MessageBox(NULL,"Please close the previous about box",NULL,MB_OK);
			else
				DialogBox(hInst,(LPCSTR)IDD_ABOUT,NULL,(DLGPROC)AboutFunc);
		break;
	}
}

void mi::MidiNote(int const channel, int const value, int const velocity) 
{
	SendVSTnoteevent(value, velocity,channel);
}

mi::mi()
{
        TrackVals = tval;
        GlobalVals = &gval;
        AttrVals = NULL;   // attributes
//		events.events=new(VstMidiEvent[MAX_TRACKS]);
	thissie=this;
	hInst=GetModuleHandle(dllName);

	plugin=NULL;
	effect=NULL;
	plugname[0]=0;
	directoryString[0]=0;
	unsigned long size=sizeof(directoryString);
	HKEY hkey;
	unsigned long action;
	RegCreateKeyEx(HKEY_CURRENT_USER,RegKeyName,NULL,
		"class",NULL,KEY_ALL_ACCESS,NULL,&hkey,&action);

	if(action==REG_OPENED_EXISTING_KEY)
		RegQueryValueEx(hkey,"plugin directory",NULL,NULL,(unsigned char*)directoryString,&size);

	RegCloseKey(hkey);

	WNDCLASS wcl;

	ZeroMemory(&wcl,sizeof(WNDCLASS));
	wcl.hInstance=hInst;
	wcl.lpszClassName="VST2pluginwrapper";
	wcl.lpfnWndProc=(WNDPROC)(DlgEditor);
	wcl.hIcon=LoadIcon(NULL,IDI_APPLICATION);
	wcl.hCursor=LoadCursor(NULL,IDC_ARROW);
	wcl.hbrBackground=(HBRUSH)GetStockObject(GRAY_BRUSH);

	RegisterClass(&wcl);

	InitCommonControls();

	editorOpen=false;

}

mi::~mi()
{
}
long mi::Master(AEffect *effect, long opcode, long index, long value, void *ptr, float opt)
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
	case	audioMasterCurrentId: return 'Zeph';	// returns the unique id of a plug that's currently loading
	case	audioMasterIdle: 	return 0;					// call application idle routine (this will call effEditIdle for all open editors too) 
	case	audioMasterPinConnected: return true;	// inquire if an input or output is beeing connected;
																	// index enumerates input or output counting from zero,
																	// value is 0 for input and != 0 otherwise. note: the
																	// return value is 0 for <true> such that older versions
																	// will always return true.

	case 	
		audioMasterUpdateDisplay:redraw=TRUE;
		return 0;
	case audioMasterGetTime: return 0;
//	case audioMasterTempoAt: return pMasterInfo->BeatsPerMin*10000;
	default: return 0;
	}
}
void mi::Init(CMachineDataInput * const pi)
{
	for (int c = 0; c < MAX_TRACKS; c++)
	{
		Tracks[c].pmi = this;
		Tracks[c].Stop();
		Tracks[c].Init();
	}
		if(pi)
	{
		pi->Read(plugname,sizeof(plugname));

		while(directoryString[0]==0)
		{
			SelectNewPluginFolder();

			if(directoryString[0]==0)
				MessageBox(NULL,"You must select a folder !!!",NULL,MB_OK);
		}

		//load dll
		char str[256];
		strcpy(str,directoryString);
		strcat(str,"\\");
		strcat(str,plugname);
		plugin=LoadLibrary(str);
		if(!plugin)
		{
			sprintf(str,"The VST plugin used in this song could not be found in the VST plugin directory.\n\nplugin:\n%s\n\ndirectory:\n%s",plugname,directoryString);
			MessageBox(NULL,str,NULL,MB_OK);
			return;
		}

		//access to main function
		AEffect *(*main)(audioMasterCallback audioMaster);

		main=(AEffect *(*)(audioMasterCallback audioMaster))GetProcAddress(plugin,"main");
		if(!main)
		{
			MessageBox(NULL,"This is not a VST plugin",NULL,MB_OK);
			return;
		}

		//obtain AEffect structure
		effect=main((audioMasterCallback)&Master);
		if(!effect || effect->magic!=kEffectMagic)
		{
			MessageBox(NULL,"This is not a VST plugin",NULL,MB_OK);
			plugin=NULL;
			effect=NULL;
			return;
		}

		//init plugin
		effect->dispatcher(effect, effOpen        ,  0, 0, NULL, 0.f);
		effect->dispatcher(effect, effMainsChanged,  0, 1, NULL, 0.f);
		effect->dispatcher(effect, effSetSampleRate, 0, 0, NULL, 44100.f);
		effect->dispatcher(effect, effSetBlockSize,  0, MAX_BUFFER_LENGTH, NULL, 0.f);

		//BLOCK SIZE ???

		//read parameters
		long activeProg;
		pi->Read(&activeProg,sizeof(long));

		for(long j=0;j<effect->numPrograms;j++)
		{
			char str[25];
			
			pi->Read(str,sizeof(str));
			effect->dispatcher(effect,effSetProgram,0,j,NULL,0.f);
			effect->dispatcher(effect,effSetProgramName,0,0,str,0.f);

			for(long i=0;i<effect->numParams;i++)
			{
				float value;
				pi->Read(&value,sizeof(float));
				effect->setParameter(effect,i,value);
			}
		}
		effect->dispatcher(effect,effSetProgram,0,activeProg,NULL,0.f);
	}


}
void mi::Save(CMachineDataOutput * const po)
{
	if(effect)
	{
		long activeProg,previousPlug;

		po->Write(plugname,sizeof(plugname));

		previousPlug= activeProg= effect->dispatcher(effect,effGetProgram,0,0,NULL,0.f);

		po->Write(&activeProg,sizeof(long));

		for(long j=0;j<effect->numPrograms;j++)
		{
			char str[25];

			effect->dispatcher(effect,effSetProgram,0,j,NULL,0.f);
			effect->dispatcher(effect,effGetProgramName,0,0,str,0.f);
			po->Write(str,sizeof(str));

			for(long i=0;i<effect->numParams;i++)
			{
				float value=effect->getParameter(effect,i);
				po->Write(&value,sizeof(float));
			}
		}

		effect->dispatcher(effect,effSetProgram,0,previousPlug,NULL,0.f);
	}
}

void mi::Tick()
{
	if (effect) for (int i=0; i<numTracks; i++) Tracks[i].Tick( tval[i]);
}

bool mi::WorkMonoToStereo(float *pin, float *pout, int numsamples, int const mode)
{
	if (effect)
	{
		if (outputs[0]) delete outputs[0];
		if (outputs[1]) delete outputs[1];
		if (inputs[0]) delete inputs[0];
		if (inputs[1]) delete inputs[1];
		outputs[0]=new(float[MAX_BUFFER_LENGTH]);
		outputs[1]=new(float[MAX_BUFFER_LENGTH]);
		inputs[0]=new(float[MAX_BUFFER_LENGTH]);
		inputs[1]=new(float[MAX_BUFFER_LENGTH]);
		for (int i=0;i<numsamples;i++) 
		{
			outputs[0][i]=outputs[1][i]=inputs[0][i]=inputs[1][i]=0;
		}
		effect->process(effect,inputs,outputs,numsamples);
			
		for (i=0;i<numsamples;i++) 
		{
			*pout++=outputs[0][i]*32768;
			*pout++=outputs[1][i]*32768;
		}
		i=0;
		return TRUE;
		while (i<numsamples) 
		{
			if (outputs[0][i]!=0) return TRUE;
			if (outputs[1][i]!=0) return TRUE;
			i++;
		}
	}
	return FALSE;
}

void mi::Stop()
{
        for( int i=0; i<MAX_TRACKS; i++) Tracks[i].Stop();
}

char const *mi::DescribeValue(int const param, int const value)
{
	static char txt[16];

	switch(param)
	{
	case 0:default:sprintf(txt, "%d %%",(int)((float)value/128*100));break;
	}

	return txt;
}
void mi::SelectNewPluginFolder(void)
{
	//there is no plugin directory specified !!
	BROWSEINFO binfo;
	ZeroMemory(&binfo,sizeof(BROWSEINFO));
	binfo.lpszTitle="Please select the folder in which you keep your VST plugins";
	
	LPITEMIDLIST plist=SHBrowseForFolder(&binfo);
	
	if(plist==NULL)
		return;

	SHGetPathFromIDList(plist,directoryString);

	HKEY hkey;
	unsigned long action;
	RegCreateKeyEx(HKEY_CURRENT_USER,RegKeyName,NULL,
		"class",NULL,KEY_ALL_ACCESS,NULL,&hkey,&action);
	
	RegSetValueEx(hkey,"plugin directory",NULL,REG_SZ,(unsigned char*)directoryString,strlen(directoryString));

	RegCloseKey(hkey);
}

void mi::getPlugname(char * dest)
{
	long i;

	if(effect)
	{
		i=0;
		while(plugname[i]!='.')
		{
			dest[i]=plugname[i];
			i++;
		}
		dest[i]=0;
	}
	else
		strcpy(dest,"[None]");
}
BOOL CALLBACK DlgEditor(HWND hwnd, UINT message, WPARAM wP, LPARAM lP)
{
	static AEffect * effect;
	static mi * pmi;
	ERect rect;
	RECT wrect;
	ERect * prect;
	int h,w;
	HMENU menu;
	MENUITEMINFO menuinfo;
	char str[256];
	char str2[256];
	long previousProg;
	long i;
	if (redraw)
	{
		GetWindowRect(hwnd,&wrect);
		InvalidateRect( hwnd, &wrect, false );
		redraw=FALSE;
	}
	
	switch(message)
	{
	case WM_ROUT:
		if(effect)
			return 0;
		effect=(AEffect*)wP;
		pmi=(mi*)lP;
		return 1;
	case WM_CREATE:
		pmi->editorOpen=true;
		effect->dispatcher(effect, effEditOpen, 0, 0, hwnd, 0.f);
		
		//resize window to editor size
		effect->dispatcher(effect, effEditGetRect, 0, 0, &prect, 0.f);
		w =	(prect->right - prect->left) + GetSystemMetrics(SM_CXEDGE) * 2; // Borders
		h =	(prect->bottom - prect->top) + GetSystemMetrics(SM_CYCAPTION)
			+ GetSystemMetrics(SM_CYMENUSIZE) + GetSystemMetrics(SM_CYEDGE); // Bar + Menu
		SetWindowPos(hwnd,HWND_TOP,0,0,w,h,SWP_NOMOVE);

		menu=GetMenu(hwnd);
		menu=GetSubMenu(menu,0);

		//add menu items
		previousProg=effect->dispatcher(effect,effGetProgram,0,0,NULL,0.f);
		for(i=0;i<effect->numPrograms;i++)
		{
			ZeroMemory(&menuinfo,sizeof(MENUITEMINFO));
			menuinfo.cbSize=sizeof(MENUITEMINFO);

			menuinfo.fMask=MIIM_DATA|MIIM_TYPE|MIIM_ID;
			menuinfo.fType=MFT_STRING;
			menuinfo.wID=i;
			effect->dispatcher(effect,effSetProgram,0,i,NULL,0.f);
			effect->dispatcher(effect,effGetProgramName,0,0,str2,0.f);
			sprintf(str,"%d  %s",i+1,str2);
			menuinfo.dwTypeData=str;
			menuinfo.cch=strlen(str);

			InsertMenuItem(menu,i,true,&menuinfo);
		}

		CheckMenuRadioItem(menu,0,effect->numPrograms,previousProg,MF_BYPOSITION);
		effect->dispatcher(effect,effSetProgram,0,previousProg,NULL,0.f);

		break;
	case WM_PAINT:
		{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint( hwnd, &ps );
		RECT clipRect;
		

		GetWindowRect(hwnd,&wrect);

		rect.left=(short)wrect.left;
		rect.right=(short)wrect.right;
		rect.top=(short)wrect.top;
		rect.bottom=(short)wrect.bottom;

//		effect->dispatcher(effect, effEditDraw, 0, 0, &ps.rcPaint, 0.f);
		effect->dispatcher(effect, effEditDraw, 0, 0, &wrect, 0.f);
		/*RECT r;
		r.left=(short)wrect.left;
		r.right=(short)wrect.right;
		r.top=(short)wrect.top;
		r.bottom=(short)wrect.bottom;
		InvalidateRect( hwnd, &r, false );*/
		EndPaint( hwnd, &ps );

		return 0;break;
		}
	case WM_SETCURSOR:
		switch(LOWORD(lP))
		{
			case	HTCLIENT:
				SetCursor (LoadCursor (NULL, IDC_ARROW));
				return 0;
		}
		break;
	case WM_ACTIVATE:
		if(LOWORD(wP) & WA_ACTIVE)
			effect->dispatcher(effect, effEditTop,0,0,NULL,0.f);
		else if(LOWORD(wP) & WA_INACTIVE)
			effect->dispatcher(effect, effEditSleep,0,0,NULL,0.f);

		GetWindowRect(hwnd,&wrect);
		InvalidateRect( hwnd, &wrect, false );

		break;
	case WM_MOUSEMOVE:
		effect->dispatcher(effect,effEditMouse,LOWORD(lP),HIWORD(lP),NULL,0.f);
		GetWindowRect(hwnd,&wrect);
		InvalidateRect( hwnd, &wrect, false );
		redraw=TRUE;
		break;
	case WM_KEYDOWN:
		effect->dispatcher(effect,effEditKey,0,wP,NULL,0.f);
		redraw=TRUE;
		break;
	case WM_CLOSE:
		effect->dispatcher(effect,effEditClose,0,0,NULL,0.f);
		effect=NULL;
		pmi->editorOpen=false;
		break;
	case WM_COMMAND:
		//---MENUS---
		i=LOWORD(wP);
		if(i>=0 && i<effect->numPrograms)
		{
			effect->dispatcher(effect,effSetProgram,0,i,NULL,0.f);
			menu=GetMenu(hwnd);
			menu=GetSubMenu(menu,0);

			CheckMenuRadioItem(menu,0,effect->numPrograms,i,MF_BYPOSITION);
			SendMessage(hwnd,WM_ACTIVATE,WA_ACTIVE,0);
			UpdateWindow(hwnd);
		}
		else if(i==ID_PROG_RENAME)
		{
			long success=RenameProgProc(NULL,WM_ROUT,(long)effect,(long)pmi);
			if(!success)
				MessageBox(NULL,"Please close the previous box",NULL,MB_OK);
			else
			{
				DialogBox(pmi->hInst,(char*)IDD_RENAMEPROG,NULL,(DLGPROC)RenameProgProc);
				//update menu with new name
				i=effect->dispatcher(effect,effGetProgram,0,0,NULL,0.f);
				
				ZeroMemory(&menuinfo,sizeof(MENUITEMINFO));
				menuinfo.cbSize=sizeof(MENUITEMINFO);
				menuinfo.fMask=MIIM_DATA|MIIM_TYPE;
				menuinfo.fType=MFT_STRING|MFT_RADIOCHECK;
				effect->dispatcher(effect,effGetProgramName,0,0,str2,0.f);
				sprintf(str,"%d  %s",i+1,str2);
				menuinfo.dwTypeData=str;
				menuinfo.cch=strlen(str);

				menu=GetMenu(hwnd);
				menu=GetSubMenu(menu,0);
				SetMenuItemInfo(menu,i,true,&menuinfo);
			}
		}
		redraw=TRUE;
		break;
	}
	return DefWindowProc(hwnd,message,wP,lP);
}


//my multi-effect editor dialog
BOOL CALLBACK EditDlgFunc(HWND hwnd, UINT message,WPARAM wP, LPARAM lP)
{
	static AEffect * effect;
	static mi * pmi;
	char str[256];
	char str2[256];
	long i;
	float value;
	HWND hwnd2;
	long previousProg;

#define NUM_TICKS 500

	switch(message)
	{
	case WM_ROUT:
		if(effect) //if window is already open
			return 0;

		effect=(AEffect*)wP;
		pmi=(mi*)lP;
		return 1;
	case WM_ROUT_BUZZ_CHANGED_PROGRAM:
		hwnd2=GetDlgItem(pmi->editorhwnd,IDC_PROGRAMS);
		i=effect->dispatcher(effect,effGetProgram,0,0,NULL,0.f);
		SendMessage(hwnd2,LB_SETCURSEL,i,0);
		hwnd=pmi->editorhwnd;
		goto updateparams;
		return 1;
	case WM_INITDIALOG:
		pmi->editorOpen=true;
		pmi->editorhwnd=hwnd;
		//---PARAMS-----
		//insert all parameters relative to this plugin
		for(i=0;i<effect->numParams;i++)
		{
			effect->dispatcher(effect,effGetParamName,i,0,str,0);
			effect->dispatcher(effect,effGetParamLabel,i,0,str2,0);
			strcat(str,"  (");
			strcat(str,str2);
			strcat(str,")");

			SendDlgItemMessage(hwnd,IDC_PARAMS,LB_ADDSTRING,0,(long)str);
		}
		SendDlgItemMessage(hwnd,IDC_PARAMS,LB_SETCURSEL,0,0);
		
		//init slider range
		SendDlgItemMessage(hwnd,IDC_SLIDER,TBM_SETRANGE,true,MAKELONG(0,NUM_TICKS));
		
		//initial text
		effect->dispatcher(effect,effGetParamDisplay,0,0,str,0.f);
		hwnd2=GetDlgItem(hwnd,IDC_TEXT);
		SetWindowText(hwnd2,str);

		//update scroll bar with initial value
		value=effect->getParameter(effect,0);
		SendDlgItemMessage(hwnd,IDC_SLIDER,TBM_SETPOS,true,(int)(value*NUM_TICKS));

		//--PROGRAMS--
		previousProg=effect->dispatcher(effect,effGetProgram,0,0,NULL,0);

		for(i=0;i<effect->numPrograms;i++)
		{
			char string[256];
			sprintf(string,"%d  ",i+1);
			effect->dispatcher(effect,effSetProgram,0,i,NULL,0);
			effect->dispatcher(effect,effGetProgramName,0,0,str,0);
			strcat(string,str);
			SendDlgItemMessage(hwnd,IDC_PROGRAMS,LB_ADDSTRING,0,(long)string);
		}
		effect->dispatcher(effect,effSetProgram,0,previousProg,NULL,0);
		SendDlgItemMessage(hwnd,IDC_PROGRAMS,LB_SETCURSEL,previousProg,0);

		//obtain plugin name
		char str[256];
		pmi->getPlugname(str);

		//set window title
		SetWindowText(hwnd,str);

		break;

	case WM_COMMAND:
		switch(LOWORD(wP))
		{
		case IDC_PARAMS:
			switch(HIWORD(wP))
			{
			case LBN_SELCHANGE:
				//get parameter value
				i=SendDlgItemMessage(hwnd,IDC_PARAMS,LB_GETCURSEL,0,0);
				value=effect->getParameter(effect,i);
				
				//update scroll bar with new value
				SendDlgItemMessage(hwnd,IDC_SLIDER,TBM_SETPOS,true,(int)(value*NUM_TICKS));
				
				//update text
				effect->dispatcher(effect,effGetParamDisplay,i,0,str,value);
				hwnd2=GetDlgItem(hwnd,IDC_TEXT);
				SetWindowText(hwnd2,str);
				return 1;
			}
			break;
		case IDC_PROGRAMS:
			switch(HIWORD(wP))
			{
			case LBN_SELCHANGE:
updateparams:
				//get parameter value
				i= SendDlgItemMessage(hwnd,IDC_PROGRAMS,LB_GETCURSEL,0,0);
				effect->dispatcher(effect,effSetProgram,0,i,NULL,0.f);
				
				//find selected parameter and update it with the new program
				i=SendDlgItemMessage(hwnd,IDC_PARAMS,LB_GETCURSEL,0,0);
				value=effect->getParameter(effect,i);
				
				//update scroll bar with new value
				SendDlgItemMessage(hwnd,IDC_SLIDER,TBM_SETPOS,true,(int)(value*NUM_TICKS));
				
				//update text too
				effect->dispatcher(effect,effGetParamDisplay,i,0,str,value);
				hwnd2=GetDlgItem(hwnd,IDC_TEXT);
				SetWindowText(hwnd2,str);

				

				return 1;
			}
			break;
		case IDC_RENAME:
				hwnd2=GetDlgItem(hwnd,IDC_NEWNAME);
				GetWindowText(hwnd2,str,sizeof(str));
				if(strlen(str)>24)
				{
					MessageBox(NULL,"Program name can't have more than 24 characters",NULL,MB_OK);
					break;
				}
				
				long cursel=SendDlgItemMessage(hwnd,IDC_PROGRAMS,LB_GETCURSEL,0,0);
				effect->dispatcher(effect,effSetProgramName,0,0,str,0.f);
				
				sprintf(str2,"%d  ",cursel+1);
				strcat(str2,str);
				SendDlgItemMessage(hwnd,IDC_PROGRAMS,LB_DELETESTRING,cursel,0);
				SendDlgItemMessage(hwnd,IDC_PROGRAMS,LB_INSERTSTRING,cursel,(long)str2);
				SendDlgItemMessage(hwnd,IDC_PROGRAMS,LB_SETCURSEL,cursel,0);
			break;
		}
		break;

	case WM_VSCROLL:
		//modify parameter
		hwnd2=GetDlgItem(hwnd,IDC_SLIDER);
		if((HWND)lP==hwnd2)
		{
			value=(float)SendDlgItemMessage(hwnd,IDC_SLIDER,TBM_GETPOS,0,0)/NUM_TICKS;
			i=SendDlgItemMessage(hwnd,IDC_PARAMS,LB_GETCURSEL,0,HIWORD(wP));
			effect->setParameter(effect,i,value);

			//update text
			effect->dispatcher(effect,effGetParamDisplay,i,0,str,value);
			hwnd2=GetDlgItem(hwnd,IDC_TEXT);
			SetWindowText(hwnd2,str);
		}
		break;
	case WM_CLOSE:
		pmi->editorOpen=false;
		effect=NULL;
		pmi=NULL;
		EndDialog(hwnd,1);
		break;
	}
	return 0;
}

BOOL CALLBACK RenameProgProc(HWND hwnd, UINT message,WPARAM wP, LPARAM lP)
{
	static AEffect * effect;
	static mi * pmi;
	HWND hwnd2;
	char str[256];

	switch(message)
	{
	case WM_ROUT:
		if(effect) //if window is already open
			return 0;
		effect=(AEffect*)wP;
		pmi=(mi*)lP;
		return 1;
	case WM_INITDIALOG:
		effect->dispatcher(effect,effGetProgramName,0,0,str,0.f);

		hwnd2=GetDlgItem(hwnd,IDC_NEWNAME);
		SetWindowText(hwnd2,str);
		SetFocus(hwnd2);
		SendDlgItemMessage(hwnd,IDC_NEWNAME,EM_SETSEL,0,4);
		return 1;
		break;
	case WM_COMMAND:
		switch(LOWORD(wP))
		{
		case IDOK:
			hwnd2=GetDlgItem(hwnd,IDC_NEWNAME);
			GetWindowText(hwnd2,str,sizeof(str));
			if(strlen(str)>24)
			{
				MessageBox(NULL,"Program name can't have more than 24 characters",NULL,MB_OK);
				return 1;
			}
			else
				effect->dispatcher(effect,effSetProgramName,0,0,str,0.f);
			effect=NULL;
			pmi=NULL;
			EndDialog(hwnd,1);
			break;
		case IDCANCEL:
			effect=NULL;
			pmi=NULL;
			EndDialog(hwnd,1);
			break;
		}
		break;

	}
	return 0;
}

BOOL CALLBACK AboutFunc(HWND hwnd, UINT message,WPARAM wP, LPARAM lP)
{
	static AEffect * effect;
	static mi * pmi;
	HWND hwnd2;

	switch(message)
	{
	case WM_ROUT:
		if(effect) //if window is already open
			return 0;
		effect=(AEffect*)wP;
		pmi=(mi*)lP;
		return 1;
	case WM_INITDIALOG:
		//obtain plugin name
		char str[256];
		pmi->getPlugname(str);

		hwnd2=GetDlgItem(hwnd,IDC_PLUGNAME);
		SetWindowText(hwnd2,str);

		hwnd2=GetDlgItem(hwnd,IDC_PLUGINFOLDER);
		SetWindowText(hwnd2,pmi->directoryString);
		break;
	case WM_COMMAND:
		switch(LOWORD(wP))
		{
		case IDC_CHANGEFOLDER:
			pmi->SelectNewPluginFolder();
			
			//update with new value
			hwnd2=GetDlgItem(hwnd,IDC_PLUGINFOLDER);
			SetWindowText(hwnd2,pmi->directoryString);
		}
		break;
	case WM_CLOSE:
		effect=NULL;
		pmi=NULL;
		EndDialog(hwnd,1);
		break;
	}
	return 0;
}