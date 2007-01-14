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
#include "pluginfinder.h"
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
#include <ngrs/ntabbook.h>
#include <ngrs/nstatusmodel.h>
#include <ngrs/ncustomstatusitem.h>

namespace ngrs {
  class NStatusBar;
  class NTextStatusItem;
  class NFileDialog;
}

namespace psycle { 
  namespace host {

    class GreetDlg;
    class AboutDlg;
    class VuMeter;
    class InstrumentEditor;
    class InfoDlg;
    class SequencerItem;
    class NewMachine;
    class AudioConfigDlg;


    /**
    @author Stefan
    */

    class ProgressStatusItem : public ngrs::NCustomStatusItem {
    public:

      ProgressStatusItem();
      ~ProgressStatusItem();

      virtual void setText( const std::string & text );


    private:

      ngrs::NProgressBar* progressBar_;

    };


    class MainWindow : public ngrs::NWindow
    {
    public:
      MainWindow();

      ~MainWindow();

      virtual void onKeyPress( const ngrs::NKeyEvent & event);	

    private:

      ngrs::NSkin songTabSkinNone;
      ngrs::NSkin songTabSkinDown;

      ngrs::NTimer timer; // we poll playpos infos here to avoid thread sync with the audio thread

      ngrs::NStatusModel statusBarData; // statusBar data for the mainWindow statusbar;

      bool oldPlayPos_;
      PluginFinder pluginFinder_; // needs ctor_init_list

      ChildView* addChildView();
      ChildView* selectedChildView_;

      bool noFileWasYetLoaded;

      SongpDlg* songpDlg_;
      ngrs::NTabBook* tabBook_;
      ngrs::NTabBar* tabBar_;

      ngrs::NPanel* toolBarPanel_;
      ngrs::NButton* barPlayFromStartBtn_;
      ngrs::NToolBar* toolBar1_;
      ngrs::NToolBar* psycleToolBar_;
      ngrs::NComboBox* genCombo_;
      ngrs::NToolBar* psycleControlBar_;
      ngrs::NSlider*   masterSlider_;
      ngrs::NStatusBar* statusBar_;
      ProgressStatusItem* progressBar_;

      ngrs::NToolBar* playBar;

      ngrs::NMenuBar* menuBar_;
      ngrs::NMenu* fileMenu_;
      ngrs::NMenu* recentFileMenu_;
      ngrs::NMenu* editMenu_;
      ngrs::NMenu* viewMenu_;
      ngrs::NMenu* configurationMenu_;
      ngrs::NMenu* performanceMenu_;
      ngrs::NMenu* communityMenu_;
      ngrs::NMenu* helpMenu_;

      ngrs::NMenuItem* noneFileItem;

      ngrs::N7SegDisplay* bpmDisplay_;

      GreetDlg* greetDlg;
      InfoDlg*  infoDlg;
      ngrs::NFileDialog* wavRecFileDlg;
      NewMachine* newMachineDlg_;

      VuMeter* vuMeter_;

      ngrs::NPanel* page;
      ngrs::NTabBook* book;

      ngrs::NComboBox* insCombo_;

      AudioConfigDlg* audioConfigDlg;

      int count;

      void initMenu();
      void initDialogs();
      void initBars();
      void initToolBar();
      void initStatusBar();
      void initMachineView();

      void updateBars();

      void showSongpDlg( ngrs::NButtonEvent* ev );

      void onBarPlay( ngrs::NButtonEvent* ev );
      void onBarPlayFromStart( ngrs::NButtonEvent* ev );
      void onBarStop( ngrs::NButtonEvent* ev );

      void onHelpMenuItemClicked( ngrs::NEvent* menuEv, ngrs::NButtonEvent* itemEv );

      void onFileNew( ngrs::NButtonEvent* ev );
      void onFileOpen( ngrs::NButtonEvent* ev );
      void onFileSave( ngrs::NButtonEvent* ev );
      void onFileSaveAs( ngrs::NButtonEvent* ev );
      void onRenderAsWave( ngrs::NButtonEvent* ev );
      void onFileExit( ngrs::NButtonEvent* ev );

