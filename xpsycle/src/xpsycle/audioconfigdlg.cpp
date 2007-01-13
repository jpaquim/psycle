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
#include "audioconfigdlg.h"
#include "configuration.h"
#include "audiodriver.h"
#include "player.h"
#include <ngrs/nlistbox.h>
#include <ngrs/nitem.h>
#include <ngrs/nalignlayout.h>
#include <ngrs/ntablelayout.h>
#include <ngrs/nlabel.h>
#include <ngrs/nobjectinspector.h>
#include <ngrs/nitemevent.h>
#include <ngrs/nbutton.h>
#include <ngrs/nframeborder.h>

namespace psycle {
	namespace host	{	

		AudioConfigDlg::AudioConfigDlg( Configuration* cfg )
		  : ngrs::NWindow( ),
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
            ngrs::NPanel* btnPanel = new ngrs::NPanel();
               btnPanel->setLayout( ngrs::NAlignLayout(5,5) );
				/*okBtn_ = new NButton( " Ok " );
					okBtn_->setFlat(false);
					okBtn_->clicked.connect( this, &AudioConfigDlg::onOkBtn );
				btnPanel->add( okBtn_, ngrs::nAlRight );*/
                closeBtn_ = new ngrs::NButton( "Close" );
					closeBtn_->setFlat(false);
					closeBtn_->clicked.connect( this, &AudioConfigDlg::onCloseBtn );
				btnPanel->add( closeBtn_, ngrs::nAlRight );				
                pane()->add( btnPanel, ngrs::nAlBottom );

			// creates a TabBook with an audio system and MIDI tab
			tabBook_ = new ngrs::NTabBook();
            audioPage_ = new ngrs::NPanel();
            audioPage_->setLayout( ngrs::NAlignLayout( 5, 5 ) );
            midiPage_  = new ngrs::NPanel();
            midiPage_->setLayout( ngrs::NAlignLayout() );
				tabBook_->addPage( audioPage_, "Audio System" );
				tabBook_->addPage( midiPage_, "Midi System" );
			pane()->add(tabBook_, ngrs::nAlClient);
			
			initAudioDriverBox();

			tabBook_->setActivePage( audioPage_ );
		}

		void AudioConfigDlg::initAudioDriverBox( )
		{
          driverBox_ = new ngrs::NGroupBox("Audio Driver");
				driverBox_->setLayout( ngrs::NAlignLayout() );
			audioPage_->add( driverBox_, ngrs::nAlClient );

            ngrs::NPanel* infoPanel = new ngrs::NPanel();
				infoPanel->setPreferredSize(200,100);
				infoPanel->setBorder( ngrs::NFrameBorder( 1, 5, 5 ) );
				infoPanel->setSpacing(10,10,10,10);
				infoPanel->setLayout( ngrs::NAlignLayout() );
                audioHeaderLbl_ = new	ngrs::NLabel( );
					audioHeaderLbl_->setWordWrap(true);
				infoPanel->add( audioHeaderLbl_, ngrs::nAlTop );
				audioDescriptionLbl_ = new ngrs::NLabel( );
					audioDescriptionLbl_->setWordWrap(true);
				infoPanel->add( audioDescriptionLbl_, ngrs::nAlTop );
			driverBox_->add( infoPanel, ngrs::nAlRight );

			ngrs::NPanel* driverSelectPanel = new ngrs::NPanel();
				driverSelectPanel->setLayout( ngrs::NAlignLayout( 5, 5 ) );
				driverCbx_ = new ngrs::NComboBox();
				driverCbx_->setWidth(100);
				driverCbx_->itemSelected.connect( this, &AudioConfigDlg::onDriverSelected );
				driverSelectPanel->add ( driverCbx_, ngrs::nAlLeft );			
				restartBtn_ = new ngrs::NButton("Restart Driver");
					restartBtn_->clicked.connect( this, &AudioConfigDlg::onRestartDriver );
					restartBtn_->setFlat( false );
				driverSelectPanel->add( restartBtn_, ngrs::nAlLeft );
			driverBox_->add( driverSelectPanel, ngrs::nAlTop );

			noteBook_ = new ngrs::NNoteBook();
			driverBox_->add( noteBook_, ngrs::nAlClient );

			initJackPage();
			initEsdPage();
			initGeneralPage();

			initDriverList();      
		}

		void AudioConfigDlg::initJackPage( )
		{
			jackPage_ = new ngrs::NPanel();
				jackPage_->setLayout( ngrs::NAlignLayout() );
				ngrs::NLabel* label = new ngrs::NLabel("No Settings here");
				jackPage_->add( label, ngrs::nAlTop );
			noteBook_->add( jackPage_ );			
		}

		void AudioConfigDlg::initEsdPage( )
		{
			esdPage_ = new ngrs::NPanel();
				esdPage_->setLayout( ngrs::NAlignLayout() );
				ngrs::NLabel* label = new ngrs::NLabel("No Settings here");
				esdPage_->add( label, ngrs::nAlTop );
			noteBook_->add( esdPage_ );
		}

