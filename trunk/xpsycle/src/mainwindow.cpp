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
#include "napp.h"
#include "nitem.h"
#include "player.h"
#include "defaultbitmaps.h"

MainWindow::MainWindow()
 : NWindow()
{
  setPosition(0,0,1024,768);

  setTitle("Psycle Modular Music Creation Studio (nattisoft xport 0.001 port of psycledelcis 1.8.1 Releases with bugfixes)");
  initMenu();
  initDialogs();
  initBars();
  initViews();
  initSignals();
}


MainWindow::~MainWindow()
{
}

void MainWindow::initMenu( )
{
   menuBar_ = new NMenuBar();
   pane()->add(menuBar_);


   fileMenu_ = new NMenu("File",'i'
      ,"New,Open,Import Module,Save,Save as,Render as Wav,|,Song properties,|,revert to Saved,recent Files,Exit");
   menuBar_->add(fileMenu_);

   editMenu_ = new NMenu("Edit",'e',
       "Undo,Redo,Pattern Cut,Pattern Copy,Pattern Paste,Pattern Mix Paster,Pattern Delete,|,Block Cut,Block Copy,Block Paste,Block Mix Paste,Block Delete,|,Sequence Cut,Sequence Copy,Sequence Delete");
   menuBar_->add(editMenu_);

   viewMenu_ = new NMenu("View",'v',
       "Toolbar,MachineBar,SequencerBar,StatusBar,|,MachineView,PatternEditor,PatternSequencer,|,Add machine,Instrument Editor");
   menuBar_->add(viewMenu_);

   configurationMenu_ = new NMenu("Configuration",'c',
       "Free Audio,AutoStop,|,Loop Playback,|,Settings");
   menuBar_->add(configurationMenu_);

   performanceMenu_ = new NMenu("Performance",'p',"CPU Monitor ...,Midi Monitor ...");
   menuBar_->add(performanceMenu_);

   helpMenu_ = new NMenu("Help",'h',
       "Help,|,./doc/readme.txt,./doc/tweaking.txt,./doc/keys.txt,./doc/tweaking.txt,./doc/whatsnew.txt,|,About,Greetings");
   menuBar_->add(helpMenu_);
}

void MainWindow::initDialogs( )
{
  songpDlg_ = new SongpDlg();
  add(songpDlg_);
  newMachineDlg_ = new NewMachine();
  add(newMachineDlg_);
}

// events from menuItems

void MainWindow::showSongpDlg( NObject * sender )
{
  songpDlg_->setVisible(true);
}

void MainWindow::initViews( )
{
  childView_ = new ChildView();
  pane()->add(childView_);
  childView_->setTitleBarText();
  sequencerBar_->setPatternView(childView_->patternView());

  octaveCombo_->setIndex(4);
  trackCombo_->setIndex(12);  // starts at 4 .. so 16 - 4 = 12 ^= 16
}

void MainWindow::initBars( )
{
  toolBarPanel_ = new NPanel();
  NFlowLayout* fl = new NFlowLayout(nAlLeft,0,0);
  toolBarPanel_->setLayout(fl);
  toolBarPanel_->setWidth(500);
  toolBarPanel_->setAlign(nAlTop);
  pane()->add(toolBarPanel_);

  initToolBar();

  statusBar_ = new NPanel();
    statusBar_->setLayout(new NFlowLayout(nAlLeft));
    statusBar_->setAlign(nAlBottom);
      progressBar_ = new NProgressBar();
      progressBar_->setValue(0);
      progressBar_->setWidth(200);
      progressBar_->setHeight(25);
      progressBar_->setVisible(false);
      Global::pSong()->loadProgress.connect(this,&MainWindow::onSongLoadProgress);
    statusBar_->add(progressBar_);
  pane()->add(statusBar_);

  sequencerBar_ = new SequencerBar();
  sequencerBar_->setAlign(nAlLeft);
  pane()->add(sequencerBar_);

  sequencerBar_->updateSequencer();
}

