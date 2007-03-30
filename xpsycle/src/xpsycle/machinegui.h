/***************************************************************************
*   Copyright (C) 2006 by  Stefan Nattkemper   *
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
#ifndef MACHINEGUI_H
#define MACHINEGUI_H

#include "skinreader.h"
#include "wiregui.h"
#include <ngrs/panel.h>
#include <ngrs/popupmenu.h>

namespace ngrs {
	class Slider;
}

namespace psycle {
	namespace host {

		class Machine;
		class FrameMachine;
		class MasterDlg;

		/**
		@author  Stefan Nattkemper
		*/

		class MachineGUI : public ngrs::Panel
		{
			class LineAttachment {
			public:

				LineAttachment( WireGUI* line, int pt ) 
					: point_( pt ), line_( line ) {
				}

				int point() const {
					return point_;
				}

				WireGUI* line() {
					return line_;
				}			

			private:

				int point_;
				WireGUI* line_;

			};

		public:
			MachineGUI( Machine & mac );

			virtual ~MachineGUI() = 0;

			sigslot::signal1<MachineGUI*> deleteRequest;

			Machine & mac();

			void attachLine( WireGUI* line, int point );
			void detachLine( WireGUI* line );

			sigslot::signal1<MachineGUI*> newConnection;
			sigslot::signal3<Machine*,int,int> moved;
			sigslot::signal3<int,int,int> patternTweakSlide;
			sigslot::signal1<MachineGUI*> selected;
			sigslot::signal1<MachineGUI*> cloneMachineSignal;

			int ident() const;

			virtual void onMouseDoublePress(int x, int y, int button);
			virtual void onMousePress( int x, int y, int button );
			virtual void onMoveStart( const ngrs::MoveEvent& moveEvent );
			virtual void onMove( const ngrs::MoveEvent& moveEvent );
			virtual void onMoveEnd( const ngrs::MoveEvent& moveEvent );
			virtual void resize();

			virtual void paint( ngrs::Graphics& g );

			virtual void repaintVUMeter();

			void setSelected( bool on );		

			void setCoordInfo( const MachineCoordInfo &  coords );
			const MachineCoordInfo & coords() const;

			virtual void updateSkin();
			void onUpdateMachinePropertiesSignal(Machine* machine);
			void onDeleteMachineSignal();
			void onPopupDeleteMachine( ngrs::ButtonEvent *event );
			void onPopupCloneMachine( ngrs::ButtonEvent *event );

			void muteMachine();
			void soloMachine();

			void deleteMachine();

		private:								

			bool selected_;
			ngrs::Region oldDrag;
			Machine* mac_;
			ngrs::PopupMenu *machActionsPopup_;
			MachineCoordInfo coords_;
			std::vector<LineAttachment> attachedLines;

			ngrs::Region linesRegion() const;
		};


		class MasterGUI : public MachineGUI
		{
		public:
			MasterGUI( Machine & mac );

			~MasterGUI();


			virtual void onMousePress(int x, int y, int button);
			virtual void onMouseDoublePress(int x, int y, int button);

			virtual void paint( ngrs::Graphics& g );

			virtual void updateSkin();

		private:

			MasterDlg* masterDlg;

			void setSkin();
		};


		class GeneratorGUI : public MachineGUI
		{
		public:

			class VUPanel : public ngrs::Panel {

				friend class GeneratorGUI;

			public:
				VUPanel(GeneratorGUI* pGui) {
					pGui_ = pGui;
				};

				virtual void paint( ngrs::Graphics& g );

			private:

				GeneratorGUI* pGui_;

			};

			GeneratorGUI( Machine & mac );

			~GeneratorGUI();

			FrameMachine* frameMachine;

			virtual void onMousePress( int x, int y, int button );

			virtual void repaintVUMeter();

			virtual void onMouseDoublePress( int x, int y, int button );

			virtual void paint( ngrs::Graphics& g );

			virtual void onKeyPress( const ngrs::KeyEvent & event);

			virtual void updateSkin();

		private:

			ngrs::Slider* panSlider_;
			VUPanel* vuPanel_;

			void setSkin();
			void customSliderPaint( ngrs::Slider* sl, ngrs::Graphics& g);		
			void onPosChanged( ngrs::Slider* sender );
			void onTweakSlide( int machine, int command, int value );
		};


		class EffektGUI : public MachineGUI
		{
		public:

			class VUPanel : public ngrs::Panel {

				friend class EffectGUI;

			public:
				VUPanel(EffektGUI* pGui) {
					pGui_ = pGui;
				};

				virtual void paint( ngrs::Graphics& g );

			private:

				EffektGUI* pGui_;

			};


			EffektGUI( Machine & mac );

			~EffektGUI();

			virtual void onMousePress(int x, int y, int button);
			virtual void onMouseDoublePress(int x, int y, int button);
			virtual void paint( ngrs::Graphics& g );
			virtual void repaintVUMeter();
			virtual void onKeyPress( const ngrs::KeyEvent & event );
			virtual void updateSkin();

		private:

			ngrs::Slider* panSlider_;
			VUPanel* vuPanel_;
			FrameMachine* frameMachine;

			void setSkin();
			void customSliderPaint( ngrs::Slider* sl, ngrs::Graphics& g);    
			void onPosChanged( ngrs::Slider* sender );
			void onTweakSlide( int machine, int command, int value );
		};

	}
}

#endif
