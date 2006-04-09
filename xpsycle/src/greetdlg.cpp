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
#include <nlabel.h>
#include <nlistbox.h>
#include <nitem.h>


GreetDlg::GreetDlg()
 : NWindow()
{
  lBox = new NListBox();

  NLabel* greetings = new NLabel();
    greetings->setText("Psyceledics Community, wants to thank the following people for their contributions in the developement of Psycle");
    greetings->setWordbreak(true);
  pane()->add(greetings,nAlTop);

  pane()->add(lBox, nAlClient);

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
  lBox->add(new NItem("_sue_ [Extreme testing]"));
  lBox->add(new NItem("[JAZ] Our project father!]"));
  lBox->add(new NItem("[SAS] SOLARiS [project-ps WebMaster]"));
  lBox->add(new NItem("Alk [Extreme testing + coding]"));
  lBox->add(new NItem("All at #psycle[Efnet]"));
  lBox->add(new NItem("All the people from the Forums"));
  lBox->add(new NItem("bohan"));
  lBox->add(new NItem("Byte"));
  lBox->add(new NItem("CyanPhase for porting [VibraSynth]"));
  lBox->add(new NItem("dazld"));
  lBox->add(new NItem("DJMirage"));
  lBox->add(new NItem("Drax_D[for asking to be here:D]"));
  lBox->add(new NItem("Druttis [Machines]"));
  lBox->add(new NItem("Frodix"));
  lBox->add(new NItem("Felix Kaplan / Spirit of India"));
  lBox->add(new NItem("Felix Petrescu 'WakaX'"));
  lBox->add(new NItem("Gerwin / Freeh2o"));
  lBox->add(new NItem("Imagineer"));
  lBox->add(new NItem("Koopeer"));
  lBox->add(new NItem("krokpitr"));
  lBox->add(new NItem("Krzysztof Foltman / fsm [coding help]"));
  lBox->add(new NItem("ksn [Psyceledics WebMaster]"));
  lBox->add(new NItem("lastfuture"));
  lBox->add(new NItem("LegoStar [iso]"));
  lBox->add(new NItem("Loby [for being away])"));
  lBox->add(new NItem("Pikari"));
  lBox->add(new NItem("pooplog[Machines+Config]"));
  lBox->add(new NItem("sampler"));
  lBox->add(new NItem("TAo-AAS"));
  lBox->add(new NItem("TimEr[Site Graphics and more]"));
  lBox->add(new NItem("Vir|us"));
}