void MainWindow::initToolBar( )
{
  toolBar1_ = new NToolBar();

  toolBarPanel_->add(toolBar1_);
    toolBar1_->setName("test1");
    NImage* img;
    img = new NImage();
    if (Global::pConfig()->iconPath=="") img->setSharedBitmap(&Global::pBitmaps()->newfile()); else
                                         img->loadFromFile(Global::pConfig()->iconPath+ "new.xpm");
    toolBar1_->add(new NButton(img,20,20));

    img = new NImage();
    if (Global::pConfig()->iconPath=="") img->setSharedBitmap(&Global::pBitmaps()->open()); else
                                         img->loadFromFile(Global::pConfig()->iconPath+ "open.xpm");
    toolBar1_->add(new NButton(img,20,20));

    img = new NImage();
    if (Global::pConfig()->iconPath=="") img->setSharedBitmap(&Global::pBitmaps()->save()); else
                                         img->loadFromFile(Global::pConfig()->iconPath+ "save.xpm");
    toolBar1_->add(new NButton(img,20,20));

    img = new NImage();
    if (Global::pConfig()->iconPath=="") img->setSharedBitmap(&Global::pBitmaps()->save_audio()); else
                                         img->loadFromFile(Global::pConfig()->iconPath+ "saveaudio.xpm");
    toolBar1_->add(new NButton(img,20,20));

    img = new NImage();
    if (Global::pConfig()->iconPath=="") img->setSharedBitmap(&Global::pBitmaps()->recordwav()); else
                                         img->loadFromFile(Global::pConfig()->iconPath+ "recordwav.xpm");
    toolBar1_->add(new NButton(img,20,20));


    toolBar1_->add(new NToolBarSeparator());

    img = new NImage();
    if (Global::pConfig()->iconPath=="") img->setSharedBitmap(&Global::pBitmaps()->undo()); else
                                         img->loadFromFile(Global::pConfig()->iconPath+ "undo.xpm");
    toolBar1_->add(new NButton(img,20,20));

    img = new NImage();
    if (Global::pConfig()->iconPath=="") img->setSharedBitmap(&Global::pBitmaps()->redo()); else
                                         img->loadFromFile(Global::pConfig()->iconPath+ "redo.xpm");
    toolBar1_->add(new NButton(img,20,20));

    img = new NImage();
    if (Global::pConfig()->iconPath=="") img->setSharedBitmap(&Global::pBitmaps()->recordnotes()); else
                                         img->loadFromFile(Global::pConfig()->iconPath+ "recordnotes.xpm");
    toolBar1_->add(new NButton(img,20,20));

    toolBar1_->add(new NToolBarSeparator());

    img = new NImage();
    if (Global::pConfig()->iconPath=="") img->setSharedBitmap(&Global::pBitmaps()->playstart()); else
                                         img->loadFromFile(Global::pConfig()->iconPath+ "recordnotes.xpm");
    toolBar1_->add(new NButton(img,20,20));

    img = new NImage();
    if (Global::pConfig()->iconPath=="") img->setSharedBitmap(&Global::pBitmaps()->playstart()); else
                                         img->loadFromFile(Global::pConfig()->iconPath+ "playstart.xpm");
    toolBar1_->add(barPlayFromStartBtn_ = new NButton(img,20,20));

    img = new NImage();
    if (Global::pConfig()->iconPath=="") img->setSharedBitmap(&Global::pBitmaps()->play()); else
                                         img->loadFromFile(Global::pConfig()->iconPath+ "play.xpm");
    toolBar1_->add(new NButton(img,20,20));

    img = new NImage();
    if (Global::pConfig()->iconPath=="") img->setSharedBitmap(&Global::pBitmaps()->playselpattern()); else
                                         img->loadFromFile(Global::pConfig()->iconPath+ "playselpattern.xpm");
    toolBar1_->add(new NButton(img,20,20));

    img = new NImage();
    if (Global::pConfig()->iconPath=="") img->setSharedBitmap(&Global::pBitmaps()->stop()); else
                                         img->loadFromFile(Global::pConfig()->iconPath+ "stop.xpm");
    NButton* stopBtn_ = new NButton(img,20,20);
       stopBtn_->click.connect(this,&MainWindow::onBarStop);
    toolBar1_->add(stopBtn_);

    img = new NImage();
    if (Global::pConfig()->iconPath=="") img->setSharedBitmap(&Global::pBitmaps()->autoStop()); else
                                         img->loadFromFile(Global::pConfig()->iconPath+ "autostop.xpm");
    toolBar1_->add(new NButton(img,20,20));

    toolBar1_->add(new NToolBarSeparator());


    img = new NImage();
    if (Global::pConfig()->iconPath=="") img->setSharedBitmap(&Global::pBitmaps()->machines()); else
                                         img->loadFromFile(Global::pConfig()->iconPath+ "machines.xpm");
    NButton* macBtn_ = new NButton(img,20,20);
      macBtn_->setFlat(false);
      macBtn_->setToggle(true);
    toolBar1_->add(macBtn_);

    img = new NImage();
    if (Global::pConfig()->iconPath=="") img->setSharedBitmap(&Global::pBitmaps()->patterns()); else
                                         img->loadFromFile(Global::pConfig()->iconPath+ "patterns.xpm");
    NButton* patBtn_ = new NButton(new NImage(Global::pConfig()->iconPath+"patterns.xpm"),20,20);
       patBtn_->setFlat(false);
       patBtn_->setToggle(true);
    toolBar1_->add(patBtn_);

    img = new NImage();
    if (Global::pConfig()->iconPath=="") img->setSharedBitmap(&Global::pBitmaps()->sequencer()); else
                                         img->loadFromFile(Global::pConfig()->iconPath+ "sequencer.xpm");
    toolBar1_->add(new NButton(img,20,20));

    toolBar1_->add(new NToolBarSeparator());


    img = new NImage();
    if (Global::pConfig()->iconPath=="") img->setSharedBitmap(&Global::pBitmaps()->newmachine()); else
                                         img->loadFromFile(Global::pConfig()->iconPath+ "newmachine.xpm");
    toolBar1_->add(new NButton(img,20,20));

    img = new NImage();
    if (Global::pConfig()->iconPath=="") img->setSharedBitmap(&Global::pBitmaps()->openeditor()); else
                                         img->loadFromFile(Global::pConfig()->iconPath+ "openeditor.xpm");
    toolBar1_->add(new NButton(img,20,20));

    toolBar1_->add(new NToolBarSeparator());

    img = new NImage();
    if (Global::pConfig()->iconPath=="") img->setSharedBitmap(&Global::pBitmaps()->p()); else
                                         img->loadFromFile(Global::pConfig()->iconPath+ "p.xpm");
    toolBar1_->add(new NButton(img,20,20));

    toolBar1_->add(new NToolBarSeparator());

    img = new NImage();
    if (Global::pConfig()->iconPath=="") img->setSharedBitmap(&Global::pBitmaps()->help()); else
                                         img->loadFromFile(Global::pConfig()->iconPath+ "help.xpm");
    toolBar1_->add(new NButton(img,20,20));

  toolBar1_->resize();

  psycleControlBar_ = new NToolBar();
    psycleControlBar_->add(new NLabel("Tracks"));
    trackCombo_ = new NComboBox();
      trackCombo_->setWidth(40);
      trackCombo_->setHeight(20);
      trackCombo_->itemSelected.connect(this,&MainWindow::onTrackChange);
    psycleControlBar_->add(trackCombo_);
      for(int i=4;i<=MAX_TRACKS;i++) {
       char s[4];
       sprintf(s,"%i",i);
       trackCombo_->add(new NItem(s));
      }
    psycleControlBar_->add(new NLabel("Tempo"));
     img = new NImage();
     if (Global::pConfig()->iconPath=="") img->setSharedBitmap(&Global::pBitmaps()->lessless()); else
                                           img->loadFromFile(Global::pConfig()->iconPath+ "lessless.xpm");
     psycleControlBar_->add(new NButton(img,20,20));

     img = new NImage();
     if (Global::pConfig()->iconPath=="") img->setSharedBitmap(&Global::pBitmaps()->less()); else
                                           img->loadFromFile(Global::pConfig()->iconPath+ "less.xpm");
     psycleControlBar_->add(new NButton(img,20,20));
    psycleControlBar_->add(new NLabel("125"));

     img = new NImage();
     if (Global::pConfig()->iconPath=="") img->setSharedBitmap(&Global::pBitmaps()->more()); else
                                           img->loadFromFile(Global::pConfig()->iconPath+ "more.xpm");
     psycleControlBar_->add(new NButton(img,20,20));


     img = new NImage();
     if (Global::pConfig()->iconPath=="") img->setSharedBitmap(&Global::pBitmaps()->moremore()); else
                                           img->loadFromFile(Global::pConfig()->iconPath+ "moremore.xpm");
     psycleControlBar_->add(new NButton(img,20,20));

    psycleControlBar_->add(new NLabel("Lines per beat"));
    img = new NImage();
     if (Global::pConfig()->iconPath=="") img->setSharedBitmap(&Global::pBitmaps()->less()); else
                                           img->loadFromFile(Global::pConfig()->iconPath+ "less.xpm");
     psycleControlBar_->add(new NButton(img,20,20));
    psycleControlBar_->add(new NLabel("4"));
     img = new NImage();
     if (Global::pConfig()->iconPath=="") img->setSharedBitmap(&Global::pBitmaps()->more()); else
                                           img->loadFromFile(Global::pConfig()->iconPath+ "more.xpm");
     psycleControlBar_->add(new NButton(img,20,20));
    psycleControlBar_->add(new NLabel("Octave"));
    octaveCombo_ = new NComboBox();
      for (int i=0; i<9; i++) octaveCombo_->add(new NItem(stringify(i)));
      octaveCombo_->itemSelected.connect(this,&MainWindow::onOctaveChange);
      octaveCombo_->setWidth(40);
      octaveCombo_->setHeight(20);
    psycleControlBar_->add(octaveCombo_);
    psycleControlBar_->add(new NLabel("VU"));
    NPanel* vuPanel = new NPanel();
    vuPanel->setPosition(0,0,100,22);
       NPanel* blackSS = new NPanel();
       blackSS->setPosition(0,2,100,7);
       blackSS->setBackground(NColor(0,0,0));
       vuPanel->add(blackSS);

       masterSlider_ = new NSlider();
       masterSlider_->setOrientation(nHorizontal);
       masterSlider_->setPosition(0,10,100,10);
       vuPanel->add(masterSlider_);
    psycleControlBar_->add(vuPanel);

  toolBarPanel_->add(psycleControlBar_);

  psycleToolBar_ = new NToolBar();
     psycleToolBar_->add(new NLabel("Pattern Step"));
     patternCombo_ = new NComboBox();
     patternCombo_->setWidth(40);
     patternCombo_->setHeight(20);
     psycleToolBar_->add(patternCombo_);
     psycleToolBar_->add(new NToolBarSeparator());
     genCombo_ = new NComboBox();
       genCombo_->setWidth(158);
       genCombo_->setHeight(20);
       updateComboGen();
     psycleToolBar_->add(genCombo_);

     img = new NImage();
     if (Global::pConfig()->iconPath=="") img->setSharedBitmap(&Global::pBitmaps()->littleleft()); else
                                           img->loadFromFile(Global::pConfig()->iconPath+ "littleleft.xpm");
     psycleToolBar_->add(new NButton(img,20,20));

     img = new NImage();
     if (Global::pConfig()->iconPath=="") img->setSharedBitmap(&Global::pBitmaps()->littleright()); else
                                           img->loadFromFile(Global::pConfig()->iconPath+ "littleright.xpm");
     psycleToolBar_->add(new NButton(img,20,20));

     psycleToolBar_->add(new NButton("Gear Rack"));
     psycleToolBar_->add(new NToolBarSeparator());
     auxSelectCombo_ = new NComboBox();
     auxSelectCombo_->setWidth(46);
     auxSelectCombo_->setHeight(20);
     psycleToolBar_->add(auxSelectCombo_);
     insCombo_ = new NComboBox();
     insCombo_->setWidth(158);
     insCombo_->setHeight(20);
     psycleToolBar_->add(insCombo_);

     img = new NImage();
     if (Global::pConfig()->iconPath=="") img->setSharedBitmap(&Global::pBitmaps()->littleleft()); else
                                           img->loadFromFile(Global::pConfig()->iconPath+ "littleleft.xpm");
     psycleToolBar_->add(new NButton(img,20,20));

     img = new NImage();
     if (Global::pConfig()->iconPath=="") img->setSharedBitmap(&Global::pBitmaps()->littleright()); else
                                           img->loadFromFile(Global::pConfig()->iconPath+ "littleright.xpm");
     psycleToolBar_->add(new NButton(img,20,20));

     psycleToolBar_->add(new NButton("Load"));
     psycleToolBar_->add(new NButton("Save"));
     psycleToolBar_->add(new NButton("Edit"));
     psycleToolBar_->add(new NButton("Wave Ed"));
     psycleToolBar_->resize();

  toolBarPanel_->add(psycleToolBar_);

  toolBarPanel_->resize();
}

