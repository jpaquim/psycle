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

#include "wiredlg.h"
#include <ngrs/listlayout.h>
#include <ngrs/alignlayout.h>
#include <cmath>

namespace psy { namespace host {

WireDlg::WireDlg()
  : ngrs::Window()
{
  setTitle("Wire Connection");

  line_ = 0;
  _pSrcMachine = _pDstMachine = 0;

  

  ngrs::Panel* btnPnl = new ngrs::Panel();
    btnPnl->setLayout( ngrs::AlignLayout() );
    /*modeBtn = new ngrs::Button("Scope Mode");
      modeBtn->setFlat(false);
    btnPnl->add(modeBtn);
    ngrs::Panel* sliderPnl = new ngrs::Panel();
      sliderPnl->setLayout( ngrs::AlignLayout() );
      holdBtn = new ngrs::Button("Hold");
        holdBtn->setFlat(false);
      sliderPnl->add(holdBtn,ngrs::nAlRight);

      slider = new ngrs::Slider();
        slider->setOrientation(ngrs::nHorizontal);
        slider->setPreferredSize(200,20);
      sliderPnl->add(slider,ngrs::nAlTop);
      slider2 = new ngrs::Slider();
        slider2->setOrientation(ngrs::nHorizontal);
        slider2->setPreferredSize(200,20);
      sliderPnl->add(slider2,ngrs::nAlTop);
    btnPnl->add(sliderPnl);
    delBtn = new ngrs::Button("Delete Connection");
      delBtn->clicked.connect(this,&WireDlg::onDeleteBtn);
      delBtn->setFlat(false);
    btnPnl->add(delBtn);*/
    delBtn = new ngrs::Button("Delete Connection");
      delBtn->clicked.connect(this,&WireDlg::onDeleteBtn);
      delBtn->setFlat(false);
    btnPnl->add(delBtn, ngrs::nAlTop);

  pane()->add(btnPnl,ngrs::nAlBottom);

  
  ngrs::Panel* volPanel = new ngrs::Panel();
   volPanel->setLayout ( ngrs::AlignLayout() );
    volSlider = new ngrs::Slider();
      volSlider->setRange( 0, 256*4 );
      volSlider->setPreferredSize( 20, 200 );
      volSlider->change.connect(this, &WireDlg::onVolPosChanged);
    volPanel->add( volSlider, ngrs::nAlClient );
  pane()->add( volPanel, ngrs::nAlRight );

  analyzer = new Analyzer();
  pane()->add( analyzer, ngrs::nAlClient);

  setPosition(10,10,300,300);
}


WireDlg::~WireDlg()
{
}

void psy::host::WireDlg::setVisible( bool on )
{  
  if (on && pDstMachine()) {
  }
  ngrs::Window::setVisible( on );
}

int WireDlg::onClose( )
{
  setVisible( false );
  return ngrs::nHideWindow;
}

void WireDlg::setMachines( Machine * pSrcMachine, Machine * pDstMachine )
{
  _pSrcMachine = pSrcMachine;
  _pDstMachine = pDstMachine;
  analyzer->setMachines(pSrcMachine,pDstMachine);

}

void WireDlg::onDeleteBtn( ngrs::ButtonEvent * ev )
{	
  deleteMe.emit(this);
  _pSrcMachine = 0;
  _pDstMachine = 0;
}

Machine * WireDlg::pSrcMachine( )
{
  return _pSrcMachine;
}

Machine * WireDlg::pDstMachine( )
{
  return _pDstMachine;
}

void WireDlg::setLine( WireGUI * line )
{
  line_ = line;
}

WireGUI * WireDlg::line( )
{
  return line_;
}

void WireDlg::onVolPosChanged( ngrs::Slider * slider )
{
}   
 
int WireDlg::wireIdx( )
{
  return -1;
}



}}






