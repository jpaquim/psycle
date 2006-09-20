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
#include "wavesavedlg.h"
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

namespace psycle { namespace host {

template<class T> inline T str_hex(const std::string &  value) {
   T result;

   std::stringstream str;
   str << value;
   if (! (str >> std::hex >> result) ) return -1;

   return result;
}


void add_font_path( char * path ) {
  int i;
  int npaths;
  char ** fontpath;
  char ** new_fontpath;

  fontpath = XGetFontPath( NApp::system().dpy(), &npaths );    
  if ( fontpath != NULL ) {
    /* check if path is already present */
    for ( i = 0; i < npaths; i++ )
      if ( strncmp( path, fontpath[i], strlen(path) ) == 0 ) {
			XFreeFontPath(fontpath);
			return;
      }
    new_fontpath = (char **) malloc( sizeof(char *)*( npaths + 1 ));
    memcpy( new_fontpath, fontpath, npaths*sizeof(char *));
    new_fontpath[npaths] = path;
    XSetFontPath( NApp::system().dpy(), new_fontpath, npaths+1 );
    XFreeFontPath(fontpath);
    free(new_fontpath);
  }
}




MainWindow::MainWindow()
  : NWindow()
{
	
//	add_font_path("/home/natti/xpsycle/fonts");

  int npaths_return;
  char** path = XGetFontPath( NApp::system().dpy(), &npaths_return);

	for ( int i = 0; i < npaths_return; i++ ) {
		std::cout << path[i] << std::endl;
	}

  SkinReader::Instance()->setDefaults();

  setTitle ("] Psycle Modular Music Creation Studio[ ( X alpha ) ");

  setPosition(0,0,1024,768);
	count = 0;

  std::cout<< "initmenu" <<std::endl;
  initMenu();
  std::cout << "initbars"<<std::endl;
  initBars();
  std::cout << "initdialogs" <<std::endl;
  initDialogs();
  std::cout << "book" << std::endl;
  book = new NTabBook();
    book->setTabBarVisible( false );
  pane()->add(book,nAlClient);

  std::cout << "newmachine" << std::endl;
  newMachineDlg_ = new NewMachine( );
  add(newMachineDlg_);
std::cout << "audioconfig" << std::endl;
	audioConfigDlg = new AudioConfigDlg( Global::pConfig() );
	add( audioConfigDlg );
std::cout << "initsongs" << std::endl;
  initSongs();
  std::cout << "enablesound" << std::endl;
  enableSound();
std::cout << " updatenewsong" << std::endl;
  updateNewSong();
 
  
//  updateStatusBar();

  //childView_->timer.timerEvent.connect(this,&MainWindow::onTimer);

 

}


MainWindow::~MainWindow()
{
}

void MainWindow::enableSound( )
{
  AudioDriver* pOut = Global::pConfig()->_pOutputDriver;
	Player::Instance()->setDriver( *pOut );  
}



void MainWindow::initSongs( )
{
  selectedChildView_ = addChildView();
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
  book->addPage( childView_, childView_->song()->name() + stringify( count ) );
  book->setActivePage( childView_ );

  count++;

	NTab* tab = book->tab( childView_ );
  tab->click.connect(this,&MainWindow::onTabChange);
  tab->setEnablePopupMenu(true);
  NPopupMenu* menu = tab->popupMenu();
  NMenuItem* closeSong = new NMenuItem("Close");
  menu->add( closeSong );
  closeSong->click.connect(this,&MainWindow::onCloseSongTabPressed);

  songMap[closeSong] = childView_;
  songTabMap[tab] = childView_;

  songpDlg_->setSong( childView_->song() );

  if (songMap.size() > 1) book->setTabBarVisible(true);

  Player::Instance()->song( childView_->song() );

  selectedChildView_ =  childView_;

  return childView_;
} 

void MainWindow::onCloseSongTabPressed( NButtonEvent* ev ) {
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
       Player::Instance()->Stop();
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

void MainWindow::onTabChange( NButtonEvent * ev )
{
 std::map<NObject*,ChildView*>::iterator it = songTabMap.find( ev->sender() ); 
  if ( it != songTabMap.end() ) {
    ChildView* view = it->second;
    std::cout << view->song()->name() << std::endl;
		Player::Instance()->Stop();
    Player::Instance()->song( view->song() );
    selectedChildView_ = view;
    updateNewSong();
    pane()->repaint();
  }
}


void MainWindow::onConfigMenuAudio(NButtonEvent* ev) {
	audioConfigDlg->setVisible( true );
}

void MainWindow::initMenu( )
{
  menuBar_ = new NMenuBar();
  pane()->add(menuBar_);

  DefaultBitmaps & icons = Global::pConfig()->icons();

  // Creates the file menu
  fileMenu_ = new NMenu("File");
    fileMenu_->add(new NMenuItem("New",icons.newfile() ))->click.connect(this,&MainWindow::onFileNew);
    fileMenu_->add(new NMenuItem("Open",icons.open()))->click.connect(this,&MainWindow::onFileOpen);
    fileMenu_->add(new NMenuItem("Import Module"));
    fileMenu_->add(new NMenuItem("Save",icons.save()));
    fileMenu_->add(new NMenuItem("Save as"))->click.connect(this,&MainWindow::onFileSaveAs);
    fileMenu_->add(new NMenuItem("Render as Wav"))->click.connect(this,&MainWindow::onRenderAsWave);
    fileMenu_->add(new NMenuSeperator());
    fileMenu_->add(new NMenuItem("Song properties"))->click.connect(this,&MainWindow::showSongpDlg);
    fileMenu_->add(new NMenuSeperator());
    fileMenu_->add(new NMenuItem("revert to saved"));
    NMenuItem* recentItem = new NMenuItem("recent files");
    fileMenu_->add(recentItem);
    recentFileMenu_ = new NMenu();
      recentItem->add(recentFileMenu_);
      noneFileItem = new NMenuItem("none");
        noneFileItem->setEnable(false);
      recentFileMenu_->add(noneFileItem);
      noFileWasYetLoaded = true;
    fileMenu_->add(new NMenuItem("exit"))->click.connect(this,&MainWindow::onFileExit);
  menuBar_->add(fileMenu_);

  // Creates the edit menu
  editMenu_ = new NMenu("Edit");
      editMenu_->add(new NMenuItem("Undo",icons.undo()))->click.connect(this,&MainWindow::onEditUndo);
      editMenu_->add(new NMenuItem("Redo",icons.redo()))->click.connect(this,&MainWindow::onEditRedo);
      editMenu_->add(new NMenuSeperator());
      editMenu_->add(new NMenuItem("Pattern Cut"))->click.connect(this,&MainWindow::onEditPatternCut);
      editMenu_->add(new NMenuItem("Pattern Copy"))->click.connect(this,&MainWindow::onEditPatternCopy);
      editMenu_->add(new NMenuItem("Pattern Paste"))->click.connect(this,&MainWindow::onEditPatternPaste);
      editMenu_->add(new NMenuItem("Pattern Mix"))->click.connect(this,&MainWindow::onEditPatternMix);
      editMenu_->add(new NMenuItem("Pattern Mix Paste"))->click.connect(this,&MainWindow::onEditPatternMixPaste);
      editMenu_->add(new NMenuItem("Pattern Delete"))->click.connect(this,&MainWindow::onEditPatternDelete);
      editMenu_->add(new NMenuSeperator());
      editMenu_->add(new NMenuItem("Block Cut"))->click.connect(this,&MainWindow::onEditBlockCut);
      editMenu_->add(new NMenuItem("Block Copy"))->click.connect(this,&MainWindow::onEditBlockCopy);
      editMenu_->add(new NMenuItem("Block Paste"))->click.connect(this,&MainWindow::onEditBlockPaste);
      editMenu_->add(new NMenuItem("Block Mix Paste"))->click.connect(this,&MainWindow::onEditBlockMixPaste);
      editMenu_->add(new NMenuItem("Block Delete"))->click.connect(this,&MainWindow::onEditBlockDelete);
      editMenu_->add(new NMenuSeperator());
      editMenu_->add(new NMenuItem("Sequence Cut"))->click.connect(this,&MainWindow::onEditSeqCut);
      editMenu_->add(new NMenuItem("Sequence Copy"))->click.connect(this,&MainWindow::onEditSeqCopy);
      editMenu_->add(new NMenuItem("Sequence Delete"))->click.connect(this,&MainWindow::onEditSeqDelete);
    menuBar_->add(editMenu_);

    // Creates the view menu
    viewMenu_ = new NMenu("View");
      viewMenu_->add(new NCheckMenuItem("Toolbar"))->click.connect(this,&MainWindow::onViewMenuToolbar);
      viewMenu_->add(new NCheckMenuItem("Machinebar"))->click.connect(this,&MainWindow::onViewMenuMachinebar);
      viewMenu_->add(new NCheckMenuItem("Sequencerbar"))->click.connect(this,&MainWindow::onViewMenuSequencerbar);
      viewMenu_->add(new NCheckMenuItem("Statusbar"))->click.connect(this,&MainWindow::onViewMenuStatusbar);
      viewMenu_->add(new NMenuSeperator());
      viewMenu_->add(new NMenuItem("PatternEditor",icons.patterns()))->click.connect(this,&MainWindow::onPatternView);
      viewMenu_->add(new NMenuItem("MachineView",icons.machines()))->click.connect(this,&MainWindow::onMachineView);
      viewMenu_->add(new NMenuItem("PatternSequencer",icons.sequencer()))->click.connect(this,&MainWindow::onSequencerView);
      viewMenu_->add(new NMenuSeperator());
      viewMenu_->add(new NMenuItem("Add machine",icons.newmachine()))->click.connect(this,&MainWindow::onNewMachine);
      viewMenu_->add(new NMenuSeperator());
//      viewMenu_->add(new NMenuItem("Instrument Editor"))->click.connect(this,&MainWindow::onEditInstrument);
    menuBar_->add(viewMenu_);

    // Creates the configuration menu
    configurationMenu_ = new NMenu("Configuration");
      configurationMenu_->add(new NMenuItem("Free Audio"));
      configurationMenu_->add(new NMenuItem("Autostop"));
      configurationMenu_->add(new NMenuSeperator());
      configurationMenu_->add(new NMenuItem("Loop Playback"));
      configurationMenu_->add(new NMenuSeperator());
      configurationMenu_->add(new NMenuItem("Audio Settings"))->click.connect(this,&MainWindow::onConfigMenuAudio);
    menuBar_->add(configurationMenu_);

    // Creates the performance menu
    performanceMenu_ = new NMenu("Performance");
      performanceMenu_->add(new NMenuItem("CPU Monitor"));
      performanceMenu_->add(new NMenuItem("MDI Monitor"));
    menuBar_->add(performanceMenu_);

    // Creates the help menu
    helpMenu_ = new NMenu("Help");
      helpMenu_->add(new NMenuItem("About"))->click.connect(this,&MainWindow::onHelpMenuAbout);
      helpMenu_->add(new NMenuItem("Greetings"))->click.connect(this,&MainWindow::onHelpMenuGreeting);
      helpMenu_->add(new NMenuSeperator());
      helpMenu_->add(new NMenuItem("readme"))->click.connect(this,&MainWindow::onHelpMenuReadme);
      helpMenu_->add(new NMenuItem("keys"))->click.connect(this,&MainWindow::onHelpMenuKeys);
      helpMenu_->add(new NMenuItem("tweaking"))->click.connect(this,&MainWindow::onHelpMenuTweaking);
      helpMenu_->add(new NMenuItem("whatsnew"))->click.connect(this,&MainWindow::onHelpMenuWhatsNew);
    menuBar_->add(helpMenu_);
}


void MainWindow::initDialogs( )
{
  // creates the song dialog for editing song name, author, and comment
  add( songpDlg_ = new SongpDlg( ) );
  // creates the greeting dialog, that greets people who help psycle development
  add( greetDlg =  new GreetDlg() );
  // creates the save dialog, that ask where to store wave files, recorded from playing a psy song
  wavRecFileDlg = new NFileDialog();
    wavRecFileDlg->setMode(nSave);
  add(wavRecFileDlg);
  // creates the info dialog, that displays in a memo readme keys tweaking and a whatsnew file
  add( infoDlg =  new InfoDlg() );
  add( wavSaveDlg = new WaveSaveDlg() );
}

// events from menuItems

void MainWindow::showSongpDlg( NButtonEvent* ev )
{
  if ( !selectedChildView_) return;

  songpDlg_->setSong( selectedChildView_->song() );
  songpDlg_->setVisible(true);
}

void MainWindow::initBars( )
{
  toolBarPanel_ = new NPanel();
    toolBarPanel_->setLayout(NFlowLayout(nAlLeft,0,2));
    toolBarPanel_->setWidth(500);
  pane()->add(toolBarPanel_, nAlTop);

  initToolBar();

  statusBar_ = new NStatusBar();
    progressBar_ = new NProgressBar();
        progressBar_->setValue(0);
        progressBar_->setMax(16385);
        progressBar_->setWidth(200);
        progressBar_->setHeight(25);
        progressBar_->setVisible(false);
      //  Global::pSong()->progress.connect(this,&MainWindow::onSongLoadProgress);
    statusBar_->add(progressBar_,nAlLeft);
    macPosStatusItem = new NTextStatusItem();
    statusBar_->add(macPosStatusItem,nAlLeft);
    //childView_->machineView()->machineMoved.connect(this,&MainWindow::onMachineMoved);

    /*editModeStatusItem = new NTextStatusItem();
    statusBar_->add(editModeStatusItem);
    octStatusItem      = new NTextStatusItem();
    statusBar_->add(octStatusItem);
    playTimeStatusItem = new NTextStatusItem();
    statusBar_->add(playTimeStatusItem);*/
    linePosStatusItem  = new NTextStatusItem();
      //childView_->patternView()->lineChanged.connect(this,&MainWindow::onLineChanged);
    statusBar_->add(linePosStatusItem);
    seqPatStatusItem   = new NTextStatusItem();
    statusBar_->add(seqPatStatusItem);
    seqPosStatusItem   = new NTextStatusItem();
    statusBar_->add(seqPosStatusItem);

  pane()->add(statusBar_,nAlBottom);
}

void MainWindow::initToolBar( )
{
  DefaultBitmaps & icons = Global::pConfig()->icons();

  toolBar1_ = new NToolBar();
  toolBarPanel_->add(toolBar1_);

  NImage* img;

  // creates the newfile button
  img = new NImage();
    img->setSharedBitmap(&icons.newfile());
    img->setPreferredSize(25,25);
  NButton* newBtn = new NButton(img);
      newBtn->setHint("New song");
  toolBar1_->add(newBtn)->clicked.connect(this,&MainWindow::onFileNew);


  img = new NImage();
    img->setSharedBitmap(&icons.open());
    img->setPreferredSize(25,25);
  NButton* fileOpenBtn = new NButton(img);
    fileOpenBtn->setHint("Song load");
  toolBar1_->add(fileOpenBtn)->clicked.connect(this,&MainWindow::onFileOpen);


  img = new NImage();
    img->setSharedBitmap(&icons.save());
    img->setPreferredSize(25,25);
  NButton* saveBtn = new NButton(img);
    saveBtn->setHint("Save");
  toolBar1_->add(saveBtn)->clicked.connect(this,&MainWindow::onFileSaveAs);;

  img = new NImage();
    img->setSharedBitmap(&icons.save_audio());
    img->setPreferredSize(25,25);
  NButton* saveAsAudioFileBtn = new NButton(img);
     saveAsAudioFileBtn->setHint("Save as audio file");
  toolBar1_->add(saveAsAudioFileBtn);

  img = new NImage();
    img->setSharedBitmap(&icons.recordwav());
    img->setPreferredSize(25,25);
  NButton* recWav = new NButton(img);
    recWav->setToggle(true);
    recWav->setFlat(false);
    recWav->setHint("Record to .wav");
    recWav->clicked.connect(this, &MainWindow::onRecordWav);
  toolBar1_->add(recWav);

  toolBar1_->add(new NToolBarSeparator());

  img = new NImage();
    img->setSharedBitmap(&icons.undo());
    img->setPreferredSize(25,25);
  toolBar1_->add(new NButton(img));

  img = new NImage();
    img->setSharedBitmap(&icons.redo());
    img->setPreferredSize(25,25);
  toolBar1_->add(new NButton(img));

  toolBar1_->add(new NToolBarSeparator());

  img = new NImage();
    img->setSharedBitmap(&icons.recordnotes());
    img->setPreferredSize(25,25);
  NButton* recNotes = new NButton(img);
    recNotes->setHint("Record Notes Mode");
    recNotes->clicked.connect(this,&MainWindow::onRecordNotesMode);
  toolBar1_->add(recNotes);

  toolBar1_->add(new NToolBarSeparator());

  img = new NImage();
    img->setSharedBitmap(&icons.playstart());
    img->setPreferredSize(25,25);
  barPlayFromStartBtn_ = new NButton(img);
    barPlayFromStartBtn_->click.connect(this,&MainWindow::onBarPlayFromStart);
    barPlayFromStartBtn_->setHint("Play from start");
  toolBar1_->add(barPlayFromStartBtn_);


  img = new NImage();
    img->setSharedBitmap(&icons.play());
    img->setPreferredSize(25,25);
  NButton* playBtn = new NButton(img);
    playBtn->setHint("Play from edit position");
  toolBar1_->add(playBtn)->clicked.connect(this,&MainWindow::onBarPlay);

  img = new NImage();
    img->setSharedBitmap(&icons.playselpattern());
    img->setPreferredSize(25,25);
  toolBar1_->add(new NButton(img));

  img = new NImage();
    img->setSharedBitmap(&icons.stop());
    img->setPreferredSize(25,25);
  NButton* stopBtn_ = new NButton(img);
    stopBtn_->click.connect(this,&MainWindow::onBarStop);
    stopBtn_->setHint("Stop");
  toolBar1_->add(stopBtn_);

  img = new NImage();
    img->setSharedBitmap(&icons.autoStop());
    img->setPreferredSize(25,25);
  toolBar1_->add(new NButton(img));

  toolBar1_->add(new NToolBarSeparator());

  img = new NImage();
    img->setSharedBitmap(&icons.newmachine());
    img->setPreferredSize(25,25);
  NButton* newMacBtn = new NButton(img);
    newMacBtn->setHint("New Machine");
  toolBar1_->add(newMacBtn);//->clicked.connect(childView_,&ChildView::onMachineViewDblClick);

  img = new NImage();
    img->setSharedBitmap(&icons.openeditor());
    img->setPreferredSize(25,25);
/*  NButton* editInsBtn = new NButton(img);
    editInsBtn->setHint("Edit Instrument");
  toolBar1_->add(editInsBtn)->clicked.connect(this,&MainWindow::onEditInstrument);*/

  toolBar1_->add(new NToolBarSeparator());

  img = new NImage();
    img->setSharedBitmap(&icons.p());
    img->setPreferredSize(25,25);
  toolBar1_->add(new NButton(img));

  toolBar1_->add(new NToolBarSeparator());

  img = new NImage();
    img->setSharedBitmap(&icons.help());
    img->setPreferredSize(25,25);
  toolBar1_->add(new NButton(img));

  toolBar1_->resize();

  psycleControlBar_ = new NToolBar();
    psycleControlBar_->add(new NLabel("Tempo"));

    img = new NImage();
      img->setSharedBitmap(&icons.lessless());
      img->setPreferredSize(25,25);
    NButton* bpmDecBtnTen = new NButton(img);
      bpmDecBtnTen->setFlat(false);
    psycleControlBar_->add(bpmDecBtnTen)->clicked.connect(this,&MainWindow::onBpmDecTen);

    img = new NImage();
      img->setSharedBitmap(&icons.less());
      img->setPreferredSize(25,25);
    NButton* bpmDecBtnOne = new NButton(img);
      bpmDecBtnOne->setFlat(false);
    psycleControlBar_->add(bpmDecBtnOne)->clicked.connect(this,&MainWindow::onBpmDecOne);

    bpmDisplay_ = new N7SegDisplay(3);
      bpmDisplay_->setColors(NColor(250,250,250),NColor(100,100,100),NColor(230,230,230));
      bpmDisplay_->setNumber(125);
    psycleControlBar_->add(bpmDisplay_);

    img = new NImage();
      img->setSharedBitmap(&icons.more());
    img->setPreferredSize(25,25);

    NButton* bpmIncBtnOne = new NButton(img);
      bpmIncBtnOne->setFlat(false);
    psycleControlBar_->add(bpmIncBtnOne)->clicked.connect(this,&MainWindow::onBpmIncOne);

    img = new NImage();
      img->setSharedBitmap(&icons.moremore());
      img->setPreferredSize(25,25);
    NButton* moremoreBmp = new NButton(img);
      moremoreBmp->setFlat(false);
      moremoreBmp->clicked.connect(this,&MainWindow::onBpmAddTen);
    psycleControlBar_->add(moremoreBmp);

    psycleControlBar_->add(new NLabel("VU"));
    NPanel* vuPanel = new NPanel();
    vuPanel->setPosition(0,0,225,10);
        vuMeter_ = new VuMeter();
        vuPanel->add(vuMeter_);
        vuMeter_->setPosition(0,0,225,10);

        masterSlider_ = new NSlider();
        masterSlider_->setOrientation(nHorizontal);
        masterSlider_->setPosition(0,10,225,10);
        vuPanel->add(masterSlider_);
    psycleControlBar_->add(vuPanel);

  toolBarPanel_->add(psycleControlBar_);

  psycleToolBar_ = new NToolBar();
      psycleToolBar_->add(new NToolBarSeparator());
      genCombo_ = new NComboBox();
        genCombo_->setWidth(158);
        genCombo_->setHeight(20);
        genCombo_->setIndex(0);
        genCombo_->enableFocus(false);
        genCombo_->itemSelected.connect(this,&MainWindow::onGeneratorCbx);
      psycleToolBar_->add(genCombo_);

      img = new NImage();
        img->setSharedBitmap(&icons.littleleft());
        img->setPreferredSize(25,25);
      psycleToolBar_->add(new NButton(img));

      img = new NImage();
        img->setSharedBitmap(&icons.littleright());
        img->setPreferredSize(25,25);
      psycleToolBar_->add(new NButton(img));

      psycleToolBar_->add(new NButton("Gear Rack"));

			insCombo_ = new NComboBox();
				insCombo_->setWidth(158);
				insCombo_->setHeight(20);
				insCombo_->enableFocus(false);
				insCombo_->itemSelected.connect(this,&MainWindow::onInstrumentCbx);
				for (int i=0;i<PREV_WAV_INS;i++)
				{
          std::ostringstream buffer;
					buffer.str("");
					buffer << std::setfill('0') << std::hex << std::setw(2);
					buffer << i << ": " << "empty";
					insCombo_->add(new NItem(buffer.str()));
				}
			psycleToolBar_->add(insCombo_);


  toolBarPanel_->add(psycleToolBar_);

  toolBarPanel_->resize();
}

	void MainWindow::onInstrumentCbx( NItemEvent * ev )
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



void MainWindow::onBarPlay( NButtonEvent * ev )
{
  if ( !selectedChildView_ ) return;
  selectedChildView_->play();
}

void MainWindow::onBarPlayFromStart( NButtonEvent * ev )
{
  if ( !selectedChildView_ ) return;
  selectedChildView_->playFromStart();
}

void MainWindow::onFileNew( NButtonEvent * ev )
{
  addChildView();  
  pane()->resize();
  pane()->repaint();
}

void MainWindow::onFileOpen( NButtonEvent * ev )
{
  // add and create the temporay fileDialog
  NFileDialog* openDialog = new NFileDialog();
    openDialog->addFilter("*.psy [psy3 song format]","!S*.psy");
  add( openDialog );

  if ( openDialog->execute() ) {
     std::string fileName = openDialog->fileName();
     //  progressBar_->setVisible(true);
     //pane()->resize();
     // pane()->repaint();
     if ( fileName != "" ) {
			 // stop player
			 Player::Instance()->Stop();
       songpDlg_->setVisible(false);
			 // disable audio driver
			 //Global::configuration()._pOutputDriver->Enable(false);
		   // add a new Song tab
       ChildView* newView = addChildView();  
       // load the song
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
       recentFileMenu_->add(new NMenuItem(fileName));
     }
     //progressBar_->setVisible(false);
  }

  // remove the "Open"-FileDialog
  removeChild( openDialog);
}

void MainWindow::onFileSave( NButtonEvent * ev )
{
 
}

void MainWindow::onFileSaveAs( NButtonEvent * ev )
{
  if ( !selectedChildView_ ) return;

  NFileDialog* saveDialog = new NFileDialog();
    saveDialog->addFilter("*.psy [psy4 song format]","!S*.psy");
    saveDialog->setMode(nSave);
  add( saveDialog );

  if ( saveDialog->execute() ) {
    selectedChildView_->song()->save( saveDialog->fileName() );
  }

  //progressBar_->setVisible(true);
  //childView_->onFileSaveSong(0);  
  //progressBar_->setVisible(false);
  
  removeChild( saveDialog );
}

void MainWindow::onSongLoadProgress( const std::uint32_t & a, const std::uint32_t & b , const std::string & t)
{
  if (a == 4) {
    progressBar_->setValue(b);
    NApp::flushEventQueue();
  } else
  if (a == 2) {
    progressBar_->setText(t);
    progressBar_->repaint();
  }
}

void MainWindow::onBarStop(NButtonEvent* ev)
{
  bool pl = Player::Instance()->_playing;
  bool blk = Player::Instance()->_playBlock;
  Player::Instance()->Stop();
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
      genCombo_->add(new NItem(buffer.str()));

        //cb->SetItemData(cb->GetCount()-1,b);
      if (!found) selected++;
      if (selectedSong_->seqBus == b) found = true;
      filled = true;
    }
  }