void MainWindow::initSignals( )
{
  fileMenu_->itemClicked.connect(this, &MainWindow::onFileMenuItemClicked);
  viewMenu_->itemClicked.connect(this, &MainWindow::onViewMenuItemClicked);
  childView_->newSongLoaded.connect(sequencerBar_,&SequencerBar::updateSequencer);
  barPlayFromStartBtn_->click.connect(this,&MainWindow::onBarPlayFromStart);
}

void MainWindow::onBarPlayFromStart( NButtonEvent * ev )
{
  childView_->playFromStart();
}

void MainWindow::onFileMenuItemClicked(NEvent* menuEv, NButtonEvent* itemEv)
{
  if (itemEv->text()=="New") {
    appNew();
  }

  if (itemEv->text()=="Open") {
     usleep(200); // ugly hack but works
     progressBar_->setVisible(true);
     childView_->onFileLoadSong(0);
     progressBar_->setVisible(false);
     updateComboGen();
     repaint(0,0,width(),height());
  } else
  if (itemEv->text()=="Save") {
     usleep(200); // ugly hack but works
     progressBar_->setVisible(true);
     childView_->onFileSaveSong(0);
     progressBar_->setVisible(false);
     repaint(0,0,width(),height());
  } else
  if (itemEv->text()=="Song properties") {
     songpDlg_->setVisible(true);
  } else
  if (itemEv->text()=="Exit") {
     closePsycle();
  }
}

