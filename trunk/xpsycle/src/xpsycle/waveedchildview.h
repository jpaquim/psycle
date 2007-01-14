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
#include <ngrs/npanel.h>
#include <ngrs/ncolor.h>
#include <ngrs/ntimer.h>
#include <ngrs/nslider.h>
#include <deque>

namespace ngrs {
  class NWindow;
  class NPopupMenu;
  class NScrollBar;
  class NButton;
  class NStatusBar;
  class NTextStatusItem;
}

namespace psycle { 
  namespace host {

    class Song;
    class WaveEdAmplifyDialog;
    class WaveEdInsertSilenceDialog;
    class WaveEdMixDialog;
    class WaveEdCrossfadeDialog;

    /// wave editor window.
    class WaveEdChildView : public ngrs::NPanel
    {
    public:

      class VolumeSlider : public ngrs::NSlider
      {
      public:
        VolumeSlider(Song* pSong_) { this->setOrientation(ngrs::nHorizontal); pSong = pSong_; }
        ~VolumeSlider() {}
        virtual void paint( ngrs::Graphics&g );
        Song *pSong;
      };

      class WavePanel : public ngrs::NPanel
      {
      public:
        WavePanel( WaveEdChildView* wView );
        ~WavePanel();
        virtual void paint( ngrs::Graphics&g);

        virtual void onMousePress( int x, int y, int button );
        virtual void onMousePressed(int x, int y, int button );
        virtual void onMouseDoublePress( int x, int y, int button );
        virtual void onMouseOver( int x, int y );
        virtual void onMouseExit();
      private:
        WaveEdChildView* wView_;
      };

      WaveEdChildView( Song* pSong_ );
      virtual ~WaveEdChildView();


      virtual void resize();


      void SetSpecificZoom( int factor );

      //refreshes position/thumb size of scroll bars and hsliders
      void ResetScrollBars( bool bNewLength=false );

      //locates the nearest zero crossing to a given sample index
      unsigned long FindNearestZero( unsigned long startpos );

      //refreshes wave display data
      void RefreshDisplayData(bool bRefreshHeader=false);

      //various dsp helpers
      void Mix(short* lhs, short *rhs, int lhsSize, int rhsSize, float lhsVol=1.0f, float rhsVol=1.0f);
      void Fade(short* data, int length, float startVol, float endVol);
      void Amplify(short* data, int length, float vol);

      void SetViewData(int ins);

      unsigned long GetWaveLength();
      unsigned long GetSelectionLength();			//returns length of current selected block
      unsigned long GetCursorPos();				//returns cursor's position
      void SetCursorPos(unsigned long newpos);	//sets cursor's position
      bool IsStereo();

      void onTimer();


      void onSelectionZoomIn( ngrs::NButtonEvent* ev );
      void onSelectionZoomSel( ngrs::NButtonEvent* ev );
      void onSelectionZoomOut( ngrs::NButtonEvent* ev );
      void onSelectionFadeIn( ngrs::NButtonEvent* ev );
      void onSelectionFadeOut( ngrs::NButtonEvent* ev );	
      void onSelectionNormalize( ngrs::NButtonEvent* ev );
      void onSelectionRemoveDC( ngrs::NButtonEvent* ev );
      void onSelectionAmplify( ngrs::NButtonEvent* ev );
      void onSelectionReverse( ngrs::NButtonEvent* ev );
      void onSelectionShowall( ngrs::NButtonEvent* ev );
      void onSelectionInsertSilence( ngrs::NButtonEvent* ev );
      void onEditCopy( ngrs::NButtonEvent* ev );
      void onEditCut( ngrs::NButtonEvent* ev );
      void onEditCrop( ngrs::NButtonEvent* ev );
      void onEditPaste( ngrs::NButtonEvent* ev );
      void onEditDelete( ngrs::NButtonEvent* ev );
      void onConvertMono( ngrs::NButtonEvent* ev );
      void onEditSelectAll( ngrs::NButtonEvent* ev );
      void onEditSnapToZero( ngrs::NButtonEvent* ev );
      void onPasteOverwrite( ngrs::NButtonEvent* ev );
      void onPasteMix( ngrs::NButtonEvent* ev );
      void onPasteCrossfade( ngrs::NButtonEvent* ev );
      void onPopupSetLoopStart( ngrs::NButtonEvent* ev );
      void onPopupSetLoopEnd( ngrs::NButtonEvent* ev );
      void onPopupSelectionToLoop( ngrs::NButtonEvent* ev );
      void onPopupZoomIn( ngrs::NButtonEvent* ev);
      void onPopupZoomOut( ngrs::NButtonEvent* ev);

