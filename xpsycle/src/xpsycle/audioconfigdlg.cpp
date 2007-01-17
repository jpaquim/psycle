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
#include "audioconfigdlg.h"
#include "configuration.h"
#include "audiodriver.h"
#include "player.h"
#include <ngrs/listbox.h>
#include <ngrs/item.h>
#include <ngrs/alignlayout.h>
#include <ngrs/tablelayout.h>
#include <ngrs/label.h>
#include <ngrs/objectinspector.h>
#include <ngrs/itemevent.h>
#include <ngrs/button.h>
#include <ngrs/frameborder.h>

namespace psycle {
	namespace host	{	

		AudioConfigDlg::AudioConfigDlg( Configuration* cfg )
		  : ngrs::Window( ),
				config_(cfg)
		{
			setTitle("Audio settings");

			setSize( 500, 450 );
			setPositionToScreenCenter();

			init();
		}

		AudioConfigDlg::~AudioConfigDlg()
		{
		}

		void AudioConfigDlg::init( )
		{
			// a ptr to the driver in the configuration, that actually is displayed and editable
			selectedDriver_ = 0;

			// creates the cancel and the ok button at the bottom of the window
            ngrs::Panel* btnPanel = new ngrs::Panel();
               btnPanel->setLayout( ngrs::AlignLayout(5,5) );
				/*okBtn_ = new Button( " Ok " );
					okBtn_->setFlat(false);
					okBtn_->clicked.connect( this, &AudioConfigDlg::onOkBtn );
				btnPanel->add( okBtn_, ngrs::nAlRight );*/
                closeBtn_ = new ngrs::Button( "Close" );
					closeBtn_->setFlat(false);
					closeBtn_->clicked.connect( this, &AudioConfigDlg::onCloseBtn );
				btnPanel->add( closeBtn_, ngrs::nAlRight );				
                pane()->add( btnPanel, ngrs::nAlBottom );

			// creates a TabBook with an audio system and MIDI tab
			tabBook_ = new ngrs::TabBook();
            audioPage_ = new ngrs::Panel();
            audioPage_->setLayout( ngrs::AlignLayout( 5, 5 ) );
            midiPage_  = new ngrs::Panel();
            midiPage_->setLayout( ngrs::AlignLayout() );
				tabBook_->addPage( audioPage_, "Audio System" );
				tabBook_->addPage( midiPage_, "Midi System" );
			pane()->add(tabBook_, ngrs::nAlClient);
			
			initAudioDriverBox();

			tabBook_->setActivePage( audioPage_ );
		}

		void AudioConfigDlg::initAudioDriverBox( )
		{
          driverBox_ = new ngrs::GroupBox("Audio Driver");
				driverBox_->setLayout( ngrs::AlignLayout() );
			audioPage_->add( driverBox_, ngrs::nAlClient );

            ngrs::Panel* infoPanel = new ngrs::Panel();
				infoPanel->setPreferredSize(200,100);
				infoPanel->setBorder( ngrs::FrameBorder( 1, 5, 5 ) );
				infoPanel->setSpacing(10,10,10,10);
				infoPanel->setLayout( ngrs::AlignLayout() );
                audioHeaderLbl_ = new	ngrs::Label( );
					audioHeaderLbl_->setWordWrap(true);
				infoPanel->add( audioHeaderLbl_, ngrs::nAlTop );
				audioDescriptionLbl_ = new ngrs::Label( );
					audioDescriptionLbl_->setWordWrap(true);
				infoPanel->add( audioDescriptionLbl_, ngrs::nAlTop );
			driverBox_->add( infoPanel, ngrs::nAlRight );

			ngrs::Panel* driverSelectPanel = new ngrs::Panel();
				driverSelectPanel->setLayout( ngrs::AlignLayout( 5, 5 ) );
				driverCbx_ = new ngrs::ComboBox();
				driverCbx_->setWidth(100);
				driverCbx_->itemSelected.connect( this, &AudioConfigDlg::onDriverSelected );
				driverSelectPanel->add ( driverCbx_, ngrs::nAlLeft );			
				restartBtn_ = new ngrs::Button("Restart Driver");
					restartBtn_->clicked.connect( this, &AudioConfigDlg::onRestartDriver );
					restartBtn_->setFlat( false );
				driverSelectPanel->add( restartBtn_, ngrs::nAlLeft );
			driverBox_->add( driverSelectPanel, ngrs::nAlTop );

			noteBook_ = new ngrs::NoteBook();
			driverBox_->add( noteBook_, ngrs::nAlClient );

			initJackPage();
			initEsdPage();
			initGeneralPage();

			initDriverList();      
		}

