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
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "songpdlg.h"
#include "childview.h"
#include "sequencerbar.h"
#include <ngrs/window.h>
#include <ngrs/menubar.h>
#include <ngrs/menu.h>
#include <ngrs/menuitem.h>
#include <ngrs/menuseperator.h>
#include <ngrs/tabbook.h>
#include <ngrs/toolbar.h>
#include <ngrs/frameborder.h>
#include <ngrs/toolbarseparator.h>
#include <ngrs/combobox.h>
#include <ngrs/slider.h>
#include <ngrs/bevelborder.h>
#include <ngrs/progressbar.h>
#include <ngrs/filedialog.h>
#include <ngrs/seg7display.h>
#include <ngrs/tabbook.h>
#include <ngrs/statusmodel.h>
#include <ngrs/customstatusitem.h>

namespace ngrs {
  class NStatusBar;
  class TextStatusItem;
  class FileDialog;
}

namespace psycle { 
  namespace host {

    class GreetDlg;
    class AboutDlg;
    class VuMeter;
    class InfoDlg;
    class SequencerItem;
    class AudioConfigDlg;


    /**
    @author  Stefan
    */

    class ProgressStatusItem : public ngrs::CustomStatusItem {
    public:

      ProgressStatusItem();
      ~ProgressStatusItem();

      virtual void setText( const std::string & text );


    private:

      ngrs::ProgressBar* progressBar_;

    };


    class MainWindow : public ngrs::Window
    {
    public:
      MainWindow();

      ~MainWindow();

      virtual void onKeyPress( const ngrs::KeyEvent & event);	

    private:

      ngrs::Skin songTabSkinNone;
      ngrs::Skin songTabSkinDown;

      ngrs::Timer timer; // we poll playpos infos here to avoid thread sync with the audio thread

      ngrs::StatusModel statusBarData; // statusBar data for the mainWindow statusbar;

      bool oldPlayPos_;

      ChildView* addChildView();
      ChildView* selectedChildView_;

      bool noFileWasYetLoaded;

      SongpDlg* songpDlg_;
      ngrs::TabBook* tabBook_;
      ngrs::TabBar* tabBar_;

      ngrs::Panel* toolBarPanel_;
      ngrs::Button* barPlayFromStartBtn_;
      ngrs::ToolBar* toolBar1_;
      ngrs::ToolBar* psycleToolBar_;
      ngrs::ComboBox* genCombo_;
      ngrs::ToolBar* psycleControlBar_;
      ngrs::Slider*   masterSlider_;
      ngrs::NStatusBar* statusBar_;
      ProgressStatusItem* progressBar_;

      ngrs::ToolBar* playBar;

      ngrs::MenuBar* menuBar_;
      ngrs::Menu* fileMenu_;
      ngrs::Menu* recentFileMenu_;
      ngrs::Menu* editMenu_;
      ngrs::Menu* viewMenu_;
      ngrs::Menu* configurationMenu_;
      ngrs::Menu* performanceMenu_;
      ngrs::Menu* communityMenu_;
      ngrs::Menu* helpMenu_;

      ngrs::MenuItem* noneFileItem;

      ngrs::Seg7Display* bpmDisplay_;

      GreetDlg* greetDlg;
      InfoDlg*  infoDlg;
      ngrs::FileDialog* wavRecFileDlg;

      VuMeter* vuMeter_;

      ngrs::Panel* page;
      ngrs::TabBook* book;

      ngrs::ComboBox* insCombo_;

      AudioConfigDlg* audioConfigDlg;

      int count;

      void initMenu();
      void initDialogs();
      void initBars();
      void initToolBar();
      void initStatusBar();
      void initMachineView();

      void updateBars();

      void showSongpDlg( ngrs::ButtonEvent* ev );

      void onBarPlay( ngrs::ButtonEvent* ev );
      void onBarPlayFromStart( ngrs::ButtonEvent* ev );
      void onBarStop( ngrs::ButtonEvent* ev );

      void onHelpMenuItemClicked( ngrs::Event* menuEv, ngrs::ButtonEvent* itemEv );

