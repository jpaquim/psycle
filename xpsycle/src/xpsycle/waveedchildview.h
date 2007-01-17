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
#include <ngrs/panel.h>
#include <ngrs/color.h>
#include <ngrs/timer.h>
#include <ngrs/slider.h>
#include <deque>

namespace ngrs {
  class Window;
  class NPopupMenu;
  class ScrollBar;
  class Button;
  class NStatusBar;
  class TextStatusItem;
}

namespace psycle { 
  namespace host {

    class Song;
    class WaveEdAmplifyDialog;
    class WaveEdInsertSilenceDialog;
    class WaveEdMixDialog;
    class WaveEdCrossfadeDialog;

    /// wave editor window.
    class WaveEdChildView : public ngrs::Panel
    {
    public:

      class VolumeSlider : public ngrs::Slider
      {
      public:
        VolumeSlider(Song* pSong_) { this->setOrientation(ngrs::nHorizontal); pSong = pSong_; }
        ~VolumeSlider() {}
        virtual void paint( ngrs::Graphics&g );
        Song *pSong;
      };

      class WavePanel : public ngrs::Panel
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


      void onSelectionZoomIn( ngrs::ButtonEvent* ev );
      void onSelectionZoomSel( ngrs::ButtonEvent* ev );
      void onSelectionZoomOut( ngrs::ButtonEvent* ev );
      void onSelectionFadeIn( ngrs::ButtonEvent* ev );
      void onSelectionFadeOut( ngrs::ButtonEvent* ev );	
      void onSelectionNormalize( ngrs::ButtonEvent* ev );
      void onSelectionRemoveDC( ngrs::ButtonEvent* ev );
      void onSelectionAmplify( ngrs::ButtonEvent* ev );
      void onSelectionReverse( ngrs::ButtonEvent* ev );
      void onSelectionShowall( ngrs::ButtonEvent* ev );
      void onSelectionInsertSilence( ngrs::ButtonEvent* ev );
      void onEditCopy( ngrs::ButtonEvent* ev );
      void onEditCut( ngrs::ButtonEvent* ev );
      void onEditCrop( ngrs::ButtonEvent* ev );
      void onEditPaste( ngrs::ButtonEvent* ev );
      void onEditDelete( ngrs::ButtonEvent* ev );
      void onConvertMono( ngrs::ButtonEvent* ev );
      void onEditSelectAll( ngrs::ButtonEvent* ev );
      void onEditSnapToZero( ngrs::ButtonEvent* ev );
      void onPasteOverwrite( ngrs::ButtonEvent* ev );
      void onPasteMix( ngrs::ButtonEvent* ev );
      void onPasteCrossfade( ngrs::ButtonEvent* ev );
      void onPopupSetLoopStart( ngrs::ButtonEvent* ev );
      void onPopupSetLoopEnd( ngrs::ButtonEvent* ev );
      void onPopupSelectionToLoop( ngrs::ButtonEvent* ev );
      void onPopupZoomIn( ngrs::ButtonEvent* ev);
      void onPopupZoomOut( ngrs::ButtonEvent* ev);

      void onHScroll( ngrs::ScrollBar *sender );
      void onVolSliderScroll( ngrs::Slider *slider );
      void onZoomSliderScroll( ngrs::Slider *slider );


    private:
      void InitZoomBar();
      void InitPopupMenu();
      void InitStatusBar();
      void UpdateStatusBar();

      WavePanel* waveArea;
      ngrs::NPopupMenu *popup;

      ngrs::Panel *zoomBar;
      ngrs::ScrollBar *scrollBar;
      VolumeSlider *volSlider;
      ngrs::Button *zoomInButton;
      ngrs::Button *zoomOutButton;
      ngrs::Slider *zoomSlider;
      ngrs::NStatusBar *statusBar;
      ngrs::TextStatusItem *lengthText;		//shows length of wave
      ngrs::TextStatusItem *selText;			//shows length of selection
      ngrs::TextStatusItem *statusText;		//displays editor status
      ngrs::TextStatusItem *modeText;			//displays mode

      Song *pSong;

      ngrs::Color clrLo;
      ngrs::Color clrMe;
      ngrs::Color clrHi;
      ngrs::Color clrBlack;
      ngrs::Color clrWhite;
      ngrs::Timer timer;
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