		void AudioConfigDlg::initJackPage( )
		{
			jackPage_ = new ngrs::Panel();
				jackPage_->setLayout( ngrs::AlignLayout() );
				ngrs::Label* label = new ngrs::Label("No Settings here");
				jackPage_->add( label, ngrs::nAlTop );
			noteBook_->add( jackPage_ );			
		}

		void AudioConfigDlg::initEsdPage( )
		{
			esdPage_ = new ngrs::Panel();
				esdPage_->setLayout( ngrs::AlignLayout() );
				ngrs::Label* label = new ngrs::Label("No Settings here");
				esdPage_->add( label, ngrs::nAlTop );
			noteBook_->add( esdPage_ );
		}

		void AudioConfigDlg::initGeneralPage( )
		{
			generalPage_ = new ngrs::Panel();
				generalPage_->setLayout( ngrs::AlignLayout() );
				ngrs::Panel* table = new ngrs::Panel();
					ngrs::TableLayout tableLayout(2,3);
					tableLayout.setVGap(5);
					tableLayout.setHGap(5);
					table->setLayout( tableLayout );
			
					table->add(new ngrs::Label("Device") , ngrs::AlignConstraint( ngrs::nAlLeft, 0, 0 ),true);
					table->add(new ngrs::Label("Buffer size"), ngrs::AlignConstraint( ngrs::nAlLeft, 0, 1),true);
					table->add(new ngrs::Label("Sample rate"), ngrs::AlignConstraint( ngrs::nAlLeft, 0, 2),true);
					table->add(new ngrs::Label("Bit depth"), ngrs::AlignConstraint( ngrs::nAlLeft, 0, 3),true);
					table->add(new ngrs::Label("Channel mode"), ngrs::AlignConstraint( ngrs::nAlLeft, 0, 4),true);

					deviceEdt_ = new ngrs::Edit();
						deviceEdt_->setPreferredSize( 150, 15 );
					sampleRateCbx_ = new ngrs::ComboBox();
						sampleRateCbx_->itemSelected.connect( this, &AudioConfigDlg::onSampleRateCbx );
						sampleRateCbx_->add( new ngrs::Item("44100"));
					bitDepthCbx_ = new ngrs::ComboBox();
						bitDepthCbx_->itemSelected.connect( this, &AudioConfigDlg::onDepthCbx );
						bitDepthCbx_->add( new ngrs::Item("16") );
					channelModeCbx_ = new ngrs::ComboBox();
						channelModeCbx_->add( new ngrs::Item("mono" ) );
						channelModeCbx_->add( new ngrs::Item("stereo") );
						channelModeCbx_->itemSelected.connect( this, &AudioConfigDlg::onChannelCbx );

					table->add( deviceEdt_, ngrs::AlignConstraint( ngrs::nAlLeft, 1, 0 ), true);
					table->add( sampleRateCbx_, ngrs::AlignConstraint( ngrs::nAlLeft, 1, 2), true);
					table->add( bitDepthCbx_, ngrs::AlignConstraint( ngrs::nAlLeft, 1, 3), true);
					table->add( channelModeCbx_, ngrs::AlignConstraint( ngrs::nAlLeft,1,4), true);

				generalPage_->add( table, ngrs::nAlClient );
			noteBook_->add( generalPage_ );
		}

