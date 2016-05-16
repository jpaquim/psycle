///\file
///\brief interface file for psycle::host::Global.
#pragma once
#include <psycle/host/detail/project.hpp>
#include "version.hpp"
#include "Constants.hpp"
#include "SongStructs.hpp"
#include <Shlwapi.h>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
//#include <universalis/os/winutf8wrapper.hpp>

// AVRT is the new "multimedia scheduling stuff"
typedef HANDLE (WINAPI *FAvSetMmThreadCharacteristics)   (LPCTSTR,LPDWORD);
typedef BOOL   (WINAPI *FAvRevertMmThreadCharacteristics)(HANDLE);

struct lua_State;

namespace psycle
{
	namespace host
	{
		class Configuration;
		class Song;
		class Player;
		class MachineLoader;
		namespace vst
		{
			class Host;
		}

		extern BOOL Is_Vista_or_Later();

		inline BOOL IsWin64() {
			SYSTEM_INFO si;
			ZeroMemory(&si, sizeof(SYSTEM_INFO));
			GetSystemInfo(&si);
			return (si.wProcessorArchitecture==PROCESSOR_ARCHITECTURE_AMD64);
		}
		inline BOOL IsWow64() {
			return IsOS(OS_WOW6432);
		}
    
    // inherit your class from Timer and override OnTimerViewRefresh to get
    // CChildview ViewRefreshTimer ticks
    class Timer { 
      public:
       Timer();
       virtual ~Timer();
       
       virtual void OnTimerViewRefresh() = 0;
       void StartTimer() { is_running_ = true; }
       void StopTimer() { is_running_ = false; }
       bool is_running() const { return is_running_; }

      private:
        bool is_running_;
    };
            
    typedef std::list<Timer*> TimerList;

    class GlobalTimer {
      friend class CChildView;
      friend Timer;
      public:
       static GlobalTimer& instance() {
         static GlobalTimer instance;
         return instance;
       }       
      private:       
       GlobalTimer() : removed_(false), it(listeners_.end()) { }
       ~GlobalTimer() { }                
       void OnViewRefresh();       
       void AddListener(Timer*);
       void RemoveListener(Timer*);
       void Clear() {
         listeners_.clear(); 
         it = listeners_.end();
         removed_ = true;
       }
       TimerList listeners_;
       TimerList::iterator it;
       bool removed_;
    };
    
    class Global
		{
			public:
				Global();
				virtual ~Global() throw();

				static FAvSetMmThreadCharacteristics    pAvSetMmThreadCharacteristics;
				static FAvRevertMmThreadCharacteristics pAvRevertMmThreadCharacteristics;
        
				static inline Song           & song() { return *pSong; }
				static inline Player         & player(){ return *pPlayer; }
				static inline Configuration  & configuration(){ return *pConfig; }
				static inline vst::Host		 & vsthost(){ return *pVstHost; }
				static inline MachineLoader  & machineload() { return *pMacLoad; }        

			protected:
				static Configuration * pConfig;
				static Song * pSong;
				static Player * pPlayer;
				static vst::Host * pVstHost;
				static MachineLoader * pMacLoad;
		};

 
	}
}
