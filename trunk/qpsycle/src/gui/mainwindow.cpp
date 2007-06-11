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

#include "global.h"
#include "configuration.h"
#include "mainwindow.h"
#include "patternbox.h"
#include "machineview.h"
#include "patternview.h"
#include "patterndraw.h"
#include "patterngrid.h"
#include "waveview.h"
#include "sequencerview.h"
#include "sequencerdraw.h"
#include "patternbox.h"
#include "machinegui.h"
#include "audioconfigdlg.h"

#include <QtGui>

#include <iostream>
#include <iomanip>

MainWindow::MainWindow()
{
	undoStack = new QUndoStack();
	song_ = createBlankSong();
	setupSound();
	psy::core::Player::Instance()->setLoopSong( true ); // FIXME: should come from config.

	macView_ = new MachineView( song_ );
	patView_ = new PatternView( song_ );
	wavView_ = new WaveView( song_ );
	seqView_ = new SequencerView( song_ );
	patternBox_ = new PatternBox( song_ );

	setupGui();
	setupSignals();

	patternBox_->populatePatternTree(); // FIXME: here because of bad design?
	populateMachineCombo();
	initSampleCombo();
	patternBox_->patternTree()->setFocus();

	startTimer( 10 );

	macView_->setOctave( 4 );
	patView_->setOctave( 4 );

	audioCnfDlg = new AudioConfigDlg( this );
	setAttribute( Qt::WA_DeleteOnClose );
	createUndoView();
}

void MainWindow::setupSound()
{
	psy::core::Player::Instance()->song( song_ );
	psy::core::AudioDriver *outDriver = Global::configuration()._pOutputDriver;
	psy::core::Player::Instance()->setDriver( *outDriver );
}

void MainWindow::setupGui()
{
	QWidget *workArea = new QWidget();

	dock_ = new QDockWidget( "Pattern Box", this );
	dock_->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
	dock_->setWidget(patternBox_);
	addDockWidget(Qt::LeftDockWidgetArea, dock_);

	views_ = new TabWidget();
	views_->addTab( macView_, QIcon(":images/machines.png"), "Machine View" );
	views_->addTab( patView_, QIcon(":images/pattern-editor.png"), "Pattern View" );
	views_->addTab( wavView_, QIcon(":images/waveed.png"), "Wave Editor" );
	views_->addTab( seqView_, QIcon(":images/sequencer.png"),"Sequencer View" );



	QGridLayout *layout = new QGridLayout;
	layout->addWidget( views_ );
	workArea->setLayout(layout);
	setCentralWidget(workArea);

	createActions();
	createMenus();
	createToolBars();
	createStatusBar();

	setWindowTitle(tr("] Psycle Modular Music Creation Studio [ ( Q v0.00001088 alpha ) "));
}

void MainWindow::setupSignals()
{
	connect( patternBox_, SIGNAL( patternSelectedInPatternBox( psy::core::SinglePattern* ) ),
		 this, SLOT( onPatternSelectedInPatternBox( psy::core::SinglePattern* ) ) );
	connect( patternBox_, SIGNAL( patternDeleted() ),
		 this, SLOT( onPatternDeleted() ) );
	connect( patternBox_, SIGNAL( addPatternToSequencerRequest( psy::core::SinglePattern* ) ),
		 this, SLOT( onAddPatternToSequencerRequest( psy::core::SinglePattern* ) ) );
	connect( patternBox_, SIGNAL( patternNameChanged() ),
		 this, SLOT( onPatternNameChanged() ) );
	connect( patternBox_, SIGNAL( categoryColorChanged() ),
		 this, SLOT( onCategoryColorChanged() ) );

	connect( macView_, SIGNAL( newMachineCreated( psy::core::Machine* ) ),
		 this, SLOT( onNewMachineCreated( psy::core::Machine* ) ) );
	connect( macView_, SIGNAL( machineChosen( MachineGui* ) ),
		 this, SLOT( onMachineChosen( MachineGui* ) ) );
	connect( macView_, SIGNAL( machineDeleted( int ) ),
		 this, SLOT( onMachineDeleted() ) );
	connect( macView_, SIGNAL( machineRenamed( ) ),
		 this, SLOT( onMachineRenamed( ) ) );

	connect( wavView_, SIGNAL( sampleAdded() ),
		 this, SLOT( refreshSampleComboBox() ) );

	connect( macCombo_, SIGNAL( currentIndexChanged( int ) ),
		 this, SLOT( onMachineComboBoxIndexChanged( int ) ) );

	connect( sampCombo_, SIGNAL( currentIndexChanged( int ) ),
		 this, SLOT( onSampleComboBoxIndexChanged( int ) ) );
}

