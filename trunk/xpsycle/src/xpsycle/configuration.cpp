/***************************************************************************
  *   Copyright (C) 2006 by Stefan   *
  *   natti@linux   *
  *                                                                         *
  *   This program is free software; you can redistribute it and/or modify  *
  *   it under the terms of the GNU General Public License as published by  *
  *   the Free Software Foundation; either version 2 of the License, or     *
  *   (at your option) any later version.                                   *
  *                                                                         *
  *   This program is distributed in the hope that it will be useful,       *
  *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
  *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
  *   GNU General Public License for more details.                          *
  *                                                                         *
  *   You should have received a copy of the GNU General Public License     *
  *   along with this program; if not, write to the                         *
  *   Free Software Foundation, Inc.,                                       *
  *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
  ***************************************************************************/

#include "configuration.h"
#include "song.h"
#include "global.h"
#include "alsaout.h"
#include "jackout.h"
#include "gstreamerout.h"
#include "esoundout.h"
#include "netaudioout.h"
#include "defaultbitmaps.h"
#include <ngrs/napp.h>
#include <ngrs/nconfig.h>
#include <ngrs/nfile.h>
#include <sys/stat.h>
#include <cstdlib>
#include <stdexcept>
#include <sstream>

namespace psycle {
		namespace host {

Configuration::Configuration()
{
  #if !defined XPSYCLE__CONFIGURATION
  std::cout << "xpsycle: warning: built without configuration" << std::endl;
  #endif
  setSkinDefaults();
  defaultPatLines = 64;
  loadConfig();
  bitmaps_ = new DefaultBitmaps(this);
  _RecordTweaks = false;
  _RecordUnarmed = true;
}


Configuration::~Configuration()
{
  delete bitmaps_;
}

void Configuration::setSkinDefaults( )
{
  _centerCursor = false;
  enableSound = 0;
  device_name = "plughw:0";

  _linenumbers       = true;
  _linenumbersHex    = false;
  _linenumbersCursor = true;
  pv_timesig = 4;

  pattern_font_x = 9;
  pattern_font_y = 11;

  vu1.setHCOLORREF(0x0080FF80);
  vu2.setHCOLORREF(0x00403731);
  vu3.setHCOLORREF(0x00262bd7);
  pvc_separator.setHCOLORREF(0x00400000);
  pvc_separator2.setHCOLORREF(0x00800000);
  pvc_background.setHCOLORREF(0x0099766C);
  pvc_background2.setHCOLORREF(0x00AA786C);
  pvc_row4beat.setHCOLORREF(0x00AF958B);
  pvc_row4beat2.setHCOLORREF(0x00AF958B);
  pvc_rowbeat.setHCOLORREF(0x00C0ACA5);
  pvc_rowbeat2.setHCOLORREF(0x00C0ACA5);
  pvc_row.setHCOLORREF(0x00D0C4BF);
  pvc_row2.setHCOLORREF(0x00D0C4BF);
  pvc_font.setHCOLORREF(0x00000000);
  pvc_font2.setHCOLORREF(0x00000000);
  pvc_fontPlay.setHCOLORREF(0x00ffffff);
  pvc_fontPlay2.setHCOLORREF(0x00ffffff);
  pvc_fontCur.setHCOLORREF(0x00ffffff);
  pvc_fontCur2.setHCOLORREF(0x00ffffff);
  pvc_fontSel.setHCOLORREF(0x00ffffff);
  pvc_fontSel2.setHCOLORREF(0x00ffffff);
  pvc_selection.setHCOLORREF(0x00e00000);
  pvc_selection2.setHCOLORREF(0x00ff5050);
  pvc_playbar.setHCOLORREF(0x0080FF00);
  pvc_playbar2.setHCOLORREF(0x0080FF00);
  pvc_cursor.setHCOLORREF(0x000000e0);
  pvc_cursor2.setHCOLORREF(0x000000e0);

  // wire arrow
  mv_polycolour.setHCOLORREF(0x00ffffff);

  machineGUITopColor.setHCOLORREF(0x00D2C2BD);
  machineGUIFontTopColor.setHCOLORREF(0x00000000);
  machineGUIBottomColor.setHCOLORREF(0x009C796D);
  machineGUIFontBottomColor.setHCOLORREF(0x00FFFFFF);

  machineGUIHTopColor.setHCOLORREF(0x00BC94A9);//highlighted param colours
  machineGUIHFontTopColor.setHCOLORREF(0x00000000);
  machineGUIHBottomColor.setHCOLORREF(0x008B5A72);
  machineGUIHFontBottomColor.setHCOLORREF(0x0044EEFF);

  machineGUITitleColor.setHCOLORREF(0x00000000);
  machineGUITitleFontColor.setHCOLORREF(0x00FFFFFF);

	{	
		AudioDriver* driver = 0;
		driver = new AudioDriver;
		_pSilentDriver = driver;
		driverMap_[ driver->info().name() ] = driver;
		#if !defined XPSYCLE__NO_ALSA
			driver = new AlsaOut;
			std::cout << "registered:" <<  driver->info().name() << std::endl;
			driverMap_[ driver->info().name() ] = driver;
		#endif
		#if !defined XPSYCLE__NO_JACK
			driver = new JackOut;
			std::cout << "registered:" <<  driver->info().name() << std::endl;
			driverMap_[ driver->info().name() ] = driver;
		#endif
		#if !defined XPSYCLE__NO_GSTREAMER
			driver = new GStreamerOut;
			std::cout << "registered:" <<  driver->info().name() << std::endl;
			driverMap_[ driver->info().name() ] = driver;
		#endif
		#if !defined XPSYCLE__NO_ESOUND
			driver = new ESoundOut;
			std::cout << "registered:" <<  driver->info().name() << std::endl;
			driverMap_[ driver->info().name() ] = driver;
		#endif
		#if !defined XPSYCLE__NO_NETAUDIO
			driver = new NetAudioOut;
			std::cout << "registered:" <<  driver->info().name() << std::endl;
			driverMap_[ driver->info().name() ] = driver;
		#endif
	}

	setDriverByName("silent");
	enableSound = false;

  #if defined XPSYCLE__CONFIGURATION
	#include <xpsycle/install_paths.h>
	hlpPath = XPSYCLE__INSTALL_PATHS__DOC "/";
	iconPath = XPSYCLE__INSTALL_PATHS__PIXMAPS "/";
	pluginPath = XPSYCLE__INSTALL_PATHS__PLUGINS "/";
	prsPath = XPSYCLE__INSTALL_PATHS__PRESETS "/";
  #else
	// we don't have any information about the installation paths,
	// so, we can only assume everything is at a fixed place, like under the user home dir
	hlpPath = NFile::replaceTilde("~/xpsycle/doc/");
	iconPath = NFile::replaceTilde("~/xpsycle/pixmaps/");
	pluginPath = NFile::replaceTilde("~/xpsycle/plugins/");
	prsPath =  NFile::replaceTilde("~/xpsycle/prs/");
  #endif

  #if !defined NDEBUG
  std::cout
    << "xpsycle: configuration: initial defaults:\n"
    << "xpsycle: configuration: pixmap dir: " << iconPath << "\n"
    << "xpsycle: configuration: plugin dir: " << pluginPath << "\n"
    << "xpsycle: configuration: preset dir: " << prsPath << "\n"
    << "xpsycle: configuration: doc    dir: " << hlpPath << "\n";
  #endif

  mv_wirewidth = 1;
  mv_triangle_size = 22;
}

void Configuration::setDriverByName( const std::string & driverName )
{
	std::map< std::string, AudioDriver*>::iterator it = driverMap_.begin();

	if ( ( it = driverMap_.find( driverName ) ) != driverMap_.end() ) {
		// driver found
		_pOutputDriver = it->second;
		std::cout << "audio driver set as: " << _pOutputDriver->info().name() << std::endl;		
	}
	else {
		// driver not found,  set silent default driver
		_pOutputDriver = _pSilentDriver;
		std::cout << "audio driver set as: " << _pOutputDriver->info().name() << std::endl;		
	}
}


void Configuration::loadConfig()
{
  NApp::config()->tagParse.connect(this,&Configuration::onConfigTagParse);

  // system-wide

	// environment
  // this is most useful for developpers.
  // you can test xpsycle directly from within the build dir,
  // pointing various paths to the source or build dir.
  char const * const path(std::getenv("XPSYCLE__CONFIGURATION"));
  if(path) {
    try {
      loadConfig(path);
    }
    catch(std::exception const & e)
    {
      std::cerr << "xpsycle: configuration: error: " << e.what() << std::endl;
    }
  } else {


  #if !defined XPSYCLE__CONFIGURATION
  // we don't have any information about the installation paths
		// try user home
  	try {
      loadConfig(NFile::replaceTilde("~/.xpsycle.xml"));
  	}
  	catch(std::exception const & e) {
    	std::cerr << "xpsycle: configuration: error: " << e.what() << std::endl;
  	}
  #else
  #include <xpsycle/install_paths.h>
  try
  {
    loadConfig(XPSYCLE__INSTALL_PATHS__CONFIGURATION "/xpsycle.xml");
  }
  catch(std::exception const & e)
  {
    std::cerr << "xpsycle: configuration: error: " << e.what() << std::endl;
  }
  #endif

  }

  
}

void Configuration::loadConfig(std::string const & path) throw(std::exception)
{
  #if !defined NDEBUG
    std::cout << "xpsycle: configuration: attempting to load file: " << path << std::endl;
  #endif

  // check whether the file is readable
  if(!NFile::fileIsReadable(path))
  {
    std::ostringstream s;
    s << "cannot read file: " << path;
    throw std::runtime_error(s.str());
  }

  // parse the file
  try
  {
    NApp::config()->loadXmlConfig(path, /* throw_allowed */ true);
  }
  catch(std::exception const & e)
  {
    std::ostringstream s;
    s <<
        "an error occured while parsing xml configuration file " << path << " ; "
        "the settings might have been only partially loaded ; "
        "message: " << e.what();
    throw std::runtime_error(s.str());
    } catch(...)
    {
      std::ostringstream s;
      s <<
        "an unidentified error occured while parsing xml configuration file " << path << " ; "
        "the settings might have been only partially loaded";
      throw std::runtime_error(s.str());
    }
    // the parser defaults to empty strings on missing values
  // so we make sure not to override previous settings with empty strings
  {
    std::string const s(NFile::replaceTilde(NApp::config()->findPath("icondir")));
    if(s.length()) iconPath = s;
  }

  {
    std::string const s(NFile::replaceTilde(NApp::config()->findPath("plugindir")));
    if(s.length()) pluginPath = s;
  }

  {
    std::string const s(NFile::replaceTilde(NApp::config()->findPath("prsdir")));
    if(s.length()) prsPath = s;
  }

  {
    std::string const s(NFile::replaceTilde(NApp::config()->findPath("hlpdir")));
    if(s.length()) hlpPath = s;
  }

  #if !defined NDEBUG
  std::cout
    << "xpsycle: configuration: after loading file: " << path << "\n"
    << "xpsycle: configuration: pixmap dir: " << iconPath << "\n"
    << "xpsycle: configuration: plugin dir: " << pluginPath << "\n"
    << "xpsycle: configuration: preset dir: " << prsPath << "\n"
    << "xpsycle: configuration: doc    dir: " << hlpPath << "\n";
  #endif

	doEnableSound = true;
}

void Configuration::onConfigTagParse(const std::string & tagName )
{
	if (tagName == "driver" && doEnableSound) {		
			setDriverByName(NApp::config()->getAttribValue("name"));
	} else
  if (tagName == "alsa") {
    device_name = NApp::config()->getAttribValue("device");
  } else
  if (tagName == "audio") {
      std::string enableStr = NApp::config()->getAttribValue("enable");
      int enable = 0;
      if (enableStr != "") enable = str<int>(enableStr);
      enableSound = enable;
      if (enable == 0) {
				setDriverByName("silent");
				doEnableSound = false;
      } else doEnableSound = true;
  } else
  if (tagName == "key") {
      std::string id         = NApp::config()->getAttribValue("id");
      std::string ctrlStr    = NApp::config()->getAttribValue("ctrl");
      int ctrl = 0;
      if (ctrlStr!="")  ctrl = str<int>(ctrlStr);
      int keyCode = 0;
      std::string keyCodeStr = NApp::config()->getAttribValue("keycode");
      if (keyCodeStr!="") keyCode = str<int>(keyCodeStr);
        std::string keyCharStr = NApp::config()->getAttribValue("keychar");
      if (keyCharStr!="")
        keyCode = keyCharStr[0];
      if (id == "add_new_machine") {
        inputHandler.changeKeyCode(cdefAddMachine,Key(ctrl,keyCode));
      } else
      if (id == "block_copy") {
        std::cout << ctrl    << std::endl;
        std::cout << keyCode << std::endl;
        inputHandler.changeKeyCode(cdefBlockCopy,Key(ctrl,keyCode));
      } else
      if (id == "block_cut") {
        inputHandler.changeKeyCode(cdefBlockCut,Key(ctrl,keyCode));
      } else
      if (id == "block_delete") {
        inputHandler.changeKeyCode(cdefBlockDelete,Key(ctrl,keyCode));
      } else
      if (id == "block_double") {
        inputHandler.changeKeyCode(cdefBlockDouble,Key(ctrl,keyCode));
      } else
      if (id == "block_end") {
        inputHandler.changeKeyCode(cdefBlockEnd,Key(ctrl,keyCode));
      } else
      if (id == "block_halve") {
        inputHandler.changeKeyCode(cdefBlockHalve,Key(ctrl,keyCode));
      } else
      if (id == "block_interpolate") {
        inputHandler.changeKeyCode(cdefBlockInterpolate,Key(ctrl,keyCode));
      } else
      if (id == "block_mix") {
        inputHandler.changeKeyCode(cdefBlockMix,Key(ctrl,keyCode));
      } else
      if (id == "block_paste") {
        inputHandler.changeKeyCode(cdefBlockPaste,Key(ctrl,keyCode));
      } else
      if (id == "block_select_all") {
        inputHandler.changeKeyCode(cdefSelectAll,Key(ctrl,keyCode));
      } else
      if (id == "block_select_bar") {
        inputHandler.changeKeyCode(cdefSelectBar,Key(ctrl,keyCode));
      } else
      if (id == "block_select_column") {
        inputHandler.changeKeyCode(cdefSelectCol,Key(ctrl,keyCode));
      } else
      if (id == "block_set_instrument") {
        inputHandler.changeKeyCode(cdefBlockSetInstr,Key(ctrl,keyCode));
      } else
      if (id == "block_set_machine") {
        inputHandler.changeKeyCode(cdefBlockSetMachine,Key(ctrl,keyCode));
      } else
      if (id == "block_start") {
        inputHandler.changeKeyCode(cdefBlockStart,Key(ctrl,keyCode));
      } else
      if (id == "block_unmark") {
        inputHandler.changeKeyCode(cdefBlockUnMark,Key(ctrl,keyCode));
      } else
      if (id == "clear_row") {
        inputHandler.changeKeyCode(cdefRowClear,Key(ctrl,keyCode));
      } else
      if (id == "current_instrument+1") {
        inputHandler.changeKeyCode(cdefInstrInc,Key(ctrl,keyCode));
      } else
      if (id == "current_instrument-1") {
        inputHandler.changeKeyCode(cdefInstrDec,Key(ctrl,keyCode));
      } else
      if (id == "current_machine+1") {
        inputHandler.changeKeyCode(cdefMachineInc,Key(ctrl,keyCode));
      } else
      if (id == "current_machine-1") {
        inputHandler.changeKeyCode(cdefMachineDec,Key(ctrl,keyCode));
      } else
      if (id == "current_octave+1") {
        inputHandler.changeKeyCode(cdefOctaveUp,Key(ctrl,keyCode));
      } else
      if (id == "current_octave-1") {
        inputHandler.changeKeyCode(cdefOctaveDn,Key(ctrl,keyCode));
      } else
      if (id == "current_pattern+1") {
        inputHandler.changeKeyCode(cdefPatternInc,Key(ctrl,keyCode));
      } else
      if (id == "current_pattern-1") {
        inputHandler.changeKeyCode(cdefPatternDec,Key(ctrl,keyCode));
      } else
      if (id == "delete_row") {
        inputHandler.changeKeyCode(cdefRowDelete,Key(ctrl,keyCode));
      } else
      if (id == "edit_instrument") {
        inputHandler.changeKeyCode(cdefEditInstr,Key(ctrl,keyCode));
      } else
      if (id == "edit_redo") {
        inputHandler.changeKeyCode(cdefRedo,Key(ctrl,keyCode));
      } else
      if (id == "edit_undo") {
        inputHandler.changeKeyCode(cdefUndo,Key(ctrl,keyCode));
      } else
      if (id.find("oct")!=std::string::npos) {
        if (id == "oct_C_0") {
            inputHandler.changeKeyCode(cdefKeyC_0,Key(ctrl,keyCode));
        } else
        if (id == "oct_CS0") {
            inputHandler.changeKeyCode(cdefKeyCS0,Key(ctrl,keyCode));
        } else
        if (id == "oct_D_0") {
            inputHandler.changeKeyCode(cdefKeyD_0,Key(ctrl,keyCode));
        } else
        if (id == "oct_DS0") {
            inputHandler.changeKeyCode(cdefKeyDS0,Key(ctrl,keyCode));
        } else
        if (id == "oct_E_0") {
            inputHandler.changeKeyCode(cdefKeyE_0,Key(ctrl,keyCode));
        } else
        if (id == "oct_F_0") {
            inputHandler.changeKeyCode(cdefKeyF_0,Key(ctrl,keyCode));
        } else
        if (id == "oct_FS0") {
            inputHandler.changeKeyCode(cdefKeyFS0,Key(ctrl,keyCode));
        } else
        if (id == "oct_G_0") {
            inputHandler.changeKeyCode(cdefKeyG_0,Key(ctrl,keyCode));
        } else
        if (id == "oct_GS0") {
            inputHandler.changeKeyCode(cdefKeyGS0,Key(ctrl,keyCode));
        } else
        if (id == "oct_A_0") {
            inputHandler.changeKeyCode(cdefKeyA_0,Key(ctrl,keyCode));
        } else
        if (id == "oct_AS0") {
            inputHandler.changeKeyCode(cdefKeyAS0,Key(ctrl,keyCode));
        } else
        if (id == "oct_B_0") {
            inputHandler.changeKeyCode(cdefKeyB_0,Key(ctrl,keyCode));
        }  else // and now the all again for octave 2
        if (id == "oct_C_1") {
            inputHandler.changeKeyCode(cdefKeyC_1,Key(ctrl,keyCode));
        } else
        if (id == "oct_CS1") {
            inputHandler.changeKeyCode(cdefKeyCS1,Key(ctrl,keyCode));
        } else
        if (id == "oct_D_1") {
            inputHandler.changeKeyCode(cdefKeyD_1,Key(ctrl,keyCode));
        } else
        if (id == "oct_DS1") {
            inputHandler.changeKeyCode(cdefKeyDS1,Key(ctrl,keyCode));
        } else
        if (id == "oct_E_1") {
            inputHandler.changeKeyCode(cdefKeyE_1,Key(ctrl,keyCode));
        } else
        if (id == "oct_F_1") {
            inputHandler.changeKeyCode(cdefKeyF_1,Key(ctrl,keyCode));
        } else
        if (id == "oct_FS1") {
            inputHandler.changeKeyCode(cdefKeyFS1,Key(ctrl,keyCode));
        } else
        if (id == "oct_G_1") {
            inputHandler.changeKeyCode(cdefKeyG_1,Key(ctrl,keyCode));
        } else
        if (id == "oct_GS1") {
            inputHandler.changeKeyCode(cdefKeyGS1,Key(ctrl,keyCode));
        } else
        if (id == "oct_A_1") {
            inputHandler.changeKeyCode(cdefKeyA_1,Key(ctrl,keyCode));
        } else
        if (id == "oct_AS1") {
            inputHandler.changeKeyCode(cdefKeyAS1,Key(ctrl,keyCode));
        } else
        if (id == "oct_B_1") {
            inputHandler.changeKeyCode(cdefKeyB_1,Key(ctrl,keyCode));
        } else // and now again for octave 2
        if (id == "oct_C_2") {
            inputHandler.changeKeyCode(cdefKeyC_2,Key(ctrl,keyCode));
        } else
        if (id == "oct_CS2") {
            inputHandler.changeKeyCode(cdefKeyCS2,Key(ctrl,keyCode));
        } else
        if (id == "oct_D_2") {
            inputHandler.changeKeyCode(cdefKeyD_2,Key(ctrl,keyCode));
        } else
        if (id == "oct_DS2") {
            inputHandler.changeKeyCode(cdefKeyDS2,Key(ctrl,keyCode));
        } else
        if (id == "oct_E_2") {
            inputHandler.changeKeyCode(cdefKeyE_2,Key(ctrl,keyCode));
        } else
        if (id == "oct_F_2") {
            inputHandler.changeKeyCode(cdefKeyF_2,Key(ctrl,keyCode));
        } else
        if (id == "oct_FS2") {
            inputHandler.changeKeyCode(cdefKeyFS2,Key(ctrl,keyCode));
        } else
        if (id == "oct_G_2") {
            inputHandler.changeKeyCode(cdefKeyG_2,Key(ctrl,keyCode));
        } else
        if (id == "oct_GS2") {
            inputHandler.changeKeyCode(cdefKeyGS2,Key(ctrl,keyCode));
        } else
        if (id == "oct_A_2") {
            inputHandler.changeKeyCode(cdefKeyA_2,Key(ctrl,keyCode));
        }
      } else
      if (id == "key_stop") {
        inputHandler.changeKeyCode(cdefKeyStop,Key(ctrl,keyCode));
      } else
      if (id == "key_stop_current") {
        inputHandler.changeKeyCode(cdefKeyStopAny,Key(ctrl,keyCode)); // not sure or key_stop ?
      }if (id == "machine_info") {
        inputHandler.changeKeyCode(cdefInfoMachine,Key(ctrl,keyCode));
      } else
      if (id == "maximise_pattern_view") {
        inputHandler.changeKeyCode(cdefMaxPattern,Key(ctrl,keyCode));
      } else
      if (id == "mcm_midi_cc") {
        inputHandler.changeKeyCode(cdefMIDICC,Key(ctrl,keyCode));
      } else
      if (id == "nav_bottom") {
        inputHandler.changeKeyCode(cdefNavBottom,Key(ctrl,keyCode));
      } else
      if (id == "nav_down") {
        inputHandler.changeKeyCode(cdefNavDn,Key(ctrl,keyCode));
      } else
      if (id == "nav_down_16") {
        inputHandler.changeKeyCode(cdefNavPageDn,Key(ctrl,keyCode));
      } else
      if (id == "nav_left") {
        inputHandler.changeKeyCode(cdefNavLeft,Key(ctrl,keyCode));
      } else
      if (id == "nav_right") {
        inputHandler.changeKeyCode(cdefNavRight,Key(ctrl,keyCode));
      } else
      if (id == "nav_top") {
        inputHandler.changeKeyCode(cdefNavTop,Key(ctrl,keyCode));
      } else
      if (id == "nav_up") {
        inputHandler.changeKeyCode(cdefNavUp,Key(ctrl,keyCode));
      } else
      if (id == "nav_up_16") {
        inputHandler.changeKeyCode(cdefNavPageUp,Key(ctrl,keyCode));
      } else
      if (id == "next_column") {
        inputHandler.changeKeyCode(cdefColumnNext,Key(ctrl,keyCode));
      } else
      if (id == "pattern_copy") {
        inputHandler.changeKeyCode(cdefPatternCopy,Key(ctrl,keyCode));
      } else
      if (id == "pattern_cut") {
        inputHandler.changeKeyCode(cdefPatternCut,Key(ctrl,keyCode));
      } else
      if (id == "pattern_delete") {
        inputHandler.changeKeyCode(cdefPatternDelete,Key(ctrl,keyCode));
      } else
      if (id == "pattern_info") {
        inputHandler.changeKeyCode(cdefInfoPattern,Key(ctrl,keyCode));
      } else
      if (id == "pattern_mix_paste") {
        inputHandler.changeKeyCode(cdefPatternMixPaste,Key(ctrl,keyCode));
      } else
      if (id == "pattern_paste") {
        inputHandler.changeKeyCode(cdefPatternPaste,Key(ctrl,keyCode));
      } else
      if (id == "pattern_track_mute") {
        inputHandler.changeKeyCode(cdefPatternTrackMute,Key(ctrl,keyCode));
      } else
      if (id == "pattern_track_record") {
        inputHandler.changeKeyCode(cdefPatternTrackRecord,Key(ctrl,keyCode));
      } else
      if (id == "pattern_track_solo") {
        inputHandler.changeKeyCode(cdefPatternTrackSolo,Key(ctrl,keyCode));
      } else
      if (id == "play_current_note") {
        inputHandler.changeKeyCode(cdefPlayFromPos,Key(ctrl,keyCode));
      } else
      if (id == "play_current_row") {
        inputHandler.changeKeyCode(cdefPlayRowTrack,Key(ctrl,keyCode));
      } else
      if (id == "play_sel_pattern_looped") {
        inputHandler.changeKeyCode(cdefKeyStopAny,Key(ctrl,keyCode));
      } else
      if (id == "play_song_current") {
        inputHandler.changeKeyCode(cdefPlayFromPos,Key(ctrl,keyCode));
      } else
      if (id == "play_song_start") {
        inputHandler.changeKeyCode(cdefPlayStart,Key(ctrl,keyCode));
      } else
      if (id == "play_song_normal") {
        inputHandler.changeKeyCode(cdefPlaySong,Key(ctrl,keyCode));
      } else
      if (id == "position+1") {
        inputHandler.changeKeyCode(cdefSongPosInc,Key(ctrl,keyCode));
      } else
      if (id == "position-1") {
        inputHandler.changeKeyCode(cdefSongPosDec,Key(ctrl,keyCode));
      } else
      if (id == "prev_column") {
        inputHandler.changeKeyCode(cdefColumnPrev,Key(ctrl,keyCode));
      } else
      if (id == "row_skip+1") {
        //inputHandler.changeKeyCode(cdefKeyStopAny,Key(ctrl,keyCode));
      } else
      if (id == "row_skip-1") {
        //inputHandler.changeKeyCode(cdefKeyStopAny,Key(ctrl,keyCode));
      } else
      if (id == "screen_machines") {
        //inputHandler.changeKeyCode(cdefKeyStopAny,Key(ctrl,keyCode));
      } else
      if (id == "screen_patterns") {
        //inputHandler.changeKeyCode(cdefKeyStopAny,Key(ctrl,keyCode));
      } else
      if (id == "select_mac_in_cursor") {
        //inputHandler.changeKeyCode(cdefKeyStopAny,Key(ctrl,keyCode));
      } else
      if (id == "show_error_log") {
        //inputHandler.changeKeyCode(cdefKeyStopAny,Key(ctrl,keyCode));
      } else
      if (id == "stop_playback") {
        inputHandler.changeKeyCode(cdefPlayStop,Key(ctrl,keyCode));
      } else
      if (id == "toggle_edit_mode") {
        //inputHandler.changeKeyCode(cdefKeyStopAny,Key(ctrl,keyCode));
      } else
      if (id == "toggle_follow_song") {
        //inputHandler.changeKeyCode(cdefKeyStopAny,Key(ctrl,keyCode));
      } else
      if (id == "transpose_block+1") {
        inputHandler.changeKeyCode(cdefTransposeBlockInc,Key(ctrl,keyCode));
      } else
      if (id == "transpose_block+12") {
        inputHandler.changeKeyCode(cdefTransposeBlockInc12,Key(ctrl,keyCode));
      } else
      if (id == "transpose_block-1") {
        inputHandler.changeKeyCode(cdefTransposeBlockDec,Key(ctrl,keyCode));
      } else
      if (id == "transpose_block-12") {
        inputHandler.changeKeyCode(cdefTransposeBlockDec12,Key(ctrl,keyCode));
      } else
      if (id == "transpose_channel+1") {
        inputHandler.changeKeyCode(cdefTransposeChannelInc,Key(ctrl,keyCode));
      } else
      if (id == "transpose_channel+12") {
        inputHandler.changeKeyCode(cdefTransposeChannelInc12,Key(ctrl,keyCode));
      } else
      if (id == "transpose_channel-1") {
        inputHandler.changeKeyCode(cdefTransposeChannelDec,Key(ctrl,keyCode));
      } else
      if (id == "transpose_channel-12") {
        inputHandler.changeKeyCode(cdefTransposeChannelDec12,Key(ctrl,keyCode));
      } else
      if (id == "tweak_parameter") {
        inputHandler.changeKeyCode(cdefTweakM,Key(ctrl,keyCode));
      } else
      if (id == "tweak_smooth_paramater") {
        inputHandler.changeKeyCode(cdefTweakS,Key(ctrl,keyCode));
      }
  }
}

DefaultBitmaps & Configuration::icons( )
{
  return *bitmaps_;
}

}
}