void MainWindow::onViewMenuItemClicked( NEvent * menuEv, NButtonEvent * itemEv )
{
  if (itemEv->text()=="Add machine") {
     if (newMachineDlg_->execute()) {
         if (newMachineDlg_->outBus()) {
           // Generator selected
           int x = 10; int y = 10;
           int fb = Global::pSong()->GetFreeBus();
           Global::pSong()->CreateMachine(MACH_PLUGIN, x, y, newMachineDlg_->getDllName().c_str(),fb);
           childView_->machineView()->addMachine(Global::pSong()->_pMachine[fb]);
           childView_->machineView()->repaint();
         }
     }
  }
}

void MainWindow::onSongLoadProgress( int chunkCount, int max, const std::string & header)
{
  progressBar_->setMax(max);
  progressBar_->setValue(chunkCount);
  progressBar_->repaint();
  NApp::flushEventQueue();
}

void MainWindow::onOctaveChange( NItemEvent * ev )
{
  std::stringstream str; 
  str << ev->item()->text();
  int octave = 0;
  str >> octave;
  childView_->patternView()->setEditOctave(octave);
}


void MainWindow::onTrackChange( NItemEvent * ev )
{
  std::stringstream str; 
  str << ev->item()->text();
  int track = 0;
  str >> track;
  Global::pSong()->SONGTRACKS=track;
  if (childView_->patternView()->cursor().x() >= Global::pSong()->SONGTRACKS)
  {
    childView_->patternView()->setCursor(NPoint3D(Global::pSong()->SONGTRACKS,childView_->patternView()->cursor().y(),0));
  }
  childView_->patternView()->repaint();
}

