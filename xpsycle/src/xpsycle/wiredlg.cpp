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
#include <ngrs/nlistlayout.h>
#include <ngrs/nalignlayout.h>
#include <cmath>

namespace psycle { namespace host {

WireDlg::WireDlg()
  : ngrs::NWindow()
{
  setTitle("Wire Connection");

  line_ = 0;
  _pSrcMachine = _pDstMachine = 0;

  

  ngrs::NPanel* btnPnl = new ngrs::NPanel();
    btnPnl->setLayout( ngrs::NAlignLayout() );
    /*modeBtn = new ngrs::NButton("Scope Mode");
      modeBtn->setFlat(false);
    btnPnl->add(modeBtn);
    ngrs::NPanel* sliderPnl = new ngrs::NPanel();
      sliderPnl->setLayout( ngrs::NAlignLayout() );
      holdBtn = new ngrs::NButton("Hold");
        holdBtn->setFlat(false);
      sliderPnl->add(holdBtn,ngrs::nAlRight);

      slider = new ngrs::NSlider();
        slider->setOrientation(ngrs::nHorizontal);
        slider->setPreferredSize(200,20);
      sliderPnl->add(slider,ngrs::nAlTop);
      slider2 = new ngrs::NSlider();
        slider2->setOrientation(ngrs::nHorizontal);
        slider2->setPreferredSize(200,20);
      sliderPnl->add(slider2,ngrs::nAlTop);
    btnPnl->add(sliderPnl);
    delBtn = new ngrs::NButton("Delete Connection");
      delBtn->clicked.connect(this,&WireDlg::onDeleteBtn);
      delBtn->setFlat(false);
    btnPnl->add(delBtn);*/
    delBtn = new ngrs::NButton("Delete Connection");
      delBtn->clicked.connect(this,&WireDlg::onDeleteBtn);
      delBtn->setFlat(false);
    btnPnl->add(delBtn, ngrs::nAlTop);

  pane()->add(btnPnl,ngrs::nAlBottom);

  
  ngrs::NPanel* volPanel = new ngrs::NPanel();
   volPanel->setLayout ( ngrs::NAlignLayout() );
    volSlider = new ngrs::NSlider();
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

void psycle::host::WireDlg::setVisible( bool on )
{  
  if (on && pDstMachine()) {
    float val = 0;
    pDstMachine()->GetWireVolume( wireIdx(), val);
	  int t = (int)std::sqrt( val *16384*4*4);
    volSlider->setPos( 254 * 4 - t );
  }
  ngrs::NWindow::setVisible( on );
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

void WireDlg::onDeleteBtn( ngrs::NButtonEvent * ev )
{	
  deleteMe.emit(this);
	_pSrcMachine = 0;
  _pDstMachine = 0;
  ///\todo for scopes we need some mutex for the buffer
	///\todo scopes disabled
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

void WireDlg::onVolPosChanged( ngrs::NSlider * slider )
{
	if ( pDstMachine() && pSrcMachine() && wireIdx() != -1 ) {
		const float curvol = ((256*4-slider->pos())*(256*4-slider->pos()))/(16384.0f*4*4);
		pDstMachine()->SetWireVolume( wireIdx(), curvol );
	} 
}   
 
int WireDlg::wireIdx( )
{
  if ( pDstMachine() && pSrcMachine() )
	  return pDstMachine()->FindInputWire( pSrcMachine()->_macIndex );
  else
    return -1;
}



}}






