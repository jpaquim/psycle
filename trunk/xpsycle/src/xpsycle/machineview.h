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
#ifndef MACHINEVIEW_H
#define MACHINEVIEW_H

#include "wiregui.h"
#include "wiredlg.h"
#include <ngrs/npage.h>
#include <ngrs/nscrollbox.h>
#include <ngrs/nlabel.h>

namespace psycle { namespace host {


class Machine;
class MachineGUI;

/**
@author Stefan
*/


class MachineWireGUI : public WireGUI {
public :
    MachineWireGUI() {
      dlg = new WireDlg();
      dlg->setLine(this);
      add(dlg);
    }
    ~MachineWireGUI() {
    }

    virtual void onMousePress  (int x, int y, int button) {
      if(button==3)
        dlg->setVisible(true);
    }
    virtual void onMouseDoublePress (int x, int y, int button) {
      if(button==1)
        dlg->setVisible(true);
    }

    WireDlg* dialog() { return dlg;}

private:

  WireDlg* dlg;

};



class MachineView : public NPanel
{
public:
    MachineView();

    ~MachineView();

    void createGUIMachines();
    void addMachine(Machine* mac);
    void removeMachines();

    void update();
    void updateVUs();

    NPanel* scrollArea();

    signal3<Machine*, int, int> machineMoved;
    signal3<int,int,int> patternTweakSlide;

    Machine* selMachine();

private:

    MachineGUI* selectedMachine_;

    MachineWireGUI* line;
    MachineGUI* startGUI;


    void init();
    NPanel* scrollArea_;
    NScrollBox* scrollBox_;

    void onCreateMachine(Machine* mac);
    void onDestroyMachine(Machine* mac);

    void onNewConnection(MachineGUI* sender);
    void onLineMousePressed(NButtonEvent* sender);

    void onWireDelete(WireDlg* dlg);

    void onMoveMachine(Machine* mac, int x, int y);

    MachineGUI* findByMachine(Machine* mac);

    std::vector<MachineGUI*> machineGUIs;
    void onTweakSlide(int machine, int command, int value);

    void onMachineSelected(MachineGUI* gui);
};

}}
#endif
