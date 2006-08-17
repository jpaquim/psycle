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
//#include "waveedchildview.hpp"

#include <ngrs/ndockpanel.h>

class NMenu;
class NMenuBar;
class NToolBar;
class NComboBox;
class NItemEvent;
class NFileDialog;

namespace psycle { namespace host {

class WaveEdChildView;
class WaveSaveDlg;
class InstrumentEditor;
class Song;


/// wave editor frame window.
class WaveEdFrame : public NDockPanel
{


public:
	WaveEdFrame( Song* song );
	virtual ~WaveEdFrame() throw();
	Song* pSong();
	//virtual void setVisible(bool on);
	//virtual int onClose();

public:
	void Notify(void);
	
	NMenuBar *menuBar;
	NMenu *processMenu;
	NMenu *editMenu;
	NMenu *viewMenu;
	NMenu *convertMenu;
	
  void updateComboIns(bool updatelist);
	
private:

	Song* _pSong;

	void InitStatusBar();
	void InitMenus();
	void InitToolBar();
 
	NToolBar *toolBar;
	NComboBox* insCombo_;
	NComboBox* auxSelectCombo_;
	WaveEdChildView *wavView;
	NFileDialog* wavSaveFileDlg;
	InstrumentEditor* instrumentEditor;

	int wsInstrument;
	
	void onPlay(NButtonEvent *ev);
	void onRelease(NButtonEvent *ev);
	void onPlayFromStart(NButtonEvent *ev);
	void onStop(NButtonEvent *ev);
	void onFastForward(NButtonEvent *ev);
	void onRewind(NButtonEvent *ev);
	void PlayFrom(unsigned long startpos);
	void Stop();

	void onLoadWave(NButtonEvent* ev);
	void onSaveWave(NButtonEvent* ev);
	void onEditInstrument(NButtonEvent* ev);
	void onEditWave(NButtonEvent* ev);
	void onInstrumentCbx(NItemEvent* ev);


	void onDecInsBtn(NButtonEvent* ev);
	void onIncInsBtn(NButtonEvent* ev);

};

}}