void MainWindow::onBarStop(NButtonEvent* ev)
{
  bool pl = Global::pPlayer()->_playing;
  bool blk = Global::pPlayer()->_playBlock;
  Global::pPlayer()->Stop();
  // pParentMain->SetAppSongBpm(0);
  // pParentMain->SetAppSongTpb(0);

  if (pl) {
    if ( Global::pConfig()->_followSong && blk) {
        //editPosition=prevEditPosition;
        //pParentMain->UpdatePlayOrder(false); // <- This restores the selected block
        //Repaint(DMPattern);
    } else {
       memset(Global::pSong()->playOrderSel,0,MAX_SONG_POSITIONS*sizeof(bool));
       Global::pSong()->playOrderSel[childView_->patternView()->editPosition()] = true;
       //Repaint(DMCursor); 
    }
  }
}

void MainWindow::closePsycle()
{
  // hopefully nobody wants to close it

  exit(0);
}


void MainWindow::updateComboGen() {

  bool filled=false;
  bool found=false;
  int selected = -1;
  int line = -1;
  char buffer[64];

  genCombo_->removeChilds();

  for (int b=0; b<MAX_BUSES; b++) // Check Generators
  {
   if( Global::pSong()->_pMachine[b]) {
     sprintf(buffer,"%.2X: %s",b,Global::pSong()->_pMachine[b]->_editName);
     genCombo_->add(new NItem(buffer));
       //cb->SetItemData(cb->GetCount()-1,b);

     if (!found) selected++;
     if (Global::pSong()->seqBus == b) found = true;
    }
   filled = true;
  }

  genCombo_->add(new NItem("----------------------------------------------------"));
  //cb->SetItemData(cb->GetCount()-1,65535);
  if (!found)  {
    selected++;
    line = selected;
  }

  for (int b=MAX_BUSES; b<MAX_BUSES*2; b++) // Write Effects Names.
  {
    if(Global::pSong()->_pMachine[b]) {
       sprintf(buffer,"%.2X: %s",b,Global::pSong()->_pMachine[b]->_editName);
       genCombo_->add(new NItem(buffer));
        //cb->SetItemData(cb->GetCount()-1,b);
     }
     if (!found) selected++;
     if (Global::pSong()->seqBus == b) found = true;
     filled = true;
  }

  if (!filled) {
     genCombo_->removeChilds();
     genCombo_->add(new NItem("No Machines Loaded"));
     selected = 0;
  } else if (!found)  {
    selected=line;
  }
}

