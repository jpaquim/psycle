/***************************************************************************
*   Copyright (C) 2007 by Psycledelics Community   *
*   psycle.sourceforge.net   *
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

#include <psycle/core/player.h>
#include <psycle/core/song.h>
#include <psycle/core/singlepattern.h>
#include <psycle/core/patterndata.h>
#include <psycle/core/patternsequence.h>

#include "mainwindow.h"
#include "machineview.h"
#include "machinegui.h"
#include "configuration.h"
#include "global.h"

#include <QtGui>

#include <iostream>
#include <iomanip>

MainWindow::MainWindow()
{
	song_ = createBlankSong();
	setupSound();

	mainWidget_ = new QWidget( this );
	macView_ = new MachineView( song_, mainWidget_ );

	setupGui();
	setupSignals();
}

void MainWindow::setupSong()
{
	// Setup a blank song.
	psy::core::PatternCategory* category0 = song_->patternSequence()->patternData()->createNewCategory("Category0");
	psy::core::SinglePattern* pattern0 = category0->createNewPattern("Pattern0");

	psy::core::SequenceLine *seqLine = song_->patternSequence()->createNewLine();
	psy::core::SequenceEntry *seqEntry = seqLine->createEntry( pattern0, 0 );
}

void MainWindow::setupSound() 
{
	psy::core::Player::Instance()->song( song_ );
	psy::core::AudioDriver *outDriver = Global::pConfig()->_pOutputDriver;
	psy::core::Player::Instance()->setDriver( *outDriver );  
}

void MainWindow::setupGui()
{
	QHBoxLayout *mainLayout = new QHBoxLayout();
	mainWidget_->setLayout( mainLayout );

	sidebar_ = new QWidget( mainWidget_ );
	QVBoxLayout *sideLay = new QVBoxLayout();
	sidebar_->setLayout( sideLay );
	breedBut_ = new QPushButton("breed");
	sideLay->addWidget( breedBut_ );
	connect( breedBut_, SIGNAL( clicked() ),
		 this, SLOT( onBreedClicked() ) );

	mainLayout->addWidget( macView_ );
	mainLayout->addWidget( sidebar_ );

	setCentralWidget( mainWidget_ );
	createStatusBar();

	setWindowTitle(tr("p3b psycle plugin preset breeder"));
}

void MainWindow::setupSignals()
{
}

psy::core::Song *MainWindow::createBlankSong() 
{
	psy::core::Song *blankSong = new psy::core::Song( psy::core::Player::Instance() );
	psy::core::PatternCategory* category0 = blankSong->patternSequence()->patternData()->createNewCategory("Category0");
	psy::core::SinglePattern* pattern0 = category0->createNewPattern("Pattern0");

	psy::core::SequenceLine *seqLine = blankSong->patternSequence()->createNewLine();
	psy::core::SequenceEntry *seqEntry = seqLine->createEntry( pattern0, 0 );

	return blankSong;
}

void MainWindow::createStatusBar()
{
	statusBar()->showMessage(tr("Ready"));
}

void MainWindow::keyPressEvent( QKeyEvent * event )
{
}


void MainWindow::onBreedClicked()
{
	MachineGui *dad = 0;
	MachineGui *mum = 0;
	bool noDad = true;
	bool noMum = true;
	for ( int i = 0; i < 4; i++ )
	{
		if ( macView_->macGuiList()[i]->isSelected() )
		{
			if ( noDad ) {
				dad = macView_->macGuiList()[i];
				noDad = false;
			} else if ( noMum ) {
				mum = macView_->macGuiList()[i];
				break;
			}
		}
	}

	if ( dad ) std::cout << dad->mac()->GetEditName() << std::endl; 
	if ( mum ) std::cout << mum->mac()->GetEditName() << std::endl; 

	if ( dad && mum ) {
		psy::core::Player::Instance()->lock();
		Global::configuration()._pOutputDriver->Enable(false);
//        song_->DestroyMachine( 0 );
		//       song_->DestroyMachine( 1 );
		//      song_->DestroyMachine( 2 );
		//     song_->DestroyMachine( 3 );
		psy::core::Machine *kidA = breed( dad->mac(), mum->mac() );
		macView_->macGuiList()[0]->setMac( kidA );
		psy::core::Machine *kidB = breed( dad->mac(), mum->mac() );
		macView_->macGuiList()[1]->setMac( kidB );
		psy::core::Machine *kidC = breed( dad->mac(), mum->mac() );
		macView_->macGuiList()[2]->setMac( kidC );
		psy::core::Machine *kidD = breed( dad->mac(), mum->mac() );
		macView_->macGuiList()[3]->setMac( kidD );
		psy::core::Player::Instance()->unlock();
		Global::configuration()._pOutputDriver->Enable(true);
	}

}

psy::core::Machine *MainWindow::breed( psy::core::Machine *dad, psy::core::Machine *mum )
{
	QSettings settings;
	std::string plugin_path = settings.value("plugins/psyclePath").toString().toStdString();
	std::string pluginLibName = settings.value( "mainPlugin/libName" ).toString().toStdString();

	psy::core::Machine *kid = &song_->CreateMachine( plugin_path, psy::core::MACH_PLUGIN, 0, 0, pluginLibName );

	int numpars = kid->GetNumParams();
	for (int c=0; c<numpars; c++)
	{
		if ( rand() % 2 == 0 ) {
			int pVal = dad->GetParamValue( c ); 
			kid->SetParameter( c, pVal );
		}
		else {
			int pVal = mum->GetParamValue( c ); 
			kid->SetParameter( c, pVal );
		}
	}

	song_->InsertConnection( kid->id(), psy::core::MASTER_INDEX, 1.0f);
	return kid;
}
