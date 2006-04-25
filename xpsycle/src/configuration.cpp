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
#include <napp.h>
#include <nconfig.h>
#include <nfile.h>


Configuration::Configuration()
{
  setSkinDefaults();

  defaultPatLines = 64;

}


Configuration::~Configuration()
{
}

void Configuration::setSkinDefaults( )
{
  autoStopMachines = false;
  _centerCursor = false;

  _linenumbers       = true;
  _linenumbersHex    = false;
  _linenumbersCursor = true;
  pv_timesig = 4;

  pattern_font_x = 9;
  pattern_font_y = 11;

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

  _numOutputDrivers = 4;
  _ppOutputDrivers = new AudioDriver*[_numOutputDrivers];
  _ppOutputDrivers[0] = new AlsaOut();
  _outputDriverIndex = 0;
  _pOutputDriver = _ppOutputDrivers[_outputDriverIndex];

  iconPath   = "";
  pluginPath = "~/xpsycle/plugins/";

  mv_wirewidth = 1;
  mv_triangle_size = 22;
}

void Configuration::loadConfig( )
{
  NApp::config()->tagParse.connect(this,&Configuration::onConfigTagParse);
  std::string oldDir = NFile::workingDir();
  NFile::cdHome();
  NApp::config()->loadXmlConfig(".xpsycle.xml");
  NFile::cd(oldDir);

  iconPath   = NApp::config()->findPath("icondir");
  pluginPath = NApp::config()->findPath("plugindir");
}

void Configuration::onConfigTagParse(const std::string & tagName )
{
  if (tagName == "key") {
     std::string id         = NApp::config()->getAttribValue("id");
     std::string ctrlStr    = NApp::config()->getAttribValue("ctrl");
     int ctrl = str<int>(ctrlStr);
     int keyCode = 0;
     std::string keyCodeStr = NApp::config()->getAttribValue("keycode");
     if (keyCodeStr!="") keyCode = str<int>(keyCodeStr);
       std::string keyCharStr = NApp::config()->getAttribValue("keychar");
     if (keyCharStr!="")
       keyCode = keyCharStr[0];
     if (id == "add_new_machine") {

     } else
     if (id == "block_copy") {
       
     } else
     if (id == "block_cut") {
       
     } else
     if (id == "block_delete") {
       
     } else
     if (id == "block_double") {
       
     } else
     if (id == "block_end") {
       
     } else
     if (id == "block_halve") {
       
     } else
     if (id == "block_interpolate") {
       
     } else
     if (id == "block_mix") {
       
     } else
     if (id == "block_paste") {
       
     } else
     if (id == "block_select_all") {
       
     } else
     if (id == "block_select_bar") {
       
     } else
     if (id == "block_select_column") {
       
     } else
     if (id == "block_set_instrument") {
       
     } else
     if (id == "block_set_machine") {
       
     } else
     if (id == "block_start") {
       
     } else
     if (id == "block_unmark") {
       
     } else
     if (id == "clear_row") {
       
     } else
     if (id == "current_instrument+1") {
       
     } else
     if (id == "current_instrument-1") {
       
     } else
     if (id == "current_machine+1") {
       
     } else
     if (id == "current_machine-1") {
       
     } else
     if (id == "current_octave+1") {

     } else
     if (id == "current_octave-1") {
       
     } else
     if (id == "current_pattern+1") {
       
     } else
     if (id == "current_pattern-1") {
       
     } else
     if (id == "delete_row") {
       
     } else
     if (id == "edit_instrument") {
       
     } else
     if (id == "edit_redo") {
       
     } else
     if (id == "edit_undo") {
       
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

     }if (id == "machine_info") {
       
     } else
     if (id == "maximise_pattern_view") {
       
     } else
     if (id == "mcm_midi_cc") {
       
     } else
     if (id == "nav_bottom") {
       
     } else
     if (id == "nav_down") {
       
     } else
     if (id == "nav_down_16") {
       
     } else
     if (id == "nav_left") {
       
     } else
     if (id == "nav_right") {
       
     } else
     if (id == "nav_top") {
       
     } else
     if (id == "nav_up") {
       
     } else
     if (id == "nav_up_16") {
       
     } else
     if (id == "next_column") {
       
     } else
     if (id == "pattern_copy") {
       
     } else
     if (id == "pattern_copy") {
       
     } else
     if (id == "pattern_delete") {
       
     } else
     if (id == "pattern_info") {
       
     } else
     if (id == "pattern_mix_paste") {
       
     } else
     if (id == "pattern_paste") {
       
     } else
     if (id == "pattern_track_mute") {
       
     } else
     if (id == "pattern_track_record") {
       
     } else
     if (id == "pattern_track_solo") {
       
     } else
     if (id == "play_current_note") {
       
     } else
     if (id == "play_current_row") {
       
     } else
     if (id == "play_sel_pattern_looped") {
       
     } else
     if (id == "play_song_current") {
       
     } else
     if (id == "play_song_start") {
       
     } else
     if (id == "play_song_normal") {
       
     } else
     if (id == "position+1") {
       
     } else
     if (id == "position-1") {
       
     } else
     if (id == "prev_column") {
       
     } else
     if (id == "row_skip+1") {
       
     } else
     if (id == "row_skip-1") {
       
     } else
     if (id == "screen_machines") {
       
     } else
     if (id == "screen_patterns") {
       
     } else
     if (id == "select_mac_in_cursor") {
       
     } else
     if (id == "show_error_log") {
       
     } else
     if (id == "stop_playback") {
       
     } else
     if (id == "toggle_edit_mode") {
       
     } else
     if (id == "toggle_follow_song") {
       
     } else
     if (id == "transpose_block+1") {
       
     } else
     if (id == "transpose_block+12") {
       
     } else
     if (id == "transpose_block-1") {
       
     } else
     if (id == "transpose_block-1") {
       
     } else
     if (id == "transpose_block-12") {
       
     } else
     if (id == "transpose_channel+1") {
       
     } else
     if (id == "transpose_channel+12") {
       
     } else
     if (id == "transpose_channel-1") {
       
     } else
     if (id == "transpose_channel-12") {
       
     } else
     if (id == "tweak_parameter") {
       
     } else
     if (id == "tweak_smooth_paramater") {
       
     }
  }
}