void MainWindow::onNewSongRequest()
{
	if ( songHasChanged() )
	{
		int response = QMessageBox::warning( this, "Save changes?",
						     "The song has been modified.\n Do you wish to save your changes?",
						     QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel,
						     QMessageBox::Save ) ;

		if ( response == QMessageBox::Save ) {
			onSaveSongRequest();
		}

		if ( response == QMessageBox::Cancel ) {
			return;
		}
	}

	psy::core::Song *blankSong = createBlankSong();
	loadSong( blankSong );
}

void MainWindow::onOpenSongRequest()
{
	if ( songHasChanged() )
	{
		int response = QMessageBox::warning( this, "Save changes?",
						     "The song has been modified.\n Do you wish to save your changes?",
						     QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel,
						     QMessageBox::Save ) ;

		if ( response == QMessageBox::Save ) {
			onSaveSongRequest();
		}

		if ( response == QMessageBox::Cancel ) {
			return;
		}
	}

	QString songPath = QString::fromStdString( Global::configuration().songPath() );
	QString fileName = QFileDialog::getOpenFileName( this, "Open Song", songPath, "Psy (*.psy)" );

	if ( !fileName.isEmpty() ) {
		psy::core::Player::Instance()->stop();
		psy::core::Song *song = new psy::core::Song(psy::core::Player::Instance());
		song->load(Global::configuration().pluginPath(), fileName.toStdString() );
		loadSong( song );
	}
}

void MainWindow::onSaveSongRequest()
{
	QString songPath = QString::fromStdString( Global::configuration().songPath() );
	QString fileName = QFileDialog::getSaveFileName(this,
							tr("Choose a file name"), songPath,
							tr("Psycle Songs (*.psy)"));
	if (fileName.isEmpty())
		return;

	song_->save( fileName.toStdString() );
}

bool MainWindow::songHasChanged()
{
	return true; // FIXME
}


psy::core::Song *MainWindow::createBlankSong()
{
	psy::core::Song *blankSong = new psy::core::Song( psy::core::Player::Instance() );
	psy::core::PatternCategory* category0 = blankSong->patternSequence()->patternData()->createNewCategory("New Category");
	psy::core::SinglePattern* pattern0 = category0->createNewPattern("Pattern0");

	psy::core::SequenceLine *seqLine = blankSong->patternSequence()->createNewLine();
	seqLine->createEntry( pattern0, 0 );

	return blankSong;
}

