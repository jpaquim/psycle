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

#include <ngrs/dockpanel.h>
#include <ngrs/xmlparser.h>

namespace ngrs {
  class Menu;
  class MenuBar;
  class ToolBar;
  class ComboBox;
  class ItemEvent;
  class FileDialog;
}

namespace psycle { 
  namespace host {

    class WaveEdChildView;
    class InstrumentEditor;
    class Song;


    /// wave editor frame window.
    class WaveEdFrame : public ngrs::DockPanel {
    public:

      WaveEdFrame( Song* song );

      ~WaveEdFrame();

      Song* pSong();

    public:
      void Notify(void);

      sigslot::signal2<int,bool> updateInstrumentCbx;

      ngrs::MenuBar *menuBar;
      ngrs::Menu *processMenu;
      ngrs::Menu *editMenu;
      ngrs::Menu *viewMenu;
      ngrs::Menu *convertMenu;

    private:

      Song* _pSong;

      void InitStatusBar();
      void InitMenus();
      void InitToolBar();

      ngrs::ToolBar *toolBar;
      ngrs::ComboBox* auxSelectCombo_;
      WaveEdChildView *wavView;
      ngrs::FileDialog* wavSaveFileDlg;
      InstrumentEditor* instrumentEditor;

      int wsInstrument;

      void onPlay( ngrs::ButtonEvent *ev );
      void onRelease( ngrs::ButtonEvent *ev );
      void onPlayFromStart( ngrs::ButtonEvent *ev );
      void onStop( ngrs::ButtonEvent *ev );
      void onFastForward( ngrs::ButtonEvent *ev );
      void onRewind( ngrs::ButtonEvent *ev );
      void PlayFrom( unsigned long startpos );
      void Stop();

      void onLoadWave( ngrs::ButtonEvent* ev );
      void onSaveWave( ngrs::ButtonEvent* ev );
      void onEditInstrument( ngrs::ButtonEvent* ev );
      void onEditWave( ngrs::ButtonEvent* ev );

      void onSlotCopy( ngrs::ButtonEvent* ev );
      void onSlotPaste( ngrs::ButtonEvent* ev );

      void onDecInsBtn( ngrs::ButtonEvent* ev );
      void onIncInsBtn( ngrs::ButtonEvent* ev );

      bool mySel_;
      int leftSize;
      int rightSize;
      bool onWaveLeft;
      unsigned char* pDataLeft;
      unsigned char* pDataRight;

      int data_pos;

      void onTagParse( const ngrs::XmlParser& parser, const std::string& tagName );


    };

  }
}
