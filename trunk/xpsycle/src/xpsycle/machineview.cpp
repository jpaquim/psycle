/***************************************************************************
*   Copyright (C) 2006 by  Stefan Nattkemper  *
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
#include "machineview.h"
#include "machinegui.h"
#include "wiredlg.h"
#include <psycore/machine.h>
#include <psycore/song.h>
#include <ngrs/autoscrolllayout.h>
#include <ngrs/alignlayout.h>
#include <ngrs/app.h>
#include <algorithm>

namespace psy { 
	namespace host 
	{

		MachineView::MachineView( psy::core::Song & song ) 
			: ngrs::Panel(), _pSong( &song )
		{
			setLayout( ngrs::AlignLayout() );

			scrollBox_ = new ngrs::ScrollBox();
			scrollArea_ = new ngrs::Panel();
			scrollArea_->setLayout( ngrs::AutoScrollLayout() );
			scrollArea_->setClientSizePolicy( ngrs::nVertical | ngrs::nHorizontal );
			scrollArea_->mousePress.connect( this, &MachineView::onViewMousePress );
			scrollBox_->setScrollPane( scrollArea_ );
			add( scrollBox_, ngrs::nAlClient );

			init();
		}

		MachineView::~MachineView()
		{
		}

		void MachineView::init( )
		{
			scrollArea_->setTransparent( false );
			selectedMachine_ = 0;
			selectedWire_ = 0;
			updateSkin();
		}


		void MachineView::onCreateMachine( psy::core::Machine& mac )
		{
		}

		void MachineView::addMachine( psy::core::Machine& mac )
		{
			onCreateMachine( mac );
		}

		void MachineView::createGUIMachines( )
		{
		}

		void MachineView::update( )
		{
			wireGUIs.clear();
			machineGUIs.clear();
			scrollArea_->removeChilds();
			createGUIMachines();
			scrollArea_->resize(); // update scroll client area
			repaint();
		}

		MachineGUI * MachineView::findByMachine( psy::core::Machine * mac )
		{
			for (std::vector<MachineGUI*>::iterator it = machineGUIs.begin() ; it < machineGUIs.end(); it++) {
				MachineGUI* machineGUI = *it;
				if ( &machineGUI->mac() == mac ) return machineGUI;
			}
			return 0;
		}

		void MachineView::onNewConnection( MachineGUI * sender )
		{
			startGUI = sender;

			int midW = sender->clientWidth()  / 2;
			int midH = sender->clientHeight() / 2;

			line = new WireGUI();
			line->setPoints(ngrs::Point(sender->left()+midW,sender->top()+midH),ngrs::Point(sender->left()+midW,sender->top()+midH));
			scrollArea_->insert( line,0 );
			line->setMoveable(ngrs::Moveable(ngrs::nMvVertical | ngrs::nMvHorizontal | ngrs::nMvPolygonPicker));
			repaint();
			line->setMoveFocus(0);
			line->moveEnd.connect(this,&MachineView::onLineMoveEnd);
			line->moveStart.connect(this,&MachineView::onLineMoveStart);  
		} 

		void MachineView::onLineMoveStart( const ngrs::MoveEvent& ev ) {
			// pickindex = 0 : rewire
			if ( ev.picker() == 0 || ev.picker() == 1 ) {

			}
		}

		void MachineView::onLineMoveEnd( const ngrs::MoveEvent& ev )
		{	
			if ( !line ) return;

			// search for a machine gui, the mouse is over.
			MachineGUI* connectToMachineGUI = 0;
			for (std::vector<MachineGUI*>::iterator it = machineGUIs.begin() ; it < machineGUIs.end(); it++) {
				MachineGUI* machineGUI = *it;
				if (machineGUI->clipBox().intersects(line->left()+ev.x(),line->top()+ev.y())) {
					connectToMachineGUI = machineGUI;
					break;
				}
			}

			std::vector<WireGUI*>::iterator it2 = find( wireGUIs.begin(), wireGUIs.end(), ev.sender() );  
			if (  it2 != wireGUIs.end() && connectToMachineGUI ) {
				// found an old line and rewire it here
				WireGUI* oldLine = *it2;
				if ( ev.picker() == 0 ) {
					MachineGUI* oldSrcGUI = findByMachine( oldLine->dialog()->pSrcMachine() );
					rewire( oldLine, oldSrcGUI, connectToMachineGUI );
				} else
					if ( ev.picker() == 1 ) {
						MachineGUI* oldDstGUI = findByMachine( oldLine->dialog()->pDstMachine() );
						rewire( oldLine, connectToMachineGUI, oldDstGUI );
					} 
			} else  
				if ( connectToMachineGUI ) {
					// a new line has been added.
//					_pSong->InsertConnection( startGUI->mac()._macIndex , connectToMachineGUI->mac()._macIndex, 1.0f );
					startGUI->attachLine( line, 0 );
					connectToMachineGUI->attachLine(line,1);
					line->dialog()->setMachines( &startGUI->mac(), &connectToMachineGUI->mac() );
					line->dialog()->deleteMe.connect( this, &MachineView::onWireDelete );	
					line->mousePress.connect( this, &MachineView::onWireSelected );
					line->bendAdded.connect( this, &MachineView::onBendAdded );
					line->removeMe.connect( this, &MachineView::onWireGUIDelete );
					wireGUIs.push_back( line );
				} else 
					if ( it2 == wireGUIs.end() ) {
						scrollArea_->removeChild( line );
						line = 0;    
					} else {
						// nth has change, but reset the line points to state before drag
						findByMachine( line->dialog()->pSrcMachine() )->attachLine( line, 0);
						findByMachine( line->dialog()->pDstMachine() )->attachLine( line, 1);
					}
					repaint();
		}

		void MachineView::rewire( WireGUI* line, MachineGUI* src, MachineGUI* dst ) {
			if ( line ) {
				// mapped engine machines
				psy::core::Machine* macSrc = line->dialog()->pSrcMachine();
				psy::core::Machine* macDst = line->dialog()->pDstMachine();
				// find old attachments to Machine GUI`s
				MachineGUI* oldSrcGUI = findByMachine( macSrc );
				MachineGUI* oldDstGUI = findByMachine( macDst );
				// detach the GUI wire
				oldDstGUI->detachLine( line );
				oldSrcGUI->detachLine( line ); 

				// detach the Engine wire	

				// attach GUI wire
				src->attachLine( line, 0 );
				dst->attachLine( line, 1 );
				// insert in Engine

				// reset wire dialog to new machines
				line->dialog()->setMachines( &src->mac(), &dst->mac() );  
			}
		}


        void MachineView::onWireGUIDelete( WireGUI * line ) {
          onWireDelete( line->dialog() );
        }

		void MachineView::onWireDelete( WireDlg * dlg )
		{
			MachineGUI* from = this->findByMachine(dlg->pSrcMachine());
			MachineGUI* to   = this->findByMachine(dlg->pDstMachine());

			from->detachLine(dlg->line());
			to->detachLine(dlg->line());

			std::vector<WireGUI*>::iterator it = wireGUIs.begin();
			it = find( wireGUIs.begin(), wireGUIs.end(), dlg->line() );
			if ( it != wireGUIs.end() ) {
				wireGUIs.erase(it);	
				if ( selectedWire_ == *it ) selectedWire_ = 0;
			}  
			scrollArea_->removeChild ( dlg->line() );

			line = 0;
			repaint();
		}

		void MachineView::removeMachines( )
		{
			wireGUIs.clear();
			scrollArea_->removeChilds();
		}

		void MachineView::onBendAdded( WireGUI* wire ) {
			setSelectedWire( wire );
		}

		void MachineView::setSelectedWire( ngrs::Object * wire ) {

			// unselect old wire

			if ( selectedWire_ ) {
				//     selectedWire_->setMoveable( ngrs::Moveable() );
				selectedWire_->repaint();
				selectedWire_ = 0;              
			}

			// try to find wire

			std::vector<WireGUI*>::iterator it = wireGUIs.begin();
			it = find( wireGUIs.begin(), wireGUIs.end(), wire );
			if ( it != wireGUIs.end() ) {
				// wire found
				selectedWire_ = *it;
				selectedWire_->setMoveable( ngrs::Moveable( ngrs::nMvPolygonPicker ) );
				selectedWire_->repaint();
			}

		}

		ngrs::Panel * MachineView::scrollArea( )
		{
			return scrollArea_;
		}

		void MachineView::updateVUs( )
		{
			for (std::vector<MachineGUI*>::iterator it = machineGUIs.begin() ; it < machineGUIs.end(); it++) {
				MachineGUI* machineGUI = *it;
				machineGUI->repaintVUMeter();
			}
		}

		void MachineView::onMoveMachine( psy::core::Machine * mac, int x, int y )
		{
			machineMoved.emit(mac,x,y);
		}

		psy::core::Machine * MachineView::selMachine( )
		{
			if ( selectedMachine_ )
				return &selectedMachine_->mac();
			else
				return 0;
		}

		void MachineView::onTweakSlide( int machine, int command, int value )
		{
			patternTweakSlide.emit(machine,command,value);
		}

		void MachineView::onMachineSelected( MachineGUI * gui )
		{
			if (selectedMachine_ && gui != selectedMachine_) {
				selectedMachine_->setSelected(false);
				selectedMachine_->repaint();
			}
			if (gui != selectedMachine_) {
				selectedMachine_ = gui;
				gui->setSelected(true);
				gui->repaint();
				selected.emit( &gui->mac() ); 
			} 
		}

		void MachineView::setSelectedMachine( psy::core::Machine* mac )
		{
			for (std::vector<MachineGUI*>::iterator it = machineGUIs.begin(); it < machineGUIs.end(); it++ ) {
				MachineGUI* machineGUI = *it;
				if ( &machineGUI->mac() == mac ) {
					onMachineSelected( machineGUI );
					break;
				}
			}
		}

		void MachineView::onMachineDeleteRequest( MachineGUI * machineGUI )
		{
			selectedMachine_ = 0;
		}

		void MachineView::updateSkin( )
		{
			setColorInfo( SkinReader::Instance()->machineview_color_info() );

			for (std::vector<MachineGUI*>::iterator it = machineGUIs.begin() ; it < machineGUIs.end(); it++) {
				MachineGUI* machineGUI = *it;
				machineGUI->updateSkin();
			}

			repaint();
		}

		void MachineView::setColorInfo( const MachineViewColorInfo & info ) {
			colorInfo_ = info;

			scrollArea_->setBackground ( colorInfo_.pane_bg_color );
		}

		const MachineViewColorInfo & MachineView::colorInfo() const {
			return colorInfo_;
		}

		void MachineView::onWireSelected( ngrs::ButtonEvent* ev ) {
			setSelectedWire( ev->sender() );
			repaint();
		}

		void MachineView::onViewMousePress( ngrs::ButtonEvent* ev ) {
			setSelectedWire( 0 );
		}


	}
}
