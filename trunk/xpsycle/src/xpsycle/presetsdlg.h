/***************************************************************************
 *   Copyright (C) 2006 by Stefan Nattkemper   *
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
#ifndef PRESETSDLG_H
#define PRESETSDLG_H

#include "plugin.h"
#include <ngrs/ndialog.h>

class NListBox;
class Machine;
class DeSerializer;
class FrameMachine;

/**
@author Stefan Nattkemper
*/

class Preset {
  public:

     Preset();

     Preset(int numpars, int dataSize);

     void loadFromFile(DeSerializer* f);

     const std::string & name() const;

     void tweakMachine(Machine* mac);

  private:

    std::string name_;

    int numpars_;
    int dataSize_;

    std::vector<int> params_;
    std::vector<byte>  data_;

};


class PresetsDlg : public NDialog
{
public:
    PresetsDlg(FrameMachine* mac);

    ~PresetsDlg();

private:

   NListBox* lBox;
   FrameMachine* fMac;

   void loadPresets();
   void addPreset(const std::string & name);

   std::map<std::string,Preset> presetMap;

   void onUseClicked(NButtonEvent* ev);

};

#endif
