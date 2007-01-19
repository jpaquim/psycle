/***************************************************************************
*   Copyright (C) 2006 by  Stefan   *
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
#include "infodlg.h"
#include "sequencergui.h"
#include "audioconfigdlg.h"
#include "skinreader.h"
#include <iomanip>
#include <ngrs/app.h>
#include <ngrs/item.h>
#include <ngrs/checkmenuitem.h>
#include <ngrs/menuseperator.h>
#include <ngrs/messagebox.h>
#include <ngrs/bevelborder.h>
#include <ngrs/statusbar.h>
#include <ngrs/textstatusitem.h>
#include <ngrs/filedialog.h>
#include <ngrs/system.h>
#include <ngrs/toolbarpanel.h>

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
      setLayout( ngrs::AlignLayout() );
      progressBar_ = new ngrs::ProgressBar();
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
      : ngrs::Window()
    { 
      setTitle ("] Psycle Modular Music Creation Studio [ ( X alpha ) ");
      setPosition( 10, 10, 800, 600 );

      SkinReader::Instance()->setDefaults();
      DefaultBitmaps & icons =  SkinReader::Instance()->bitmaps();

      songTabSkinDown.setGradient( 
        ngrs::Color(255,255,255), // start gradient color
        ngrs::Color(243,243,239), // mid gradient color
        ngrs::Color(236,233,216), // end gradient color
        1,                         // 1: rect, 2: rounded
        ngrs::nVertical, 10,5,5
        );
      songTabSkinDown.setTransparent( false );
      songTabSkinNone.setGradient(  
        ngrs::Color(255,255,255),
        ngrs::Color(243,243,239),
        ngrs::Color(214,208,192),
        1,                         // 1: rect, 2: rounded
        ngrs::nVertical,10,5,5
      );
      songTabSkinNone.setTransparent( false );
      
      count = 0;

      initMenu();
      initBars();
      initDialogs();

      book = new ngrs::TabBook();    
      pane()->add(book,ngrs::nAlClient);
      tabBar_ = new ngrs::TabBar();
      book->setTabBar( tabBar_ );   

      ngrs::ToolBar* playBar = new ngrs::ToolBar();
      playBar->setSkin( ngrs::Skin() );
      playBar->setSpacing( ngrs::Size( 5,0,0,0) );
      ngrs::Image* img = new ngrs::Image();
      img->setSharedBitmap(&icons.playstart());
      img->setPreferredSize(25,25);
      barPlayFromStartBtn_ = new ngrs::Button(img);
      barPlayFromStartBtn_->click.connect(this,&MainWindow::onBarPlayFromStart);
      barPlayFromStartBtn_->setHint("Play from start");
      playBar->add(barPlayFromStartBtn_);

      img = new ngrs::Image();
      img->setSharedBitmap(&icons.play());
      img->setPreferredSize(25,25);
      ngrs::Button* playBtn = new ngrs::Button(img);
      playBtn->setHint("Play from edit position");
      playBar->add(playBtn)->clicked.connect(this,&MainWindow::onBarPlay);

      img = new ngrs::Image();
      img->setSharedBitmap(&icons.playselpattern());
      img->setPreferredSize(25,25);
      playBar->add(new ngrs::Button(img));

      img = new ngrs::Image();
      img->setSharedBitmap(&icons.stop());
      img->setPreferredSize(25,25);
      ngrs::Button* stopBtn_ = new ngrs::Button(img);
      stopBtn_->click.connect(this,&MainWindow::onBarStop);
      stopBtn_->setHint("Stop");
      playBar->add(stopBtn_);

      img = new ngrs::Image();
      img->setSharedBitmap(&icons.autoStop());
      img->setPreferredSize(25,25);
      playBar->add(new ngrs::Button(img));

      playBar->add(new ngrs::ToolBarSeparator());

      img = new ngrs::Image();
      img->setSharedBitmap(&icons.recordnotes());
      img->setPreferredSize(25,25);
      ngrs::Button* recNotes = new ngrs::Button(img);
      recNotes->setHint("Record Notes Mode");
      recNotes->clicked.connect(this,&MainWindow::onRecordNotesMode);
      playBar->add(recNotes);

      ngrs::ToolBar* vuToolBar = new ngrs::ToolBar();
      vuToolBar->add(new ngrs::Label("VU"));
      ngrs::Panel* vuPanel = new ngrs::Panel();
      vuPanel->setPreferredSize( 225, 10 );
      vuMeter_ = new VuMeter();
      vuPanel->add(vuMeter_);
      vuMeter_->setPosition(0,0,225,10);
      masterSlider_ = new ngrs::Slider();
      masterSlider_->setOrientation(ngrs::nHorizontal);
      masterSlider_->setPosition(0,10,225,10);
      vuPanel->add(masterSlider_);
      vuToolBar->add(vuPanel);      

      ngrs::FlowLayout fl;
      fl.setAlign( ngrs::nAlLeft );
      fl.setHgap( 2 );
      fl.setVgap( 0 );
      fl.setBaseLine( ngrs::nAlBottom );
      tabBar_->setLayout( fl );
      
      ngrs::Panel* logoPnl = new ngrs::Panel();
      logoPnl->setLayout( ngrs::AlignLayout() );
      logoPnl->setSpacing( ngrs::Size( 10,5,10,5 ) );
      img = new ngrs::Image( icons.logoRight()  );
      img->setVAlign( ngrs::nAlCenter );
      logoPnl->add( img , ngrs::nAlRight );
      img = new ngrs::Image( icons.logoLeft() );
      img->setVAlign( ngrs::nAlCenter );
      logoPnl->add( img , ngrs::nAlLeft );
      img = new ngrs::Image( icons.logoMid() );
      img->setLayout( ngrs::AlignLayout() );
      img->add( tabBar_, ngrs::nAlBottom );     
      img->add( vuToolBar, ngrs::nAlTop );      
      tabBar_->add( playBar, ngrs::nAlLeft );
      ngrs::Panel* spacer = new ngrs::Panel();
      spacer->setPreferredSize(20,5);
      tabBar_->add( spacer, ngrs::nAlLeft );
      img->setVAlign( ngrs::nAlCenter );
      img->setHAlign( ngrs::nAlWallPaper );
      img->setEvents( true );
      logoPnl->add( img , ngrs::nAlClient );
      pane()->add( logoPnl, ngrs::nAlTop );

      audioConfigDlg = new AudioConfigDlg( Global::pConfig() );
      add( audioConfigDlg );	

      selectedChildView_ = addChildView();
      updateNewSong();

      enableSound();

      oldPlayPos_ = 0;
      timer.setIntervalTime(10);
      timer.timerEvent.connect( this, &MainWindow::onTimer );
      timer.enableTimer();
    }

    MainWindow::~MainWindow()
    {
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
      childView_->machineView()->machineDeleted.connect(this,&MainWindow::onMachineDeleted);
      childView_->machineView()->machineNameChanged.connect(this, &MainWindow::onMachineNameChanged);
      book->addPage( childView_, childView_->song()->name() + stringify( count ) );      
      ngrs::NTab* tab = book->tab( childView_ );
      tab->setSkin( songTabSkinNone, songTabSkinDown, 0 );
      book->setActivePage( childView_ );

      count++;

      
      tab->click.connect(this,&MainWindow::onTabChange);
      tab->setEnablePopupMenu(true);
      ngrs::NPopupMenu* menu = tab->popupMenu();
      ngrs::MenuItem* closeSong = new ngrs::MenuItem("Close");
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

    void MainWindow::onCloseSongTabPressed( ngrs::ButtonEvent* ev ) {
      std::map<Object*,ChildView*>::iterator it = songMap.find( ev->sender() ); 
      if ( it != songMap.end() ) {
        std::map<Object*,ChildView*>::iterator tabIt = songTabMap.begin();
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
          std::map<Object*,ChildView*>::iterator songIt = songTabMap.begin();
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

    void MainWindow::onTabChange( ngrs::ButtonEvent * ev )
    {
      std::map<Object*,ChildView*>::iterator it = songTabMap.find( ev->sender() ); 
      if ( it != songTabMap.end() ) {
        ChildView* view = it->second;
        Player::Instance()->stop();
        Player::Instance()->song( view->song() );
        selectedChildView_ = view;
        updateNewSong();
        pane()->repaint();
      }
    }


    void MainWindow::onConfigMenuAudio( ngrs::ButtonEvent* ev ) {
      audioConfigDlg->setVisible( true );
    }

    void MainWindow::initMenu( )
    {
      menuBar_ = new ngrs::MenuBar();
      pane()->add( menuBar_ );

      DefaultBitmaps & icons =  SkinReader::Instance()->bitmaps();

      // Creates the file menu
      fileMenu_ = new ngrs::Menu("File");
      fileMenu_->add(new ngrs::MenuItem("New",icons.newfile() ))->click.connect(this,&MainWindow::onFileNew);
      fileMenu_->add(new ngrs::MenuItem("Open",icons.open()))->click.connect(this,&MainWindow::onFileOpen);
      fileMenu_->add(new ngrs::MenuItem("Import Module"));
      fileMenu_->add(new ngrs::MenuItem("Save",icons.save()));
      fileMenu_->add(new ngrs::MenuItem("Save as"))->click.connect(this,&MainWindow::onFileSaveAs);
      fileMenu_->add(new ngrs::MenuItem("Render as Wav"))->click.connect(this,&MainWindow::onRenderAsWave);
      fileMenu_->add(new ngrs::MenuSeperator());
      fileMenu_->add(new ngrs::MenuItem("Song properties"))->click.connect(this,&MainWindow::showSongpDlg);
      fileMenu_->add(new ngrs::MenuSeperator());
      fileMenu_->add(new ngrs::MenuItem("revert to saved"));
      ngrs::MenuItem* recentItem = new ngrs::MenuItem("recent files");
      fileMenu_->add(recentItem);
      recentFileMenu_ = new ngrs::Menu();
      recentItem->add(recentFileMenu_);
      noneFileItem = new ngrs::MenuItem("none");
      noneFileItem->setEnable(false);
      recentFileMenu_->add(noneFileItem);
      noFileWasYetLoaded = true;
      fileMenu_->add(new ngrs::MenuItem("exit"))->click.connect(this,&MainWindow::onFileExit);
      menuBar_->add(fileMenu_);

      // Creates the edit menu
      editMenu_ = new ngrs::Menu("Edit");
      editMenu_->add(new ngrs::MenuItem("Undo",icons.undo()))->click.connect(this,&MainWindow::onEditUndo);
      editMenu_->add(new ngrs::MenuItem("Redo",icons.redo()))->click.connect(this,&MainWindow::onEditRedo);
      editMenu_->add(new ngrs::MenuSeperator());
      editMenu_->add(new ngrs::MenuItem("Pattern Cut"))->click.connect(this,&MainWindow::onEditPatternCut);
      editMenu_->add(new ngrs::MenuItem("Pattern Copy"))->click.connect(this,&MainWindow::onEditPatternCopy);
      editMenu_->add(new ngrs::MenuItem("Pattern Paste"))->click.connect(this,&MainWindow::onEditPatternPaste);
      editMenu_->add(new ngrs::MenuItem("Pattern Mix"))->click.connect(this,&MainWindow::onEditPatternMix);
      editMenu_->add(new ngrs::MenuItem("Pattern Mix Paste"))->click.connect(this,&MainWindow::onEditPatternMixPaste);
      editMenu_->add(new ngrs::MenuItem("Pattern Delete"))->click.connect(this,&MainWindow::onEditPatternDelete);
      editMenu_->add(new ngrs::MenuSeperator());
      editMenu_->add(new ngrs::MenuItem("Block Cut"))->click.connect(this,&MainWindow::onEditBlockCut);
      editMenu_->add(new ngrs::MenuItem("Block Copy"))->click.connect(this,&MainWindow::onEditBlockCopy);
      editMenu_->add(new ngrs::MenuItem("Block Paste"))->click.connect(this,&MainWindow::onEditBlockPaste);
      editMenu_->add(new ngrs::MenuItem("Block Mix Paste"))->click.connect(this,&MainWindow::onEditBlockMixPaste);
      editMenu_->add(new ngrs::MenuItem("Block Delete"))->click.connect(this,&MainWindow::onEditBlockDelete);
      editMenu_->add(new ngrs::MenuSeperator());
      editMenu_->add(new ngrs::MenuItem("Sequence Cut"))->click.connect(this,&MainWindow::onEditSeqCut);
      editMenu_->add(new ngrs::MenuItem("Sequence Copy"))->click.connect(this,&MainWindow::onEditSeqCopy);
      editMenu_->add(new ngrs::MenuItem("Sequence Delete"))->click.connect(this,&MainWindow::onEditSeqDelete);
      menuBar_->add(editMenu_);

      // Creates the view menu
      viewMenu_ = new ngrs::Menu("View");
      viewMenu_->add(new ngrs::CheckMenuItem("Toolbar"))->click.connect(this,&MainWindow::onViewMenuToolbar);
      viewMenu_->add(new ngrs::CheckMenuItem("Machinebar"))->click.connect(this,&MainWindow::onViewMenuMachinebar);
      viewMenu_->add(new ngrs::CheckMenuItem("Sequencerbar"))->click.connect(this,&MainWindow::onViewMenuSequencerbar);
      viewMenu_->add(new ngrs::CheckMenuItem("Statusbar"))->click.connect(this,&MainWindow::onViewMenuStatusbar);
      viewMenu_->add(new ngrs::MenuSeperator());
      viewMenu_->add(new ngrs::MenuItem("PatternEditor",icons.patterns()))->click.connect(this,&MainWindow::onPatternView);
      viewMenu_->add(new ngrs::MenuItem("MachineView",icons.machines()))->click.connect(this,&MainWindow::onMachineView);
      viewMenu_->add(new ngrs::MenuItem("PatternSequencer",icons.sequencer()))->click.connect(this,&MainWindow::onSequencerView);
      viewMenu_->add(new ngrs::MenuSeperator());
      viewMenu_->add(new ngrs::MenuItem("Add machine",icons.newmachine()))->click.connect(this,&MainWindow::onNewMachine);
      viewMenu_->add(new ngrs::MenuSeperator());
      //      viewMenu_->add(new ngrs::MenuItem("Instrument Editor"))->click.connect(this,&MainWindow::onEditInstrument);
      menuBar_->add(viewMenu_);

      // Creates the configuration menu
      configurationMenu_ = new ngrs::Menu("Configuration");
      configurationMenu_->add(new ngrs::MenuItem("Free Audio"));
      configurationMenu_->add(new ngrs::MenuItem("Autostop"));
      configurationMenu_->add(new ngrs::MenuSeperator());
      configurationMenu_->add(new ngrs::MenuItem("Loop Playback"));
      configurationMenu_->add(new ngrs::MenuSeperator());
      configurationMenu_->add(new ngrs::MenuItem("Audio Settings"))->click.connect(this,&MainWindow::onConfigMenuAudio);
      configurationMenu_->add(new ngrs::MenuItem("Load Skin"))->click.connect(this,&MainWindow::onConfigMenuSkin);
      menuBar_->add(configurationMenu_);

      // Creates the performance menu
      performanceMenu_ = new ngrs::Menu("Performance");
      performanceMenu_->add(new ngrs::MenuItem("CPU Monitor"));
      performanceMenu_->add(new ngrs::MenuItem("MDI Monitor"));
      menuBar_->add(performanceMenu_);

      // Creates the community menu
      communityMenu_ = new ngrs::Menu("Community");
      ngrs::MenuItem* comSearchItem = new ngrs::MenuItem("Community Search");	  
      communityMenu_->add( new ngrs::MenuItem("Ask a Question") );
      communityMenu_->add( new ngrs::MenuItem("Sent Feedback" ) );
      communityMenu_->add( new ngrs::MenuSeperator() );
      communityMenu_->add( new ngrs::MenuItem("Psycledelics Center") );
      communityMenu_->add( new ngrs::MenuSeperator() );
      communityMenu_->add( comSearchItem );
      ngrs::Menu* comSearch = new ngrs::Menu();
      comSearch->add( new ngrs::MenuItem("Songs") );
      comSearch->add( new ngrs::MenuItem("Presets") );
      comSearch->add( new ngrs::MenuItem("Skins") );
      comSearch->add( new ngrs::MenuItem("Samples") );
      comSearchItem->add( comSearch );	
      menuBar_->add( communityMenu_ );

      // Creates the help menu
      helpMenu_ = new ngrs::Menu("Help");
      helpMenu_->add(new ngrs::MenuItem("About"))->click.connect(this,&MainWindow::onHelpMenuAbout);
      helpMenu_->add(new ngrs::MenuItem("Greetings"))->click.connect(this,&MainWindow::onHelpMenuGreeting);
      helpMenu_->add(new ngrs::MenuSeperator());
      helpMenu_->add(new ngrs::MenuItem("readme"))->click.connect(this,&MainWindow::onHelpMenuReadme);
      helpMenu_->add(new ngrs::MenuItem("keys"))->click.connect(this,&MainWindow::onHelpMenuKeys);
      helpMenu_->add(new ngrs::MenuItem("tweaking"))->click.connect(this,&MainWindow::onHelpMenuTweaking);
      helpMenu_->add(new ngrs::MenuItem("whatsnew"))->click.connect(this,&MainWindow::onHelpMenuWhatsNew);
      menuBar_->add(helpMenu_);
    }


    void MainWindow::initDialogs( )
    {
      // creates the song dialog for editing song name, author, and comment
      add( songpDlg_ = new SongpDlg( ) );
      // creates the greeting dialog, that greets people who help psycle development
      add( greetDlg =  new GreetDlg() );
      // creates the save dialog, that ask where to store wave files, recorded from playing a psy song
      wavRecFileDlg = new ngrs::FileDialog();
      wavRecFileDlg->setMode( ngrs::nSave );
      add( wavRecFileDlg );
      // creates the info dialog, that displays in a memo readme keys tweaking and a whatsnew file
      add( infoDlg =  new InfoDlg() );
    }

    // events from menuItems

    void MainWindow::showSongpDlg( ngrs::ButtonEvent* ev )
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
        statusBar_->add( new ngrs::TextStatusItem(i) );    
      statusBar_->setModel( statusBarData );
      statusBarData.setText( "Ready", 0 );
      statusBarData.setText( "Ln 0", 2 );
      statusBarData.setText( "Tr 0", 3 );
      statusBarData.setText( "Play 00:00:00:00", 4 );
      pane()->add( statusBar_, ngrs::nAlBottom );
    }

    void MainWindow::initToolBar( )
    {
      toolBarPanel_ = new ngrs::ToolBarPanel();    
      pane()->add(toolBarPanel_, ngrs::nAlTop);

      DefaultBitmaps & icons =  SkinReader::Instance()->bitmaps();

      ngrs::Image* img;

      toolBar1_ = new ngrs::ToolBar();
      toolBarPanel_->add(toolBar1_);

      // creates the newfile button
      img = new ngrs::Image();
      img->setSharedBitmap(&icons.newfile());
      img->setPreferredSize(25,25);
      ngrs::Button* newBtn = new ngrs::Button(img);
      newBtn->setHint("New song");
      toolBar1_->add(newBtn)->clicked.connect(this,&MainWindow::onFileNew);


      img = new ngrs::Image();
      img->setSharedBitmap(&icons.open());
      img->setPreferredSize(25,25);
      ngrs::Button* fileOpenBtn = new ngrs::Button(img);
      fileOpenBtn->setHint("Song load");
      toolBar1_->add(fileOpenBtn)->clicked.connect(this,&MainWindow::onFileOpen);


      img = new ngrs::Image();
      img->setSharedBitmap(&icons.save());
      img->setPreferredSize(25,25);
      ngrs::Button* saveBtn = new ngrs::Button(img);
      saveBtn->setHint("Save");
      toolBar1_->add(saveBtn)->clicked.connect(this,&MainWindow::onFileSaveAs);;

      img = new ngrs::Image();
      img->setSharedBitmap(&icons.save_audio());
      img->setPreferredSize(25,25);
      ngrs::Button* saveAsAudioFileBtn = new ngrs::Button(img);
      saveAsAudioFileBtn->setHint("Save as audio file");
      toolBar1_->add(saveAsAudioFileBtn);

      img = new ngrs::Image();
      img->setSharedBitmap(&icons.recordwav());
      img->setPreferredSize(25,25);
      ngrs::Button* recWav = new ngrs::Button(img);
      recWav->setToggle(true);
      recWav->setHint("Record to .wav");
      recWav->clicked.connect(this, &MainWindow::onRecordWav);
      toolBar1_->add(recWav);

      toolBar1_->add(new ngrs::ToolBarSeparator());

      img = new ngrs::Image();
      img->setSharedBitmap(&icons.undo());
      img->setPreferredSize(25,25);
      toolBar1_->add(new ngrs::Button(img));

      img = new ngrs::Image();
      img->setSharedBitmap(&icons.redo());
      img->setPreferredSize(25,25);
      toolBar1_->add(new ngrs::Button(img));

      toolBar1_->add(new ngrs::ToolBarSeparator());                  

      img = new ngrs::Image();
      img->setSharedBitmap(&icons.newmachine());
      img->setPreferredSize(25,25);
      ngrs::Button* newMacBtn = new ngrs::Button(img);
      newMacBtn->setHint("New Machine");
      toolBar1_->add(newMacBtn);//->clicked.connect(childView_,&ChildView::onMachineViewDblClick);

      img = new ngrs::Image();
      img->setSharedBitmap(&icons.openeditor());
      img->setPreferredSize(25,25);
      /*  ngrs::Button* editInsBtn = new ngrs::Button(img);
      editInsBtn->setHint("Edit Instrument");
      toolBar1_->add(editInsBtn)->clicked.connect(this,&MainWindow::onEditInstrument);*/

      toolBar1_->add(new ngrs::ToolBarSeparator());

      img = new ngrs::Image();
      img->setSharedBitmap(&icons.p());
      img->setPreferredSize(25,25);
      toolBar1_->add(new ngrs::Button(img));

      toolBar1_->add(new ngrs::ToolBarSeparator());

      img = new ngrs::Image();
      img->setSharedBitmap(&icons.help());
      img->setPreferredSize(25,25);
      toolBar1_->add(new ngrs::Button(img));

      toolBar1_->resize();

           
      psycleControlBar_ = new ngrs::ToolBar();
      psycleControlBar_->add(new ngrs::Label("Tempo"));

      img = new ngrs::Image();
      img->setSharedBitmap(&icons.lessless());
      img->setPreferredSize(25,25);
      ngrs::Button* bpmDecBtnTen = new ngrs::Button(img);
      bpmDecBtnTen->setFlat(false);
      psycleControlBar_->add(bpmDecBtnTen)->clicked.connect(this,&MainWindow::onBpmDecTen);

      img = new ngrs::Image();
      img->setSharedBitmap(&icons.less());
      img->setPreferredSize(25,25);
      ngrs::Button* bpmDecBtnOne = new ngrs::Button(img);
      bpmDecBtnOne->setFlat(false);
      psycleControlBar_->add(bpmDecBtnOne)->clicked.connect(this,&MainWindow::onBpmDecOne);

      bpmDisplay_ = new ngrs::Seg7Display(3);
      bpmDisplay_->setColors( ngrs::Color(250,250,250), ngrs::Color(100,100,100), ngrs::Color(230,230,230));
      bpmDisplay_->setNumber( 125 );
      psycleControlBar_->add( bpmDisplay_ );

      img = new ngrs::Image();
      img->setSharedBitmap(&icons.more());
      img->setPreferredSize( 25, 25 );

      ngrs::Button* bpmIncBtnOne = new ngrs::Button(img);
      bpmIncBtnOne->setFlat(false);
      psycleControlBar_->add(bpmIncBtnOne)->clicked.connect(this,&MainWindow::onBpmIncOne);

      img = new ngrs::Image();
      img->setSharedBitmap(&icons.moremore());
      img->setPreferredSize(25,25);
      ngrs::Button* moremoreBmp = new ngrs::Button(img);
      moremoreBmp->setFlat(false);
      moremoreBmp->clicked.connect(this,&MainWindow::onBpmAddTen);
      psycleControlBar_->add(moremoreBmp);

      

      psycleToolBar_ = new ngrs::ToolBar();
      psycleToolBar_->add(new ngrs::ToolBarSeparator());
      genCombo_ = new ngrs::ComboBox();
      genCombo_->setPreferredSize( 158, 20 );
      genCombo_->setIndex(0);
      genCombo_->enableFocus(false);
      genCombo_->itemSelected.connect(this,&MainWindow::onGeneratorCbx);
      psycleToolBar_->add(genCombo_);

      img = new ngrs::Image();
      img->setSharedBitmap(&icons.littleleft());
      img->setPreferredSize(25,25);
      psycleToolBar_->add(new ngrs::Button(img));

      img = new ngrs::Image();
      img->setSharedBitmap(&icons.littleright());
      img->setPreferredSize(25,25);
      psycleToolBar_->add(new ngrs::Button(img));

      psycleToolBar_->add(new ngrs::Button("Gear Rack"));

      insCombo_ = new ngrs::ComboBox();
      insCombo_->setPreferredSize( 158, 20 );
      insCombo_->enableFocus(false);
      insCombo_->itemSelected.connect(this,&MainWindow::onInstrumentCbx);
      psycleToolBar_->add(insCombo_);


      toolBarPanel_->add(psycleToolBar_);

      toolBarPanel_->resize();
    }

    void MainWindow::onConfigMenuSkin( ngrs::ButtonEvent* ev ) {
      // add and create the temporay fileDialog
      ngrs::FileDialog* openDialog = new ngrs::FileDialog();
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
      ngrs::App::addRemovePipe( openDialog );
    }

    void MainWindow::onInstrumentCbx( ngrs::ItemEvent * ev )
    {
    }

    void MainWindow::updateComboIns( bool updatelist )
    {
    }

    void MainWindow::onBarPlay( ngrs::ButtonEvent * ev )
    {
      if ( !selectedChildView_ ) return;
      selectedChildView_->play();
    }

    void MainWindow::onBarPlayFromStart( ngrs::ButtonEvent * ev )
    {
      if ( !selectedChildView_ ) return;
      selectedChildView_->playFromStart();
    }

    void MainWindow::onFileNew( ngrs::ButtonEvent * ev )
    {
      addChildView();  
      pane()->resize();
      pane()->repaint();
    }

    void MainWindow::onFileOpen( ngrs::ButtonEvent * ev )
    {	
      // add and create the temporay fileDialog
      ngrs::FileDialog* openDialog = new ngrs::FileDialog();
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
          // load the song
          statusBarData.setText("loading \"" + openDialog->fileName() + "\"" );
          newView->song()->load(fileName);
          // update gui to new song
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
          recentFileMenu_->add(new ngrs::MenuItem(fileName));
          statusBarData.setText("loaded \"" + openDialog->fileName() + "\"");
        }
        //progressBar_->setVisible(false);
      }

      // remove the "Open"-FileDialog
      erase( openDialog );
      ngrs::App::addRemovePipe( openDialog );
    }

    void MainWindow::onFileSave( ngrs::ButtonEvent * ev )
    {

    }

    void MainWindow::onFileSaveAs( ngrs::ButtonEvent * ev )
    {
      if ( !selectedChildView_ ) return;

      ngrs::FileDialog* saveDialog = new ngrs::FileDialog();
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
      ngrs::App::addRemovePipe( saveDialog );
    }

    void MainWindow::onSongLoadProgress( const std::uint32_t & a, const std::uint32_t & b , const std::string & t)
    {
      if (a == 4) {
        std::cout << "progress" << std::endl;
        statusBarData.setText( stringify( (int) b), 1 );
        ngrs::App::flushEventQueue();
      } else
        if (a == 2) {
          statusBarData.setText( t );
        }
    }

    void MainWindow::onBarStop( ngrs::ButtonEvent* ev )
    {
      bool pl = Player::Instance()->playing();
      Player::Instance()->stop();
    }

    void MainWindow::closePsycle()
    {
      exit(0);
    }

    void MainWindow::updateComboGen() {
    }

    void MainWindow::onBpmIncOne( ngrs::ButtonEvent* ev )  // OnBpmAddOne
    {
      setAppSongBpm(1);
    }

    void MainWindow::onBpmAddTen( ngrs::ButtonEvent* ev )
    {
      setAppSongBpm(10);
    }

    void MainWindow::onBpmDecOne( ngrs::ButtonEvent* ev )
    {
      setAppSongBpm(-1);
    }

    void MainWindow::onBpmDecTen( ngrs::ButtonEvent* ev )
    {
      setAppSongBpm(-10);
    }

    void MainWindow::setAppSongBpm( double  x )
    {
    }

    void MainWindow::onRecordWav( ngrs::ButtonEvent * ev )
    {
    }

    void MainWindow::onMachineSelected( Machine* mac ) {
    }

    void MainWindow::onTimer( )
    {
    }

    void MainWindow::updateBars( )
    {
    }

    int MainWindow::close( )
    {
      closePsycle();
      return ngrs::nDestroyWindow;
    }

    void MainWindow::onMachineView( ngrs::ButtonEvent* ev ) {
      if ( !selectedChildView_ ) return;
      selectedChildView_->showMachineView();
    }

    void MainWindow::onPatternView( ngrs::ButtonEvent* ev ) {
      if ( !selectedChildView_ ) return;
      selectedChildView_->showPatternView();
    }

    bool MainWindow::checkUnsavedSong( )
    {
      if ( !selectedChildView_ ) return true;
      Song* selectedSong_ = selectedChildView_->song();
      DefaultBitmaps & icons =  SkinReader::Instance()->bitmaps();

      ngrs::MessageBox* box = new ngrs::MessageBox("Save changes of : "+selectedSong_->fileName+" ?");
      box->setTitle("New Song");
      box->setButtonText("Yes","No","Abort");
      box->icon()->setSharedBitmap( &icons.alert() );
      add(box);
      bool result = false;
      switch ( box->execute() ) {
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
      ngrs::App::addRemovePipe(box);
      return result;
    }  

    // Sequencer menu events

    void MainWindow::onSequencerView( ngrs::ButtonEvent * ev )
    {
      if ( !selectedChildView_ ) return;

      selectedChildView_->showSequencerView();
    }

    void MainWindow::onViewMenuToolbar( ngrs::ButtonEvent * ev )
    {
      toolBar1_->setVisible(!toolBar1_->visible());
      pane()->resize();
      pane()->repaint();
    }

    void MainWindow::onViewMenuMachinebar( ngrs::ButtonEvent * ev )
    {
      psycleToolBar_->setVisible(!psycleToolBar_->visible());
      pane()->resize();
      pane()->repaint();
    }

    void MainWindow::onViewMenuSequencerbar( ngrs::ButtonEvent * ev )
    {
      if ( !selectedChildView_ ) return;
      selectedChildView_->sequencerBar()->setVisible(!selectedChildView_->sequencerBar()->visible() );
    }

    void MainWindow::onViewMenuStatusbar( ngrs::ButtonEvent * ev )
    {
      statusBar_->setVisible(!statusBar_->visible());
      pane()->resize();
      pane()->repaint();
    }

    // Help menu events

    void MainWindow::onHelpMenuAbout( ngrs::ButtonEvent * ev )
    {
      ngrs::MessageBox* about = new ngrs::MessageBox();
      about->setTitle("About Psycle(X)");
      about->setText( std::string("Psycle version (X alpha 0.1)\n") +
        std::string("(c) 2006 by  Stefan Nattkemper\n") +
        std::string("            Josep Segura\n") +
        std::string("            D.W. Aley\n") +
        std::string("GNU Public Licence 2.0") );
      about->setButtons( ngrs::nMsgOkBtn );
      add( about );
      about->execute();
      ngrs::App::addRemovePipe(about);
    }

    void MainWindow::onHelpMenuGreeting( ngrs::ButtonEvent * ev )
    {
      greetDlg->setVisible(true);
    }

    void MainWindow::onHelpMenuReadme( ngrs::ButtonEvent * ev )
    {
      infoDlg->loadFromFile( Global::pConfig()->hlpPath() + "readme.txt" );
      infoDlg->setVisible(true);
    }

    void MainWindow::onHelpMenuWhatsNew( ngrs::ButtonEvent * ev )
    {
      infoDlg->loadFromFile( Global::pConfig()->hlpPath() + "keys.txt" );
      infoDlg->setVisible( true );
    }

    void MainWindow::onHelpMenuTweaking( ngrs::ButtonEvent * ev )
    {
      infoDlg->loadFromFile( Global::pConfig()->hlpPath() + "tweaking.txt" );
      infoDlg->setVisible( true );
    }

    void MainWindow::onHelpMenuKeys( ngrs::ButtonEvent * ev )
    {
      infoDlg->loadFromFile( Global::pConfig()->hlpPath() + "whatsnew.txt" );
      infoDlg->setVisible( true );
    }

    void MainWindow::onNewMachine( ngrs::ButtonEvent * ev )
    {
    }

    void MainWindow::onRenderAsWave( ngrs::ButtonEvent * ev )
    {
    }

    void MainWindow::onEditUndo( ngrs::ButtonEvent * ev )
    {
      if ( !selectedChildView_ ) return;

      selectedChildView_->patternView()->doUndo();
    }

    void MainWindow::onEditRedo( ngrs::ButtonEvent * ev )
    {
    }

    void MainWindow::onEditPatternCut( ngrs::ButtonEvent * ev )
    {

    }

    void MainWindow::onEditPatternCopy( ngrs::ButtonEvent * ev )
    {
    }

    void MainWindow::onEditPatternPaste( ngrs::ButtonEvent * ev )
    {
    }

    void MainWindow::onEditPatternMix( ngrs::ButtonEvent * ev )
    {
    }

    void MainWindow::onEditPatternMixPaste( ngrs::ButtonEvent * ev )
    {
    }

    void MainWindow::onEditPatternDelete( ngrs::ButtonEvent * ev )
    {
    }

    void MainWindow::onEditBlockMixPaste( ngrs::ButtonEvent * ev )
    {
    }

    void MainWindow::onEditBlockDelete( ngrs::ButtonEvent * ev )
    {
    }

    void MainWindow::onEditBlockMix( ngrs::ButtonEvent * ev )
    {
    }

    void MainWindow::onEditBlockPaste( ngrs::ButtonEvent * ev )
    {
    }

    void MainWindow::onEditBlockCopy( ngrs::ButtonEvent * ev )
    {
    }

    void MainWindow::onEditBlockCut( ngrs::ButtonEvent * ev )
    {
    }

    void MainWindow::onEditSeqDelete( ngrs::ButtonEvent * ev )
    {
    }

    void MainWindow::onEditSeqCopy( ngrs::ButtonEvent * ev )
    {
    }

    void MainWindow::onEditSeqCut( ngrs::ButtonEvent * ev )
    {
    }

    void MainWindow::onFileExit( ngrs::ButtonEvent * ev )
    {
    }

    void MainWindow::onRecordNotesMode( ngrs::ButtonEvent * ev )
    {
    }

    void MainWindow::onSeqAdded( SinglePattern * pattern )
    {
      if ( !selectedChildView_ ) return;
      selectedChildView_->sequencerView()->addPattern( pattern);
    }

    void MainWindow::onNewMachineDialogAdded( Machine * mac )
    {
    }

    // ngrs::New index selected by a mouse click.
    void MainWindow::onGeneratorCbx( ngrs::ItemEvent * ev )
    {
      onNewIndexGeneratorCbx();
    }
   
    void MainWindow::onSequencerEntryClick( SequencerItem * item )
    {
      if (!selectedChildView_) return;
      selectedChildView_->sequencerBar()->setEntry(item);
    }

    void MainWindow::onKeyPress( const ngrs::KeyEvent& event )
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
        }
      }

      ngrs::Window::onKeyPress( event);
    }


    void MainWindow::updateNewSong( )
    {
    }

    void MainWindow::onUpdateInstrumentCbx( int index , bool update )
    {
    }

    void MainWindow::onMachineNameChanged( int machineIndex )
    {
    }

    void MainWindow::onMachineDeleted( int machineIndex )
    {
    }

  }
}
