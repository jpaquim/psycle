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
#include "analyzer.h"
#include "nwindow.h"

Analyzer::Analyzer()
 : NPanel()
{
  NFont font = NFont("Suse sans",6,nMedium | nStraight);
    font.setTextColor(NColor(0x60,0x60,0x60));
  setFont(font);
  mode_ = 0;
  setTransparent(false);
  _pSrcMachine = _pDstMachine = 0;
  _dstWireIndex = 0;
  peakL = peakR = peak2L = peak2R = 0.0f;

  memset(pSamplesL,0,sizeof(pSamplesL));
  memset(pSamplesR,0,sizeof(pSamplesR));

  setBackground(NColor(0,0,0));

  timer.setIntervalTime(100);
  //timer.enableTimer();
  timer.timerEvent.connect(this,&Analyzer::onTimer);
}


Analyzer::~Analyzer()
{
}

void Analyzer::paint( NGraphics * g )
{
  switch (mode_) {
  case 0:
    g->setForeground(NColor(0x60,0x60,0x60));
    g->fillRect(32+24,32-8,256-32-24-32-24,1);
    g->drawText(32-1,32-8-6+g->textAscent(),"+6 db");
    g->drawText(256-32-22,32-8-6+g->textAscent(),"+6 db");

    g->setForeground(NColor(0x60,0x60,0x60));
    g->fillRect(32+24,32+44,256-32-24-32-24,1);
    g->drawText(32-1+4,32+44-6+g->textAscent(),"-6 db");
    g->drawText(256-32-22,32+44-6+g->textAscent(),"-6 db");

    g->setForeground(NColor(0x60,0x60,0x60));
    g->fillRect(32+24,32+44+16,256-32-24-32-24,1);
    g->drawText(32-1-6+4,32+44+16-6+g->textAscent(),"-12 db");
    g->drawText(256-32-22,32+44+16-6+g->textAscent(),"-12 db");

    g->setForeground(NColor(0x60,0x60,0x60));
    g->fillRect(32+24,32+44+16+18,256-32-24-32-24,1);
    g->drawText(32-1-6+4, 32+44+16+18-6+g->textAscent(), "-24db");
    g->drawText(256-32-22, 32+44+16+18-6+g->textAscent(), "-24db");

    g->setForeground(NColor(0x60,0x60,0x60));
    g->fillRect(32+24,32+23,256-32-24-32-24,1);
    g->drawText(32-1+6, 32+23-6+g->textAscent(), "0 db");
    g->drawText(256-32-22, 32+23-6+g->textAscent(), "0 db");
  break;
  }
}

void Analyzer::setMachines( Machine * pSrcMachine, Machine * pDstMachine )
{
  _pSrcMachine = pSrcMachine;
  _pDstMachine = pDstMachine;

  isrcMac = pSrcMachine->_macIndex;
  _dstWireIndex = _pDstMachine->FindInputWire(isrcMac);
  float val;
  _pDstMachine->GetWireVolume(_dstWireIndex,val);
  invol = val;
  mult = 1.0f;

  _pSrcMachine->_pScopeBufferL = pSamplesL;
  _pSrcMachine->_pScopeBufferR = pSamplesR;
}

void Analyzer::onTimer( )
{
  if ((window()!=0) && (window()->mapped())) {
  int index = _pSrcMachine->_scopeBufferIndex;
  float tawl,tawr;
  tawl = 0;
  tawr = 0;
  for (int i=0;i<SCOPE_SPEC_SAMPLES;i++) 
  {
    index--;
    index&=(SCOPE_BUF_SIZE-1);
    float awl=fabsf(pSamplesL[index]*invol*mult*_pSrcMachine->_lVol);///32768;
    float awr=fabsf(pSamplesR[index]*invol*mult*_pSrcMachine->_rVol);///32768;
    if (awl>tawl) {
      tawl = awl;
    }
    if (awr>tawr)
    {
      tawr = awr;
    }
  }
  if (tawl>peak2L)
  {
    peak2L = tawl;
    peakLifeL = 1576;
    peakL = tawl;
  }
  else if (tawl>peakL)
  {
    peakL = tawl;
  }
  if (tawr>peak2R)
  {
    peak2R = tawr;
    peakLifeR = 1576;
    peakR = tawr;
  }
  else if (tawr>peakR)
  {
    peakR = tawr;
  }

  int y;

  // LEFT CHANNEL
  y = 128-f2i(sqrtf(peakL/6));
  if (y < 0)
  {
    y = 0;
  }

  NRect rect;

  int cd = (peakLifeL/24)+24;
  long cv = int(0xC0 * (cd/128.0f))<<16 | int(0x80 * (cd/128.0f))<<8 | int(((y<32+23)?0xF0:0x80)*(cd/128.0f)) ;
  NColor lbColor; lbColor.setHCOLORREF(cv);

  rect.setLeft(128-32-24);
  rect.setWidth(48);
  rect.setTop(y);

  y = 128-f2i(sqrtf(tawl/6));
  if (y < 0)
  {
    y = 0;
  }
  rect.setHeight(y - rect.top());

  NGraphics* g = window()->graphics(this);

  g->setForeground(NColor(0,0,0));
  g->fillRect(0,0,300,300);
  paint(g);
  g->setForeground(lbColor);
  g->fillRect(rect);
  g->swap(NRect(absoluteGeometry()));

  /*if (peak2L)
  {
      CPen *oldpen = bufDC.SelectObject(&linepenL);
      bufDC.MoveTo(rect.left-1,128-f2i(sqrtf(peak2L/6)));
      bufDC.LineTo(rect.right-1,128-f2i(sqrtf(peak2L/6)));
      bufDC.SelectObject(oldpen);
  }

  rect.top = rect.bottom;
  rect.bottom = 128;
  lbColor = 0xC08000+((y<32+23)?0xF0:0x80);

  bufDC.FillSolidRect(&rect,lbColor);*/
}
}