void MainWindow::loadSong( psy::core::Song *song )
{
	song_ = song;
	// update gui to new song FIXME: very crappy way of doing it for now.
	delete patternBox_;
	delete macView_;
	delete patView_;
	delete wavView_;
	delete seqView_;

	psy::core::Player::Instance()->song( song_ );

	macView_ = new MachineView( song_ );
	patView_ = new PatternView( song_ );
	wavView_ = new WaveView( song_ );
	seqView_ = new SequencerView( song_ );
	macView_->setOctave( 4 );
	patView_->setOctave( 4 );
	views_->addTab( macView_, QIcon(":images/machines.png"), "Machine View" );
	views_->addTab( patView_, QIcon(":images/pattern-editor.png"), "Pattern View" );
	views_->addTab( wavView_, QIcon(":images/waveed.png"), "Wave Editor" );
	views_->addTab( seqView_, QIcon(":images/sequencer.png"),"Sequencer View" );
	patternBox_ = new PatternBox( song_ );
	dock_->setWidget( patternBox_ );
	patternBox_->populatePatternTree();
	patView_->setPattern(patternBox_->currentPattern());
	populateMachineCombo();
	initSampleCombo();
	patternBox_->patternTree()->setFocus();
	createActions();
	setupSignals();
	// enable audio driver
	//Global::configuration()._pOutputDriver->Enable(true);
	// update file recent open sub menu
	//recentFileMenu_->add(new ngrs::MenuItem(fileName));
}


 void MainWindow::undo()
 {
 
 }

 void MainWindow::redo()
 {
 
 }

 void MainWindow::aboutQpsycle()
 {
    QMessageBox::about(this, tr("About qpsycle"),
             tr("It makes music and stuff."));
 }

void MainWindow::createActions()
{
	newAct = new QAction(QIcon(":/images/new.png"), tr("&New Song"), this);
	newAct->setShortcut(tr("Ctrl+N"));
	newAct->setStatusTip(tr("Create a new song"));
	connect( newAct, SIGNAL( triggered() ), this, SLOT( onNewSongRequest() ) );

	openAct = new QAction(QIcon(":/images/open.png"), tr("&Open..."), this);
	openAct->setShortcut(tr("Ctrl+O"));
	openAct->setStatusTip(tr("Open an existing song"));
	connect(openAct, SIGNAL(triggered()), this, SLOT(onOpenSongRequest()));

	saveAct = new QAction(QIcon(":/images/save.png"), tr("&Save..."), this);
	saveAct->setShortcut(tr("Ctrl+S"));
	saveAct->setStatusTip(tr("Save the current song"));
	connect(saveAct, SIGNAL(triggered()), this, SLOT(onSaveSongRequest()));

	undoAct = new QAction(QIcon(":/images/undo.png"), tr("&Undo"), this);
	undoAct->setShortcut(tr("Ctrl+Z"));
	undoAct->setStatusTip(tr("Undo the last action"));
	connect(undoAct, SIGNAL(triggered()), this, SLOT(undo()));

	redoAct = new QAction(QIcon(":/images/redo.png"), tr("&Redo"), this);
	redoAct->setShortcut(tr("Ctrl+Y"));
	redoAct->setStatusTip(tr("Redo the last undone action"));
	connect(redoAct, SIGNAL(triggered()), this, SLOT(redo()));
	
	showUnReAct = new QAction(tr("&Undo List"), this);
	showUnReAct->setStatusTip(tr("Shows/Hides the Undo/Redo Window"));
//	connect(showUnReAct, SIGNAL(triggered()), undoView, SLOT(showUndoView()));

	quitAct = new QAction(tr("&Quit"), this);
	quitAct->setShortcut(tr("Ctrl+Q"));
	quitAct->setStatusTip(tr("Quit the application"));
	connect(quitAct, SIGNAL(triggered()), this, SLOT(close()));

	audioConfAct = new QAction( tr("Audio Settings"), this );
	connect( audioConfAct, SIGNAL( triggered() ),
		 this, SLOT( showAudioConfigDlg() ) );


	aboutAct = new QAction(tr("&About qpsycle"), this);
	aboutAct->setStatusTip(tr("About qpsycle"));
	connect(aboutAct, SIGNAL(triggered()), this, SLOT(aboutQpsycle()));

	playFromStartAct = new QAction(QIcon(":/images/playstart.png"), tr("&Play from start"), this);
	connect( playFromStartAct, SIGNAL( triggered() ), this, SLOT( playFromStart() ) );
	playFromSeqPosAct = new QAction(QIcon(":/images/play.png"), tr("Play from &sequencer position"), this);
	connect( playFromSeqPosAct, SIGNAL( triggered() ), this, SLOT( playFromSeqPos() ) );
	playStopAct = new QAction(QIcon(":images/stop.png"), tr("&Stop playback"), this);
	connect( playStopAct, SIGNAL( triggered() ), this, SLOT( playStop() ) );
//     playPatAct = new QAction(QIcon(":/images/playselpattern.png"), tr("Play selected p&attern"), this);

}

