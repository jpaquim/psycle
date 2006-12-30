/**************************************************************************
*   Copyright (C) 2006 by Stefan Nattkemper                               *
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
#include <ngrs/npage.h>
#include <ngrs/nscrollbox.h>
#include <ngrs/nlabel.h>
#include <ngrs/npopupmenu.h>

namespace psycle {
	namespace host {


		class Machine;
		class MachineGUI;
		class Song;

		/**
		@author Stefan Nattkemper
		*/

		class MachineView : public NPanel
		{
		public:
			
			MachineView( Song & song );

			~MachineView();

			void setSelectedMachine( Machine* mac);

			void createGUIMachines();
			void addMachine( Machine & mac );
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
			MachineViewColorInfo colorInfo_;

			MachineGUI* startGUI;
			MachineGUI* selectedMachine_;

			std::vector<MachineGUI*> machineGUIs;
			std::vector<WireGUI*> wireGUIs;

			WireGUI* line;
			WireGUI* selectedWire_;

			NPanel* scrollArea_;
			NScrollBox* scrollBox_;							

			void init();

			void onCreateMachine( Machine & mac );
			void onNewConnection( MachineGUI* sender );

			void onLineMoveStart( const NMoveEvent & event );
			void onLineMoveEnd( const NMoveEvent & event );

            void onWireGUIDelete( WireGUI * line );
			void onWireDelete(WireDlg* dlg);
			void onWireSelected( NButtonEvent* ev );
			void onMoveMachine(Machine* mac, int x, int y);

			MachineGUI* findByMachine(Machine* mac);
	
			void onTweakSlide(int machine, int command, int value);
			void onMachineSelected(MachineGUI* gui);

			void onMachineDeleteRequest( MachineGUI* machineGUI );
			void onUpdateMachinePropertiesSignal(Machine* machine);

			void onBendAdded( WireGUI* gui );
			void setSelectedWire( NObject* wire );
			void onViewMousePress( NButtonEvent* ev );

			void rewire( WireGUI* line, MachineGUI* src, MachineGUI* dst );
		};

	} // end of host namespace
} // end of psycle namespace
#endif
