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
#include "nalignlayout.h"
#include "nlabel.h"
#include "nimage.h"

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
  setTitle("Message");

  iconImg = new NImage();
  pane()->add(iconImg, nAlLeft);

  NPanel* btnPnl = new NPanel();
    btnPnl->setLayout(NAlignLayout(5,5));
    okBtn = new NButton("ok");
      okBtn->setFlat(false);
      okBtn->clicked.connect(this,&NMessageBox::onBtnPress);
    btnPnl->add(okBtn,nAlRight);
    useBtn = new NButton("use");
      useBtn->setFlat(false);
      useBtn->clicked.connect(this,&NMessageBox::onBtnPress);
    btnPnl->add(useBtn,nAlRight);
    cancelBtn = new NButton("cancel");
      cancelBtn->setFlat(false);
      cancelBtn->clicked.connect(this,&NMessageBox::onBtnPress);
    btnPnl->add(cancelBtn,nAlRight);
  pane()->add(btnPnl,nAlBottom);

  textLbl = new NLabel();
  pane()->add(textLbl,nAlTop);

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
     doClose(nMsgOkBtn);
  } else
  if (ev->sender()==useBtn) {
     doClose(nMsgUseBtn);
  } else
  if (ev->sender()==cancelBtn) {
    doClose(nMsgCancelBtn);
  }
}

void NMessageBox::setVisible( bool on )
{
  if (on) pack();
  NDialog::setVisible(on);
}

NImage * NMessageBox::icon( )
{
  return iconImg;
}





