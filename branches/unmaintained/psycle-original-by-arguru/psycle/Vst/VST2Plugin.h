#pragma once

#ifndef __VST20pluginloader
#define __VST20pluginloader

#include <math.h>
#include "dsplib.h"
#include <windows.h>
#include "aeffectx.h"
#include "dialog.h"
#include <malloc.h>
#include <stdlib.h>
#include "resource.h"
#include <commctrl.h> 
#include "AEffEditor.hpp"
#include <shlobj.h>
#include "dialog.h"
#include "machineinterface.h"

#define MAX_TRACKS 20

//typedef AEffect *( __cdecl * mainCall)(audioMasterCallback cb);

char dllName[]="Zephod VST2.dll";
char RegKeyName[]="Software\\Zephod\\VST2 Plugin Loader";

#define WM_ROUT (WM_USER+'R')
#define WM_ROUT_BUZZ_CHANGED_PROGRAM (WM_ROUT+1)

CMachineParameter const paraVolume =
{
	pt_byte,    
	"Volume",   
	"Volume",   
	0,                    
	128,                  
	255,
	MPF_STATE,            
	100                      
};

CMachineParameter const paraChannel =
{
	pt_byte,    
	"Channel",   
	"Channel",   
	0,                    
	15,                  
	255,                    
	MPF_STATE,            
	0                      
};
CMachineParameter const paraEffect=
{
	pt_byte,    
	"Effect",   
	"Effect",   
	0,                    
	254,                  
	255,                    
	MPF_STATE,            
	0                      
};
CMachineParameter const paraEffectvalue=
{
	pt_word,    
	"Effectvalue",   
	"Effectvalue",   
	0,                    
	1000,                  
	1001,                    
	MPF_STATE,            
	0                      
};


CMachineParameter const paraNote = 
{ 
	pt_note,               
	"Note",                
	"Note",                
	NOTE_MIN,              
	NOTE_OFF,              
	NOTE_NO,               
	0,                     
	0                      
};
CMachineParameter const *pParameters[] =
{

	&paraNote,
	&paraVolume,
	&paraChannel,
	&paraEffect,
	&paraEffectvalue

};

#pragma pack(1)

class gvals  
{
	public:
};

class tvals  
{
	public:
		byte note;
		byte volume;
		byte channel;
		byte effect;
		word effectvalue;
};

#pragma pack()

CMachineInfo const MacInfo = 
{
	MT_GENERATOR,   
	MI_VERSION,     
	MIF_MONO_TO_STEREO,
	1,              
	MAX_TRACKS,     
	0, //global params
	5, //track params             
	pParameters,    
	0,              
	NULL,           
	"Zephod VST2 plugin loader",
	"VST2plug",       
	"Zephod",       
	"Load plugin\n"
	"Edit parameters\n"                            
	"About/options.. "
};

class mi;

class CTrack
{
	public:
        void Tick(tvals const &tv);
        void Stop();
        void Init();

		mi *pmi; 

		
        int Note;
		int lastmidinote;
		int volume;
		bool noteactive;
		byte channel;
};

class mi : public CMachineInterface
{
	public:
        mi();
        virtual ~mi();
        virtual void Init(CMachineDataInput * const pi);
		virtual void Save(CMachineDataOutput * const po);
        virtual void Tick();
		virtual bool WorkMonoToStereo(float *psamples, float *, int numsamples, int const mode);
		virtual void SetNumTracks(int const n) { numTracks = n; }
        virtual void Stop();
		virtual void MidiNote(int const channel, int const value, int const velocity);
		virtual char const *DescribeValue(int const param, int const value);
		virtual void Command(int const i);
		static long Master(AEffect *effect, long opcode, long index, long value, void *ptr, float opt);

		void SelectNewPluginFolder(void);
		void getPlugname(char * str);
	
		void SendVSTnoteevent(int note,int volume,byte channel);
		void Newnote(int note,int volume,byte channel);
		void Endnote(int note,byte channel);
		void loadVSTplugin(void);
        tvals tval[MAX_TRACKS];
        gvals gval;

		float * outputs[2];
		float * inputs[2];
        
		int numTracks;
		bool pluginloaded;
        CTrack Tracks[MAX_TRACKS];

		HINSTANCE plugin;
		char plugname[MAX_PATH];
		char directoryString[MAX_PATH];
	
		AEffect * effect;
		bool editorOpen;
		HWND editorhwnd;

		HINSTANCE hInst;
		VstEvents *events;
		static void newpaint(void);
		mi *thissie;
};

#endif 