  genCombo_->add(new NItem("----------------------------------------------------"));
  //cb->SetItemData(cb->GetCount()-1,65535);
  if (!found)  {
    selected++;
    line = selected;
  }

  for (int b=MAX_BUSES; b<MAX_BUSES*2; b++) // Write Effects Names.
  {
    if(selectedSong_->_pMachine[b]) {
      buffer.str("");
      buffer << std::setfill('0') << std::hex << std::setw(2);
      buffer << b << ": " << selectedSong_->_pMachine[b]->_editName;
      genCombo_->add(new NItem(buffer.str()));
      //cb->SetItemData(cb->GetCount()-1,b);
      if (!found) selected++;
      if (selectedSong_->seqBus == b) found = true;
      filled = true;
    }
  }

  if (!filled) {
      genCombo_->removeChilds();
      genCombo_->add(new NItem("No Machines Loaded"));
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

void MainWindow::onBpmIncOne(NButtonEvent* ev)  // OnBpmAddOne
{
  setAppSongBpm(1);
}

void MainWindow::onBpmAddTen(NButtonEvent* ev)
{
  setAppSongBpm(10);
}

void MainWindow::onBpmDecOne(NButtonEvent* ev)
{
  setAppSongBpm(-1);
}

void MainWindow::onBpmDecTen(NButtonEvent* ev)
{
  setAppSongBpm(-10);
}

void MainWindow::setAppSongBpm(int x)
{
  if ( !selectedChildView_ ) return;

  Song* selectedSong_ = selectedChildView_->song();

    int bpm = 0;
    if ( x != 0 ) {
      if ( Player::Instance()->_playing )  {
        selectedSong_->setBpm(  Player::Instance()->timeInfo().bpm()+x);
      } else selectedSong_->setBpm(selectedSong_->bpm()+x);
       Player::Instance()->setBpm( (int) selectedSong_->bpm() );
      bpm = selectedSong_->bpm();
    }
    else bpm =  Player::Instance()->bpm();

    bpmDisplay_->setNumber( (int)  Player::Instance()->bpm() );

    bpmDisplay_->repaint();
}

void MainWindow::onRecordWav( NButtonEvent * ev )
{

}

void MainWindow::onMachineSelected( Machine* mac ) {

  if ( !selectedChildView_ ) return;

  Song* selectedSong_ = selectedChildView_->song();


  std::vector< NCustomItem * > items = genCombo_->items();
  std::vector< NCustomItem * >::iterator it = items.begin();

  int idx = 0;
  for ( ; it < items.end(); it++) {
     NCustomItem* item = *it;
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

  if ( Player::Instance()->_playing) {
    
  }

  vuMeter_->setPegel(selectedSong_->_pMachine[MASTER_INDEX]->_lMax,
  selectedSong_->_pMachine[MASTER_INDEX]->_rMax );
  vuMeter_->repaint();
  ((Master*)selectedSong_->_pMachine[MASTER_INDEX])->vuupdated = true;

  if ( !selectedChildView_ ) return;
  selectedChildView_->machineView()->updateVUs();
}

void MainWindow::updateBars( )
{
}

int MainWindow::close( )
{
  closePsycle();
}

void MainWindow::onMachineView(NButtonEvent* ev) {
  if ( !selectedChildView_ ) return;
  selectedChildView_->showMachineView();
}

void MainWindow::onPatternView(NButtonEvent* ev) {
  if ( !selectedChildView_ ) return;
  selectedChildView_->showPatternView();
}

bool MainWindow::checkUnsavedSong( )
{
  if ( !selectedChildView_ ) return true;
  Song* selectedSong_ = selectedChildView_->song();
   
 NMessageBox* box = new NMessageBox("Save changes of : "+selectedSong_->fileName+" ?");
  box->setTitle("New Song");
  box->setButtonText("Yes","No","Abort");
  box->icon()->setSharedBitmap(&Global::pConfig()->icons().alert());
  box->icon()->setPreferredSize(Global::pConfig()->icons().alert().width(),Global::pConfig()->icons().alert().height());
  add(box);
  bool result = false;
  int choice = box->execute();
  switch (choice) {
    case nMsgOkBtn :
      onFileSave(0);
      result = true;
    break;
    case nMsgUseBtn:
      result = true;
    break;
    case nMsgCancelBtn:
      result = false;
    break;

  }
  NApp::addRemovePipe(box);
  return result;
}  

// Sequencer menu events

void MainWindow::onSequencerView( NButtonEvent * ev )
{
  if ( !selectedChildView_ ) return;

  selectedChildView_->showSequencerView();
}
   
void MainWindow::onViewMenuToolbar( NButtonEvent * ev )
{
  toolBar1_->setVisible(!toolBar1_->visible());
  pane()->resize();
  pane()->repaint();
}

void MainWindow::onViewMenuMachinebar( NButtonEvent * ev )
{
  psycleToolBar_->setVisible(!psycleToolBar_->visible());
  pane()->resize();
  pane()->repaint();
}

void MainWindow::onViewMenuSequencerbar( NButtonEvent * ev )
{
  if ( !selectedChildView_ ) return;
  selectedChildView_->sequencerBar()->setVisible(!selectedChildView_->sequencerBar()->visible() );

  

  //sequencerBar_->setVisible(!sequencerBar_->visible());
  //pane()->resize();
  //pane()->repaint();
}

void MainWindow::onViewMenuStatusbar( NButtonEvent * ev )
{
  statusBar_->setVisible(!statusBar_->visible());
  pane()->resize();
  pane()->repaint();
}

// Help menu events

void MainWindow::onHelpMenuAbout( NButtonEvent * ev )
{
  NMessageBox* about = new NMessageBox();
    about->setTitle("About Psycle(X)");
    about->setText( std::string("Psycle version (X alpha 0.1)\n") +
                    std::string("(c) 2006 by Psycledelics Community\n") +
                    std::string("GNU Public Licence 2.0") );
    about->setButtons(nMsgOkBtn);
  add(about);
  about->execute();
  NApp::addRemovePipe(about);
}

void MainWindow::onHelpMenuGreeting( NButtonEvent * ev )
{
  greetDlg->setVisible(true);
}

void MainWindow::onHelpMenuReadme( NButtonEvent * ev )
{
  infoDlg->loadFromFile(Global::pConfig()->hlpPath+"readme.txt");
  infoDlg->setVisible(true);

}

void MainWindow::onHelpMenuWhatsNew( NButtonEvent * ev )
{
  infoDlg->loadFromFile(Global::pConfig()->hlpPath+"keys.txt");
  infoDlg->setVisible(true);
}

void MainWindow::onHelpMenuTweaking( NButtonEvent * ev )
{
  infoDlg->loadFromFile(Global::pConfig()->hlpPath+"tweaking.txt");
  infoDlg->setVisible(true);
}

void MainWindow::onHelpMenuKeys( NButtonEvent * ev )
{
  infoDlg->loadFromFile(Global::pConfig()->hlpPath+"whatsnew.txt");
  infoDlg->setVisible(true);
}

void MainWindow::onNewMachine( NButtonEvent * ev )
{
  if ( !selectedChildView_ ) return;

  if (ev->button()==1) {
   if (newMachineDlg_->execute()) {
      if (newMachineDlg_->outBus()) {
          // Generator selected
          int x = 10; int y = 10;
          int fb = selectedChildView_->song()->GetFreeBus();
          if (newMachineDlg_->selectedType() == MACH_SAMPLER) {
            selectedChildView_->song()->CreateMachine(MACH_SAMPLER, x, y, "SAMPLER", fb);
            selectedChildView_->machineView()->addMachine( selectedChildView_->song()->_pMachine[fb]);
            selectedChildView_->newMachineAdded.emit( selectedChildView_->song()->_pMachine[fb]);
            selectedChildView_->machineView()->repaint();
          } else 
					if (newMachineDlg_->selectedType() == MACH_PLUGIN)
					{
            selectedChildView_->song()->CreateMachine(MACH_PLUGIN, x, y, newMachineDlg_->getDllName(),fb);
            selectedChildView_->machineView()->addMachine( selectedChildView_->song()->_pMachine[fb]);
            selectedChildView_->newMachineAdded.emit( selectedChildView_->song()->_pMachine[fb]);
            selectedChildView_->machineView()->repaint();
          } else 
					if (newMachineDlg_->selectedType() == MACH_LADSPA){
						selectedChildView_->song()->CreateMachine(MACH_LADSPA, x, y, newMachineDlg_->getDllName(),fb, newMachineDlg_->pluginIndex());
            selectedChildView_->machineView()->addMachine( selectedChildView_->song()->_pMachine[fb]);
            selectedChildView_->newMachineAdded.emit( selectedChildView_->song()->_pMachine[fb]);
            selectedChildView_->machineView()->repaint();
					}
      }
    }
  }
}

void MainWindow::onRenderAsWave( NButtonEvent * ev )
{
  if (wavSaveDlg->execute()) {

  }
}

void MainWindow::onEditUndo( NButtonEvent * ev )
{
}

void MainWindow::onEditRedo( NButtonEvent * ev )
{
}

void MainWindow::onEditPatternCut( NButtonEvent * ev )
{
  
}

void MainWindow::onEditPatternCopy( NButtonEvent * ev )
{
}

void MainWindow::onEditPatternPaste( NButtonEvent * ev )
{
}

void MainWindow::onEditPatternMix( NButtonEvent * ev )
{
}

void MainWindow::onEditPatternMixPaste( NButtonEvent * ev )
{
}

void MainWindow::onEditPatternDelete( NButtonEvent * ev )
{
}

void MainWindow::onEditBlockMixPaste( NButtonEvent * ev )
{
  if ( !selectedChildView_ ) return;

//  selectedChildView_->patternView()->pasteBlock(selectedChildView_->patternView()->cursor().track(), selectedChildView_->patternView()->cursor().line(), true);
}

void MainWindow::onEditBlockDelete( NButtonEvent * ev )
{
  if ( !selectedChildView_ ) return;

  selectedChildView_->patternView()->deleteBlock();
}

void MainWindow::onEditBlockMix( NButtonEvent * ev )
{
  
}

void MainWindow::onEditBlockPaste( NButtonEvent * ev )
{
  if ( !selectedChildView_ ) return;

//  selectedChildView_->patternView()->pasteBlock(selectedChildView_->patternView()->cursor().track(), selectedChildView_->patternView()->cursor().line(), false);
}

void MainWindow::onEditBlockCopy( NButtonEvent * ev )
{
  if ( !selectedChildView_ ) return;
  selectedChildView_->patternView()->copyBlock(false);
}

void MainWindow::onEditBlockCut( NButtonEvent * ev )
{
  if ( !selectedChildView_ ) return;
  selectedChildView_->patternView()->copyBlock(true);
}

void MainWindow::onEditSeqDelete( NButtonEvent * ev )
{
}

void MainWindow::onEditSeqCopy( NButtonEvent * ev )
{
}

void MainWindow::onEditSeqCut( NButtonEvent * ev )
{
}


void MainWindow::onMachineMoved( Machine * mac, int x, int y )
{
  macPosStatusItem->setText(stringify(mac->_macIndex)+":"+mac->_editName+" "+stringify(x) +","+ stringify(y));
  statusBar_->resize();
  statusBar_->repaint();
}


void MainWindow::onLineChanged(int line) {
  linePosStatusItem->setText("Line: "+stringify(line));
  statusBar_->resize();
  statusBar_->repaint();
}

void MainWindow::updateStatusBar( )
{
  if ( !selectedChildView_ ) return;  

  Machine* mac = selectedChildView_->machineView()->selMachine();
  if (mac) {
    macPosStatusItem->setText(stringify(mac->_macIndex)+":"+mac->_editName+" "+stringify(mac->_x) +","+ stringify(mac->_y));
  }
/*  seqPosStatusItem->setText("Pos: "+stringify(sequencerBar_->seqList()->selIndex()));
  seqPatStatusItem->setText("Pat: "+sequencerBar_->patternPos());
  linePosStatusItem->setText("Line: "+stringify(childView_->patternView()->cursor().y()));*/

  statusBar_->resize();
  statusBar_->repaint();
}


void MainWindow::onFileExit( NButtonEvent * ev )
{
  if (checkUnsavedSong()) {
    exit(0);
  }
}

void MainWindow::onRecordNotesMode( NButtonEvent * ev )
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

void MainWindow::onGeneratorCbx( NItemEvent * ev )
{
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

void MainWindow::onKeyPress( const NKeyEvent & event )
{
  if ( selectedChildView_ ) {
		switch (event.scancode() ) {
			case XK_F2 :
				selectedChildView_->showMachineView();
			break;
			case XK_F3 :
				selectedChildView_->showPatternView();
			break;
			case XK_F4 :
				selectedChildView_->showWaveView();
			break;
			case XK_F5 :
				selectedChildView_->showSequencerView();
			break;
			case XK_F9 : {
				 selectedChildView_->showMachineView();
         NButtonEvent btnEvent( this, 0,0,1, "");
				 onNewMachine( &btnEvent );
        }
			break;
			case XK_Up :
				if (NApp::system().keyState() & ControlMask) {
					selectedChildView_->sequencerBar()->selectPrevPattern();
				}
			break;
			case XK_Down :
				if (NApp::system().keyState() & ControlMask) {
					selectedChildView_->sequencerBar()->selectNextPattern();
				}
			break;
		}
	}

  NWindow::onKeyPress( event);
}

}}

void psycle::host::MainWindow::updateNewSong( )
{
  if (!selectedChildView_) return;

	updateComboIns(true);
  insCombo_->setIndex(0);
  updateComboGen();

  bpmDisplay_->setNumber( (int) selectedChildView_->song()->bpm() );
  bpmDisplay_->repaint();
}

void psycle::host::MainWindow::onUpdateInstrumentCbx( int index , bool update )
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

void psycle::host::MainWindow::onMachineDeleted( int machineIndex )
{
  updateComboGen();
  genCombo_->repaint();
}