		void AudioConfigDlg::initGeneralPage( )
		{
			generalPage_ = new ngrs::NPanel();
				generalPage_->setLayout( ngrs::NAlignLayout() );
				ngrs::NPanel* table = new ngrs::NPanel();
					ngrs::NTableLayout tableLayout(2,3);
					tableLayout.setVGap(5);
					tableLayout.setHGap(5);
					table->setLayout( tableLayout );
			
					table->add(new ngrs::NLabel("Device") , ngrs::NAlignConstraint( ngrs::nAlLeft, 0, 0 ),true);
					table->add(new ngrs::NLabel("Buffer size"), ngrs::NAlignConstraint( ngrs::nAlLeft, 0, 1),true);
					table->add(new ngrs::NLabel("Sample rate"), ngrs::NAlignConstraint( ngrs::nAlLeft, 0, 2),true);
					table->add(new ngrs::NLabel("Bit depth"), ngrs::NAlignConstraint( ngrs::nAlLeft, 0, 3),true);
					table->add(new ngrs::NLabel("Channel mode"), ngrs::NAlignConstraint( ngrs::nAlLeft, 0, 4),true);

					deviceEdt_ = new ngrs::NEdit();
						deviceEdt_->setPreferredSize( 150, 15 );
					sampleRateCbx_ = new ngrs::NComboBox();
						sampleRateCbx_->itemSelected.connect( this, &AudioConfigDlg::onSampleRateCbx );
						sampleRateCbx_->add( new ngrs::NItem("44100"));
					bitDepthCbx_ = new ngrs::NComboBox();
						bitDepthCbx_->itemSelected.connect( this, &AudioConfigDlg::onDepthCbx );
						bitDepthCbx_->add( new ngrs::NItem("16") );
					channelModeCbx_ = new ngrs::NComboBox();
						channelModeCbx_->add( new ngrs::NItem("mono" ) );
						channelModeCbx_->add( new ngrs::NItem("stereo") );
						channelModeCbx_->itemSelected.connect( this, &AudioConfigDlg::onChannelCbx );

					table->add( deviceEdt_, ngrs::NAlignConstraint( ngrs::nAlLeft, 1, 0 ), true);
					table->add( sampleRateCbx_, ngrs::NAlignConstraint( ngrs::nAlLeft, 1, 2), true);
					table->add( bitDepthCbx_, ngrs::NAlignConstraint( ngrs::nAlLeft, 1, 3), true);
					table->add( channelModeCbx_, ngrs::NAlignConstraint( ngrs::nAlLeft,1,4), true);

				generalPage_->add( table, ngrs::nAlClient );
			noteBook_->add( generalPage_ );
		}

		void AudioConfigDlg::initDriverList( )
		{
			std::map<std::string, AudioDriver*> & driverMap =  config_->driverMap();
			std::map<std::string, AudioDriver*>::iterator it = driverMap.begin();
			for ( ; it != driverMap.end(); it++ ) {
				std::string driverName = it->first;
				driverCbx_->add( new ngrs::NItem( driverName ) );
			}
		}

		void AudioConfigDlg::onDriverSelected( ngrs::NItemEvent * ev )
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

        void AudioConfigDlg::onRestartDriver( ngrs::NButtonEvent * ev )
		{		
			if ( selectedDriver_ ) {
				// disable old driver
				Player::Instance()->driver().Enable( false );
				// set new Driver to Player
				Player::Instance()->setDriver( *selectedDriver_ );
			}
		}

		void AudioConfigDlg::onOkBtn( ngrs::NButtonEvent* ev ) {
                        
		}

        void AudioConfigDlg::onCloseBtn( ngrs::NButtonEvent* ev ) {
			onClose();
		}


		void AudioConfigDlg::setVisible( bool on ) {
			if (on) {
				AudioDriverInfo driverInfo = Player::Instance()->driver().info();
				std::vector<ngrs::NCustomItem*>::iterator it = driverCbx_->items().begin();
				int idx = 0;
				for ( ; it < driverCbx_->items().end(); it++, idx++ ) {
                  ngrs::NCustomItem* item = *it;
					if ( item->text() == driverInfo.name() ) {
						driverCbx_->setIndex( idx );
                        ngrs::NItemEvent ev(item,item->text());
						onDriverSelected( &ev );
						break;
					}
				}
				pack();
				setPositionToScreenCenter();
			}
			NWindow::setVisible(on);
		}

		int AudioConfigDlg::onClose( )
		{
			setVisible(false);
            return ngrs::nHideWindow;
		}

		void AudioConfigDlg::onChannelCbx( ngrs::NItemEvent * ev )
		{
		}

		void AudioConfigDlg::onSampleRateCbx( ngrs::NItemEvent * ev )
		{
		}

		void AudioConfigDlg::onDepthCbx( ngrs::NItemEvent * ev )
		{
		}

	}
}