		void AudioConfigDlg::initDriverList( )
		{
			std::map<std::string, AudioDriver*> & driverMap =  config_->driverMap();
			std::map<std::string, AudioDriver*>::iterator it = driverMap.begin();
			for ( ; it != driverMap.end(); it++ ) {
				std::string driverName = it->first;
				driverCbx_->add( new ngrs::Item( driverName ) );
			}
		}

		void AudioConfigDlg::onDriverSelected( ngrs::ItemEvent * ev )
		{
			std::map<std::string, AudioDriver*> & driverMap =  config_->driverMap();
			std::map<std::string, AudioDriver*>::iterator it = driverMap.find( ev->text() );
			if ( it != driverMap.end() ) {
				AudioDriver* driver = it->second;
				selectedDriver_ = driver;
				AudioDriverInfo driverInfo = driver->info();
				audioHeaderLbl_->setText( driverInfo.header() ) ;
				audioDescriptionLbl_->setText( driverInfo.description() );
				if ( driverInfo.name() == "jack" ) {
					// show jack page
					noteBook_->setActivePage( jackPage_ );
				} else 
				if ( driverInfo.name() == "esd") {
					// show esd page
					noteBook_->setActivePage( esdPage_ );
				} else {
					// show generalPage
					noteBook_->setActivePage( generalPage_ );
					updateGeneralPage();
				}
				
				driverBox_->resize();
				driverBox_->repaint();
   		}
		}

		void AudioConfigDlg::updateGeneralPage() {
			if ( selectedDriver_) {
				// get the information from the selected driver
				AudioDriverSettings settings = selectedDriver_->settings();

				// now write the information to the gui
				deviceEdt_->setText( settings.deviceName() );

				switch ( settings.samplesPerSec() ) {
					case 44100 : sampleRateCbx_->setIndex( 0 ); 
					break;
				}

				switch ( settings.bitDepth() ) {
					case 8  : ;  
					break;
					case 16 : bitDepthCbx_->setIndex( 0);
					break;

				}

				switch ( settings.channelMode() ) {
					case 0 :
						// mono
						channelModeCbx_->setIndex( 0 );
					break;
					case 1 :
						// mono
						channelModeCbx_->setIndex( 0 );
					break;
					case 2 :
						// mono
						channelModeCbx_->setIndex( 0 );
					break;
					case 3 :
						// stereo
						channelModeCbx_->setIndex( 1 );
					break;
				}

				driverBox_->resize();
				driverBox_->repaint();
			}
		}

        void AudioConfigDlg::onRestartDriver( ngrs::ButtonEvent * ev )
		{		
			if ( selectedDriver_ ) {
				// disable old driver
				Player::Instance()->driver().Enable( false );
				// set new Driver to Player
				Player::Instance()->setDriver( *selectedDriver_ );
			}
		}

		void AudioConfigDlg::onOkBtn( ngrs::ButtonEvent* ev ) {
                        
		}

        void AudioConfigDlg::onCloseBtn( ngrs::ButtonEvent* ev ) {
			onClose();
		}


		void AudioConfigDlg::setVisible( bool on ) {
			if (on) {
				AudioDriverInfo driverInfo = Player::Instance()->driver().info();
				std::vector<ngrs::CustomItem*>::iterator it = driverCbx_->items().begin();
				int idx = 0;
				for ( ; it < driverCbx_->items().end(); it++, idx++ ) {
                  ngrs::CustomItem* item = *it;
					if ( item->text() == driverInfo.name() ) {
						driverCbx_->setIndex( idx );
                        ngrs::ItemEvent ev(item,item->text());
						onDriverSelected( &ev );
						break;
					}
				}
				pack();
				setPositionToScreenCenter();
			}
			Window::setVisible(on);
		}

		int AudioConfigDlg::onClose( )
		{
			setVisible(false);
            return ngrs::nHideWindow;
		}

		void AudioConfigDlg::onChannelCbx( ngrs::ItemEvent * ev )
		{
		}

		void AudioConfigDlg::onSampleRateCbx( ngrs::ItemEvent * ev )
		{
		}

		void AudioConfigDlg::onDepthCbx( ngrs::ItemEvent * ev )
		{
		}

	}
}



