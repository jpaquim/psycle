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

#include <nwindow.h>
#include <nmenubar.h>
#include <nmenu.h>
#include <nmenuitem.h>
#include <nmenuseperator.h>
#include <songpdlg.h>
#include <ntabbook.h>
#include <childview.h>
#include <sequencerbar.h>
#include <ntoolbar.h>
#include <nframeborder.h>
#include <ntoolbarseparator.h>
#include <ncombobox.h>
#include <nslider.h>
#include <nbevelborder.h>
#include <nprogressbar.h>
#include "newmachine.h"


class GreetDlg;
class AboutDlg;

/**
@author Stefan
*/
class MainWindow : public NWindow
{
public:
    MainWindow();

    ~MainWindow();

private:

    NewMachine* newMachineDlg_;
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
    NPanel* statusBar_;
      NProgressBar* progressBar_;

    NMenuBar* menuBar_;
    NMenu* fileMenu_;
    NMenu* editMenu_;
    NMenu* viewMenu_;
    NMenu* configurationMenu_;
    NMenu* performanceMenu_;
    NMenu* helpMenu_;

    NLabel* bpmLabel_;

    NFlowLayout* toolBarPanelLayout;
    NFlowLayout* statusBarPanelLayout;

    GreetDlg* greetDlg;
    AboutDlg* aboutDlg;

    void initMenu();
    void initDialogs();
    void initViews();
    void initBars();
    void initToolBar();
    void initSignals();
    void initMachineView();

    void showSongpDlg(NObject* sender);

    void onBarPlayFromStart(NButtonEvent* ev);
    void onBarStop(NButtonEvent* ev);

    void onFileMenuItemClicked(NEvent* menuEv, NButtonEvent* itemEv);
    void onViewMenuItemClicked(NEvent* menuEv, NButtonEvent* itemEv);
    void onHelpMenuItemClicked(NEvent* menuEv, NButtonEvent* itemEv);

    void onSongLoadProgress(int chunkCount, int max, const std::string & header);
    void onOctaveChange(NItemEvent* ev);
    void onTrackChange(NItemEvent* ev);
    void onNewMachine(NButtonEvent* ev);

    void setAppSongBpm(int x);
    void onBpmIncOne(NButtonEvent* ev);
    void onBpmAddTen(NButtonEvent* ev);
    void onBpmDecOne(NButtonEvent* ev);
    void onBpmDecTen(NButtonEvent* ev);

    void updateComboGen();
    void appNew();

    void closePsycle();  // last but not least, all has an end
};

#endif
