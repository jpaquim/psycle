///\file
///\brief implementation file for psycle::host::Global.
#include <psycle/host/detail/project.private.hpp>
#include "Global.hpp"
#include "LuaPlugin.hpp"
#include "machineloader.hpp"

#define _GetProc(fun, type, name) \
{                                                  \
    fun = (type) GetProcAddress(hDInputDLL,name);  \
    if (fun == NULL) { return false; }             \
}


namespace portaudio
{
	static HMODULE hDInputDLL = 0;

	//Dynamic load and unload of avrt.dll, so the executable can run on Windows 2K and XP.
	static bool SetupAVRT()
	{
		using namespace psycle::host;
		hDInputDLL = LoadLibraryA("avrt.dll");
		if (hDInputDLL == NULL)
			return false;

		_GetProc(Global::pAvSetMmThreadCharacteristics,   FAvSetMmThreadCharacteristics,   "AvSetMmThreadCharacteristicsA");
		_GetProc(Global::pAvRevertMmThreadCharacteristics,FAvRevertMmThreadCharacteristics,"AvRevertMmThreadCharacteristics");
	    
		return Global::pAvSetMmThreadCharacteristics && 
			Global::pAvRevertMmThreadCharacteristics;
	}

	// ------------------------------------------------------------------------------------------
	static void CloseAVRT()
	{
		if (hDInputDLL != NULL)
			FreeLibrary(hDInputDLL);
		hDInputDLL = NULL;
	}
}

namespace psycle
{
	namespace host
	{
		Configuration * Global::pConfig(0);
		Song * Global::pSong(0);
		Player * Global::pPlayer(0);
		vst::Host * Global::pVstHost(0);
		MachineLoader * Global::pMacLoad(0);    

		FAvSetMmThreadCharacteristics    Global::pAvSetMmThreadCharacteristics(NULL);
		FAvRevertMmThreadCharacteristics Global::pAvRevertMmThreadCharacteristics(NULL);

		BOOL Is_Vista_or_Later() 
		{
		   OSVERSIONINFOEX osvi;
		   DWORDLONG dwlConditionMask = 0;
		   int op=VER_GREATER_EQUAL;

		   ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
		   osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
		   osvi.dwMajorVersion = 6;
		   osvi.dwMinorVersion = 0;

		   VER_SET_CONDITION( dwlConditionMask, VER_MAJORVERSION, op );
		   VER_SET_CONDITION( dwlConditionMask, VER_MINORVERSION, op );

		   // Perform the test.

		   return VerifyVersionInfo(
			  &osvi, VER_MAJORVERSION | VER_MINORVERSION,
			  dwlConditionMask);
		}
    
    


    Timer::Timer() { GlobalTimer::instance().AddListener(this); }
    Timer::~Timer() { GlobalTimer::instance().RemoveListener(this); }

    void GlobalTimer::AddListener(Timer* listener) {                
      listeners_.push_back(listener);
    }
    
    void GlobalTimer::RemoveListener(Timer* listener) {  
      if (!listeners_.empty()) {
        TimerList::iterator i = std::find(listeners_.begin(), listeners_.end(), listener);
        if (i != listeners_.end()) {
          if (i!=it) {
            listeners_.erase(i);
          } else {
            listeners_.erase(it++);
            removed_ = true;
          }          
        }
      }
    }
    
    void GlobalTimer::OnViewRefresh() {          
      it = listeners_.begin();
      while (it != listeners_.end()) {
        (*it)->OnTimerViewRefresh();
        if (!removed_) {          
          ++it;
        } else {
          removed_ = false;
        }
      }
    }       

		Global::Global()
		{
			if (Is_Vista_or_Later()) {
				portaudio::SetupAVRT();
			}
		}

		Global::~Global()
		{
			portaudio::CloseAVRT();
		}


    
  }    
}
