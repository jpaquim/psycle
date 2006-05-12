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
#include "nmessagebox.h"
#include "nbutton.h"
#include "nflowlayout.h"
#include "nlabel.h"

NMessageBox::NMessageBox()
 : NDialog()
{
  init();
}

NMessageBox::NMessageBox( const std::string & msgText )
{
  init();
  textLbl->setText(msgText);
}


void NMessageBox::init( )
{
  NPanel* btnPnl = new NPanel();
    btnPnl->setLayout(NFlowLayout(nAlRight,5,5));
    okBtn = new NButton("ok");
      okBtn->setFlat(false);
      okBtn->clicked.connect(this,&NMessageBox::onBtnPress);
    btnPnl->add(okBtn);
    useBtn = new NButton("use");
      useBtn->setFlat(false);
      useBtn->clicked.connect(this,&NMessageBox::onBtnPress);
    btnPnl->add(useBtn);
    cancelBtn = new NButton("cancel");
      cancelBtn->setFlat(false);
      cancelBtn->clicked.connect(this,&NMessageBox::onBtnPress);
    btnPnl->add(cancelBtn);
  pane()->add(btnPnl,nAlBottom);

  textLbl = new NLabel();
  pane()->add(textLbl,nAlClient);
  textLbl->setWordbreak(true);

  setPosition(10,10,300,150);
  setTitle("Message");
}


NMessageBox::~NMessageBox()
{
}

void NMessageBox::setButtons( int btnMask )
{
  okBtn->setVisible(false);
  useBtn->setVisible(false);
  cancelBtn->setVisible(false);

  switch (btnMask) {
    case nMsgOkBtn :
       okBtn->setVisible(true);
    break;
    case nMsgCancelBtn :
       cancelBtn->setVisible(true);
    break;
    case nMsgUseBtn :
       useBtn->setVisible(true);
    break;
  }
}

void NMessageBox::setButtonText( const std::string & ok, const std::string & use, const std::string & cancel )
{
  okBtn->setText(ok);
  cancelBtn->setText(cancel);
  useBtn->setText(use);
}

void NMessageBox::setText( const std::string & msgText )
{
  textLbl->setText(msgText);
}

void NMessageBox::onBtnPress( NButtonEvent * ev )
{
  if (ev->sender()==okBtn) {
     doClose(true);
  } else
  if (ev->sender()==useBtn) {
     use.emit(this);
  } else
  if (ev->sender()==cancelBtn) {
    doClose(false);
  }
}