void MainWindow::createMenus()
{
	fileMenu = menuBar()->addMenu(tr("&File"));
	fileMenu->addAction(newAct);
	fileMenu->addAction(openAct);
	fileMenu->addAction(saveAct);
	fileMenu->addSeparator();
	fileMenu->addAction(quitAct);

	editMenu = menuBar()->addMenu(tr("&Edit"));
	editMenu->addAction(undoAct);
	editMenu->addAction(redoAct);

	viewMenu = menuBar()->addMenu(tr("&View"));
	viewMenu->addAction( showUnReAct );
	configMenu = menuBar()->addMenu(tr("&Configuration"));
	configMenu->addAction( audioConfAct );

	performMenu = menuBar()->addMenu(tr("&Performance"));
	communityMenu = menuBar()->addMenu(tr("&Community"));

	menuBar()->addSeparator();

	helpMenu = menuBar()->addMenu(tr("&Help"));
	helpMenu->addAction(aboutAct);
}

void MainWindow::createToolBars()
{
	fileToolBar = addToolBar(tr("File"));
	fileToolBar->addAction(newAct);
	fileToolBar->addAction(openAct);
	fileToolBar->addAction(saveAct);

	editToolBar = addToolBar(tr("Edit"));
	editToolBar->addAction(undoAct);
	editToolBar->addAction(redoAct);

	playToolBar = addToolBar(tr("Play"));
	playToolBar->addAction(playFromStartAct);
	playToolBar->addAction(playFromSeqPosAct);
//     playToolBar->addAction(playPatAct);
	playToolBar->addAction(playStopAct);

	machToolBar = addToolBar(tr("Machines"));
	macCombo_ = new QComboBox();
	macCombo_->setSizeAdjustPolicy(QComboBox::AdjustToContents);
	sampCombo_ = new QComboBox();
	octCombo_ = new QComboBox();
	sampCombo_->setSizeAdjustPolicy(QComboBox::AdjustToContents);

	QLabel *macLabel = new QLabel(" Machines: ");
	QLabel *sampLabel = new QLabel(" Samples: ");
	machToolBar->addWidget(macLabel);
	machToolBar->addWidget( macCombo_ );
	machToolBar->addWidget(sampLabel);
	machToolBar->addWidget( sampCombo_ );

	octToolBar_ = addToolBar( "Octave" );
	octToolBar_->addWidget( new QLabel( "Octave: " ) );
	octToolBar_->addWidget( octCombo_ );
	for ( int i = 0; i < 9; i++ ) {
		octCombo_->addItem( QString::number( i ) );
	}
	connect( octCombo_, SIGNAL( currentIndexChanged( int ) ),
		 this, SLOT( onOctaveComboBoxIndexChanged( int ) ) );
	octCombo_->setCurrentIndex( 4 );
}

void MainWindow::createStatusBar()
{
    statusBar()->showMessage(tr("Ready"));
}

void MainWindow::populateMachineCombo()
{
	if (!song_) return;
	int currentIndex = macCombo_->currentIndex();

	macCombo_->clear();

	bool comboIsEmpty=true;
	std::ostringstream buffer;
	buffer.setf(std::ios::uppercase);

	for (int b=0; b<psy::core::MAX_BUSES; b++) // Generators.
	{
		if( song_->machine(b)) {
			buffer.str("");
			buffer << std::setfill('0') << std::hex << std::setw(2);
			buffer << b << ": " << song_->machine(b)->GetEditName();
			macCombo_->addItem( QString::fromStdString( buffer.str() ),
					    song_->machine(b)->id() );

			comboIsEmpty = false;
		}
	}

	macCombo_->addItem( "--------------------------");

	for (int b=psy::core::MAX_BUSES; b<psy::core::MAX_BUSES*2; b++) // Effects.
	{
		if(song_->machine(b)) {
			buffer.str("");
			buffer << std::setfill('0') << std::hex << std::setw(2);
			buffer << b << ": " << song_->machine(b)->GetEditName();
			macCombo_->addItem( QString::fromStdString( buffer.str() ),
					    song_->machine(b)->id() );
			comboIsEmpty = false;
		}
	}

	if (comboIsEmpty) {
		macCombo_->addItem( "No Machines Loaded" );
		currentIndex = 1;
	}
	macCombo_->setCurrentIndex( currentIndex );
	macCombo_->update();
}

