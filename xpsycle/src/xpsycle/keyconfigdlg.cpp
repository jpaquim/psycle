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
#include "keyconfigdlg.h"
#include "configuration.h"
#include <ngrs/listbox.h>
#include <ngrs/item.h>
#include <ngrs/alignlayout.h>
#include <ngrs/tablelayout.h>
#include <ngrs/label.h>
#include <ngrs/objectinspector.h>
#include <ngrs/itemevent.h>
#include <ngrs/button.h>
#include <ngrs/frameborder.h>

namespace psy {
	namespace host	{	

		KeyConfigDlg::KeyConfigDlg( )
		  : ngrs::Window( )
		{
			setTitle("Key Bindings Settings");

			setSize( 440, 320 );
			setPositionToScreenCenter();

			init();
		}

		KeyConfigDlg::~KeyConfigDlg()
		{
		}

		void KeyConfigDlg::init( )
		{
			// creates the cancel and the ok button at the bottom of the window
            ngrs::Panel* btnPanel = new ngrs::Panel();
               btnPanel->setLayout( ngrs::AlignLayout(5,5) );
				okBtn_ = new ngrs::Button( "Ok" );
					okBtn_->setFlat(false);
					okBtn_->clicked.connect( this, &KeyConfigDlg::onOkBtn );
				applyBtn_ = new ngrs::Button( "Apply" );
					applyBtn_->setFlat(false);
					applyBtn_->clicked.connect( this, &KeyConfigDlg::onApplyBtn );
                cancelBtn_ = new ngrs::Button( "Cancel" );
					cancelBtn_->setFlat(false);
					cancelBtn_->clicked.connect( this, &KeyConfigDlg::onCancelBtn );
				btnPanel->add( cancelBtn_, ngrs::nAlRight );				
				btnPanel->add( applyBtn_, ngrs::nAlRight );
				btnPanel->add( okBtn_, ngrs::nAlRight );
                pane()->add( btnPanel, ngrs::nAlBottom );

			
            actionsBox_ = new ngrs::GroupBox("Actions");
				actionsBox_->setLayout( ngrs::AlignLayout() );
                actionsTabbook_ = new ngrs::TabBook();
                    macPage_ = new ngrs::Panel();
                    macPage_->setLayout( ngrs::AlignLayout() );
                    macList_ = new ngrs::ListBox();
                    macList_->add( new ngrs::Item("Do something             Ctrl+A") );
                    macList_->add( new ngrs::Item("Do something else        Ctrl+P") );
                    macList_->add( new ngrs::Item("etc.") );
                    macList_->add( new ngrs::Item("etc.") );
                    macPage_->add( macList_, ngrs::nAlClient );

                    patPage_ = new ngrs::Panel();
                    patPage_->setLayout( ngrs::AlignLayout() );
                    patList_ = new ngrs::ListBox();
                    patPage_->add( patList_, ngrs::nAlClient );

                    wavPage_ = new ngrs::Panel();
                    wavPage_->setLayout( ngrs::AlignLayout() );
                    wavList_ = new ngrs::ListBox();
                    wavPage_->add( wavList_, ngrs::nAlClient );

                    seqPage_ = new ngrs::Panel();
                    seqPage_->setLayout( ngrs::AlignLayout() );
                    seqList_ = new ngrs::ListBox();
                    seqPage_->add( seqList_, ngrs::nAlClient );

                    genPage_ = new ngrs::Panel();
                    genPage_->setLayout( ngrs::AlignLayout() );
                    genList_ = new ngrs::ListBox();
                    genPage_->add( genList_, ngrs::nAlClient );

                actionsTabbook_->addPage( macPage_, "Machines" );
                actionsTabbook_->addPage( patPage_, "Patterns" );
                actionsTabbook_->addPage( wavPage_, "Wave Ed" );
                actionsTabbook_->addPage( seqPage_, "Sequencer" );
                actionsTabbook_->addPage( genPage_, "General" );
                actionsTabbook_->setActivePage( macPage_ );


                bindPnl_ = new ngrs::Panel();
                    bindPnl_->setLayout( ngrs::AlignLayout() );
                    bindLbl_ = new	ngrs::Label("Key:");
                        bindLbl_->setWordWrap(false);
                    bindEdit_ = new ngrs::Edit();
                        bindEdit_->setPreferredSize( 150, 15 );
                    bindClearBtn_ = new ngrs::Button( "Clear binding" );
                        bindClearBtn_->setFlat(false);
                bindPnl_->add(bindLbl_, ngrs::nAlLeft);
                bindPnl_->add(bindEdit_, ngrs::nAlLeft);
                bindPnl_->add(bindClearBtn_, ngrs::nAlRight);

			actionsBox_->add(actionsTabbook_, ngrs::nAlTop);
			actionsBox_->add(bindPnl_, ngrs::nAlBottom);

            presetsBox_ = new ngrs::GroupBox("Keymap presets");
            presetsBox_->setLayout( ngrs::AlignLayout() );
                prsBtnPnl_ = new ngrs::Panel();
                prsBtnPnl_->setLayout( ngrs::AlignLayout(5,5) );
                prsDefaultsBtn_ = new ngrs::Button( "Reset to Defaults" );
                    prsDefaultsBtn_->setFlat(false);
                prsLoadBtn_ = new ngrs::Button( "Load keymap" );
                    prsLoadBtn_->setFlat(false);
                prsSaveBtn_ = new ngrs::Button( "Save keymap" );
                    prsSaveBtn_->setFlat(false);
                prsBtnPnl_->add( prsDefaultsBtn_, ngrs::nAlLeft );
                prsBtnPnl_->add( prsLoadBtn_, ngrs::nAlLeft );
                prsBtnPnl_->add( prsSaveBtn_, ngrs::nAlLeft );				
            presetsBox_->add( prsBtnPnl_, ngrs::nAlClient );

			pane()->add(actionsBox_, ngrs::nAlTop);
			pane()->add(presetsBox_, ngrs::nAlTop);
		}

		void KeyConfigDlg::onOkBtn( ngrs::ButtonEvent* ev ) {
                        
		}

		void KeyConfigDlg::onApplyBtn( ngrs::ButtonEvent* ev ) {
                        
		}

        void KeyConfigDlg::onCancelBtn( ngrs::ButtonEvent* ev ) {
			onClose();
		}


		void KeyConfigDlg::setVisible( bool on ) {
			Window::setVisible(on);
		}

		int KeyConfigDlg::onClose( )
		{
			setVisible(false);
            return ngrs::nHideWindow;
		}

	}
}



