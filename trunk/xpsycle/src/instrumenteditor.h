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
#ifndef INSTRUMENTEDITOR_H
#define INSTRUMENTEDITOR_H

#include <nwindow.h>

class NLabel;
class NButton;
class NEdit;
class NComboBox;
class NSlider;
class NCheckBox;
class NItemEvent;

/**
@author Stefan Nattkemper
*/
class InstrumentEditor : public NWindow
{
public:
    InstrumentEditor();

    ~InstrumentEditor();

    void setInstrument(int index);
    int instrumentIndex();

    virtual int onClose();

private:

    NLabel*    instNumberLbl;
    NButton*   incInstBtn;
    NButton*   decInstBtn;
    NEdit*     instNameEd;
    NButton*   killBtn;
    NComboBox* newNoteActionCb;
    NSlider*   panningSlider;
    NLabel*    panningLbl;
    NCheckBox* rndPanningCbx;
    NCheckBox* rndVCFCutCbx;
    NCheckBox* rndVCFResoCbx;
    NCheckBox* playSampleFitCbx;
    NLabel*    patRowLbl;
    NSlider*   volumeSlider;
    NLabel*    volumeLbl;
    NSlider*   fineTuneSlider;
    NLabel*    fineTuneLbl;
    NButton*   octDecBtn;
    NButton*   noteDecBtn;
    NButton*   noteIncBtn;
    NButton*   octIncBtn;
    NLabel*    octLbl;
    NLabel*    loopLbl;
    NLabel*    loopAtFromLbl;
    NLabel*    loopAtToLbl;
    NLabel*    lenLbl;

    void init();
    std::string noteToString( int value );

    void onBtnPress(NButtonEvent* ev);
    void onComboSelected(NItemEvent* ev);
    void onSliderMove(NSlider* sender, double pos);

};

#endif
