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
#include "greetdlg.h"
#include <ngrs/nlabel.h>
#include <ngrs/nmemo.h>
#include <ngrs/nitem.h>

namespace psycle { namespace host {

GreetDlg::GreetDlg()
  : NWindow()
{
  memo = new NMemo();
  memo->setReadOnly(true);

  NLabel* greetings = new NLabel();
    greetings->setText("Psyceledics Community, wants to thank the following people for their contributions in the developement of Psycle");
    greetings->setWordWrap(true);
  pane()->add(greetings,nAlTop);

  pane()->add(memo, nAlClient);

  setGreetz();

  setPosition(10,10,600,500);
}


GreetDlg::~GreetDlg()
{
}

int GreetDlg::onClose( )
{
  setVisible(false);
  return nHideWindow;
}

void GreetDlg::setGreetz( )
{
  memo->setText("_sue_ [Extreme testing]\n[JAZ] Our project father!]\n[SAS] SOLARiS [project-ps WebMaster]\nAlk [Extreme testing + coding]\nAll at #psycle[Efnet]\nAll the people from the Forums\nbohan\nByte\nCyanPhase for porting [VibraSynth]\ndazld\nDJMirage\nDrax_D[for asking to be here:D]\nDruttis [Machines]\nErodix[helping in math + build testing]\nFelix Kaplan / Spirit of India\nFelix Petrescu 'WakaX'\nGerwin / Freeh2o\nImagineer\nKoopeer\nkrokpitr\nKrzysztof Foltman / fsm [coding help]\nksn [Psyceledics WebMaster]\nlastfuture\nLegoStar [iso]\nLoby [for being away])\nPikari\npooplog[Machines+Config]\nsampler\nTAo-AAS\nTimEr[Site Graphics and more\nVir|us\n");
}

}}
