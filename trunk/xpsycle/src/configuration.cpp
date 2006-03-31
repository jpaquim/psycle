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
#include "napp.h"
#include "nconfig.h"


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
  NApp::config()->loadXmlConfig("~/.xpsycle.xml");
  iconPath   = NApp::config()->findPath("icondir");
  pluginPath = NApp::config()->findPath("plugindir");
}