      void onFileNew( ngrs::ButtonEvent* ev );
      void onFileOpen( ngrs::ButtonEvent* ev );
      void onFileSave( ngrs::ButtonEvent* ev );
      void onFileSaveAs( ngrs::ButtonEvent* ev );
      void onRenderAsWave( ngrs::ButtonEvent* ev );
      void onFileExit( ngrs::ButtonEvent* ev );

      void onEditUndo( ngrs::ButtonEvent* ev );
      void onEditRedo( ngrs::ButtonEvent* ev );
      void onEditPatternCut( ngrs::ButtonEvent* ev );
      void onEditPatternCopy( ngrs::ButtonEvent* ev );
      void onEditPatternPaste( ngrs::ButtonEvent* ev );
      void onEditPatternMix( ngrs::ButtonEvent* ev );
      void onEditPatternMixPaste( ngrs::ButtonEvent* ev );
      void onEditPatternDelete( ngrs::ButtonEvent* ev );
      void onEditBlockCut( ngrs::ButtonEvent* ev );
      void onEditBlockCopy( ngrs::ButtonEvent* ev );
      void onEditBlockPaste( ngrs::ButtonEvent* ev );
      void onEditBlockMix( ngrs::ButtonEvent* ev );
      void onEditBlockMixPaste( ngrs::ButtonEvent* ev );
      void onEditBlockDelete( ngrs::ButtonEvent* ev );
      void onEditSeqCut( ngrs::ButtonEvent* ev );
      void onEditSeqCopy( ngrs::ButtonEvent* ev );
      void onEditSeqDelete( ngrs::ButtonEvent* ev );

      void onViewMenuToolbar( ngrs::ButtonEvent* ev );
      void onViewMenuMachinebar( ngrs::ButtonEvent* ev );
      void onViewMenuSequencerbar( ngrs::ButtonEvent* ev );
      void onViewMenuStatusbar( ngrs::ButtonEvent* ev );

      void onConfigMenuAudio( ngrs::ButtonEvent* ev );
      void onConfigMenuSkin( ngrs::ButtonEvent* ev );

      void onHelpMenuAbout( ngrs::ButtonEvent* ev );
      void onHelpMenuGreeting( ngrs::ButtonEvent* ev );
      void onHelpMenuReadme( ngrs::ButtonEvent* ev );
      void onHelpMenuKeys( ngrs::ButtonEvent* ev );
      void onHelpMenuTweaking( ngrs::ButtonEvent* ev );
      void onHelpMenuWhatsNew( ngrs::ButtonEvent* ev );

      void onMachineView( ngrs::ButtonEvent* ev );
      void onPatternView( ngrs::ButtonEvent* ev );
      void onSequencerView( ngrs::ButtonEvent* ev );

      void onSongLoadProgress( const std::uint32_t& , const std::uint32_t& , const std::string&);
      void onNewMachine( ngrs::ButtonEvent* ev );

      void onRecordWav( ngrs::ButtonEvent* ev );
      void onRecordNotesMode( ngrs::ButtonEvent* ev );

      void onMachineDeleted( int machineIndex );
      void onMachineNameChanged( int machineIndex );

      void setAppSongBpm( double x );

      void onBpmIncOne( ngrs::ButtonEvent* ev );
      void onBpmAddTen( ngrs::ButtonEvent* ev );
      void onBpmDecOne( ngrs::ButtonEvent* ev );
      void onBpmDecTen( ngrs::ButtonEvent* ev );

      void onUpdateInstrumentCbx( int index , bool update );
      void onInstrumentCbx( ngrs::ItemEvent* ev );
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

      void onGeneratorCbx( ngrs::ItemEvent* ev );
      void onNewIndexGeneratorCbx();
      void changeGeneratorCbxViaKey( int dir ); 
      void changeInstrumentCbxViaKey( int dir );

      void onSequencerEntryClick( SequencerItem* item );
      void onMachineSelected( Machine* mac );

      void onCloseSongTabPressed( ngrs::ButtonEvent* ev );
      void onTabChange( ngrs::ButtonEvent* ev );
      std::map<Object*, ChildView*> songMap;
      std::map<Object*, ChildView*> songTabMap;

      void updateNewSong();

    };

  }
}

#endif