void MainWindow::appNew( )
{
  //if (CheckUnsavedSong("New Song"))
  {
    //KillUndo();
    //KillRedo();
    Global::pPlayer()->Stop();
    childView_->machineView()->removeMachines();

    //Sleep(LOCK_LATENCY);
    //_outputActive = false;
    //Global::pConfig->_pOutputDriver->Enable(false);
    // midi implementation
    //Global::pConfig->_pMidiInput->Close();
    //Sleep(LOCK_LATENCY);
    Global::pSong()->New();
    //_outputActive = true;
    //if (!Global::pConfig->_pOutputDriver->Enable(true))
    //{
      //_outputActive = false;
    //}
    //else
    //{
    // midi implementation
    //Global::pConfig->_pMidiInput->Open();
  }
  childView_->setTitleBarText();
  childView_->patternView()->setEditPosition(0);
  Global::pSong()->seqBus=0;
  //pParentMain->PsybarsUpdate(); // Updates all values of the bars
//  pParentMain->WaveEditorBackUpdate();
//  pParentMain->m_wndInst.WaveUpdate();
//  pParentMain->RedrawGearRackList();
//  pParentMain->UpdateSequencer();
//  pParentMain->UpdatePlayOrder(false); // should be done always after updatesequencer
				//pParentMain->UpdateComboIns(); PsybarsUpdate calls UpdateComboGen that always call updatecomboins
  pane()->repaint();

}
  //cb->SetCurSel(selected);

