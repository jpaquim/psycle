/**************************************************************************
*   Copyright (C) 2006 by  Stefan Nattkemper                               *
*   natti@linux                                                           *
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
#include "skinreader.h"
#include <ngrs/scrollbox.h>
#include <ngrs/label.h>
#include <ngrs/popupmenu.h>

namespace psy {
  namespace core {
    class Song;
    class Machine;
  }

	namespace host {

		class MachineGUI;		

		/**
		@author  Stefan Nattkemper
		*/

		class MachineView : public ngrs::Panel
		{
		public:
			
          MachineView( psy::core::Song& song );

			~MachineView();

			void setSelectedMachine( psy::core::Machine* mac);

			void createGUIMachines();
			void addMachine( psy::core::Machine& mac );
			void removeMachines();

			void update();
			void updateVUs();

			Panel* scrollArea();

			sigslot::signal1<psy::core::Machine*> selected;
			sigslot::signal3<psy::core::Machine*, int, int> machineMoved;
			sigslot::signal3<int,int,int> patternTweakSlide;
			sigslot::signal1<int> machineDeleted;
			sigslot::signal1<int> machineNameChanged;

			psy::core::Machine* selMachine();

			void updateSkin();
			void setColorInfo( const MachineViewColorInfo & info );
			const MachineViewColorInfo & colorInfo() const;


		private:

			psy::core::Song* _pSong;
			MachineViewColorInfo colorInfo_;

			MachineGUI* startGUI;
			MachineGUI* selectedMachine_;

			std::vector<MachineGUI*> machineGUIs;
			std::vector<WireGUI*> wireGUIs;

			WireGUI* line;
			WireGUI* selectedWire_;

			ngrs::Panel* scrollArea_;
            ngrs::ScrollBox* scrollBox_;							

			void init();

			void onCreateMachine( psy::core::Machine& mac );
			void onNewConnection( MachineGUI* sender );

			void onLineMoveStart( const ngrs::MoveEvent & event );
			void onLineMoveEnd( const ngrs::MoveEvent & event );

            void onWireGUIDelete( WireGUI * line );
			void onWireDelete( WireDlg* dlg );
			void onWireSelected( ngrs::ButtonEvent* ev );
			void onMoveMachine( psy::core::Machine* mac, int x, int y );

			MachineGUI* findByMachine( psy::core::Machine* mac );
	
			void onTweakSlide( int machine, int command, int value );
			void onMachineSelected( MachineGUI* gui );

			void onMachineDeleteRequest( MachineGUI* machineGUI );
			void onUpdateMachinePropertiesSignal( psy::core::Machine* machine );

			void onBendAdded( WireGUI* gui );
			void setSelectedWire( ngrs::Object* wire );
			void onViewMousePress( ngrs::ButtonEvent* ev );

			void rewire( WireGUI* line, MachineGUI* src, MachineGUI* dst );
		};

	} // end of host namespace
} // end of psycle namespace
#endif
