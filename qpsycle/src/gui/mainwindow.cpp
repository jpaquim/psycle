// -*- mode:c++; indent-tabs-mode:t -*-
/**************************************************************************
*   Copyright (C) 2007 by Psycledelics Community                          *
*   psycle.sourceforge.net                                                *
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
#include <psycle/audiodrivers/audiodriver.h>
#include <psycle/core/patternpool.h>
#include <psycle/core/patternsequence.h>
#include <psycle/core/player.h>
#include <psycle/core/singlepattern.h>
#include <psycle/core/song.h>
#include <psycle/core/machinefactory.h>

#include "../model/instrumentsmodel.hpp"
#include "configdlg/configdlg.hpp"
#include "configuration.hpp"
#include "global.hpp"
#include "logconsole.hpp"
#include "machineview/machinegui.hpp"
#include "machineview/machineview.hpp"
#include "mainwindow.hpp"
#include "patternbox.hpp"
#include "patternview/patterndraw.hpp"
#include "patternview/patterngrid.hpp"
#include "patternview/patternview.hpp"
#include "samplebrowser.hpp"
#include "sequencer/sequencerdraw.hpp"
#include "sequencer/sequencerview.hpp"
#include "waveview/waveview.hpp"

#include <QApplication>
#include <QComboBox>
#include <QDebug>
#include <QDockWidget>
#include <QFileDialog>
#include <QLineEdit>
#include <QMenuBar>
#include <QMessageBox>
#include <QSettings>
#include <QStatusBar>
#include <QTextCodec>
#include <QTimer>
#include <QUndoStack>
#include <QUndoView>

#include <iomanip>
#include <iostream>
#include <sstream>

#define SUBVERSION_REVISION  "$Revision$"

namespace qpsycle {

	TabWidget::TabWidget( QWidget *parent )
		: QTabWidget( parent )
	{}

	// <nmather> I overrode this as by default a QTabWidget uses 1,2,3,4, etc.
	// to switch to the 1st,2nd,3rd,4th etc. tab.  We don't want this behaviour,
	// as we use the numeric keys (for example, in the pattern view.) There might
	// be a better way to get around this if anyone can think of one.
	bool TabWidget::event( QEvent *event )
	{

		switch (event->type()) {
		case QEvent::KeyPress: {
			QKeyEvent *k = (QKeyEvent *)event;
			if (k->key() == Qt::Key_1 || k->key() == Qt::Key_2 || k->key() == Qt::Key_3 || k->key() == Qt::Key_4 )
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

	MainWindow::MainWindow()
	{
		fileMenu = editMenu = viewMenu = configMenu = performMenu = communityMenu = helpMenu = 0;
		fileToolBar = editToolBar = playToolBar = machToolBar = octToolBar_ = 0;
		newAct = openAct = saveAct = undoAct = redoAct = aboutAct = quitAct = showUnReAct = showLogConsAct = playFromStartAct = playFromSeqPosAct = playPatAct = playStopAct = togglePatBox_ = audioConfAct = 0;
		macCombo_ = sampCombo_ = octCombo_ = 0;
		undoView = 0;
		patternBox_ = 0;
		logConsole_ = 0;
		sampleBrowser_ = 0;
		views_ = 0;
		macView_ = 0;
		patView_ = 0;
		wavView_ = 0;
		seqView_ = 0;
		dock_ = dockL_ = 0;
		audioCnfDlg = 0;
		settingsDlg = 0;
		instrumentsModel_ = 0;
		playbackTimer_ = 0;

		Global::Instance();
		psy::core::Player &player = *psy::core::Player::Instance();
		// If you use a derived pluginfinder class, instantiate it before this call, and pass its address to the machinefactory Initialize function.
		psy::core::MachineFactory& mfactory = psy::core::MachineFactory::getInstance();
		mfactory.Initialize(&player);
		mfactory.setPsyclePath(Global::configuration().pluginPath());
		mfactory.setLadspaPath(Global::configuration().ladspaPath());



		song_ = createBlankSong();
		setupSound();
		psy::core::Player::Instance()->setLoopSong( true ); ///\todo: should this option should perhaps be a GUI setting, not something the player cares about?

		instrumentsModel_ = new InstrumentsModel( song_ );

		macView_ = new MachineView( song_ );
		patView_ = new PatternView( song_ );
		wavView_ = new WaveView( instrumentsModel_ );
		seqView_ = new SequencerView( song_ );
		sampleBrowser_ = new SampleBrowser( instrumentsModel_, this );
		patternBox_ = new PatternBox( song_ );
		logConsole_ = new LogConsole();

		// Playback timer to periodically update certain GUI elements
		// (e.g. sequencer bar position) to match playback position.
		playbackTimer_ = new QTimer( this );
		connect( playbackTimer_, SIGNAL( timeout() ), this, SLOT( updatePlaybackGraphics() ) );

		setupGui();
		setupSignals();
	
		///\todo: the undo framework is not implemented at all at present.
		undoStack = new QUndoStack();
		connect(undoStack, SIGNAL(canRedoChanged(bool)),
			redoAct, SLOT(setEnabled(bool)));
		connect(undoStack, SIGNAL(canUndoChanged(bool)),
			undoAct, SLOT(setEnabled(bool)));

		populateMachineCombo();
		initSampleCombo();

		///\todo: <nmather> iirc the pattern box needs to populated
		// here (rather than upon creation) but I can't remember why.
		patternBox_->populatePatternTree();
		patternBox_->patternTree()->setFocus();

		macView_->setOctave( 4 );
		patView_->setOctave( 4 );

		//setAttribute( Qt::WA_DeleteOnClose );
		createUndoView();
	}

	MainWindow::~MainWindow()
	{
		//std::cout << "~MainWindow() " << this << "\n";
	}

	void MainWindow::keyPressEvent( QKeyEvent * event )
	{
		// <nmather> IIRC, this is here to stop the default Qt behaviour for a tab press
		// (namely, cycling through focus on the widgets.)  We want to use tab for other
		// things (e.g. moving around in the pattern view.)
		if ( event->key() == Qt::Key_Tab )
			return;


		int command = Global::configuration().inputHandler().getEnumCodeByKey( Key( event->modifiers(), event->key() ) );

		switch ( command ) {
			// All actions are handled with QActions at present, but you
			// can put something here if you need to.
		default:;
		}
	}

	void MainWindow::setupSound()
	{
		psy::core::Player::Instance()->song( song_ );
		psy::core::AudioDriver *outDriver = Global::configuration()._pOutputDriver;
		psy::core::Player::Instance()->setDriver( *outDriver );
	}

	void MainWindow::setupGui()
	{
		QByteArray sheetName = settings.value( "looks/sheet", "default.qss" ).toByteArray();
		QFile file( ":/themes/" + sheetName.toLower() );
		file.open( QFile::ReadOnly );
		QString styleSheet = QLatin1String( file.readAll() );
		qApp->setStyleSheet( styleSheet );


		setAnimated( false ); // Turns off animation when moving dock widgets or toolbars.
		// (these animations were quite slow on Windows.)

		QWidget *workArea = new QWidget();

		dock_ = new QDockWidget( "Pattern Box", this );
		dock_->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
		dock_->setWidget(patternBox_);
		addDockWidget(Qt::LeftDockWidgetArea, dock_);
	
#if 0
		///\todo
		dockL_ = new QDockWidget("Logging Console", this);
		dockL_->setAllowedAreas(Qt::BottomDockWidgetArea);
		dockL_->setWidget(logConsole_);
		dockL_->setAttribute(Qt::WA_QuitOnClose, false);
		addDockWidget(Qt::BottomDockWidgetArea, dockL_);
#endif
	
		views_ = new TabWidget();
		views_->addTab( macView_, QIcon(":images/machines.png"), "Machine View" );
		views_->addTab( patView_, QIcon(":images/pattern-editor.png"), "Pattern View" );
		views_->addTab( wavView_, QIcon(":images/waveed.png"), "Wave Editor" );
		views_->addTab( seqView_, QIcon(":images/sequencer.png"),"Sequencer View" );
		///\todo not sure sample browser needs to be a permanent member of the tab bar.
		// It is accessed much less frequently than other tabs.
		views_->addTab( sampleBrowser_, QIcon(":images/sample-browser.png"), "Sample Browser" );

		QGridLayout *layout = new QGridLayout;
		layout->addWidget( views_ );
		workArea->setLayout(layout);
		setCentralWidget(workArea);

		createActions();
		createMenus();
		createToolBars();
		createStatusBar();

		setWindowTitle( " qpsycle v0.1." + QString(SUBVERSION_REVISION) );

	}

	void MainWindow::setupSignals()
	{
		connect( patternBox_, SIGNAL( patternSelectedInPatternBox( psy::core::SinglePattern* ) ), this, SLOT( onPatternSelectedInPatternBox( psy::core::SinglePattern* ) ) );
		connect( patternBox_, SIGNAL( patternDeleted() ), this, SLOT( onPatternDeleted() ) );
		connect( patternBox_, SIGNAL( addPatternToSequencerRequest( psy::core::SinglePattern* ) ), this, SLOT( onAddPatternToSequencerRequest( psy::core::SinglePattern* ) ) );
		connect( patternBox_, SIGNAL( patternNameChanged() ), this, SLOT( onPatternNameChanged() ) );
		connect( patternBox_, SIGNAL( categoryColorChanged() ), this, SLOT( onCategoryColorChanged() ) );

		connect( macView_, SIGNAL( newMachineCreated( psy::core::Machine* ) ), this, SLOT( onNewMachineCreated( psy::core::Machine* ) ) );
		connect( macView_, SIGNAL( machineChosen( MachineGui* ) ), this, SLOT( onMachineChosen( MachineGui* ) ) );
		connect( macView_, SIGNAL( machineDeleted( int ) ), this, SLOT( onMachineDeleted() ) );
		connect( macView_, SIGNAL( machineRenamed( ) ), this, SLOT( onMachineRenamed( ) ) );

		connect( macCombo_, SIGNAL( currentIndexChanged( int ) ), this, SLOT( onMachineComboBoxIndexChanged( int ) ) );

		connect( sampCombo_, SIGNAL( currentIndexChanged( int ) ), this, SLOT( onSampleComboBoxIndexChanged( int ) ) );
	}

	void MainWindow::onNewSongRequest()
	{
		if ( songHasChanged() )
		{
			int response = QMessageBox::warning( this, "Save changes?", "The song has been modified.\n Do you wish to save your changes?", QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel, QMessageBox::Save ) ;

			if ( response == QMessageBox::Save )
				onSaveSongRequest();

			if ( response == QMessageBox::Cancel )
				return;
		}
	
		psy::core::Song *blankSong = createBlankSong();
		loadSong( blankSong );
	}

	void MainWindow::onOpenSongRequest()
	{
		if ( songHasChanged() )
		{
			int response = QMessageBox::warning( this, "Save changes?", "The song has been modified.\n Do you wish to save your changes?", QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel, QMessageBox::Save ) ;

			if ( response == QMessageBox::Save )
				onSaveSongRequest();

			if ( response == QMessageBox::Cancel )
				return;
		}

		QString songPath = settings.value( "paths/songPath", "." ).toString();
		QString fileName = QFileDialog::getOpenFileName( this, "Open Song", songPath, "Psycle Songs (*.psy)" );

		if ( !fileName.isEmpty() ) {
			psy::core::Player::Instance()->stop();
			psy::core::Song *song = new psy::core::Song();
			QTextCodec::setCodecForCStrings(QTextCodec::codecForLocale());
			if( song->load(fileName.toStdString() ))
			{
				loadSong( song );
			}
			else
			{
				///\Show some error message.
			}
		}
	}

	void MainWindow::onSaveSongRequest()
	{
		QString songPath = settings.value( "paths/songPath" ).toString();
		QString fileName = QFileDialog::getSaveFileName(this,
								tr("Choose a file name"), songPath,
								tr("Psycle Songs (*.psy)"));
		if ( fileName.isEmpty() ) {
			return;
		}
		QTextCodec::setCodecForCStrings(QTextCodec::codecForLocale());
		bool success = song_->save( fileName.toStdString() );

		if (!success) {
			QMessageBox::critical(this, tr("Saving Failed!"), tr("Could not save song, for some reason!"), QMessageBox::Ok, QMessageBox::NoButton);
		}
		else {
			logConsole_->AddSuccessText("Song Saved");
		}
	}

	bool MainWindow::songHasChanged()
	{
		return true; ///\todo Can implement this once undo/redo is implemented.
	}


	psy::core::Song *MainWindow::createBlankSong()
	{
		psy::core::Song *blankSong = new psy::core::Song( );
		psy::core::PatternCategory* category0 = blankSong->patternSequence()->patternPool()->createNewCategory("New Category");
		psy::core::SinglePattern* pattern0 = category0->createNewPattern("Pattern0");

		psy::core::SequenceLine *seqLine = blankSong->patternSequence()->createNewLine();
		seqLine->createEntry( pattern0, 0 );

		return blankSong;
	}

	void MainWindow::loadSong( psy::core::Song *song )
	{
		psy::core::Player::Instance()->driver().Enable(false);
		if ( song_ ) delete song_;
		song_ = song;
		// Update gui to new song 
		///\todo this is a very crappy way of doing it for now.
		delete instrumentsModel_; instrumentsModel_ = NULL;
		delete patternBox_; patternBox_ = NULL;
		delete macView_; macView_ = NULL;
		delete patView_; patView_ = NULL;
		delete wavView_; wavView_ = NULL;
		delete seqView_; seqView_ = NULL;
		delete sampleBrowser_; sampleBrowser_ = NULL;
	
		logConsole_->Clear();

		psy::core::Player::Instance()->song( song_ );

		instrumentsModel_ = new InstrumentsModel( song_ );
		macView_ = new MachineView( song_ );
		patView_ = new PatternView( song_ );
		wavView_ = new WaveView( instrumentsModel_ );
		seqView_ = new SequencerView( song_ );
		sampleBrowser_ = new SampleBrowser( instrumentsModel_, this );
		macView_->setOctave( 4 );
		patView_->setOctave( 4 );
		views_->addTab( macView_, QIcon(":images/machines.png"), "Machine View" );
		views_->addTab( patView_, QIcon(":images/pattern-editor.png"), "Pattern View" );
		views_->addTab( wavView_, QIcon(":images/waveed.png"), "Wave Editor" );
		views_->addTab( seqView_, QIcon(":images/sequencer.png"),"Sequencer View" );
		views_->addTab( sampleBrowser_, QIcon(":images/sample-browser.png"), "Sample Browser" );
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
		psy::core::Player::Instance()->driver().Enable(true);
		logConsole_->AddSuccessText("Song Loaded Successfuly");
	}

	void MainWindow::undo()
	{
	
	}

	void MainWindow::redo()
	{
	
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

		songPropsAct_ = new QAction( tr("Song &Properties"), this );
		songPropsAct_->setStatusTip(tr("View/edit song properties"));
		connect( songPropsAct_, SIGNAL(triggered()), this, SLOT(showSongPropertiesDialog()) );

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
		connect(showUnReAct, SIGNAL(triggered()), this, SLOT(showUndoView()));
	
		showLogConsAct = new QAction(tr("&Logging Console"), this);
		showLogConsAct->setStatusTip(tr("Shows/Hides Logging Console"));
		connect(showLogConsAct, SIGNAL(triggered()), this, SLOT(showLogCons()));

		quitAct = new QAction(tr("&Quit"), this);
		quitAct->setShortcut(tr("Ctrl+Q"));
		quitAct->setStatusTip(tr("Quit the application"));
		connect(quitAct, SIGNAL(triggered()), this, SLOT(close()));

		settingsConfAct = new QAction( tr("&Settings..."), this );
		connect( settingsConfAct, SIGNAL( triggered() ), this, SLOT( showSettingsDlg() ) );



		aboutAct = new QAction(tr("&About qpsycle"), this);
		aboutAct->setStatusTip(tr("About qpsycle"));
		connect(aboutAct, SIGNAL(triggered()), this, SLOT(aboutQpsycle()));

		// View actions.
		QByteArray showPatternBoxSetting = settings.value( "keys/showPatternBox", "F1" ).toByteArray();
		QByteArray showMachineViewSetting = settings.value( "keys/showMachineView", "F2" ).toByteArray();
		QByteArray showPatternViewSetting = settings.value( "keys/showPatternView", "F3" ).toByteArray();
		QByteArray showWaveEditorSetting = settings.value( "keys/showWaveEditor", "F4" ).toByteArray();
		QByteArray showSequencerViewSetting = settings.value( "keys/showSequencerView", "F5" ).toByteArray();

		showPatternBoxAct_ = new QAction( tr("Pattern &Box"), this );
		showPatternBoxAct_->setShortcut( tr( showPatternBoxSetting.data() ) );
		connect( showPatternBoxAct_, SIGNAL( triggered() ), this, SLOT( showPatternBox() ) );

		showMachineViewAct_ = new QAction( tr("&Machine view"), this );
		showMachineViewAct_->setShortcut( tr( showMachineViewSetting.data() ) );
		connect( showMachineViewAct_, SIGNAL( triggered() ), this, SLOT( showMachineView() ) );

		showPatternViewAct_ = new QAction( tr("&Pattern view"), this );
		showPatternViewAct_->setShortcut( tr( showPatternViewSetting.data() ) );
		connect( showPatternViewAct_, SIGNAL( triggered() ), this, SLOT( showPatternView() ) );

		showWaveEditorAct_ = new QAction( tr("&Wave editor"), this );
		showWaveEditorAct_->setShortcut( tr( showWaveEditorSetting.data() ) );
		connect( showWaveEditorAct_, SIGNAL( triggered() ), this, SLOT( showWaveEditor() ) );

		showSequencerViewAct_ = new QAction( tr("&Sequencer view"), this );
		showSequencerViewAct_->setShortcut( tr( showSequencerViewSetting.data() ) );
		connect( showSequencerViewAct_, SIGNAL( triggered() ), this, SLOT( showSequencerView() ) );

		QByteArray instrumentIncSetting = settings.value( "keys/instrumentIncrement", "Ctrl+Up" ).toByteArray();
		QByteArray instrumentDecSetting = settings.value( "keys/instrumentDecrement", "Ctrl+Down" ).toByteArray();
		///\todo How to use * and / from the keypad only?  (not working at present.)
		QByteArray octaveIncSetting = settings.value( "keys/octaveIncrement", "Keypad+*" ).toByteArray();
		QByteArray octaveDecSetting = settings.value( "keys/octaveDecrement", "Keypad+/" ).toByteArray();
		QByteArray machineIncSetting = settings.value( "keys/machineIncrement", "Ctrl+Right" ).toByteArray();
		QByteArray machineDecSetting = settings.value( "keys/machineDecrement", "Ctrl+Left" ).toByteArray();


		instrumentIncAct_ = new QAction( tr( "Instrument up" ), this );
		instrumentIncAct_->setShortcut( tr( instrumentIncSetting.data() ) );
		connect( instrumentIncAct_, SIGNAL( triggered() ), this, SLOT( instrumentIncrement() ) );

		instrumentDecAct_ = new QAction( tr( "Instrument down" ), this );
		instrumentDecAct_->setShortcut( tr( instrumentDecSetting.data() ) );
		connect( instrumentDecAct_, SIGNAL( triggered() ), this, SLOT( instrumentDecrement() ) );

		octaveIncAct_ = new QAction( tr("Octave up"), this );
		octaveIncAct_->setShortcut( tr( octaveIncSetting.data() ) );
		connect( octaveIncAct_, SIGNAL( triggered() ), this, SLOT( octaveIncrement() ) );

		octaveDecAct_ = new QAction( tr("Octave down"), this );
		octaveDecAct_->setShortcut( tr( octaveDecSetting.data() ) );
		connect( octaveDecAct_, SIGNAL( triggered() ), this, SLOT( octaveDecrement() ) );

		machineDecAct_ = new QAction( tr("Machine down"), this );
		machineDecAct_->setShortcut( tr( machineDecSetting.data() ) );
		connect( machineDecAct_, SIGNAL( triggered() ), this, SLOT( machineDecrement() ) );

		machineIncAct_ = new QAction( tr("Machine down"), this );
		machineIncAct_->setShortcut( tr( machineIncSetting.data() ) );
		connect( machineIncAct_, SIGNAL( triggered() ), this, SLOT( machineIncrement() ) );
	

		// Playback actions.

		QByteArray playFromStartSetting = settings.value( "keys/playFromStart", "Shift+F6" ).toByteArray();
		QByteArray playFromSeqPosSetting = settings.value( "keys/playFromSeqPos", "F6" ).toByteArray();
		QByteArray playStopSetting = settings.value( "keys/playStop", "F8" ).toByteArray();
		QByteArray playPatSetting = settings.value( "keys/playPattern", "Shift+F6" ).toByteArray();

		playFromStartAct = new QAction(QIcon(":/images/playstart.png"), tr("&Play from start"), this);
		playFromStartAct->setShortcut( tr( playFromStartSetting.data() ) );
		connect( playFromStartAct, SIGNAL( triggered() ), this, SLOT( playFromStart() ) );

		playFromSeqPosAct = new QAction(QIcon(":/images/play.png"), tr("Play from &sequencer position"), this);
		playFromSeqPosAct->setShortcut( tr( playFromSeqPosSetting.data() ) );
		connect( playFromSeqPosAct, SIGNAL( triggered() ), this, SLOT( playFromSeqPos() ) );
		playFromSeqPosAct->setCheckable(true);

		playStopAct = new QAction(QIcon(":images/stop.png"), tr("&Stop playback"), this);
		playStopAct->setShortcut( tr( playStopSetting.data() ) );
		connect( playStopAct, SIGNAL( triggered() ), this, SLOT( playStop() ) );

		///\todo Doesn't do anything yet.
		playPatAct = new QAction(QIcon(":/images/playselpattern.png"), tr("Play selected p&attern"), this);
	}

	void MainWindow::createMenus()
	{
		fileMenu = menuBar()->addMenu(tr("&File"));
		fileMenu->addAction(newAct);
		fileMenu->addAction(openAct);
		fileMenu->addAction(saveAct);
		fileMenu->addSeparator();
		fileMenu->addAction( songPropsAct_ );
		fileMenu->addSeparator();
		fileMenu->addAction(quitAct);

		editMenu = menuBar()->addMenu(tr("&Edit"));
		editMenu->addAction(undoAct);
		editMenu->addAction(redoAct);
		editMenu->addSeparator();
		editMenu->addAction( instrumentIncAct_ );
		editMenu->addAction( instrumentDecAct_ );
		editMenu->addAction( octaveIncAct_ );
		editMenu->addAction( octaveDecAct_ );
		editMenu->addAction( machineIncAct_ );
		editMenu->addAction( machineDecAct_ );

		viewMenu = menuBar()->addMenu(tr("&View"));
		viewMenu->addAction( showPatternBoxAct_ );
		viewMenu->addAction( showMachineViewAct_ );
		viewMenu->addAction( showPatternViewAct_ );
		viewMenu->addAction( showWaveEditorAct_ );
		viewMenu->addAction( showSequencerViewAct_ );
		viewMenu->addSeparator();
		viewMenu->addAction( showUnReAct );
		viewMenu->addAction( showLogConsAct );
	
		configMenu = menuBar()->addMenu(tr("&Configuration"));
		configMenu->addAction( settingsConfAct );

		performMenu = menuBar()->addMenu(tr("&Performance"));

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
		playToolBar->addAction(playPatAct);
		playToolBar->addAction(playStopAct);

		machToolBar = addToolBar(tr("Machines"));
		macCombo_ = new QComboBox();
		macCombo_->setSizeAdjustPolicy(QComboBox::AdjustToContents);
		sampCombo_ = new QComboBox();
		sampCombo_->setSizeAdjustPolicy(QComboBox::AdjustToContents);
		sampCombo_->setMaxVisibleItems( 20 );
		octCombo_ = new QComboBox();

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
		connect( octCombo_, SIGNAL( currentIndexChanged( int ) ), this, SLOT( onOctaveComboBoxIndexChanged( int ) ) );
		octCombo_->setCurrentIndex( 4 );
	}

	void MainWindow::createStatusBar()
	{
		statusBar()->showMessage(tr("Ready"));
		statusBar()->addPermanentWidget( new QLabel( "hi" ) );
	}

	void MainWindow::populateMachineCombo()
	{
		// <nmather> I think it would be preferable to populate
		// this from a machines model that can be shared with other widgets,
		// rather than directly accessing the CoreSong.
		if (!song_) return;
		int currentIndex = macCombo_->currentIndex();

		macCombo_->clear();

		bool noMachinesLoaded=true;
		std::ostringstream buffer;
		buffer.setf( std::ios::uppercase );

		for ( int b=0; b<psy::core::MAX_BUSES; b++ ) // Generators.
		{
			if ( song_->machine(b) ) {
				buffer.str("");
				buffer << std::setfill('0') << std::hex << std::setw(2);
				buffer << b << ": " << song_->machine(b)->GetEditName();
				macCombo_->addItem( QString::fromStdString( buffer.str() ), song_->machine(b)->id() );

				noMachinesLoaded = false;
			}
		}

		macCombo_->addItem( "--------------------------");

		for ( int b=psy::core::MAX_BUSES; b<psy::core::MAX_BUSES*2; b++ ) // Effects.
		{
			if ( song_->machine(b) ) {
				buffer.str("");
				buffer << std::setfill('0') << std::hex << std::setw(2);
				buffer << b << ": " << song_->machine(b)->GetEditName();
				macCombo_->addItem( QString::fromStdString( buffer.str() ), song_->machine(b)->id() );
				noMachinesLoaded = false;
			}
		}

		if ( noMachinesLoaded ) {
			macCombo_->setItemText( 0, "No Machines Loaded" );
			currentIndex = 0;
		}

		macCombo_->setCurrentIndex( currentIndex );
		macCombo_->update();
	}

	void MainWindow::initSampleCombo()
	{
		sampCombo_->setModel( instrumentsModel_ );
		connect ( instrumentsModel_, SIGNAL(selectedInstrumentChanged(int)), sampCombo_, SLOT(setCurrentIndex(int)) );
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
		instrumentsModel_->setSelectedInstrumentIndex( newIndex );
	}

	void MainWindow::onPatternSelectedInPatternBox( psy::core::SinglePattern* selectedPattern )
	{
		patView_->setPattern( selectedPattern );
	}

	void MainWindow::onNewMachineCreated( psy::core::Machine *mac )
	{
		populateMachineCombo();
		//if ( mac->mode() == psy::core::MACHMODE_GENERATOR )
			//macCombo_->setCurrentIndex( macCombo_->findData( mac->id() ) );
		logConsole_->AddSuccessText("Machine Created Successfuly");
	}

	void MainWindow::onMachineChosen( MachineGui *macGui )
	{
		int comboIdx = macCombo_->findData( macGui->mac()->id() );
		macCombo_->setCurrentIndex( comboIdx );
	}

	void MainWindow::onMachineDeleted()
	{
		populateMachineCombo(); ///\todo: perhaps a bit unnecessary to repopulate the whole thing.
	}

	void MainWindow::onMachineRenamed()
	{
		populateMachineCombo(); ///\todo: perhaps a bit unnecessary to repopulate the whole thing.
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
		playbackTimer_->start( 10 );

		playFromSeqPosAct->setChecked(true);
		psy::core::Player::Instance()->setLoopSequenceEntry( 0 );
		psy::core::Player::Instance()->start( 0.0 );
	}

	void MainWindow::playFromSeqPos()
	{
		playbackTimer_->start( 10 );

		playFromSeqPosAct->setChecked(true);
		psy::core::Player::Instance()->start( psy::core::Player::Instance()->playPos() );
	}

	void MainWindow::playStop()
	{
		playbackTimer_->stop();

		playFromSeqPosAct->setChecked(false);
		psy::core::Player::Instance()->stop();
	}

	void MainWindow::showMachineView()
	{
		views_->setCurrentWidget( macView_ );
	}

	void MainWindow::showPatternView()
	{
		views_->setCurrentWidget( patView_ );
	}
	void MainWindow::showWaveEditor()
	{
		views_->setCurrentWidget( wavView_ );
	}
	void MainWindow::showSequencerView()
	{
		views_->setCurrentWidget( seqView_ );
	}

	void MainWindow::showSettingsDlg()
	{
		ConfigDialog configDlg;
		configDlg.exec();
	}


	void MainWindow::createUndoView()
	{
		undoView = new QUndoView( undoStack );
		undoView->setWindowTitle(tr("Undo List"));
		undoView->setAttribute(Qt::WA_QuitOnClose, false);
		undoView->setEmptyLabel("<Initial State>");
		undoView->setWindowFlags(Qt::WindowStaysOnTopHint);
	}

	void MainWindow::showUndoView() 
	{
		undoView->setVisible(!undoView->isVisible());
	}

	void MainWindow::showLogCons()
	{
		dockL_->setVisible(!dockL_->isVisible());
	}

	// Toggles through states : focus pattern box, hide pattern box, show pattern box.
	void MainWindow::showPatternBox()
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

	void MainWindow::machineIncrement()
	{
		if ( macCombo_->currentIndex() + 1 > macCombo_->count()-1 )
			macCombo_->setCurrentIndex( 0 );
		else
			macCombo_->setCurrentIndex( macCombo_->currentIndex() + 1 );
	}

	void MainWindow::machineDecrement()
	{
		if ( macCombo_->currentIndex() - 1 < 0  )
			macCombo_->setCurrentIndex( macCombo_->count()-1 );
		else
			macCombo_->setCurrentIndex( macCombo_->currentIndex() - 1 );
	}

	void MainWindow::instrumentDecrement()
	{
		sampCombo_->setCurrentIndex( std::max( 0, sampCombo_->currentIndex() - 1 ) );
	}

	void MainWindow::instrumentIncrement()
	{
		sampCombo_->setCurrentIndex( std::min( sampCombo_->currentIndex() + 1, psy::core::MAX_INSTRUMENTS-1 ) );
	}

	void MainWindow::octaveDecrement()
	{
		octCombo_->setCurrentIndex( std::max( 0, octCombo_->currentIndex() - 1 ) );
	}

	void MainWindow::octaveIncrement()
	{
		octCombo_->setCurrentIndex( std::min( octCombo_->currentIndex() + 1 , 8 ) );
	}

	void MainWindow::aboutQpsycle() 
	{
		QDialog aboutDialog( this );
		aboutDialog.setWindowTitle( "About qpsycle" );

		QVBoxLayout layout;
		layout.setAlignment( Qt::AlignCenter );
		aboutDialog.setLayout( &layout );

		QLabel aboutImageHolder;
		QImage aboutImage( ":images/qpsycle.png" );
		aboutImageHolder.setPixmap( QPixmap::fromImage( aboutImage ) );

		QLabel aboutText;
		aboutText.setText( "It makes music and stuff." );
		aboutText.setAlignment( Qt::AlignHCenter );

		layout.addWidget( &aboutImageHolder );
		layout.addWidget( &aboutText );
		aboutDialog.exec();

		///\todo Check if everything related to the about dialog gui
		// is definitely removed from memory here.
	}

	void MainWindow::showSongPropertiesDialog()
	{
		QDialog songPropsDlg( this );
		songPropsDlg.setWindowTitle( "Song properties" );

		QVBoxLayout layout;
		layout.setAlignment( Qt::AlignCenter );
		songPropsDlg.setLayout( &layout );

		QLabel songNameLabel( "Title" );
		QLabel artistNameLabel( "Composer / Credits" );
		QLabel songNotesLabel( "Extended Comments" );
		
		QLineEdit songNameEdit( &songPropsDlg );
		songNameEdit.setText( QString::fromStdString( song_->name() ) );
		QLineEdit artistNameEdit( &songPropsDlg );
		artistNameEdit.setText( QString::fromStdString( song_->author() ) );
		QTextEdit songNotesEdit( &songPropsDlg );
		songNotesEdit.setText( QString::fromStdString( song_->comment() ) );

		QWidget buttonsContainer;
		QHBoxLayout buttonsLayout;
		buttonsContainer.setLayout( &buttonsLayout );

		QPushButton cancelButton( "Cancel" );
		connect( &cancelButton, SIGNAL( clicked() ), &songPropsDlg, SLOT( reject() ) );
		QPushButton okButton( "OK" );
		connect( &okButton, SIGNAL( clicked() ), &songPropsDlg, SLOT( accept() ) );

		buttonsLayout.addWidget( &cancelButton );
		buttonsLayout.addWidget( &okButton );

		layout.addWidget( &songNameLabel );
		layout.addWidget( &songNameEdit );
		layout.addWidget( &artistNameLabel );
		layout.addWidget( &artistNameEdit );
		layout.addWidget( &songNotesLabel );
		layout.addWidget( &songNotesEdit );
		layout.addWidget( &buttonsContainer );

		int returnStatus = songPropsDlg.exec();

		if ( returnStatus == QDialog::Accepted ) 
		{
			QTextCodec::setCodecForCStrings(QTextCodec::codecForLocale());
			song_->setName( songNameEdit.text().toStdString() );
			song_->setAuthor( artistNameEdit.text().toStdString() );
			song_->setComment( songNotesEdit.toPlainText().toStdString() );

			statusBar()->showMessage( "Song properties updated." );
		} 

		///\todo Check everything related to the song dialog gui
		// is definitely removed from memory here.
	}

	void MainWindow::updatePlaybackGraphics()
	{
		if ( psy::core::Player::Instance()->playing() )
		{
			seqView_->updatePlayPos();

			psy::core::SinglePattern* visiblePattern = 0;
			visiblePattern = patView_->pattern();
			if ( visiblePattern ) 
			{
				double entryStart = 0;
				bool isPlayPattern = song_->patternSequence()->getPlayInfo( visiblePattern, psy::core::Player::Instance()->playPos() , 4 , entryStart );

				if ( isPlayPattern )
					patView_->onTick( psy::core::Player::Instance()->playPos() - entryStart ) ;
			}
		}
	}

} // namespace qpsycle

