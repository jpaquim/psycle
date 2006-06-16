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
class NWindow;
class NPopupMenu;
class NScrollBar;
class NButton;
class NStatusBar;
class NTextStatusItem;

namespace psycle { namespace host {
	
class Song;
class WaveEdAmplifyDialog;
class WaveEdInsertSilenceDialog;
class WaveEdMixDialog;
class WaveEdCrossfadeDialog;

/// wave editor window.
class WaveEdChildView : public NPanel
{
public:

	class VolumeSlider : public NSlider
	{
	public:
		VolumeSlider(Song* pSong_) { this->setOrientation(nHorizontal); pSong = pSong_; }
		~VolumeSlider() {}
		virtual void paint(NGraphics *g);
		Song *pSong;
	};
	
	class WavePanel : public NPanel
	{
		public:
		WavePanel(WaveEdChildView* wView);
		~WavePanel();
		virtual void paint(NGraphics *g);

		virtual void onMousePress  (int x, int y, int button);
		virtual void onMousePressed(int x, int y, int button);
		virtual void onMouseDoublePress(int x, int y, int button);
		virtual void onMouseOver(int x, int y);
		virtual void onMouseExit();
		private:
			WaveEdChildView* wView_;
	};
	
	WaveEdChildView(Song* pSong_);
	virtual ~WaveEdChildView();
			
    
  virtual void resize();

    
	void SetSpecificZoom(int factor);

	//refreshes position/thumb size of scroll bars and hsliders
	void ResetScrollBars(bool bNewLength=false);

	//locates the nearest zero crossing to a given sample index
	unsigned long FindNearestZero(unsigned long startpos);

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


	void onSelectionZoomIn(NButtonEvent* ev);
	void onSelectionZoomSel(NButtonEvent* ev);
	void onSelectionZoomOut(NButtonEvent* ev);
	void onSelectionFadeIn(NButtonEvent* ev);
	void onSelectionFadeOut(NButtonEvent* ev);	
	void onSelectionNormalize(NButtonEvent* ev);
	void onSelectionRemoveDC(NButtonEvent* ev);
	void onSelectionAmplify(NButtonEvent* ev);
	void onSelectionReverse(NButtonEvent* ev);
	void onSelectionShowall(NButtonEvent* ev);
	void onSelectionInsertSilence(NButtonEvent* ev);
	void onEditCopy(NButtonEvent* ev);
	void onEditCut(NButtonEvent* ev);
	void onEditCrop(NButtonEvent* ev);
	void onEditPaste(NButtonEvent* ev);
	void onEditDelete(NButtonEvent* ev);
	void onConvertMono(NButtonEvent* ev);
	void onEditSelectAll(NButtonEvent* ev);
	void onEditSnapToZero(NButtonEvent* ev);
	void onPasteOverwrite(NButtonEvent* ev);
	void onPasteMix(NButtonEvent* ev);
	void onPasteCrossfade(NButtonEvent* ev);
	void onPopupSetLoopStart(NButtonEvent* ev);
	void onPopupSetLoopEnd(NButtonEvent* ev);
	void onPopupSelectionToLoop(NButtonEvent* ev);
	void onPopupZoomIn(NButtonEvent* ev);
	void onPopupZoomOut(NButtonEvent* ev);
	
	void onHScroll( NObject *sender, int pos );
	void onVolSliderScroll( NSlider *slider, double pos);
	void onZoomSliderScroll( NSlider *slider, double pos);
		
	
private:
	void InitZoomBar();
	void InitPopupMenu();
	void InitStatusBar();
	void UpdateStatusBar();

	WavePanel* waveArea;
	NPopupMenu *popup;

	NPanel *zoomBar;
	NScrollBar *scrollBar;
	VolumeSlider *volSlider;
	NButton *zoomInButton;
	NButton *zoomOutButton;
	NSlider *zoomSlider;
	NStatusBar *statusBar;
	NTextStatusItem *lengthText;		//shows length of wave
	NTextStatusItem *selText;			//shows length of selection
	NTextStatusItem *statusText;		//displays editor status
	NTextStatusItem *modeText;			//displays mode
	
	Song *pSong;

	NColor clrLo;
	NColor clrMe;
	NColor clrHi;
	NColor clrBlack;
	NColor clrWhite;
	NTimer timer;
	NFont *fntLoop;
	
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

			WaveEdAmplifyDialog			*AmpDlg;
			WaveEdMixDialog 			*MixDlg;
			WaveEdInsertSilenceDialog	*SilenceDlg;
			WaveEdCrossfadeDialog 		*XFadeDlg;

};

}}
