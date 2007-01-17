/***************************************************************************
 *   Copyright (C) 2005, 2006, 2007 by  Stefan Nattkemper   *
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
#include "messagebox.h"
#include "button.h"
#include "flowlayout.h"
#include "alignlayout.h"
#include "label.h"
#include "image.h"

namespace ngrs {

  MessageBox::MessageBox()
    : Dialog()
  {
    init();
  }

  MessageBox::MessageBox( const std::string & msgText )
  {
    init();
    textLbl->setText(msgText);
  }


  void MessageBox::init( )
  {
    setTitle("Message");

    iconImg = new Image();
    pane()->add(iconImg, nAlLeft);

    Panel* btnPnl = new Panel();
    btnPnl->setLayout(AlignLayout(5,5));
    okBtn = new Button("ok");
    okBtn->setFlat(false);
    okBtn->clicked.connect(this,&MessageBox::onBtnPress);
    btnPnl->add(okBtn,nAlRight);
    useBtn = new Button("use");
    useBtn->setFlat(false);
    useBtn->clicked.connect(this,&MessageBox::onBtnPress);
    btnPnl->add(useBtn,nAlRight);
    cancelBtn = new Button("cancel");
    cancelBtn->setFlat(false);
    cancelBtn->clicked.connect(this,&MessageBox::onBtnPress);
    btnPnl->add(cancelBtn,nAlRight);
    pane()->add(btnPnl,nAlBottom);

    textLbl = new Label();
    pane()->add(textLbl,nAlTop);

  }


  MessageBox::~MessageBox()
  {
  }

  void MessageBox::setButtons( int btnMask )
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

  void MessageBox::setButtonText( const std::string & ok, const std::string & use, const std::string & cancel )
  {
    okBtn->setText(ok);
    cancelBtn->setText(cancel);
    useBtn->setText(use);
  }

  void MessageBox::setText( const std::string & msgText )
  {
    textLbl->setText(msgText);
  }

  void MessageBox::onBtnPress( ButtonEvent * ev )
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

  void MessageBox::setVisible( bool on )
  {
    if (on) pack();
    Dialog::setVisible(on);
  }

  Image * MessageBox::icon( )
  {
    return iconImg;
  }

}
