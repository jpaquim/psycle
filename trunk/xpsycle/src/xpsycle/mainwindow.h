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
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "songpdlg.h"
#include "childview.h"
#include "sequencerbar.h"
#include "newmachine.h"
#include <ngrs/nwindow.h>
#include <ngrs/nmenubar.h>
#include <ngrs/nmenu.h>
#include <ngrs/nmenuitem.h>
#include <ngrs/nmenuseperator.h>
#include <ngrs/ntabbook.h>
#include <ngrs/ntoolbar.h>
#include <ngrs/nframeborder.h>
#include <ngrs/ntoolbarseparator.h>
#include <ngrs/ncombobox.h>
#include <ngrs/nslider.h>
#include <ngrs/nbevelborder.h>
#include <ngrs/nprogressbar.h>
#include <ngrs/nfiledialog.h>
#include <ngrs/n7segdisplay.h>

class NStatusBar;
class NTextStatusItem;

namespace psycle { namespace host {

class GreetDlg;
class AboutDlg;
class VuMeter;
class InstrumentEditor;
class InfoDlg;
class WaveSaveDlg;


/**
@author Stefan
*/
class MainWindow : public NWindow
{
public:
    MainWindow();

    ~MainWindow();

private:

    bool noFileWasYetLoaded;

    ChildView* childView_;
    NWindow* songpDlg_;
    NTabBook* tabBook_;
    SequencerBar* sequencerBar_;
    NPanel* toolBarPanel_;
        NButton* barPlayFromStartBtn_;
    NToolBar* toolBar1_;
    NToolBar* psycleToolBar_;
      NComboBox* patternCombo_;
      NComboBox* genCombo_;
      NComboBox* insCombo_;
      NComboBox* auxSelectCombo_;
    NToolBar* psycleControlBar_;
      NComboBox* trackCombo_;
      NComboBox* octaveCombo_;
      NSlider*   masterSlider_;
    NStatusBar* statusBar_;
      NProgressBar* progressBar_;

    NMenuBar* menuBar_;
    NMenu* fileMenu_;
    NMenu* recentFileMenu_;
    NMenu* editMenu_;
    NMenu* viewMenu_;
    NMenu* configurationMenu_;
    NMenu* performanceMenu_;
    NMenu* helpMenu_;

    NMenuItem* noneFileItem;

    N7SegDisplay* bpmDisplay_;
    N7SegDisplay* tpbDisplay_;

    GreetDlg* greetDlg;
    InfoDlg*  infoDlg;
    NFileDialog* wavRecFileDlg;
    WaveSaveDlg* wavSaveDlg;

    VuMeter* vuMeter_;

    InstrumentEditor* instrumentEditor;

    NTextStatusItem* macPosStatusItem;
    NTextStatusItem* seqPosStatusItem;
    NTextStatusItem* seqPatStatusItem;
    NTextStatusItem* linePosStatusItem;
    NTextStatusItem* playTimeStatusItem;
    NTextStatusItem* octStatusItem;
    NTextStatusItem* editModeStatusItem;


    void initMenu();
    void initDialogs();
    void initViews();
    void initBars();
    void initToolBar();
    void initSignals();
    void initMachineView();

    void updateBars();

    void showSongpDlg(NButtonEvent* ev);

    void onBarPlay(NButtonEvent* ev);
    void onBarPlayFromStart(NButtonEvent* ev);
    void onBarStop(NButtonEvent* ev);

    void onFileMenuItemClicked(NEvent* menuEv, NButtonEvent* itemEv);
    void onViewMenuItemClicked(NEvent* menuEv, NButtonEvent* itemEv);
    void onHelpMenuItemClicked(NEvent* menuEv, NButtonEvent* itemEv);
    void onEditMenuItemClicked(NEvent* menuEv, NButtonEvent* itemEv);

    void onFileNew(NButtonEvent* ev);
    void onFileOpen(NButtonEvent* ev);
    void onFileSave(NButtonEvent* ev);
    void onFileSaveAs(NButtonEvent* ev);
    void onRenderAsWave(NButtonEvent* ev);

    void onEditUndo(NButtonEvent* ev);
    void onEditRedo(NButtonEvent* ev);
    void onEditPatternCut(NButtonEvent* ev);
    void onEditPatternCopy(NButtonEvent* ev);
    void onEditPatternPaste(NButtonEvent* ev);
    void onEditPatternMix(NButtonEvent* ev);
    void onEditPatternMixPaste(NButtonEvent* ev);
    void onEditPatternDelete(NButtonEvent* ev);
    void onEditBlockCut(NButtonEvent* ev);
    void onEditBlockCopy(NButtonEvent* ev);
    void onEditBlockPaste(NButtonEvent* ev);
    void onEditBlockMix(NButtonEvent* ev);
    void onEditBlockMixPaste(NButtonEvent* ev);
    void onEditBlockDelete(NButtonEvent* ev);
    void onEditSeqCut(NButtonEvent* ev);
    void onEditSeqCopy(NButtonEvent* ev);
    void onEditSeqDelete(NButtonEvent* ev);

    void onViewMenuToolbar(NButtonEvent* ev);
    void onViewMenuMachinebar(NButtonEvent* ev);
    void onViewMenuSequencerbar(NButtonEvent* ev);
    void onViewMenuStatusbar(NButtonEvent* ev);

    void onHelpMenuAbout(NButtonEvent* ev);
    void onHelpMenuGreeting(NButtonEvent* ev);
    void onHelpMenuReadme(NButtonEvent* ev);
    void onHelpMenuKeys(NButtonEvent* ev);
    void onHelpMenuTweaking(NButtonEvent* ev);
    void onHelpMenuWhatsNew(NButtonEvent* ev);

    void onLoadWave(NButtonEvent* ev);
    void onEditInstrument(NButtonEvent* ev);
    void onInstrumentCbx(NItemEvent* ev);

    void onMachineView(NButtonEvent* ev);
    void onPatternView(NButtonEvent* ev);
    void onSequencerView(NButtonEvent* ev);

    void onSongLoadProgress(const std::uint32_t& , const std::uint32_t& , const std::string&);
    void onOctaveChange(NItemEvent* ev);
    void onPatternStepChange(NItemEvent* ev);
    void onTrackChange(NItemEvent* ev);
    void onNewMachine(NButtonEvent* ev);

    void onRecordWav(NButtonEvent* ev);

    void setAppSongBpm(int x);
    void setAppSongTpb(int x);

    void onBpmIncOne(NButtonEvent* ev);
    void onBpmAddTen(NButtonEvent* ev);
    void onBpmDecOne(NButtonEvent* ev);
    void onBpmDecTen(NButtonEvent* ev);

    void onTpbDecOne(NButtonEvent* ev);
    void onTpbIncOne(NButtonEvent* ev);

    void onDecInsBtn(NButtonEvent* ev);
    void onIncInsBtn(NButtonEvent* ev);

    void updateComboGen();
    void appNew();

    void closePsycle();  // last but not least, all has an end

    void onTimer();

    bool checkUnsavedSong();

    int close();
    void updateComboIns(bool updatelist);

    void onMachineMoved(Machine* mac , int x, int y);
    void onLineChanged(int line);
    void onSeqSelected(int pos, int pat);
};

}}

#endif