      void onEditUndo( ngrs::NButtonEvent* ev );
      void onEditRedo( ngrs::NButtonEvent* ev );
      void onEditPatternCut( ngrs::NButtonEvent* ev );
      void onEditPatternCopy( ngrs::NButtonEvent* ev );
      void onEditPatternPaste( ngrs::NButtonEvent* ev );
      void onEditPatternMix( ngrs::NButtonEvent* ev );
      void onEditPatternMixPaste( ngrs::NButtonEvent* ev );
      void onEditPatternDelete( ngrs::NButtonEvent* ev );
      void onEditBlockCut( ngrs::NButtonEvent* ev );
      void onEditBlockCopy( ngrs::NButtonEvent* ev );
      void onEditBlockPaste( ngrs::NButtonEvent* ev );
      void onEditBlockMix( ngrs::NButtonEvent* ev );
      void onEditBlockMixPaste( ngrs::NButtonEvent* ev );
      void onEditBlockDelete( ngrs::NButtonEvent* ev );
      void onEditSeqCut( ngrs::NButtonEvent* ev );
      void onEditSeqCopy( ngrs::NButtonEvent* ev );
      void onEditSeqDelete( ngrs::NButtonEvent* ev );

      void onViewMenuToolbar( ngrs::NButtonEvent* ev );
      void onViewMenuMachinebar( ngrs::NButtonEvent* ev );
      void onViewMenuSequencerbar( ngrs::NButtonEvent* ev );
      void onViewMenuStatusbar( ngrs::NButtonEvent* ev );

      void onConfigMenuAudio( ngrs::NButtonEvent* ev );
      void onConfigMenuSkin( ngrs::NButtonEvent* ev );

      void onHelpMenuAbout( ngrs::NButtonEvent* ev );
      void onHelpMenuGreeting( ngrs::NButtonEvent* ev );
      void onHelpMenuReadme( ngrs::NButtonEvent* ev );
      void onHelpMenuKeys( ngrs::NButtonEvent* ev );
      void onHelpMenuTweaking( ngrs::NButtonEvent* ev );
      void onHelpMenuWhatsNew( ngrs::NButtonEvent* ev );

      void onMachineView( ngrs::NButtonEvent* ev );
      void onPatternView( ngrs::NButtonEvent* ev );
      void onSequencerView( ngrs::NButtonEvent* ev );

      void onSongLoadProgress( const std::uint32_t& , const std::uint32_t& , const std::string&);
      void onNewMachine( ngrs::NButtonEvent* ev );

      void onRecordWav( ngrs::NButtonEvent* ev );
      void onRecordNotesMode( ngrs::NButtonEvent* ev );

      void onMachineDeleted( int machineIndex );
      void onMachineNameChanged( int machineIndex );

      void setAppSongBpm( double x );

      void onBpmIncOne( ngrs::NButtonEvent* ev );
      void onBpmAddTen( ngrs::NButtonEvent* ev );
      void onBpmDecOne( ngrs::NButtonEvent* ev );
      void onBpmDecTen( ngrs::NButtonEvent* ev );

      void onUpdateInstrumentCbx( int index , bool update );
      void onInstrumentCbx( ngrs::NItemEvent* ev );
      void updateComboIns( bool updatelist );

      void updateComboGen();
      void appNew();

      void enableSound( );
      void closePsycle();  // last but not least, all has an end

      void onTimer();

      bool checkUnsavedSong();

      int close();

      void onSeqAdded( SinglePattern* pattern );

      void onNewMachineDialogAdded( Machine* mac );

      void onGeneratorCbx( ngrs::NItemEvent* ev );
      void onNewIndexGeneratorCbx();
      void changeGeneratorCbxViaKey( int dir ); 
      void changeInstrumentCbxViaKey( int dir );

      void onSequencerEntryClick( SequencerItem* item );
      void onMachineSelected( Machine* mac );

      void onCloseSongTabPressed( ngrs::NButtonEvent* ev );
      void onTabChange( ngrs::NButtonEvent* ev );
      std::map<NObject*, ChildView*> songMap;
      std::map<NObject*, ChildView*> songTabMap;

      void updateNewSong();

    };

  }
}

#endif