void MainWindow::initSampleCombo()
{
	for ( int i=0; i < psy::core::MAX_INSTRUMENTS; i++ ) // PREV_WAV_INS = 255
	{
		sampCombo_->addItem( "Empty" );
	}
}

void MainWindow::refreshSampleComboBox()
{
	std::ostringstream buffer;
	buffer.setf(std::ios::uppercase);

	int listlen = 0;
	for ( int i=0; i < psy::core::MAX_INSTRUMENTS; i++ ) // PREV_WAV_INS = 255
	{
		buffer.str("");
		buffer << std::setfill('0') << std::hex << std::setw(2);
		buffer << i << ": " << song_->_pInstrument[i]->_sName;
		QString name = QString::fromStdString( buffer.str() );
		sampCombo_->setItemText( i, name );
		listlen++;
	}
	if (song_->auxcolSelected >= listlen) {
		song_->auxcolSelected = 0;
	}

}

void MainWindow::onMachineComboBoxIndexChanged( int newIndex )
{
	song_->seqBus = newIndex;

	// Choose the necessary MachineGui in the MachineView.
	MachineGui *macGui = macView_->findMachineGuiByCoreMachineIndex( newIndex );
	macView_->setChosenMachine( macGui );
	macView_->update();
}

void MainWindow::onSampleComboBoxIndexChanged( int newIndex )
{
	song_->instSelected   = newIndex;
	song_->auxcolSelected = newIndex;
	// FIXME: when wave editor is more advanced, we need to notify it of this change.
}

void MainWindow::onPatternSelectedInPatternBox( psy::core::SinglePattern* selectedPattern )
{
    patView_->setPattern( selectedPattern );
}



void MainWindow::onNewMachineCreated( psy::core::Machine *mac )
{
	populateMachineCombo();
	if ( mac->mode() == psy::core::MACHMODE_GENERATOR )
		macCombo_->setCurrentIndex( macCombo_->findData( mac->id() ) );
}

void MainWindow::onMachineChosen( MachineGui *macGui )
{
	int comboIdx = macCombo_->findData( macGui->mac()->id() );
	macCombo_->setCurrentIndex( comboIdx );
}

void MainWindow::onMachineDeleted()
{
	populateMachineCombo(); // FIXME: a bit inefficient to repopulate the whole thing.
}

void MainWindow::onMachineRenamed()
{
	populateMachineCombo(); // FIXME: a bit inefficient to repopulate the whole thing.
}




void MainWindow::onPatternDeleted()
{
	patView_->setPattern( 0 );
}

void MainWindow::onPatternNameChanged()
{
	seqView_->onPatternNameChanged();
}



void MainWindow::onAddPatternToSequencerRequest( psy::core::SinglePattern *pattern )
{
	seqView_->addPattern( pattern );
}

void MainWindow::onCategoryColorChanged()
{
	seqView_->onCategoryColorChanged();
}


void MainWindow::onOctaveComboBoxIndexChanged( int newIndex )
{
	patView_->setOctave( newIndex );
	macView_->setOctave( newIndex );
}



void MainWindow::playFromStart()
{
	psy::core::Player::Instance()->setLoopSequenceEntry( 0 );
	psy::core::Player::Instance()->start( 0.0 );
}

void MainWindow::playFromSeqPos()
{
	psy::core::Player::Instance()->start( psy::core::Player::Instance()->playPos() );
}

