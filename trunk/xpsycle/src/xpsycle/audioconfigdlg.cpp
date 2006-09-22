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
		  : NWindow( ),
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
			NPanel* btnPanel = new NPanel();
				btnPanel->setLayout( NAlignLayout(5,5) );
				okBtn_ = new NButton( " Ok " );
					okBtn_->setFlat(false);
					okBtn_->clicked.connect( this, &AudioConfigDlg::onOkBtn );
				btnPanel->add( okBtn_, nAlRight );
				cancelBtn_ = new NButton( "Cancel" );
					cancelBtn_->setFlat(false);
					cancelBtn_->clicked.connect( this, &AudioConfigDlg::onCancelBtn );
				btnPanel->add( cancelBtn_, nAlRight );				
			pane()->add( btnPanel, nAlBottom );

			// creates a TabBook with an audio system and MIDI tab
			tabBook_ = new NTabBook();
				audioPage_ = new NPanel();
					audioPage_->setLayout( NAlignLayout( 5, 5 ) );
				tabBook_->addPage( audioPage_, "Audio System" );
				midiPage_  = new NPanel();
					midiPage_->setLayout( NAlignLayout() );
				tabBook_->addPage( midiPage_, "Midi System" );
			pane()->add(tabBook_, nAlClient);
			
			initAudioDriverBox();

			tabBook_->setActivePage( audioPage_ );
		}

		void AudioConfigDlg::initAudioDriverBox( )
		{
			driverBox_ = new NGroupBox("Audio Driver");
				driverBox_->setLayout( NAlignLayout() );
			audioPage_->add( driverBox_, nAlClient );

			NPanel* infoPanel = new NPanel();
				infoPanel->setPreferredSize(200,100);
				infoPanel->setBorder( NFrameBorder( 1, 5, 5 ) );
				infoPanel->setSpacing(10,10,10,10);
				infoPanel->setLayout( NAlignLayout() );
				audioHeaderLbl_ = new	NLabel( );
					audioHeaderLbl_->setWordWrap(true);
				infoPanel->add( audioHeaderLbl_, nAlTop );
				audioDescriptionLbl_ = new NLabel( );
					audioDescriptionLbl_->setWordWrap(true);
				infoPanel->add( audioDescriptionLbl_, nAlTop );
			driverBox_->add( infoPanel, nAlRight );

			NPanel* driverSelectPanel = new NPanel();
				driverSelectPanel->setLayout( NAlignLayout( 5, 5 ) );
				driverCbx_ = new NComboBox();
				driverCbx_->setWidth(100);
				driverCbx_->itemSelected.connect( this, &AudioConfigDlg::onDriverSelected );
				driverSelectPanel->add ( driverCbx_, nAlLeft );			
				restartBtn_ = new NButton("Restart Driver");
					restartBtn_->clicked.connect( this, &AudioConfigDlg::onRestartDriver );
					restartBtn_->setFlat( false );
				driverSelectPanel->add( restartBtn_, nAlLeft );
			driverBox_->add( driverSelectPanel, nAlTop );

			noteBook_ = new NNoteBook();
			driverBox_->add( noteBook_, nAlClient );

			initJackPage();
			initEsdPage();
			initGeneralPage();

			initDriverList();      
		}

		void AudioConfigDlg::initJackPage( )
		{
			jackPage_ = new NPanel();
				jackPage_->setLayout( NAlignLayout() );
				NLabel* label = new NLabel("No Settings here");
				jackPage_->add( label, nAlTop );
			noteBook_->add( jackPage_ );			
		}

		void AudioConfigDlg::initEsdPage( )
		{
			esdPage_ = new NPanel();
				esdPage_->setLayout( NAlignLayout() );
				NLabel* label = new NLabel("No Settings here");
				esdPage_->add( label, nAlTop );
			noteBook_->add( esdPage_ );
		}

		void AudioConfigDlg::initGeneralPage( )
		{
			generalPage_ = new NPanel();
				generalPage_->setLayout( NAlignLayout() );
				NPanel* table = new NPanel();
					NTableLayout tableLayout(2,3);
					tableLayout.setVGap(5);
					tableLayout.setHGap(5);
					table->setLayout(tableLayout);
			
					table->add(new NLabel("Device") , NAlignConstraint(nAlLeft,0,0),true);
					table->add(new NLabel("Buffer size"), NAlignConstraint(nAlLeft,0,1),true);
					table->add(new NLabel("Sample rate"), NAlignConstraint(nAlLeft,0,2),true);
					table->add(new NLabel("Bit depth"), NAlignConstraint(nAlLeft,0,3),true);
					table->add(new NLabel("Channel mode"), NAlignConstraint(nAlLeft,0,4),true);

					deviceEdt_ = new NEdit();
						deviceEdt_->setPreferredSize(150,15);
					sampleRateCbx_ = new NComboBox();
						sampleRateCbx_->add( new NItem("44100"));
					bitDepthCbx_ = new NComboBox();
						bitDepthCbx_->add( new NItem("16") );
					channelModeCbx_ = new NComboBox();
						channelModeCbx_->add( new NItem("mono" ) );
						channelModeCbx_->add( new NItem("stereo") );

					table->add( deviceEdt_, NAlignConstraint(nAlLeft,1,0),true);
					table->add( sampleRateCbx_, NAlignConstraint(nAlLeft,1,2),true);
					table->add( bitDepthCbx_, NAlignConstraint(nAlLeft,1,3),true);
					table->add( channelModeCbx_, NAlignConstraint(nAlLeft,1,4),true);

				generalPage_->add( table, nAlClient );
			noteBook_->add( generalPage_ );
		}

		void AudioConfigDlg::initDriverList( )
		{
			std::map<std::string, AudioDriver*> & driverMap =  config_->driverMap();
			std::map<std::string, AudioDriver*>::iterator it = driverMap.begin();
			for ( ; it != driverMap.end(); it++ ) {
				std::string driverName = it->first;
				driverCbx_->add( new NItem( driverName ) );
			}
		}

		void AudioConfigDlg::onDriverSelected( NItemEvent * ev )
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
				std::cout << settings.deviceName() << std::endl;
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

		void AudioConfigDlg::onRestartDriver( NButtonEvent * ev )
		{		
			if ( selectedDriver_ ) {
				// disable old driver
				Player::Instance()->driver().Enable( false );
				// set new Driver to Player
				Player::Instance()->setDriver( *selectedDriver_ );
			}
		}

		void AudioConfigDlg::onOkBtn( NButtonEvent* ev ) {

		}

		void AudioConfigDlg::onCancelBtn( NButtonEvent* ev ) {
			onClose();
		}


		void AudioConfigDlg::setVisible( bool on ) {
			if (on) {
				AudioDriverInfo driverInfo = Player::Instance()->driver().info();
				std::vector<NCustomItem*>::iterator it = driverCbx_->items().begin();
				int idx = 0;
				for ( ; it < driverCbx_->items().end(); it++, idx++ ) {
					NCustomItem* item = *it;
					if ( item->text() == driverInfo.name() ) {
						driverCbx_->setIndex( idx );
						NItemEvent ev(item,item->text());
						onDriverSelected( &ev );
						break;
					}
				}
			}
			NWindow::setVisible(on);
		}

		int AudioConfigDlg::onClose( )
		{
			setVisible(false);
  		return nHideWindow;
		}

	}
}

