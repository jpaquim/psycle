/***************************************************************************
*   Copyright (C) 2006 by Stefan   *
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
#include "mainwindow.h"
#include "configuration.h"
#include "song.h"
#include "player.h"
#include "defaultbitmaps.h"
#include "greetdlg.h"
#include "vumeter.h"
#include "instrumenteditor.h"
#include "infodlg.h"
#include "internal_machines.h"
#include "waveedframe.h"
#include "sequencergui.h"
#include "newmachine.h"
#include "audioconfigdlg.h"
#include "skinreader.h"
#include <iomanip>
#include <ngrs/napp.h>
#include <ngrs/nitem.h>
#include <ngrs/ncheckmenuitem.h>
#include <ngrs/nmenuseperator.h>
#include <ngrs/nmessagebox.h>
#include <ngrs/nbevelborder.h>
#include <ngrs/nstatusbar.h>
#include <ngrs/ntextstatusitem.h>
#include <ngrs/nfiledialog.h>
#include <ngrs/nsystem.h>
#include <ngrs/ntoolbarpanel.h>

namespace psycle { 
  namespace host {

    template<class T> inline T str_hex(const std::string &  value) {
      T result;

      std::stringstream str;
      str << value;
      if (! (str >> std::hex >> result) ) return -1;

      return result;
    }

    // progressbar subclassing

    ProgressStatusItem::ProgressStatusItem() {
      setLayout( ngrs::NAlignLayout() );
      progressBar_ = new ngrs::NProgressBar();
      progressBar_->setValue(0);
      progressBar_->setMax(16385);
      progressBar_->setPreferredSize(200,10);
      add( progressBar_, ngrs::nAlClient );
    }

    ProgressStatusItem::~ProgressStatusItem() {
    }

    void ProgressStatusItem::setText( const std::string & text ) {
      int value;
      if ( std::istringstream( text ) >> value ) {
        std::cout << value << std::endl;
        progressBar_->setValue( value );
        progressBar_->repaint();
      }
    }

    // progressbar subclassing end

    MainWindow::MainWindow()
      : ngrs::NWindow(), pluginFinder_( *Global::pConfig() )
    { 
      setTitle ("] Psycle Modular Music Creation Studio [ ( X alpha ) ");

      setPosition( 10, 10, 800, 600 );

      SkinReader::Instance()->setDefaults();

      count = 0;

      initMenu();
      initBars();
      initDialogs();

      book = new ngrs::NTabBook();
      book->setTabBarVisible( false );
      pane()->add(book,ngrs::nAlClient);

      initStartPage();

      newMachineDlg_ = new NewMachine( pluginFinder_ );
      add(newMachineDlg_);

      audioConfigDlg = new AudioConfigDlg( Global::pConfig() );
      add( audioConfigDlg );	

      selectedChildView_ = 0;
      enableSound();

      oldPlayPos_ = 0;
      timer.setIntervalTime(10);
      timer.timerEvent.connect(this,&MainWindow::onTimer);
      timer.enableTimer();

    }

    MainWindow::~MainWindow()
    {
    }

    void MainWindow::initStartPage() {
      ngrs::NImage* img;

      ngrs::NPanel* test = new ngrs::NPanel();
      test->setLayout( ngrs::NAlignLayout() );
      ngrs::NPanel* logoPnl = new ngrs::NPanel();
      logoPnl->setLayout( ngrs::NAlignLayout() );
      logoPnl->setSpacing( ngrs::NSize( 10,5,10,5 ) );
      img = new ngrs::NImage( Global::pConfig()->icons().logoRight()  );
      img->setVAlign( ngrs::nAlCenter );
      logoPnl->add( img , ngrs::nAlRight );
      img = new ngrs::NImage( Global::pConfig()->icons().logoLeft()  );
      img->setVAlign( ngrs::nAlCenter );
      logoPnl->add( img , ngrs::nAlLeft );
      img = new ngrs::NImage( Global::pConfig()->icons().logoMid()  );
      img->setVAlign( ngrs::nAlCenter );
      img->setHAlign( ngrs::nAlWallPaper );
      logoPnl->add( img , ngrs::nAlClient );
      test->add( logoPnl, ngrs::nAlTop );

      ngrs::NGroupBox* gBox = new ngrs::NGroupBox();
      gBox->setLayout( ngrs::NAlignLayout() );
      gBox->setHeaderText("Recent Songs");
      ngrs::NPanel* recentSongOptionPanel = new ngrs::NPanel();
      recentSongOptionPanel->setLayout( ngrs::NAlignLayout() ) ;
      recentSongOptionPanel->add( new ngrs::NLabel("open"), ngrs::nAlTop);
      recentSongOptionPanel->add( new ngrs::NLabel("new"), ngrs::nAlTop);
      gBox->add( recentSongOptionPanel, ngrs::nAlBottom );
      ngrs::NListBox* recentSongListBox = new ngrs::NListBox();	  
      recentSongListBox->scrollPane()->setSkin( pane()->skin_);
      recentSongListBox->setPreferredSize(250,300);
      gBox->add( recentSongListBox, ngrs::nAlClient );
      test->add( gBox, ngrs::nAlLeft );

      ngrs::NGroupBox* gBox1 = new ngrs::NGroupBox("Psycledelics Community : Psycle XI");
      gBox1->setLayout( ngrs::NAlignLayout() );
      ngrs::NLabel* lb = new ngrs::NLabel("10, Wed, Startpage meets Tip of the Day Dialog in the afternoon");
      lb->setWordWrap( true );
      gBox1->add( lb, ngrs::nAlTop);
      test->add( gBox1,ngrs::nAlClient );

      book->addPage( test, "Start Page" );
    }


    void MainWindow::enableSound( )
    {
      AudioDriver* pOut = Global::pConfig()->_pOutputDriver;
      Player::Instance()->setDriver( *pOut );  
    }

    ChildView* MainWindow::addChildView()
    {
      ChildView* childView_ = new ChildView( );
      childView_->newMachineAdded.connect(this, &MainWindow::onNewMachineDialogAdded);
      childView_->sequencerView()->entryClick.connect(this,&MainWindow::onSequencerEntryClick);
      childView_->machineSelected.connect(this,&MainWindow::onMachineSelected);
      childView_->machineViewDblClick.connect(this,&MainWindow::onNewMachine);
      childView_->waveEditor()->updateInstrumentCbx.connect(this,&MainWindow::onUpdateInstrumentCbx);
      childView_->machineView()->machineDeleted.connect(this,&MainWindow::onMachineDeleted);
      childView_->machineView()->machineNameChanged.connect(this, &MainWindow::onMachineNameChanged);
      book->addPage( childView_, childView_->song()->name() + stringify( count ) );
      book->setActivePage( childView_ );

      count++;

      ngrs::NTab* tab = book->tab( childView_ );
      tab->click.connect(this,&MainWindow::onTabChange);
      tab->setEnablePopupMenu(true);
      ngrs::NPopupMenu* menu = tab->popupMenu();
      ngrs::NMenuItem* closeSong = new ngrs::NMenuItem("Close");
      menu->add( closeSong );
      closeSong->click.connect(this,&MainWindow::onCloseSongTabPressed);

      songMap[closeSong] = childView_;
      songTabMap[tab] = childView_;

      songpDlg_->setSong( childView_->song() );

      if (songMap.size() > 0) book->setTabBarVisible(true);

      Player::Instance()->song( childView_->song() );

      selectedChildView_ =  childView_;

      return childView_;
    } 

    void MainWindow::onCloseSongTabPressed( ngrs::NButtonEvent* ev ) {
      std::map<NObject*,ChildView*>::iterator it = songMap.find( ev->sender() ); 
      if ( it != songMap.end() ) {
        std::map<NObject*,ChildView*>::iterator tabIt = songTabMap.begin();
        for ( ; tabIt != songTabMap.end() ; tabIt++ ) {
          if ( tabIt->second == it->second ) {
            songTabMap.erase( tabIt );
            break;
          }
        }

        bool update = false;
        ChildView* view = it->second;
        if (view == selectedChildView_) {
          Player::Instance()->stop();
          Player::Instance()->song(0);       
          selectedChildView_ = 0;
          update = true;
        }
        songMap.erase(it);
        book->removePage(view);

        if (update) {
          std::map<NObject*,ChildView*>::iterator songIt = songTabMap.begin();
          for ( ; songIt != songTabMap.end() ; songIt++ ) {
            if ( songIt->second == book->activePage() ) {
              selectedChildView_ = songIt->second;         
              Player::Instance()->song( selectedChildView_->song() );
              updateNewSong();
            } 
          }
        } 

        if (songMap.size() <= 1) {
          book->setTabBarVisible(false);
        }
        pane()->resize();
        pane()->repaint();
      }
    }

    void MainWindow::onTabChange( ngrs::NButtonEvent * ev )
    {
      std::map<NObject*,ChildView*>::iterator it = songTabMap.find( ev->sender() ); 
      if ( it != songTabMap.end() ) {
        ChildView* view = it->second;
        Player::Instance()->stop();
        Player::Instance()->song( view->song() );
        selectedChildView_ = view;
        updateNewSong();
        pane()->repaint();
      }
    }


    void MainWindow::onConfigMenuAudio( ngrs::NButtonEvent* ev ) {
      audioConfigDlg->setVisible( true );
    }

    void MainWindow::initMenu( )
    {
      menuBar_ = new ngrs::NMenuBar();
      pane()->add( menuBar_ );

      DefaultBitmaps & icons = Global::pConfig()->icons();

      // Creates the file menu
      fileMenu_ = new ngrs::NMenu("File");
      fileMenu_->add(new ngrs::NMenuItem("New",icons.newfile() ))->click.connect(this,&MainWindow::onFileNew);
      fileMenu_->add(new ngrs::NMenuItem("Open",icons.open()))->click.connect(this,&MainWindow::onFileOpen);
      fileMenu_->add(new ngrs::NMenuItem("Import Module"));
      fileMenu_->add(new ngrs::NMenuItem("Save",icons.save()));
      fileMenu_->add(new ngrs::NMenuItem("Save as"))->click.connect(this,&MainWindow::onFileSaveAs);
      fileMenu_->add(new ngrs::NMenuItem("Render as Wav"))->click.connect(this,&MainWindow::onRenderAsWave);
      fileMenu_->add(new ngrs::NMenuSeperator());
      fileMenu_->add(new ngrs::NMenuItem("Song properties"))->click.connect(this,&MainWindow::showSongpDlg);
      fileMenu_->add(new ngrs::NMenuSeperator());
      fileMenu_->add(new ngrs::NMenuItem("revert to saved"));
      ngrs::NMenuItem* recentItem = new ngrs::NMenuItem("recent files");
      fileMenu_->add(recentItem);
      recentFileMenu_ = new ngrs::NMenu();
      recentItem->add(recentFileMenu_);
      noneFileItem = new ngrs::NMenuItem("none");
      noneFileItem->setEnable(false);
      recentFileMenu_->add(noneFileItem);
      noFileWasYetLoaded = true;
      fileMenu_->add(new ngrs::NMenuItem("exit"))->click.connect(this,&MainWindow::onFileExit);
      menuBar_->add(fileMenu_);

      // Creates the edit menu
      editMenu_ = new ngrs::NMenu("Edit");
      editMenu_->add(new ngrs::NMenuItem("Undo",icons.undo()))->click.connect(this,&MainWindow::onEditUndo);
      editMenu_->add(new ngrs::NMenuItem("Redo",icons.redo()))->click.connect(this,&MainWindow::onEditRedo);
      editMenu_->add(new ngrs::NMenuSeperator());
      editMenu_->add(new ngrs::NMenuItem("Pattern Cut"))->click.connect(this,&MainWindow::onEditPatternCut);
      editMenu_->add(new ngrs::NMenuItem("Pattern Copy"))->click.connect(this,&MainWindow::onEditPatternCopy);
      editMenu_->add(new ngrs::NMenuItem("Pattern Paste"))->click.connect(this,&MainWindow::onEditPatternPaste);
      editMenu_->add(new ngrs::NMenuItem("Pattern Mix"))->click.connect(this,&MainWindow::onEditPatternMix);
      editMenu_->add(new ngrs::NMenuItem("Pattern Mix Paste"))->click.connect(this,&MainWindow::onEditPatternMixPaste);
      editMenu_->add(new ngrs::NMenuItem("Pattern Delete"))->click.connect(this,&MainWindow::onEditPatternDelete);
      editMenu_->add(new ngrs::NMenuSeperator());
      editMenu_->add(new ngrs::NMenuItem("Block Cut"))->click.connect(this,&MainWindow::onEditBlockCut);
      editMenu_->add(new ngrs::NMenuItem("Block Copy"))->click.connect(this,&MainWindow::onEditBlockCopy);
      editMenu_->add(new ngrs::NMenuItem("Block Paste"))->click.connect(this,&MainWindow::onEditBlockPaste);
      editMenu_->add(new ngrs::NMenuItem("Block Mix Paste"))->click.connect(this,&MainWindow::onEditBlockMixPaste);
      editMenu_->add(new ngrs::NMenuItem("Block Delete"))->click.connect(this,&MainWindow::onEditBlockDelete);
      editMenu_->add(new ngrs::NMenuSeperator());
      editMenu_->add(new ngrs::NMenuItem("Sequence Cut"))->click.connect(this,&MainWindow::onEditSeqCut);
      editMenu_->add(new ngrs::NMenuItem("Sequence Copy"))->click.connect(this,&MainWindow::onEditSeqCopy);
      editMenu_->add(new ngrs::NMenuItem("Sequence Delete"))->click.connect(this,&MainWindow::onEditSeqDelete);
      menuBar_->add(editMenu_);

      // Creates the view menu
      viewMenu_ = new ngrs::NMenu("View");
      viewMenu_->add(new ngrs::NCheckMenuItem("Toolbar"))->click.connect(this,&MainWindow::onViewMenuToolbar);
      viewMenu_->add(new ngrs::NCheckMenuItem("Machinebar"))->click.connect(this,&MainWindow::onViewMenuMachinebar);
      viewMenu_->add(new ngrs::NCheckMenuItem("Sequencerbar"))->click.connect(this,&MainWindow::onViewMenuSequencerbar);
      viewMenu_->add(new ngrs::NCheckMenuItem("Statusbar"))->click.connect(this,&MainWindow::onViewMenuStatusbar);
      viewMenu_->add(new ngrs::NMenuSeperator());
      viewMenu_->add(new ngrs::NMenuItem("PatternEditor",icons.patterns()))->click.connect(this,&MainWindow::onPatternView);
      viewMenu_->add(new ngrs::NMenuItem("MachineView",icons.machines()))->click.connect(this,&MainWindow::onMachineView);
      viewMenu_->add(new ngrs::NMenuItem("PatternSequencer",icons.sequencer()))->click.connect(this,&MainWindow::onSequencerView);
      viewMenu_->add(new ngrs::NMenuSeperator());
      viewMenu_->add(new ngrs::NMenuItem("Add machine",icons.newmachine()))->click.connect(this,&MainWindow::onNewMachine);
      viewMenu_->add(new ngrs::NMenuSeperator());
      //      viewMenu_->add(new ngrs::NMenuItem("Instrument Editor"))->click.connect(this,&MainWindow::onEditInstrument);
      menuBar_->add(viewMenu_);

      // Creates the configuration menu
      configurationMenu_ = new ngrs::NMenu("Configuration");
      configurationMenu_->add(new ngrs::NMenuItem("Free Audio"));
      configurationMenu_->add(new ngrs::NMenuItem("Autostop"));
      configurationMenu_->add(new ngrs::NMenuSeperator());
      configurationMenu_->add(new ngrs::NMenuItem("Loop Playback"));
      configurationMenu_->add(new ngrs::NMenuSeperator());
      configurationMenu_->add(new ngrs::NMenuItem("Audio Settings"))->click.connect(this,&MainWindow::onConfigMenuAudio);
      configurationMenu_->add(new ngrs::NMenuItem("Load Skin"))->click.connect(this,&MainWindow::onConfigMenuSkin);
      menuBar_->add(configurationMenu_);

      // Creates the performance menu
      performanceMenu_ = new ngrs::NMenu("Performance");
      performanceMenu_->add(new ngrs::NMenuItem("CPU Monitor"));
      performanceMenu_->add(new ngrs::NMenuItem("MDI Monitor"));
      menuBar_->add(performanceMenu_);

      // Creates the community menu
      communityMenu_ = new ngrs::NMenu("Community");
      ngrs::NMenuItem* comSearchItem = new ngrs::NMenuItem("Community Search");	  
      communityMenu_->add( new ngrs::NMenuItem("Ask a Question") );
      communityMenu_->add( new ngrs::NMenuItem("Sent Feedback" ) );
      communityMenu_->add( new ngrs::NMenuSeperator() );
      communityMenu_->add( new ngrs::NMenuItem("Psycledelics Center") );
      communityMenu_->add( new ngrs::NMenuSeperator() );
      communityMenu_->add( comSearchItem );
      ngrs::NMenu* comSearch = new ngrs::NMenu();
      comSearch->add( new ngrs::NMenuItem("Songs") );
      comSearch->add( new ngrs::NMenuItem("Presets") );
      comSearch->add( new ngrs::NMenuItem("Skins") );
      comSearch->add( new ngrs::NMenuItem("Samples") );
      comSearchItem->add( comSearch );	
      menuBar_->add( communityMenu_ );

      // Creates the help menu
      helpMenu_ = new ngrs::NMenu("Help");
      helpMenu_->add(new ngrs::NMenuItem("About"))->click.connect(this,&MainWindow::onHelpMenuAbout);
      helpMenu_->add(new ngrs::NMenuItem("Greetings"))->click.connect(this,&MainWindow::onHelpMenuGreeting);
      helpMenu_->add(new ngrs::NMenuSeperator());
      helpMenu_->add(new ngrs::NMenuItem("readme"))->click.connect(this,&MainWindow::onHelpMenuReadme);
      helpMenu_->add(new ngrs::NMenuItem("keys"))->click.connect(this,&MainWindow::onHelpMenuKeys);
      helpMenu_->add(new ngrs::NMenuItem("tweaking"))->click.connect(this,&MainWindow::onHelpMenuTweaking);
      helpMenu_->add(new ngrs::NMenuItem("whatsnew"))->click.connect(this,&MainWindow::onHelpMenuWhatsNew);
      menuBar_->add(helpMenu_);
    }


    void MainWindow::initDialogs( )
    {
      // creates the song dialog for editing song name, author, and comment
      add( songpDlg_ = new SongpDlg( ) );
      // creates the greeting dialog, that greets people who help psycle development
      add( greetDlg =  new GreetDlg() );
      // creates the save dialog, that ask where to store wave files, recorded from playing a psy song
      wavRecFileDlg = new ngrs::NFileDialog();
      wavRecFileDlg->setMode( ngrs::nSave );
      add( wavRecFileDlg );
      // creates the info dialog, that displays in a memo readme keys tweaking and a whatsnew file
      add( infoDlg =  new InfoDlg() );
    }

    // events from menuItems

    void MainWindow::showSongpDlg( ngrs::NButtonEvent* ev )
    {
      if ( !selectedChildView_) return;

      songpDlg_->setSong( selectedChildView_->song() );
      songpDlg_->setVisible(true);
    }

    void MainWindow::initBars( )
    {    
      initToolBar();
      initStatusBar();
    }

    void MainWindow::initStatusBar() {
      setStatusModel( statusBarData );
      statusBar_ = new ngrs::NStatusBar();    
      ProgressStatusItem* progressBar_ = new ProgressStatusItem();
      progressBar_->setModelIndex( 1 );
      statusBar_->add( progressBar_, ngrs::nAlLeft );	    
      for ( unsigned int i = 5; i >= 2; i-- )
        statusBar_->add( new ngrs::NTextStatusItem(i) );    
      statusBar_->setModel( statusBarData );
      statusBarData.setText( "Ready", 0 );
      statusBarData.setText( "Ln 0", 2 );
      statusBarData.setText( "Tr 0", 3 );
      statusBarData.setText( "Play 00:00:00:00", 4 );
      pane()->add( statusBar_, ngrs::nAlBottom );
    }

    void MainWindow::initToolBar( )
    {
      toolBarPanel_ = new ngrs::NToolBarPanel();    
      pane()->add(toolBarPanel_, ngrs::nAlTop);

      DefaultBitmaps & icons = Global::pConfig()->icons();

      ngrs::NImage* img;

      toolBar1_ = new ngrs::NToolBar();
      toolBarPanel_->add(toolBar1_);

      // creates the newfile button
      img = new ngrs::NImage();
      img->setSharedBitmap(&icons.newfile());
      img->setPreferredSize(25,25);
      ngrs::NButton* newBtn = new ngrs::NButton(img);
      newBtn->setHint("New song");
      toolBar1_->add(newBtn)->clicked.connect(this,&MainWindow::onFileNew);


      img = new ngrs::NImage();
      img->setSharedBitmap(&icons.open());
      img->setPreferredSize(25,25);
      ngrs::NButton* fileOpenBtn = new ngrs::NButton(img);
      fileOpenBtn->setHint("Song load");
      toolBar1_->add(fileOpenBtn)->clicked.connect(this,&MainWindow::onFileOpen);


      img = new ngrs::NImage();
      img->setSharedBitmap(&icons.save());
      img->setPreferredSize(25,25);
      ngrs::NButton* saveBtn = new ngrs::NButton(img);
      saveBtn->setHint("Save");
      toolBar1_->add(saveBtn)->clicked.connect(this,&MainWindow::onFileSaveAs);;

      img = new ngrs::NImage();
      img->setSharedBitmap(&icons.save_audio());
      img->setPreferredSize(25,25);
      ngrs::NButton* saveAsAudioFileBtn = new ngrs::NButton(img);
      saveAsAudioFileBtn->setHint("Save as audio file");
      toolBar1_->add(saveAsAudioFileBtn);

      img = new ngrs::NImage();
      img->setSharedBitmap(&icons.recordwav());
      img->setPreferredSize(25,25);
      ngrs::NButton* recWav = new ngrs::NButton(img);
      recWav->setToggle(true);
      recWav->setFlat(false);
      recWav->setHint("Record to .wav");
      recWav->clicked.connect(this, &MainWindow::onRecordWav);
      toolBar1_->add(recWav);

      toolBar1_->add(new ngrs::NToolBarSeparator());

      img = new ngrs::NImage();
      img->setSharedBitmap(&icons.undo());
      img->setPreferredSize(25,25);
      toolBar1_->add(new ngrs::NButton(img));

      img = new ngrs::NImage();
      img->setSharedBitmap(&icons.redo());
      img->setPreferredSize(25,25);
      toolBar1_->add(new ngrs::NButton(img));

      toolBar1_->add(new ngrs::NToolBarSeparator());

      img = new ngrs::NImage();
      img->setSharedBitmap(&icons.recordnotes());
      img->setPreferredSize(25,25);
      ngrs::NButton* recNotes = new ngrs::NButton(img);
      recNotes->setHint("Record Notes Mode");
      recNotes->clicked.connect(this,&MainWindow::onRecordNotesMode);
      toolBar1_->add(recNotes);

      toolBar1_->add(new ngrs::NToolBarSeparator());

      img = new ngrs::NImage();
      img->setSharedBitmap(&icons.playstart());
      img->setPreferredSize(25,25);
      barPlayFromStartBtn_ = new ngrs::NButton(img);
      barPlayFromStartBtn_->click.connect(this,&MainWindow::onBarPlayFromStart);
      barPlayFromStartBtn_->setHint("Play from start");
      toolBar1_->add(barPlayFromStartBtn_);


      img = new ngrs::NImage();
      img->setSharedBitmap(&icons.play());
      img->setPreferredSize(25,25);
      ngrs::NButton* playBtn = new ngrs::NButton(img);
      playBtn->setHint("Play from edit position");
      toolBar1_->add(playBtn)->clicked.connect(this,&MainWindow::onBarPlay);

      img = new ngrs::NImage();
      img->setSharedBitmap(&icons.playselpattern());
      img->setPreferredSize(25,25);
      toolBar1_->add(new ngrs::NButton(img));

      img = new ngrs::NImage();
      img->setSharedBitmap(&icons.stop());
      img->setPreferredSize(25,25);
      ngrs::NButton* stopBtn_ = new ngrs::NButton(img);
      stopBtn_->click.connect(this,&MainWindow::onBarStop);
      stopBtn_->setHint("Stop");
      toolBar1_->add(stopBtn_);

      img = new ngrs::NImage();
      img->setSharedBitmap(&icons.autoStop());
      img->setPreferredSize(25,25);
      toolBar1_->add(new ngrs::NButton(img));

      toolBar1_->add(new ngrs::NToolBarSeparator());

      img = new ngrs::NImage();
      img->setSharedBitmap(&icons.newmachine());
      img->setPreferredSize(25,25);
      ngrs::NButton* newMacBtn = new ngrs::NButton(img);
      newMacBtn->setHint("New Machine");
      toolBar1_->add(newMacBtn);//->clicked.connect(childView_,&ChildView::onMachineViewDblClick);

      img = new ngrs::NImage();
      img->setSharedBitmap(&icons.openeditor());
      img->setPreferredSize(25,25);
      /*  ngrs::NButton* editInsBtn = new ngrs::NButton(img);
      editInsBtn->setHint("Edit Instrument");
      toolBar1_->add(editInsBtn)->clicked.connect(this,&MainWindow::onEditInstrument);*/

      toolBar1_->add(new ngrs::NToolBarSeparator());

      img = new ngrs::NImage();
      img->setSharedBitmap(&icons.p());
      img->setPreferredSize(25,25);
      toolBar1_->add(new ngrs::NButton(img));

      toolBar1_->add(new ngrs::NToolBarSeparator());

      img = new ngrs::NImage();
      img->setSharedBitmap(&icons.help());
      img->setPreferredSize(25,25);
      toolBar1_->add(new ngrs::NButton(img));

      toolBar1_->resize();

      psycleControlBar_ = new ngrs::NToolBar();
      psycleControlBar_->add(new ngrs::NLabel("Tempo"));

      img = new ngrs::NImage();
      img->setSharedBitmap(&icons.lessless());
      img->setPreferredSize(25,25);
      ngrs::NButton* bpmDecBtnTen = new ngrs::NButton(img);
      bpmDecBtnTen->setFlat(false);
      psycleControlBar_->add(bpmDecBtnTen)->clicked.connect(this,&MainWindow::onBpmDecTen);

      img = new ngrs::NImage();
      img->setSharedBitmap(&icons.less());
      img->setPreferredSize(25,25);
      ngrs::NButton* bpmDecBtnOne = new ngrs::NButton(img);
      bpmDecBtnOne->setFlat(false);
      psycleControlBar_->add(bpmDecBtnOne)->clicked.connect(this,&MainWindow::onBpmDecOne);

      bpmDisplay_ = new ngrs::N7SegDisplay(3);
      bpmDisplay_->setColors( ngrs::NColor(250,250,250), ngrs::NColor(100,100,100), ngrs::NColor(230,230,230));
      bpmDisplay_->setNumber( 125 );
      psycleControlBar_->add( bpmDisplay_ );

      img = new ngrs::NImage();
      img->setSharedBitmap(&icons.more());
      img->setPreferredSize( 25, 25 );

      ngrs::NButton* bpmIncBtnOne = new ngrs::NButton(img);
      bpmIncBtnOne->setFlat(false);
      psycleControlBar_->add(bpmIncBtnOne)->clicked.connect(this,&MainWindow::onBpmIncOne);

      img = new ngrs::NImage();
      img->setSharedBitmap(&icons.moremore());
      img->setPreferredSize(25,25);
      ngrs::NButton* moremoreBmp = new ngrs::NButton(img);
      moremoreBmp->setFlat(false);
      moremoreBmp->clicked.connect(this,&MainWindow::onBpmAddTen);
      psycleControlBar_->add(moremoreBmp);

      psycleControlBar_->add(new ngrs::NLabel("VU"));
      ngrs::NPanel* vuPanel = new ngrs::NPanel();
      vuPanel->setPreferredSize( 225, 10 );
      vuMeter_ = new VuMeter();
      vuPanel->add(vuMeter_);
      vuMeter_->setPosition(0,0,225,10);

      masterSlider_ = new ngrs::NSlider();
      masterSlider_->setOrientation(ngrs::nHorizontal);
      masterSlider_->setPosition(0,10,225,10);
      vuPanel->add(masterSlider_);
      psycleControlBar_->add(vuPanel);

      toolBarPanel_->add(psycleControlBar_);

      psycleToolBar_ = new ngrs::NToolBar();
      psycleToolBar_->add(new ngrs::NToolBarSeparator());
      genCombo_ = new ngrs::NComboBox();
      genCombo_->setPreferredSize( 158, 20 );
      genCombo_->setIndex(0);
      genCombo_->enableFocus(false);
      genCombo_->itemSelected.connect(this,&MainWindow::onGeneratorCbx);
      psycleToolBar_->add(genCombo_);

      img = new ngrs::NImage();
      img->setSharedBitmap(&icons.littleleft());
      img->setPreferredSize(25,25);
      psycleToolBar_->add(new ngrs::NButton(img));

      img = new ngrs::NImage();
      img->setSharedBitmap(&icons.littleright());
      img->setPreferredSize(25,25);
      psycleToolBar_->add(new ngrs::NButton(img));

      psycleToolBar_->add(new ngrs::NButton("Gear Rack"));

      insCombo_ = new ngrs::NComboBox();
      insCombo_->setPreferredSize( 158, 20 );
      insCombo_->enableFocus(false);
      insCombo_->itemSelected.connect(this,&MainWindow::onInstrumentCbx);
      for (int i=0;i<PREV_WAV_INS;i++)
      {
        std::ostringstream buffer;
        buffer.str("");
        buffer << std::setfill('0') << std::hex << std::setw(2);
        buffer << i << ": " << "empty";
        insCombo_->add(new ngrs::NItem(buffer.str()));
      }
      psycleToolBar_->add(insCombo_);


      toolBarPanel_->add(psycleToolBar_);

      toolBarPanel_->resize();
    }

    void MainWindow::onConfigMenuSkin( ngrs::NButtonEvent* ev ) {
      // add and create the temporay fileDialog
      ngrs::NFileDialog* openDialog = new ngrs::NFileDialog();
      openDialog->addFilter("*.zip [xpsy skin format]","!S*.zip");
      add( openDialog );

      if ( openDialog->execute() ) {
        std::string fileName = openDialog->fileName();
        SkinReader::Instance()->loadSkin( fileName );

        if ( selectedChildView_ ) {
          selectedChildView_->patternView()->updateSkin();
          selectedChildView_->machineView()->updateSkin();
        }			

        pane()->resize();
        pane()->repaint();	
      }

      // remove the temp Dialog
      erase( openDialog );
      ngrs::NApp::addRemovePipe( openDialog );
    }

    void MainWindow::onInstrumentCbx( ngrs::NItemEvent * ev )
    {
      if ( !selectedChildView_) return;

      int index = insCombo_->selIndex();
      selectedChildView_->song()->instSelected=   index;
      selectedChildView_->song()->auxcolSelected= index;
      selectedChildView_->waveEditor()->Notify();
    }

    void MainWindow::updateComboIns( bool updatelist )
    {
      if ( !selectedChildView_) return;

      if (updatelist)  {
        std::ostringstream buffer;
        buffer.setf(std::ios::uppercase);

        int listlen = 0;
        for (int i=0;i<PREV_WAV_INS;i++)
        {
          buffer.str("");
          buffer << std::setfill('0') << std::hex << std::setw(2);
          buffer << i << ": " << selectedChildView_->song()->_pInstrument[i]->_sName;
          insCombo_->itemAt(i)->setText( buffer.str());
          listlen++;
        }
        if (selectedChildView_->song()->auxcolSelected >= listlen) {
          selectedChildView_->song()->auxcolSelected = 0;
        }    
      }
    }



    void MainWindow::onBarPlay( ngrs::NButtonEvent * ev )
    {
      if ( !selectedChildView_ ) return;
      selectedChildView_->play();
    }

    void MainWindow::onBarPlayFromStart( ngrs::NButtonEvent * ev )
    {
      if ( !selectedChildView_ ) return;
      selectedChildView_->playFromStart();
    }

    void MainWindow::onFileNew( ngrs::NButtonEvent * ev )
    {
      addChildView();  
      pane()->resize();
      pane()->repaint();
    }

    void MainWindow::onFileOpen( ngrs::NButtonEvent * ev )
    {	
      // add and create the temporay fileDialog
      ngrs::NFileDialog* openDialog = new ngrs::NFileDialog();
      openDialog->addFilter("*.psy [psy3 song format]","!S*.psy");
      add( openDialog );

      if ( openDialog->execute() ) {

        statusBarData.setText("searching for \"" + openDialog->fileName() + "\"");
        std::string fileName = openDialog->fileName();
        //  progressBar_->setVisible(true);
        //pane()->resize();
        // pane()->repaint();
        if ( fileName != "" ) {
          // stop player
          Player::Instance()->stop();
          songpDlg_->setVisible(false);
          // disable audio driver
          //Global::configuration()._pOutputDriver->Enable(false);
          // add a new Song tab
          ChildView* newView = addChildView();  
          newView->song()->progress.connect(this,&MainWindow::onSongLoadProgress);
          // load the song
          statusBarData.setText("loading \"" + openDialog->fileName() + "\"" );
          newView->song()->load(fileName);
          // update gui to new song
          newView->song()->progress.disconnect_all();
          newView->update();
          updateNewSong();
          pane()->resize();
          pane()->repaint();
          // enable audio driver
          //Global::configuration()._pOutputDriver->Enable(true);
          // update file recent open sub menu
          if (noFileWasYetLoaded) {
            recentFileMenu_->removeChilds();
            noFileWasYetLoaded = false;
          }
          recentFileMenu_->add(new ngrs::NMenuItem(fileName));
          statusBarData.setText("loaded \"" + openDialog->fileName() + "\"");
        }
        //progressBar_->setVisible(false);
      }

      // remove the "Open"-FileDialog
      erase( openDialog );
      ngrs::NApp::addRemovePipe( openDialog );
    }

    void MainWindow::onFileSave( ngrs::NButtonEvent * ev )
    {

    }

    void MainWindow::onFileSaveAs( ngrs::NButtonEvent * ev )
    {
      if ( !selectedChildView_ ) return;

      ngrs::NFileDialog* saveDialog = new ngrs::NFileDialog();
      saveDialog->addFilter("*.psy [psy4 song format]","!S*.psy");
      saveDialog->setMode(ngrs::nSave);
      add( saveDialog );

      if ( saveDialog->execute() ) {
        selectedChildView_->song()->save( saveDialog->fileName() );
      }

      //progressBar_->setVisible(true);
      //childView_->onFileSaveSong(0);  
      //progressBar_->setVisible(false);

      erase( saveDialog );
      ngrs::NApp::addRemovePipe( saveDialog );
    }

    void MainWindow::onSongLoadProgress( const std::uint32_t & a, const std::uint32_t & b , const std::string & t)
    {
      if (a == 4) {
        std::cout << "progress" << std::endl;
        statusBarData.setText( stringify( (int) b), 1 );
        ngrs::NApp::flushEventQueue();
      } else
        if (a == 2) {
          statusBarData.setText( t );
        }
    }

    void MainWindow::onBarStop( ngrs::NButtonEvent* ev )
    {
      bool pl = Player::Instance()->playing();
      Player::Instance()->stop();
    }

    void MainWindow::closePsycle()
    {
      exit(0);
    }


    void MainWindow::updateComboGen() {

      if (!selectedChildView_) return;

      Song* selectedSong_  = selectedChildView_->song();

      bool filled=false;
      bool found=false;
      int selected = -1;
      int line = -1;
      std::ostringstream buffer;
      buffer.setf(std::ios::uppercase);

      genCombo_->removeChilds();

      for (int b=0; b<MAX_BUSES; b++) // Check Generators
      {
        if( selectedSong_->_pMachine[b]) {
          buffer.str("");
          buffer << std::setfill('0') << std::hex << std::setw(2);
          buffer << b << ": " << selectedSong_->_pMachine[b]->_editName;
          genCombo_->add(new ngrs::NItem(buffer.str()));

          //cb->SetItemData(cb->GetCount()-1,b);
          if (!found) selected++;
          if (selectedSong_->seqBus == b) found = true;
          filled = true;
        }
      }

      genCombo_->add(new ngrs::NItem("----------------------------------------------------"));
      //cb->SetItemData(cb->GetCount()-1,65535);
      if (!found)  {
        selected++;
        line = selected;
      }

      for (int b=MAX_BUSES; b<MAX_BUSES*2; b++) // Write Effects ngrs::Names.
      {
        if(selectedSong_->_pMachine[b]) {
          buffer.str("");
          buffer << std::setfill('0') << std::hex << std::setw(2);
          buffer << b << ": " << selectedSong_->_pMachine[b]->_editName;
          genCombo_->add(new ngrs::NItem(buffer.str()));
          //cb->SetItemData(cb->GetCount()-1,b);
          if (!found) selected++;
          if (selectedSong_->seqBus == b) found = true;
          filled = true;
        }
      }

      if (!filled) {
        genCombo_->removeChilds();
        genCombo_->add(new ngrs::NItem("No Machines Loaded"));
        selected = 0;
      } else if (!found)  {
        selected=line;
      }
      genCombo_->setIndex(selected);
    }

    void MainWindow::appNew( )
    {
      if (checkUnsavedSong())
      {
        // todo rework
      }
    }

    void MainWindow::onBpmIncOne( ngrs::NButtonEvent* ev )  // OnBpmAddOne
    {
      setAppSongBpm(1);
    }

    void MainWindow::onBpmAddTen( ngrs::NButtonEvent* ev )
    {
      setAppSongBpm(10);
    }

    void MainWindow::onBpmDecOne( ngrs::NButtonEvent* ev )
    {
      setAppSongBpm(-1);
    }

    void MainWindow::onBpmDecTen( ngrs::NButtonEvent* ev )
    {
      setAppSongBpm(-10);
    }

    void MainWindow::setAppSongBpm( double  x )
    {
      if ( !selectedChildView_ ) return;

      Song* selectedSong_ = selectedChildView_->song();

      double bpm = 0;
      if ( x != 0 ) {
        if ( Player::Instance()->playing() )  {
          selectedSong_->setBpm(  Player::Instance()->timeInfo().bpm() +x );
        } else selectedSong_->setBpm( selectedSong_->bpm() + x );
        Player::Instance()->setBpm( (int) selectedSong_->bpm() );
        bpm = selectedSong_->bpm();
      }
      else bpm =  Player::Instance()->bpm();

      bpmDisplay_->setNumber( (int)  Player::Instance()->bpm() );
      bpmDisplay_->repaint();
    }

    void MainWindow::onRecordWav( ngrs::NButtonEvent * ev )
    {
    }

    void MainWindow::onMachineSelected( Machine* mac ) {

      if ( !selectedChildView_ ) return;

      Song* selectedSong_ = selectedChildView_->song();


      std::vector< ngrs::NCustomItem * > items = genCombo_->items();
      std::vector< ngrs::NCustomItem * >::iterator it = items.begin();

      int idx = 0;
      for ( ; it < items.end(); it++) {
        ngrs::NCustomItem* item = *it;
        if (item->text().length() > 2) {
          int macIdx = str_hex<int>( item->text().substr(0,2) );

          if (macIdx == mac->_macIndex) {
            selectedSong_->seqBus = macIdx;
            genCombo_->setIndex(idx);
            genCombo_->repaint();
            break;
          }
        }
        idx++;
      }
    }

    void MainWindow::onTimer( )
    {
      if ( !selectedChildView_ ) return;
      Song* selectedSong_ = selectedChildView_->song();

      if ( Player::Instance()->playing() ) {
        selectedChildView_->sequencerView()->updatePlayPos();				

        SinglePattern* visiblePattern = selectedChildView_->patternView()->pattern();
        if ( visiblePattern ) {			
          double entryStart = 0;
          bool isPlayPattern = selectedSong_->patternSequence()->getPlayInfo( visiblePattern, Player::Instance()->playPos() , 4 , entryStart );
          if ( isPlayPattern ) {
            selectedChildView_->patternView()->onTick( entryStart );
          }			
        }
      }

      /*vuMeter_->setPegel(selectedSong_->_pMachine[MASTER_INDEX]->_lMax,
      selectedSong_->_pMachine[MASTER_INDEX]->_rMax );
      vuMeter_->repaint();
      ((Master*)selectedSong_->_pMachine[MASTER_INDEX])->vuupdated = true;

      if ( !selectedChildView_ ) return;
      selectedChildView_->machineView()->updateVUs();*/
    }

    void MainWindow::updateBars( )
    {
    }

    int MainWindow::close( )
    {
      closePsycle();
      return ngrs::nDestroyWindow;
    }

    void MainWindow::onMachineView( ngrs::NButtonEvent* ev ) {
      if ( !selectedChildView_ ) return;
      selectedChildView_->showMachineView();
    }

    void MainWindow::onPatternView( ngrs::NButtonEvent* ev ) {
      if ( !selectedChildView_ ) return;
      selectedChildView_->showPatternView();
    }

    bool MainWindow::checkUnsavedSong( )
    {
      if ( !selectedChildView_ ) return true;
      Song* selectedSong_ = selectedChildView_->song();

      ngrs::NMessageBox* box = new ngrs::NMessageBox("Save changes of : "+selectedSong_->fileName+" ?");
      box->setTitle("New Song");
      box->setButtonText("Yes","No","Abort");
      box->icon()->setSharedBitmap(&Global::pConfig()->icons().alert());
      box->icon()->setPreferredSize(Global::pConfig()->icons().alert().width(),Global::pConfig()->icons().alert().height());
      add(box);
      bool result = false;
      int choice = box->execute();
      switch (choice) {
    case ngrs::nMsgOkBtn :
      onFileSave(0);
      result = true;
      break;
    case ngrs::nMsgUseBtn:
      result = true;
      break;
    case ngrs::nMsgCancelBtn:
      result = false;
      break;

      }
      ngrs::NApp::addRemovePipe(box);
      return result;
    }  

    // Sequencer menu events

    void MainWindow::onSequencerView( ngrs::NButtonEvent * ev )
    {
      if ( !selectedChildView_ ) return;

      selectedChildView_->showSequencerView();
    }

    void MainWindow::onViewMenuToolbar( ngrs::NButtonEvent * ev )
    {
      toolBar1_->setVisible(!toolBar1_->visible());
      pane()->resize();
      pane()->repaint();
    }

    void MainWindow::onViewMenuMachinebar( ngrs::NButtonEvent * ev )
    {
      psycleToolBar_->setVisible(!psycleToolBar_->visible());
      pane()->resize();
      pane()->repaint();
    }

    void MainWindow::onViewMenuSequencerbar( ngrs::NButtonEvent * ev )
    {
      if ( !selectedChildView_ ) return;
      selectedChildView_->sequencerBar()->setVisible(!selectedChildView_->sequencerBar()->visible() );



      //sequencerBar_->setVisible(!sequencerBar_->visible());
      //pane()->resize();
      //pane()->repaint();
    }

    void MainWindow::onViewMenuStatusbar( ngrs::NButtonEvent * ev )
    {
      statusBar_->setVisible(!statusBar_->visible());
      pane()->resize();
      pane()->repaint();
    }

    // Help menu events

    void MainWindow::onHelpMenuAbout( ngrs::NButtonEvent * ev )
    {
      ngrs::NMessageBox* about = new ngrs::NMessageBox();
      about->setTitle("About Psycle(X)");
      about->setText( std::string("Psycle version (X alpha 0.1)\n") +
        std::string("(c) 2006 by Stefan Nattkemper\n") +
        std::string("            Josep Segura\n") +
        std::string("            D.W. Aley\n") +
        std::string("GNU Public Licence 2.0") );
      about->setButtons( ngrs::nMsgOkBtn );
      add( about );
      about->execute();
      ngrs::NApp::addRemovePipe(about);
    }

    void MainWindow::onHelpMenuGreeting( ngrs::NButtonEvent * ev )
    {
      greetDlg->setVisible(true);
    }

    void MainWindow::onHelpMenuReadme( ngrs::NButtonEvent * ev )
    {
      infoDlg->loadFromFile( Global::pConfig()->hlpPath() + "readme.txt" );
      infoDlg->setVisible(true);

    }

    void MainWindow::onHelpMenuWhatsNew( ngrs::NButtonEvent * ev )
    {
      infoDlg->loadFromFile( Global::pConfig()->hlpPath() + "keys.txt" );
      infoDlg->setVisible( true );
    }

    void MainWindow::onHelpMenuTweaking( ngrs::NButtonEvent * ev )
    {
      infoDlg->loadFromFile( Global::pConfig()->hlpPath() + "tweaking.txt" );
      infoDlg->setVisible( true );
    }

    void MainWindow::onHelpMenuKeys( ngrs::NButtonEvent * ev )
    {
      infoDlg->loadFromFile( Global::pConfig()->hlpPath() + "whatsnew.txt" );
      infoDlg->setVisible( true );
    }

    void MainWindow::onNewMachine( ngrs::NButtonEvent * ev )
    {
      if ( !selectedChildView_ ) return;

      if (ev->button()==1) {
        if (newMachineDlg_->execute()) {

          PluginFinderKey key = newMachineDlg_->pluginKey();

          // search for an unused machine slot
          int fb = selectedChildView_->song()->GetFreeBus();

          // create machine, tell where to place the new machine--get from mouse.	  
          Machine* mac = selectedChildView_->song()->createMachine( pluginFinder_, key, ev->x(), ev->y() );
          if ( mac ) {
            selectedChildView_->machineView()->addMachine( *mac );
            selectedChildView_->newMachineAdded.emit( mac );
            selectedChildView_->machineView()->repaint();
          }

        }
      }
    }

    void MainWindow::onRenderAsWave( ngrs::NButtonEvent * ev )
    {
    }

    void MainWindow::onEditUndo( ngrs::NButtonEvent * ev )
    {
      if ( !selectedChildView_ ) return;

      selectedChildView_->patternView()->doUndo();
    }

    void MainWindow::onEditRedo( ngrs::NButtonEvent * ev )
    {
    }

    void MainWindow::onEditPatternCut( ngrs::NButtonEvent * ev )
    {

    }

    void MainWindow::onEditPatternCopy( ngrs::NButtonEvent * ev )
    {
    }

    void MainWindow::onEditPatternPaste( ngrs::NButtonEvent * ev )
    {
    }

    void MainWindow::onEditPatternMix( ngrs::NButtonEvent * ev )
    {
    }

    void MainWindow::onEditPatternMixPaste( ngrs::NButtonEvent * ev )
    {
    }

    void MainWindow::onEditPatternDelete( ngrs::NButtonEvent * ev )
    {
    }

    void MainWindow::onEditBlockMixPaste( ngrs::NButtonEvent * ev )
    {
      if ( !selectedChildView_ ) return;

      //  selectedChildView_->patternView()->pasteBlock(selectedChildView_->patternView()->cursor().track(), selectedChildView_->patternView()->cursor().line(), true);
    }

    void MainWindow::onEditBlockDelete( ngrs::NButtonEvent * ev )
    {
      if ( !selectedChildView_ ) return;

      selectedChildView_->patternView()->deleteBlock();
    }

    void MainWindow::onEditBlockMix( ngrs::NButtonEvent * ev )
    {

    }

    void MainWindow::onEditBlockPaste( ngrs::NButtonEvent * ev )
    {
      if ( !selectedChildView_ ) return;

      //  selectedChildView_->patternView()->pasteBlock(selectedChildView_->patternView()->cursor().track(), selectedChildView_->patternView()->cursor().line(), false);
    }

    void MainWindow::onEditBlockCopy( ngrs::NButtonEvent * ev )
    {
      if ( !selectedChildView_ ) return;
      selectedChildView_->patternView()->copyBlock(false);
    }

    void MainWindow::onEditBlockCut( ngrs::NButtonEvent * ev )
    {
      if ( !selectedChildView_ ) return;
      selectedChildView_->patternView()->copyBlock(true);
    }

    void MainWindow::onEditSeqDelete( ngrs::NButtonEvent * ev )
    {
    }

    void MainWindow::onEditSeqCopy( ngrs::NButtonEvent * ev )
    {
    }

    void MainWindow::onEditSeqCut( ngrs::NButtonEvent * ev )
    {
    }

    void MainWindow::onFileExit( ngrs::NButtonEvent * ev )
    {
      if (checkUnsavedSong()) {
        exit(0);
      }
    }

    void MainWindow::onRecordNotesMode( ngrs::NButtonEvent * ev )
    {
      /*bEditMode = true;
      if (sequencerBar_->followSong() && bEditMode)
      {
      bEditMode = false;
      }
      else
      {
      sequencerBar_->setFollowSong(true);
      bEditMode = true;
      }*/
    }

    void MainWindow::onSeqAdded( SinglePattern * pattern )
    {
      if ( !selectedChildView_ ) return;

      selectedChildView_->sequencerView()->addPattern( pattern);
    }

    void MainWindow::onNewMachineDialogAdded( Machine * mac )
    {
      if ( !selectedChildView_ ) return;

      selectedChildView_->patternView()->setActiveMachineIdx(mac->_macIndex);
      updateComboGen();
      genCombo_->repaint();
    }

    // ngrs::New index selected by a mouse click.
    void MainWindow::onGeneratorCbx( ngrs::NItemEvent * ev )
    {
      onNewIndexGeneratorCbx();
    }

    void MainWindow::changeGeneratorCbxViaKey(int dir) {
      int old_index = genCombo_->selIndex();
      int new_index = 0;
      // 0 = prev gen, 1 = next gen.
      if (dir == 0) {
        new_index = std::max(old_index - 1, 0);
      } else if (dir == 1) {
        new_index = old_index + 1;
      }
      genCombo_->setIndex(new_index);
      genCombo_->repaint();

      onNewIndexGeneratorCbx();
    }

    void MainWindow::changeInstrumentCbxViaKey(int dir) {
      int old_index = insCombo_->selIndex();
      int new_index = 0;
      if (dir == 0) {
        new_index = std::max(old_index - 1, 0);
      } else if (dir == 1) {
        new_index = old_index + 1;
      }
      insCombo_->setIndex(new_index);
      insCombo_->repaint();
      selectedChildView_->song()->instSelected=   new_index;
      selectedChildView_->song()->auxcolSelected= new_index;
      selectedChildView_->waveEditor()->Notify();
    }

    // Call when a new index has been selected in
    // the generator combo box.
    void MainWindow::onNewIndexGeneratorCbx() {
      if ( !selectedChildView_ ) return;
      Song* selectedSong_ = selectedChildView_->song();

      std::string text = genCombo_->text();
      if (text.length() > 2) {
        std::string hexNumber = text.substr(0,2);
        std::stringstream hexStream(hexNumber); 
        int hex = -1;
        hexStream >> std::hex >> hex;
        if (hex != -1) {
          selectedSong_->seqBus = hex;
          selectedChildView_->machineView()->setSelectedMachine( selectedSong_->_pMachine[hex] );
        }
      }
    }


    void MainWindow::onSequencerEntryClick( SequencerItem * item )
    {
      if (!selectedChildView_) return;

      selectedChildView_->sequencerBar()->setEntry(item);
    }

    void MainWindow::onKeyPress( const ngrs::NKeyEvent & event )
    {
      if ( selectedChildView_ ) {
        int key = Global::pConfig()->inputHandler().getEnumCodeByKey(Key(event.shift(),event.scancode()));
        switch (key)
        {
        case cdefEditMachine:
          selectedChildView_->showMachineView();
          break;
        case cdefEditPattern:
          selectedChildView_->showPatternView();
          break;
        case cdefEditInstr:
          selectedChildView_->showWaveView();
          break;
        case cdefEditSequence:
          selectedChildView_->showSequencerView();
          break;
        case cdefPlayStart:
          selectedChildView_->playFromStart();
          break;
        case cdefPlaySong:
          selectedChildView_->play();
          break;
        case cdefPlayFromPos:
          break;
        case cdefPlayStop:
          selectedChildView_->stop();
          break;
        case cdefAddMachine: {
          selectedChildView_->showMachineView();
          ngrs::NButtonEvent btnEvent( this, 0,0,1, "");
          onNewMachine( &btnEvent );
                             }
                             break;
                             /*			case ngrs::ngrs::NK_Up :
                             if (NApp::system().keyState() & ControlMask) {
                             selectedChildView_->sequencerBar()->selectPrevPattern();
                             }
                             break;
                             case ngrs::ngrs::NK_Down :
                             if (NApp::system().keyState() & ControlMask) {
                             selectedChildView_->sequencerBar()->selectNextPattern();
                             }
                             break;*/
        case cdefInstrDec: // current_instrument-1
          changeInstrumentCbxViaKey(0);
          break;
        case cdefInstrInc: // current_instrument+1
          changeInstrumentCbxViaKey(1);
          break;
        case cdefMachineDec: // current_machine-1
          changeGeneratorCbxViaKey(0);
          break;
        case cdefMachineInc: // current_machine+1
          changeGeneratorCbxViaKey(1);
          break;
        }
      }

      ngrs::NWindow::onKeyPress( event);
    }


    void MainWindow::updateNewSong( )
    {
      if (!selectedChildView_) return;

      updateComboIns(true);
      insCombo_->setIndex(0);
      updateComboGen();

      bpmDisplay_->setNumber( (int) selectedChildView_->song()->bpm() );
      bpmDisplay_->repaint();
    }

    void MainWindow::onUpdateInstrumentCbx( int index , bool update )
    {
      if ( !selectedChildView_ ) return;

      if (update) {
        updateComboIns(true);
        insCombo_->setIndex(0);
        insCombo_->repaint();
      }
      else {
        insCombo_->setIndex(index);
        insCombo_->repaint();
      }

    }

    void MainWindow::onMachineNameChanged( int machineIndex )
    {
      updateComboGen();
      genCombo_->repaint();
    }

    void MainWindow::onMachineDeleted( int machineIndex )
    {
      updateComboGen();
      genCombo_->repaint();
    }

  }
}