void MainWindow::playStop()
{
	psy::core::Player::Instance()->stop();
}




void MainWindow::showAudioConfigDlg()
{
	audioCnfDlg->exec();
}



void MainWindow::keyPressEvent( QKeyEvent * event )
{
	if ( event->key() == Qt::Key_Tab )
		return;
	int command = Global::configuration().inputHandler().getEnumCodeByKey( Key( event->modifiers(), event->key() ) );

	switch ( command ) {
        case commands::show_pattern_box:
        {
		if ( !dock_->isVisible() ) {
			dock_->setVisible( true );
			patternBox_->patternTree()->setFocus();
		} else {
			if ( patternBox_->patternTree()->hasFocus() ) {
				dock_->setVisible( false );
			} else {
				patternBox_->patternTree()->setFocus();
			}
		}
        }
        break;
        case commands::show_machine_view:
		views_->setCurrentWidget( macView_ );
		break;
        case commands::show_pattern_view:
		views_->setCurrentWidget( patView_ );
		patView_->patDraw()->setFocus();
		patView_->patDraw()->scene()->setFocusItem( patView_->patDraw()->patternGrid() );
		break;
        case commands::show_wave_editor:
		views_->setCurrentWidget( wavView_ );
		break;
        case commands::show_sequencer_view:
		views_->setCurrentWidget( seqView_ );
		break;
		// Play controls.
        case commands::play_start:
		playFromStartAct->trigger();
		break;
        case commands::play_from_position:
		playFromSeqPosAct->trigger();
		break;
        case commands::play_stop:
		playStopAct->trigger();
		break;
        case commands::play_loop_entry:
        {
//            psy::core::Player::Instance()->setLoopPatternEntry( ... );
        }
        break;
        case commands::instrument_inc:
		sampCombo_->setCurrentIndex( sampCombo_->currentIndex() + 1 );
		break;
        case commands::instrument_dec:
		sampCombo_->setCurrentIndex( sampCombo_->currentIndex() - 1 );
		break;
        case commands::octave_up:
		octCombo_->setCurrentIndex( std::max( 0, octCombo_->currentIndex() + 1 ) );
		break;
        case commands::octave_down:
		octCombo_->setCurrentIndex( std::min( 8, octCombo_->currentIndex() - 1 ) );
		break;

        default:;
	}
}

void MainWindow::timerEvent( QTimerEvent *ev )
{
	Q_UNUSED( ev );
	if ( psy::core::Player::Instance()->playing() ) {
		seqView_->updatePlayPos();

		psy::core::SinglePattern* visiblePattern = 0;
		visiblePattern = patView_->pattern();
		if ( visiblePattern ) {
			double entryStart = 0;
			bool isPlayPattern = song_->patternSequence()->getPlayInfo( visiblePattern, psy::core::Player::Instance()->playPos() , 4 , entryStart );
			if ( isPlayPattern ) {
				patView_->onTick( entryStart );
			}
		}
	}
}

TabWidget::TabWidget( QWidget *parent )
	: QTabWidget( parent )
{}

bool TabWidget::event( QEvent *event )
{

	switch (event->type()) {
        case QEvent::KeyPress: {
		QKeyEvent *k = (QKeyEvent *)event;
		if (k->key() == Qt::Key_1 || k->key() == Qt::Key_2
		    || k->key() == Qt::Key_3 || k->key() == Qt::Key_4 )
		{
			return true;
		} else {
			QTabWidget::keyPressEvent(k);
			return true;
		}
        }
        default:
		return QTabWidget::event( event );
	}
}

void MainWindow::createUndoView()
{
	undoView = new QUndoView( undoStack );
	undoView->setWindowTitle(tr("Undo List"));
	undoView->show();
	undoView->setAttribute(Qt::WA_QuitOnClose, false);
}

void MainWindow::showUndoView() //is there a reason why this code isn't working????
{
	if (undoView->isVisible())
	{
		undoView->setVisible(false);
	}
	else
	{
		undoView->setVisible(true);
	}
}
