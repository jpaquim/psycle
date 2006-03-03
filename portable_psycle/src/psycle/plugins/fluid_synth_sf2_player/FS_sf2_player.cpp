/////////////////////////////////////////////////////////////////////
// Dmitry "Sartorius" Kulikov FluidSynth plugin for PSYCLE
// v0.2 beta
//
#include <project.private.hpp>
#include <psycle/plugin_interface.hpp>
//#include <fluidsynth/fluidsynth.h>
//#include <cstdlib>
//#include <cstring>
//#include <vector>
//#include <windows.h>
#include "FS_SF2_UI.hpp"


#define NUMPARAMETERS 2

CMachineParameter const paraNULL = 
{
	"knob1",
	"knob1",									// description
	-1,											// MinValue	
	1,											// MaxValue
	MPF_STATE,										// Flags
	0
};

CMachineParameter const paraNULL2 = 
{
	"knob2",
	"knob2",									// description
	-1,											// MinValue	
	1,											// MaxValue
	MPF_STATE,										// Flags
	0
};

CMachineParameter const *pParameters[] = 
{ 
	&paraNULL,
	&paraNULL2
};


CMachineInfo const MacInfo = 
{
	MI_VERSION,	
	GENERATOR,								// flags
	NUMPARAMETERS,							// numParameters
	pParameters,							// Pointer to parameters
#ifdef _DEBUG
	"FluidSynth (Debug build)",		// name
#else
	"FluidSynth",						// name
#endif
	"FluidSynth",							// short name
	"ported by Sartorius",							// author
	"Load",									// A command, that could be use for open an editor, etc...
	1
};

class mi : public CMachineInterface
{
public:
	mi();
	virtual ~mi();

	virtual void Init();
	virtual void SequencerTick();
	virtual void Work(float *psamplesleft, float* psamplesright, int numsamples,int tracks);
	virtual bool DescribeValue(char* txt,int const param, int const value);
	virtual void Command();
	virtual void ParameterTweak(int par, int val);
	virtual void SeqTick(int channel, int note, int ins, int cmd, int val);
	virtual void Stop();

private:

	//FS_SF2_UI	fs_ui;

};

PSYCLE__PLUGIN__INSTANCIATOR(mi, MacInfo)

mi::mi()
{
	Vals=new int[NUMPARAMETERS];
}

mi::~mi()
{
	// Destroy dinamically allocated objects/memory here
	delete Vals;
}

void mi::Init()
{
// Initialize your stuff here

}

void mi::Stop()
{

}

void mi::SequencerTick()
{
// Called on each tick while sequencer is playing
}

void mi::ParameterTweak(int par, int val)
{
	// Called when a parameter is changed by the host app / user gui
	Vals[par]=val;
}

// Work... where all is cooked 
void mi::Work(float *psamplesleft, float *psamplesright , int numsamples,int tracks)
{

}

// Function that describes value on client's displaying
bool mi::DescribeValue(char* txt,int const param, int const value)
{
	return false;
}

//////////////////////////////////////////////////////////////////////
// The SeqTick function where your notes and pattern command handlers
// should be processed. Called each tick.
// Is called by the host sequencer
	
void mi::SeqTick(int channel, int note, int ins, int cmd, int val)
{
	// Note Off			== 120
	// Empty Note Row	== 255
	// Less than note off value??? == NoteON!

}

HINSTANCE dllInstance;


BOOL WINAPI DllMain ( HANDLE hModule, DWORD fwdreason, LPVOID lpReserved )
{
   switch (fwdreason) {
   case DLL_PROCESS_ATTACH:
      dllInstance = (HINSTANCE) hModule;
      break;

   case DLL_THREAD_ATTACH: break;
   case DLL_THREAD_DETACH: break;
   case DLL_PROCESS_DETACH: break;
   }
   return TRUE;
}

BOOL APIENTRY AboutDialog(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
   switch(uMsg) {
   case WM_INITDIALOG: 
      return 1;

   case WM_SHOWWINDOW: 
      return 1;

   case WM_CLOSE:
      EndDialog (hDlg, TRUE);
      return 0;

   case WM_COMMAND:
      switch (LOWORD (wParam))
      {
      case IDOK:
         EndDialog(hDlg, TRUE);
         return 1;
      default:
         return 0;
      }
      break;
   }
   return 0;
}


void mi::Command()
{
// Called when user presses editor button
// Probably you want to show your custom window here
// or an about button

	DialogBox(dllInstance, MAKEINTRESOURCE (IDD_FS_SF2_DIALOG), GetForegroundWindow(), (DLGPROC) &AboutDialog);
}