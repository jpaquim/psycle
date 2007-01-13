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

#include <ngrs/ndockpanel.h>
#include <ngrs/nxmlparser.h>

namespace ngrs {
  class NMenu;
  class NMenuBar;
  class NToolBar;
  class NComboBox;
  class NItemEvent;
  class NFileDialog;
}

namespace psycle { 
  namespace host {

    class WaveEdChildView;
    class InstrumentEditor;
    class Song;


    /// wave editor frame window.
    class WaveEdFrame : public ngrs::NDockPanel {
    public:

      WaveEdFrame( Song* song );

      ~WaveEdFrame();

      Song* pSong();

    public:
      void Notify(void);

      signal2<int,bool> updateInstrumentCbx;

      ngrs::NMenuBar *menuBar;
      ngrs::NMenu *processMenu;
      ngrs::NMenu *editMenu;
      ngrs::NMenu *viewMenu;
      ngrs::NMenu *convertMenu;

    private:

      Song* _pSong;

      void InitStatusBar();
      void InitMenus();
      void InitToolBar();

      ngrs::NToolBar *toolBar;
      ngrs::NComboBox* auxSelectCombo_;
      WaveEdChildView *wavView;
      ngrs::NFileDialog* wavSaveFileDlg;
      InstrumentEditor* instrumentEditor;

      int wsInstrument;

      void onPlay( ngrs::NButtonEvent *ev );
      void onRelease( ngrs::NButtonEvent *ev );
      void onPlayFromStart( ngrs::NButtonEvent *ev );
      void onStop( ngrs::NButtonEvent *ev );
      void onFastForward( ngrs::NButtonEvent *ev );
      void onRewind( ngrs::NButtonEvent *ev );
      void PlayFrom( unsigned long startpos );
      void Stop();

      void onLoadWave( ngrs::NButtonEvent* ev );
      void onSaveWave( ngrs::NButtonEvent* ev );
      void onEditInstrument( ngrs::NButtonEvent* ev );
      void onEditWave( ngrs::NButtonEvent* ev );

      void onSlotCopy( ngrs::NButtonEvent* ev );
      void onSlotPaste( ngrs::NButtonEvent* ev );

      void onDecInsBtn( ngrs::NButtonEvent* ev );
      void onIncInsBtn( ngrs::NButtonEvent* ev );

      bool mySel_;
      int leftSize;
      int rightSize;
      bool onWaveLeft;
      unsigned char* pDataLeft;
      unsigned char* pDataRight;

      int data_pos;

      void onTagParse( const ngrs::NXmlParser& parser, const std::string& tagName );


    };

  }
}
