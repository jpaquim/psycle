/***************************************************************************
  *   Copyright (C) 2006 by Stefan Nattkemper   *
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
#ifndef WAVESAVEDLG_H
#define WAVESAVEDLG_H

#include "song.h"
#include <ngrs/ndialog.h>

class NFileDialog;
class NEdit;
class NCheckBox;
class NRadioButton;
class NComboBox;
class NProgressBar;
class NButton;
class NItemEvent;

/**
@author Stefan Nattkemper
*/
class WaveSaveDlg : public NDialog
{
public:
    WaveSaveDlg();

    ~WaveSaveDlg();

    // psycle engine code
    int kill_thread;
    int threadopen;

    void saveTick();
    void saveEnd();

    virtual void setVisible(bool on);

private:

    // gui elements
    NFileDialog* fileDlg;
    NEdit* pathEdt;
    NCheckBox* wireChkBox;
    NCheckBox* trackChkBox;
    NCheckBox* generatorChkBox;
    NRadioButton* entireRBtn;
    NRadioButton* numberRBtn;
    NEdit* numberEdt;
    NRadioButton* seqRBtn;
    NEdit* fromEdt;
    NEdit* toEdt;
    NComboBox* sampleRateCbx;
    NComboBox* bitDepthCbx;
    NComboBox* channelsCbx;
    NProgressBar* progressBar;
    NButton* closeBtn;

    // psycle sound engine variables
    int rate;
    int bits;
    int channelmode;

    int lastpostick;
    int lastlinetick;
    int tickcont;

    bool _Muted[MAX_BUSES];
    bool autostop;
    bool playblock;
    bool loopsong;
    bool sel[MAX_SONG_POSITIONS];
    bool saving;
    int current;
    std::string rootname;
    int m_recmode;

    void saveWav(std::string file, int bits, int rate, int channelmode);

    static int audioOutThread(void * ptr);
    int iret1;
    pthread_t threadid;

    void initVars();

    // button event methods
    void onBrowseBtn(NButtonEvent* ev);
    void onCloseBtn(NButtonEvent* ev);
    void onSaveBtn(NButtonEvent* ev);

    // combobox events
    void onSampleRateCbx(NItemEvent* ev);
    void onBitSelCbx(NItemEvent* ev);
    void onChannelSelCbx(NItemEvent* ev);

    // checkbox events
    void onWireChkBox(NButtonEvent* ev);
    void onTrackChkBox(NButtonEvent* ev);
    void onGeneratorChkBox(NButtonEvent* ev);
};

#endif
