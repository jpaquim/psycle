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
#include "skinreader.h"

#include <ngrs/napp.h>
#include <ngrs/npage.h>
#include <ngrs/nscrollbox.h>
#include <ngrs/nlabel.h>

namespace psycle { namespace host {


class Machine;
class MachineGUI;
class Song;

/**
@author Stefan
*/

enum WireState
{
        WIRESTATE_WIRED = 0,
        WIRESTATE_NEWCONNECTION = 1,
        WIRESTATE_REWIRING_DST = 2,
        WIRESTATE_REWIRING_SRC = 3
};
typedef WireState wire_state;

class MachineWireGUI : public WireGUI {
public :
    MachineWireGUI() {
      dlg = new WireDlg();
      dlg->setLine(this);
      wireState_ = WIRESTATE_NEWCONNECTION; 
    }
    ~MachineWireGUI() {
        std::cout << "delete dialog" << std::endl;
        delete dlg;
        std::cout << "after delete dialog" << std::endl;
    }

    virtual void onMousePress  (int x, int y, int button) {
      if(button==3) {
        int modifier = NApp::system().shiftState();
        rewireBegin.emit(this,modifier);
      }
    }
    virtual void onMouseDoublePress (int x, int y, int button) {
      if(button==1)
        dlg->setVisible(true);
    }

    virtual void onMoveEnd (const NMoveEvent & ev ) {
        wireMoveEnd.emit(this, ev); // override onMoveEnd because we want to send 
                                    // the wire in the signal too.
    }
   
    WireDlg* dialog() { return dlg;}
    void setWireState(wire_state newState) { wireState_ = newState; };
    wire_state wireState() { return wireState_; };

    signal2<MachineWireGUI*, int> rewireBegin;
    signal2<MachineWireGUI*, const NMoveEvent &> wireMoveEnd;

private:

  WireDlg* dlg;
  wire_state wireState_; 

};



class MachineView : public NPanel
{
public:
    MachineView( Song* song );

    ~MachineView();

    void setSelectedMachine( Machine* mac);

    void createGUIMachines();
    void addMachine(Machine* mac);
    void removeMachines();

    void update();
    void updateVUs();

    NPanel* scrollArea();

    signal1<Machine*> selected;
    signal3<Machine*, int, int> machineMoved;
    signal3<int,int,int> patternTweakSlide;
    signal1<int> machineDeleted;
    signal1<int> machineNameChanged;

    Machine* selMachine();

		void updateSkin();

		void setColorInfo( const MachineViewColorInfo & info );
		const MachineViewColorInfo & colorInfo() const;

private:

    Song* _pSong;
    MachineGUI* selectedMachine_;

    MachineWireGUI* line;
    MachineWireGUI* rewireLine;
    MachineGUI* startGUI;


    void init();
    NPanel* scrollArea_;
    NScrollBox* scrollBox_;

    void onCreateMachine(Machine* mac);
    void onDestroyMachine(Machine* mac);

    void onNewConnection(MachineGUI* sender);
    void onLineRewireBeginSignal(MachineWireGUI* line, int rewireType);
    void onLineMoveEnd(MachineWireGUI*, const NMoveEvent & event);

    void onWireDelete(WireDlg* dlg);

    void onMoveMachine(Machine* mac, int x, int y);

    MachineGUI* findByMachine(Machine* mac);

    std::vector<MachineGUI*> machineGUIs;
    std::vector<MachineWireGUI*> wireGUIs;

    void onTweakSlide(int machine, int command, int value);

    void onMachineSelected(MachineGUI* gui);

		void onMachineDeleteRequest( MachineGUI* machineGUI );
		void onUpdateMachinePropertiesSignal(Machine* machine);

		MachineViewColorInfo colorInfo_;

		std::vector<WireDlg*> wireDlg;

};

}}
#endif
