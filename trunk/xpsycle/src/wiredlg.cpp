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
#include "wiredlg.h"
#include <nlistlayout.h>

WireDlg::WireDlg()
 : NWindow()
{
  line_ = 0;
  _pSrcMachine = _pDstMachine = 0;

  NPanel* btnPnl = new NPanel();
    btnPnl->setLayout(new NListLayout(),true);
    modeBtn = new NButton("Scope Mode");
      modeBtn->setFlat(false);
    btnPnl->add(modeBtn);
    NPanel* sliderPnl = new NPanel();
      sliderPnl->setLayout(new NAlignLayout(),true);
      holdBtn = new NButton("Hold");
        holdBtn->setFlat(false);
      sliderPnl->add(holdBtn,nAlRight);
      slider = new NSlider();
        slider->setOrientation(nHorizontal);
        slider->setPreferredSize(200,20);
      sliderPnl->add(slider,nAlTop);
      slider2 = new NSlider();
        slider2->setOrientation(nHorizontal);
        slider2->setPreferredSize(200,20);
      sliderPnl->add(slider2,nAlTop);
    btnPnl->add(sliderPnl);
    delBtn = new NButton("Delete Connection");
      delBtn->clicked.connect(this,&WireDlg::onDeleteBtn);
      delBtn->setFlat(false);
    btnPnl->add(delBtn);
    analyzer = new Analyzer();
    pane()->add(analyzer,nAlClient);
  pane()->add(btnPnl,nAlBottom);
}


WireDlg::~WireDlg()
{
}

int WireDlg::onClose( )
{
  setVisible(false);
  return nHideWindow;
}

void WireDlg::setMachines( Machine * pSrcMachine, Machine * pDstMachine )
{
  _pSrcMachine = pSrcMachine;
  _pDstMachine = pDstMachine;
  analyzer->setMachines(pSrcMachine,pDstMachine);

}

void WireDlg::onDeleteBtn( NButtonEvent * ev )
{
  deleteMe.emit(this);
}

Machine * WireDlg::pSrcMachine( )
{
  return _pSrcMachine;
}

Machine * WireDlg::pDstMachine( )
{
  return _pDstMachine;
}

void WireDlg::setLine( NLine * line )
{
  line_ = line;
}

NLine * WireDlg::line( )
{
  return line_;
}
