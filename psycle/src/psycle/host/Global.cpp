///\file
///\brief implementation file for psycle::host::Global.
#include "Global.hpp"

#include "Configuration.hpp"
#include "MainFrm.hpp"

#include <psycle/core/song.h>
#include <psycle/core/player.h>

#if !defined WINAMP_PLUGIN
	#include "InputHandler.hpp"
#endif //!defined WINAMP_PLUGIN

#include <psycle/helpers/dsp.hpp>

#if !defined NDEBUG
   #define new DEBUG_NEW
   #undef THIS_FILE
   static char THIS_FILE[] = __FILE__;
#endif

namespace psycle { namespace host {

Player * Global::pPlayer(0);
helpers::dsp::Resampler * Global::pResampler(0);
Configuration * Global::pConfig(0);
cpu::cycles_type Global::_cpuHz(cpu::cycles_per_second());

#if !defined WINAMP_PLUGIN
	InputHandler * Global::pInputHandler(0);
#endif //!defined WINAMP_PLUGIN

Global::Global() {
	_cpuHz = cpu::cycles_per_second();
	pConfig = new Configuration();
	pResampler = new helpers::dsp::Cubic();
	pResampler->SetQuality(helpers::dsp::R_LINEAR);
	#if !defined WINAMP_PLUGIN
	pInputHandler = &InputHandler::instance();
	#endif //!defined WINAMP_PLUGIN
	pPlayer = &Player::singleton();	
}

Global::~Global() {
	delete pConfig; pConfig = 0;
	delete pResampler; pResampler = 0;
}

extern CPsycleApp theApp;


Song& Global::song() {
	return ((CMainFrame*) theApp.m_pMainWnd)->projects()->active_project()->song();
}

}}