      void onHScroll( ngrs::NScrollBar *sender );
      void onVolSliderScroll( ngrs::NSlider *slider );
      void onZoomSliderScroll( ngrs::NSlider *slider );


    private:
      void InitZoomBar();
      void InitPopupMenu();
      void InitStatusBar();
      void UpdateStatusBar();

      WavePanel* waveArea;
      ngrs::NPopupMenu *popup;

      ngrs::NPanel *zoomBar;
      ngrs::NScrollBar *scrollBar;
      VolumeSlider *volSlider;
      ngrs::NButton *zoomInButton;
      ngrs::NButton *zoomOutButton;
      ngrs::NSlider *zoomSlider;
      ngrs::NStatusBar *statusBar;
      ngrs::NTextStatusItem *lengthText;		//shows length of wave
      ngrs::NTextStatusItem *selText;			//shows length of selection
      ngrs::NTextStatusItem *statusText;		//displays editor status
      ngrs::NTextStatusItem *modeText;			//displays mode

      Song *pSong;

      ngrs::NColor clrLo;
      ngrs::NColor clrMe;
      ngrs::NColor clrHi;
      ngrs::NColor clrBlack;
      ngrs::NColor clrWhite;
      ngrs::NTimer timer;
      ngrs::NFont *fntLoop;

      float const static zoomBase;	//base of the logarithmic scale used for zooming with zoom slider

      // Wave data
      signed short* wdLeft;
      signed short* wdRight;
      bool wdStereo;
      unsigned long wdLength;
      unsigned long wdLoopS;
      unsigned long wdLoopE;
      bool wdLoop;

      //clipboard data
      std::vector<signed short> cbLeft;
      std::vector<signed short> cbRight;
      bool cbStereo;

      // Display data
      unsigned long diStart;		//first sample in current window
      unsigned long diLength;		//number of samples in window
      unsigned long blStart;		//first sample of selection
      unsigned long blLength;		//number of samples selected
      unsigned long cursorPos;	//location of the cursor
      bool blSelection;			//whether data is selected currently
      bool wdWave;				//whether we have a wave to display
      bool cursorBlink;			//switched on timer messages.. cursor is visible when true


      int wsInstrument;
      bool drawwave;
      bool bSnapToZero;
      bool bDragLoopStart, bDragLoopEnd;	//indicates that the user is dragging the loop start/end
      bool bLButtonDown;
      unsigned long SelStart;		//the end of the selection -not- being moved 

      //used for finding invalid rect when resizing selection/moving loop points
      //	int prevHeadX, prevBodyX;
      //	int prevBodyLoopS, prevHeadLoopS;
      //	int prevBodyLoopE, prevHeadLoopE;

      int rbX, rbY;				//mouse pos on rbuttonup- used for some location-sensitive context menu commands

      //wave display data
      std::deque<std::pair<short, short> > lDisplay;
      std::deque<std::pair<short, short> > rDisplay;
      std::deque<std::pair<short, short> > lHeadDisplay;
      std::deque<std::pair<short, short> > rHeadDisplay;

      WaveEdAmplifyDialog *AmpDlg;
      WaveEdMixDialog *MixDlg;
      WaveEdInsertSilenceDialog	*SilenceDlg;
      WaveEdCrossfadeDialog *XFadeDlg;

    };

  }
}
