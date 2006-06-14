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
#include "wavesavedlg.h"
#include "defaultbitmaps.h"
#include "internal_machines.h"
#include <ngrs/napp.h>
#include <ngrs/nitem.h>
#include <ngrs/ncheckmenuitem.h>
#include <ngrs/nmenuseperator.h>
#include <ngrs/nmessagebox.h>
#include <ngrs/nbevelborder.h>
#include <ngrs/nstatusbar.h>

namespace psycle { namespace host {

MainWindow::MainWindow()
  : NWindow()
{
  setPosition(0,0,1024,768);

  initMenu();
  initDialogs();
  childView_ = new ChildView();
  initBars();
  initViews();
  initSignals();

  childView_->timer.timerEvent.connect(this,&MainWindow::onTimer);
}


MainWindow::~MainWindow()
{
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
    fileMenu_->add(new NMenuItem("exit"));
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
      viewMenu_->add(new NMenuItem("Instrument Editor"))->click.connect(this,&MainWindow::onEditInstrument);
    menuBar_->add(viewMenu_);

    // Creates the configuration menu
    configurationMenu_ = new NMenu("Configuration");
      configurationMenu_->add(new NMenuItem("Free Audio"));
      configurationMenu_->add(new NMenuItem("Autostop"));
      configurationMenu_->add(new NMenuSeperator());
      configurationMenu_->add(new NMenuItem("Loop Playback"));
      configurationMenu_->add(new NMenuSeperator());
      configurationMenu_->add(new NMenuItem("Settings"));
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
  add( songpDlg_ = new SongpDlg(Global::pSong()) );
  // creates the instrument editor for editing samples
  add( instrumentEditor = new InstrumentEditor() );
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
  songpDlg_->setVisible(true);
}

void MainWindow::initViews( )
{
  pane()->add(childView_);
  childView_->setTitleBarText();
  sequencerBar_->setPatternView(childView_->patternView());
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
        progressBar_->setVisible(false);        Global::pSong()->progress.connect(this,&MainWindow::onSongLoadProgress);
    statusBar_->add(progressBar_,nAlLeft);
  pane()->add(statusBar_,nAlBottom);

  pane()->add(sequencerBar_ = new SequencerBar(), nAlLeft);

  updateComboIns(true);
  insCombo_->setIndex(0);
  octaveCombo_->setIndex(4);
  childView_->patternView()->setEditOctave(4);
  trackCombo_->setIndex(12);  // starts at 4 .. so 16 - 4 = 12 ^= 16
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
    img->setSharedBitmap(&icons.machines());
    img->setPreferredSize(25,25);
  NButton* macBtn_ = new NButton(img);
    macBtn_->setFlat(false);
    macBtn_->setToggle(true);
    macBtn_->setHint("Machines");
    macBtn_->clicked.connect(this,&MainWindow::onMachineView);
  toolBar1_->add(macBtn_);

  img = new NImage();
    img->setSharedBitmap(&icons.patterns());
    img->setPreferredSize(25,25);
  NButton* patBtn_ = new NButton(img);
    patBtn_->clicked.connect(this,&MainWindow::onPatternView);
    patBtn_->setFlat(false);
    patBtn_->setToggle(true);
    patBtn_->setHint("Patterns");
  toolBar1_->add(patBtn_);

  img = new NImage();
    img->setSharedBitmap(&icons.sequencer());
    img->setPreferredSize(25,25);
  NButton* seqBtn = new NButton(img);
    seqBtn->setHint("Sequencer");
  toolBar1_->add(seqBtn)->clicked.connect(this,&MainWindow::onSequencerView);

  toolBar1_->add(new NToolBarSeparator());

  img = new NImage();
    img->setSharedBitmap(&icons.newmachine());
    img->setPreferredSize(25,25);
  NButton* newMacBtn = new NButton(img);
    newMacBtn->setHint("New Machine");
  toolBar1_->add(newMacBtn)->clicked.connect(childView_,&ChildView::onMachineViewDblClick);

  img = new NImage();
    img->setSharedBitmap(&icons.openeditor());
    img->setPreferredSize(25,25);
  NButton* editInsBtn = new NButton(img);
    editInsBtn->setHint("Edit Instrument");
  toolBar1_->add(editInsBtn)->clicked.connect(this,&MainWindow::onEditInstrument);

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
    psycleControlBar_->add(new NLabel("Tracks"));
    trackCombo_ = new NComboBox();
      trackCombo_->setWidth(40);
      trackCombo_->setHeight(20);
      trackCombo_->itemSelected.connect(this,&MainWindow::onTrackChange);
    psycleControlBar_->add(trackCombo_);
      for(int i=4;i<=MAX_TRACKS;i++) {
        trackCombo_->add(new NItem(stringify(i)));
      }
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
      img->loadFromFile(Global::pConfig()->iconPath+ "more.xpm");
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

    psycleControlBar_->add(new NLabel("Lines per beat"));

    img = new NImage();
      img->setSharedBitmap(&icons.less());
      img->setPreferredSize(25,25);
    NButton* lessTpbButton = new NButton(img);
        lessTpbButton->setFlat(false);
        lessTpbButton->clicked.connect(this,&MainWindow::onTpbDecOne);
    psycleControlBar_->add(lessTpbButton);

    tpbDisplay_ = new N7SegDisplay(2);
      tpbDisplay_->setColors(NColor(250,250,250),NColor(100,100,100),NColor(230,230,230));
      tpbDisplay_->setNumber(4);
    psycleControlBar_->add(tpbDisplay_);

    img = new NImage();
      img->setSharedBitmap(&icons.more());
      img->setPreferredSize(25,25);
    NButton* moreTpbButton = new NButton(img);
        moreTpbButton->setFlat(false);
        moreTpbButton->clicked.connect(this,&MainWindow::onTpbIncOne);
    psycleControlBar_->add(moreTpbButton);

    psycleControlBar_->add(new NLabel("Octave"));
    octaveCombo_ = new NComboBox();
      for (int i=0; i<9; i++) octaveCombo_->add(new NItem(stringify(i)));
      octaveCombo_->itemSelected.connect(this,&MainWindow::onOctaveChange);
      octaveCombo_->setWidth(40);
      octaveCombo_->setHeight(20);
    psycleControlBar_->add(octaveCombo_);

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
      psycleToolBar_->add(new NLabel("Pattern Step"));
      patternCombo_ = new NComboBox();
      for (int i = 1; i <=16; i++) 
        patternCombo_->add(new NItem(stringify(i)));
      patternCombo_->setIndex(0);
      patternCombo_->itemSelected.connect(this,&MainWindow::onPatternStepChange);
      patternCombo_->setWidth(40);
      patternCombo_->setHeight(20);
      psycleToolBar_->add(patternCombo_);
      psycleToolBar_->add(new NToolBarSeparator());
      genCombo_ = new NComboBox();
        genCombo_->setWidth(158);
        genCombo_->setHeight(20);
        updateComboGen();
        genCombo_->setIndex(0);
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
      psycleToolBar_->add(new NToolBarSeparator());
      auxSelectCombo_ = new NComboBox();
      auxSelectCombo_->setWidth(70);
      auxSelectCombo_->setHeight(20);
      auxSelectCombo_->add(new NItem("Wave"));
      auxSelectCombo_->setIndex(0);
      psycleToolBar_->add(auxSelectCombo_);
      insCombo_ = new NComboBox();
        insCombo_->setWidth(158);
        insCombo_->setHeight(20);
        insCombo_->itemSelected.connect(this,&MainWindow::onInstrumentCbx);
      psycleToolBar_->add(insCombo_);

      img = new NImage();
        img->setSharedBitmap(&icons.littleleft());
        img->setPreferredSize(25,25);
      psycleToolBar_->add(new NButton(img))->clicked.connect(this,&MainWindow::onDecInsBtn);

      img = new NImage();
        img->setSharedBitmap(&icons.littleright());
        img->setPreferredSize(25,25);
      psycleToolBar_->add(new NButton(img))->clicked.connect(this,&MainWindow::onIncInsBtn);

      psycleToolBar_->add(new NButton("Load"))->clicked.connect(this,&MainWindow::onLoadWave);
      psycleToolBar_->add(new NButton("Save"));
      psycleToolBar_->add(new NButton("Edit"))->clicked.connect(this,&MainWindow::onEditInstrument);
      psycleToolBar_->add(new NButton("Wave Ed"));
      psycleToolBar_->resize();

  toolBarPanel_->add(psycleToolBar_);

  toolBarPanel_->resize();
}

void MainWindow::initSignals( )
{
/*  fileMenu_->itemClicked.connect(this, &MainWindow::onFileMenuItemClicked);
  helpMenu_->itemClicked.connect(this, &MainWindow::onHelpMenuItemClicked);
  viewMenu_->itemClicked.connect(this, &MainWindow::onViewMenuItemClicked);
  childView_->newSongLoaded.connect(sequencerBar_,&SequencerBar::updateSequencer);
  barPlayFromStartBtn_->click.connect(this,&MainWindow::onBarPlayFromStart);*/
}


void MainWindow::onBarPlay( NButtonEvent * ev )
{
  childView_->play();
}

void MainWindow::onBarPlayFromStart( NButtonEvent * ev )
{
  childView_->playFromStart();
}

void MainWindow::onFileNew( NButtonEvent * ev )
{
  appNew();
}

void MainWindow::onFileOpen( NButtonEvent * ev )
{
  usleep(200); // ugly hack but works
  progressBar_->setVisible(true);
  pane()->resize();
  pane()->repaint();
  std::string fileName;
  if ( (fileName = childView_->onFileLoadSong(0)) != "" ) {
    if (noFileWasYetLoaded) {
      recentFileMenu_->removeChilds();
      noFileWasYetLoaded = false;
    }
    recentFileMenu_->add(new NMenuItem(fileName));
  }
  progressBar_->setVisible(false);
  pane()->resize();
  updateComboGen();
  sequencerBar_->updateSequencer();
  pane()->repaint();
}

void MainWindow::onFileSave( NButtonEvent * ev )
{
}

void MainWindow::onFileSaveAs( NButtonEvent * ev )
{
  usleep(200); // ugly hack but works
  progressBar_->setVisible(true);
  childView_->onFileSaveSong(0);
  progressBar_->setVisible(false);
  pane()->repaint();
}

void MainWindow::onFileMenuItemClicked(NEvent* menuEv, NButtonEvent* itemEv)
{
  if (itemEv->text()=="Song properties") {
      songpDlg_->setVisible(true);
  } else
  if (itemEv->text()=="Exit") {
      closePsycle();
  }
}

void MainWindow::onViewMenuItemClicked( NEvent * menuEv, NButtonEvent * itemEv )
{
  if (itemEv->text()=="Toolbar") {
  } else
  if (itemEv->text()=="MachineBar") {
  } else
  if (itemEv->text()=="SequencerBar") {
      
  } else
  if (itemEv->text()=="StatusBar") {
  } else
  if (itemEv->text()=="Add machine") {
      if (childView_->newMachineDlg()->execute()) {
          if (childView_->newMachineDlg()->outBus()) {
            // Generator selected
            int x = 10; int y = 10;
            int fb = Global::pSong()->GetFreeBus();
            Global::pSong()->CreateMachine(MACH_PLUGIN, x, y, childView_->newMachineDlg()->getDllName().c_str(),fb);
            childView_->machineView()->addMachine(Global::pSong()->_pMachine[fb]);
            childView_->machineView()->repaint();
          }
      }
  }
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
  Global::pSong()->tracks(track);
  if (childView_->patternView()->cursor().x() >= Global::pSong()->tracks() )
  {
    childView_->patternView()->setCursor(NPoint3D(Global::pSong()->tracks() ,childView_->patternView()->cursor().y(),0));
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
      sprintf(buffer,"%.2X: %s",b,Global::pSong()->_pMachine[b]->_editName.c_str());
      genCombo_->add(new NItem(buffer));
        //cb->SetItemData(cb->GetCount()-1,b);

      if (!found) selected++;
      if (Global::pSong()->seqBus == b) found = true;
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
    if(Global::pSong()->_pMachine[b]) {
        sprintf(buffer,"%.2X: %s",b,Global::pSong()->_pMachine[b]->_editName.c_str());
        genCombo_->add(new NItem(buffer));
        //cb->SetItemData(cb->GetCount()-1,b);
      if (!found) selected++;
      if (Global::pSong()->seqBus == b) found = true;
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
}

void MainWindow::appNew( )
{
  if (checkUnsavedSong())
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
  sequencerBar_->updateSequencer();
  childView_->machineView()->createGUIMachines();
  //pParentMain->PsybarsUpdate(); // Updates all values of the bars
//  pParentMain->WaveEditorBackUpdate();
//  pParentMain->m_wndInst.WaveUpdate();
//  pParentMain->RedrawGearRackList();
//  pParentMain->UpdateSequencer();
//  pParentMain->UpdatePlayOrder(false); // should be done always after updatesequencer
        //pParentMain->UpdateComboIns(); PsybarsUpdate calls UpdateComboGen that always call updatecomboins
  childView_->patternView()->repaint();
  childView_->machineView()->repaint();
  sequencerBar_->repaint();


}


void MainWindow::onEditMenuItemClicked( NEvent * menuEv, NButtonEvent * itemEv )
{
  if (itemEv->text()=="Undo") {

  } else
  if (itemEv->text()=="Redo") {

  } else
  if (itemEv->text()=="Pattern Cut") {

  } else 
  if (itemEv->text()=="Pattern Copy") {

  } else
  if (itemEv->text()=="Pattern Paste") {

  } else
  if (itemEv->text()=="Pattern Mix Paster") {

  } else
  if (itemEv->text()=="Pattern Delete") {

  } else
  if (itemEv->text()=="Block Cut") {
    childView_->patternView()->copyBlock(true);
    childView_->patternView()->repaint();
  } else
  if (itemEv->text()=="Block Copy") {
    childView_->patternView()->copyBlock(false);
    childView_->patternView()->repaint();
  } else
  if (itemEv->text()=="Block Paste") {
    PatternView* pView = childView_->patternView();
    pView->pasteBlock(pView->cursor().x(),pView->cursor().y(),false);
    childView_->patternView()->repaint();
  } else
  if (itemEv->text()=="Block Mix Paste") {

  } else
  if (itemEv->text()=="Block Delete") {
    childView_->patternView()->deleteBlock();
    childView_->patternView()->repaint();
  } else
  if (itemEv->text()=="Sequence Cut") {

  } else
  if (itemEv->text()=="Sequence Copy") {

  } else
  if (itemEv->text()=="Sequence Delete") {

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
    int bpm = 0;
    if ( x != 0 ) {
      if (Global::pPlayer()->_playing )  {
        Global::pSong()->BeatsPerMin(Global::pPlayer()->bpm+x);
      } else Global::pSong()->BeatsPerMin(Global::pSong()->BeatsPerMin()+x);
      Global::pPlayer()->SetBPM(Global::pSong()->BeatsPerMin(),Global::pSong()->LinesPerBeat());
      bpm = Global::pSong()->BeatsPerMin();
    }
    else bpm = Global::pPlayer()->bpm;

    bpmDisplay_->setNumber(Global::pPlayer()->bpm);

    bpmDisplay_->repaint();
}

void MainWindow::setAppSongTpb(int x)
{
  int tpb = 0;

  if ( x != 0)
  {
      if (Global::pPlayer()->_playing )
        Global::pSong()->LinesPerBeat(Global::pPlayer()->tpb+x);
      else 
        Global::pSong()->LinesPerBeat(Global::pSong()->LinesPerBeat()+x);
        Global::pPlayer()->SetBPM(Global::pSong()->BeatsPerMin(), Global::pSong()->LinesPerBeat());
        tpb = Global::pSong()->LinesPerBeat();
  } else tpb = Global::pPlayer()->tpb;

  tpbDisplay_->setNumber(tpb);

  psycleControlBar_->repaint();
}


void MainWindow::onTpbDecOne(NButtonEvent* ev)
{
  setAppSongTpb(-1);
  childView_->patternView()->repaint();
}

void MainWindow::onTpbIncOne(NButtonEvent* ev)
{
  setAppSongTpb(1);
  childView_->patternView()->repaint();
}

void MainWindow::onRecordWav( NButtonEvent * ev )
{
  if (!Global::pPlayer()->_recording)
  {
      if (wavRecFileDlg->execute()) {
        Global::pPlayer()->StartRecording(wavRecFileDlg->fileName());
      }
      if ( Global::pConfig()->autoStopMachines )
      {
        //OnAutostop();
      }
  }
  else
  {
      Global::pPlayer()->StopRecording();
  }
}

void MainWindow::onTimer( )
{
  if (Global::pPlayer()->_playing) {
    int oldPos = childView_->patternView()->editPosition();
    childView_->patternView()->updatePlayBar(sequencerBar_->followSong());

    if (sequencerBar_->followSong() && oldPos != Global::pPlayer()->_playPosition) {
        sequencerBar_->updatePlayOrder(true);
        sequencerBar_->updateSequencer();
    }
  }

  vuMeter_->setPegel(Global::pSong()->_pMachine[MASTER_INDEX]->_lMax,
  Global::pSong()->_pMachine[MASTER_INDEX]->_rMax );
  vuMeter_->repaint();
  ((Master*)Global::pSong()->_pMachine[MASTER_INDEX])->vuupdated = true;

  childView_->machineView()->updateVUs();
}

void MainWindow::updateBars( )
{
  int p[] = {1, 2, 3, 4, 5};
  std::vector<int> a(p, p+5);
}

int MainWindow::close( )
{
  closePsycle();
}

void MainWindow::onMachineView(NButtonEvent* ev) {
  childView_->setActivePage(0);
  childView_->repaint();
}

void MainWindow::onPatternView(NButtonEvent* ev) {
  childView_->setActivePage(1);
  childView_->repaint();
}

void MainWindow::onLoadWave( NButtonEvent * ev )
{
  NFileDialog* dialog = new NFileDialog();
  add(dialog);

  dialog->addFilter("Wav Files(*.wav)","!S*.wav");

  if (dialog->execute()) {
    int si = Global::pSong()->instSelected;
    //added by sampler
    if ( Global::pSong()->_pInstrument[si]->waveLength != 0)
    {
        //if (MessageBox("Overwrite current sample on the slot?","A sample is already loaded here",MB_YESNO) == IDNO)  return;
    }

    if (Global::pSong()->WavAlloc(si,dialog->fileName().c_str()))
    {
      updateComboIns(true);
      //m_wndStatusBar.SetWindowText("New wave loaded");
      //WaveEditorBackUpdate();
      //m_wndInst.WaveUpdate();
    }
  }

  NApp::addRemovePipe(dialog);
}

void MainWindow::updateComboIns( bool updatelist )
{
  if (updatelist)  {
    insCombo_->removeChilds();
    char buffer[64];
    int listlen = 0;
    for (int i=0;i<PREV_WAV_INS;i++)
    {
      sprintf(buffer, "%.2X: %s", i, Global::pSong()->_pInstrument[i]->_sName);
      insCombo_->add(new NItem(buffer));
      listlen++;
    }
    if (Global::pSong()->auxcolSelected >= listlen) {
      Global::pSong()->auxcolSelected = 0;
    }
  }
}

void MainWindow::onEditInstrument( NButtonEvent * ev )
{
  instrumentEditor->setInstrument(Global::pSong()->instSelected);
  instrumentEditor->setVisible(true);
}

void MainWindow::onDecInsBtn( NButtonEvent * ev )
{
  int index = Global::pSong()->instSelected -1;
  if (index >=0 ) {
    Global::pSong()->instSelected=   index;
    Global::pSong()->auxcolSelected= index;

    insCombo_->setIndex(index);
    insCombo_->repaint();
  }
}

void MainWindow::onIncInsBtn( NButtonEvent * ev )
{
  int index = Global::pSong()->instSelected +1;
  if (index <= 255) {
    Global::pSong()->instSelected=   index;
    Global::pSong()->auxcolSelected= index;

    insCombo_->setIndex(index);
    insCombo_->repaint();
  }
}

bool MainWindow::checkUnsavedSong( )
{
  NMessageBox* box = new NMessageBox("Save changes of : "+Global::pSong()->fileName+" ?");
  box->setTitle("New Song");
  box->setButtonText("Yes","No","Abort");
  add(box);
  bool result = box->execute();
  if (result == true) {
      childView_->onFileSaveSong(0);
  }
  NApp::addRemovePipe(box);
  return result;
}

void MainWindow::onPatternStepChange( NItemEvent * ev )
{
  if (patternCombo_->selIndex()!=-1) {
      childView_->patternView()->setPatternStep(patternCombo_->selIndex()+1);
  }
}

// Sequencer menu events

void MainWindow::onSequencerView( NButtonEvent * ev )
{
  NMessageBox* box = new NMessageBox("This feature is unimplemented in this release. Use the left side sequence now");
  box->setTitle("Psycle Notice");
  box->setButtons(nMsgOkBtn);
  box->execute();
  NApp::addRemovePipe(box);
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
  sequencerBar_->setVisible(!sequencerBar_->visible());
  pane()->resize();
  pane()->repaint();
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
  childView_->onMachineViewDblClick(ev);
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
  childView_->patternView()->pasteBlock(childView_->patternView()->cursor().x(), childView_->patternView()->cursor().y(), true);
}

void MainWindow::onEditBlockDelete( NButtonEvent * ev )
{
  childView_->patternView()->deleteBlock();
}

void MainWindow::onEditBlockMix( NButtonEvent * ev )
{
  
}

void MainWindow::onEditBlockPaste( NButtonEvent * ev )
{
  childView_->patternView()->pasteBlock(childView_->patternView()->cursor().x(), childView_->patternView()->cursor().y(), false);
}

void MainWindow::onEditBlockCopy( NButtonEvent * ev )
{
  childView_->patternView()->copyBlock(false);
}

void MainWindow::onEditBlockCut( NButtonEvent * ev )
{
  childView_->patternView()->copyBlock(true);
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

void MainWindow::onInstrumentCbx( NItemEvent * ev )
{
  int index = insCombo_->selIndex();
  Global::pSong()->instSelected=   index;
  Global::pSong()->auxcolSelected= index;
  insCombo_->setIndex(index);
}

}}