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
#ifndef ANALYZER_H
#define ANALYZER_H

#include <npanel.h>
#include <machine.h>
#include <ntimer.h>


/**
@author Stefan
*/

const int MAX_SCOPE_BANDS = 128;
const int SCOPE_BUF_SIZE  = 4096;
const int SCOPE_SPEC_SAMPLES = 256;

class Analyzer : public NPanel
{
public:
    Analyzer();

    ~Analyzer();

    void setMachines(Machine* pSrcMachine, Machine* pDstMachine);

    virtual void paint(NGraphics* g);

private:

    NTimer timer;

    int mode_;
    float invol;
    float mult;

    Machine* _pSrcMachine;
    Machine* _pDstMachine;
    int _dstWireIndex;
    int isrcMac;
    float peakL,peakR;
    float peak2L,peak2R;
    int peakLifeL,peakLifeR;

    float pSamplesL[SCOPE_BUF_SIZE];
    float pSamplesR[SCOPE_BUF_SIZE];

    void onTimer();

};

#endif
