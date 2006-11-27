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
#include "macprop.h"
#include "machine.h"

#include <ngrs/npanel.h>
#include <ngrs/nedit.h>
#include <ngrs/nbutton.h>
#include <ngrs/nlabel.h>
#include <ngrs/nalignlayout.h>

namespace psycle {
namespace host {

MacProp::MacProp(MachineGUI* mGUI)
  : NWindow()
{
  pMGUI_ = mGUI;

  setPosition(0,0,260,260);

  init();
}


MacProp::~MacProp()
{
}

void MacProp::init( )
{
      setTitle(pMGUI_->pMac()->GetEditName() + " Properties");
      NLabel* machNameLbl = new NLabel("Name:");
      nameEdit_ = new NEdit();
      nameEdit_->setText(pMGUI_->pMac()->GetEditName());
      NPanel* buttonPnl = new NPanel();
      buttonPnl->setLayout(NAlignLayout(5,5));
              NButton* deleteBtn = new NButton("Delete Machine");
                      deleteBtn->setFlat(false);
              buttonPnl->add(deleteBtn,nAlTop);
              NButton* cloneBtn = new NButton("Clone Machine");
                      cloneBtn->setFlat(false);
              buttonPnl->add(cloneBtn,nAlTop);
      NButton* okBtn = new NButton("OK");
              okBtn->setFlat(false);
              okBtn->clicked.connect(this,&MacProp::onOKBtn);

      pane()->add(machNameLbl,nAlTop);
      pane()->add(nameEdit_,nAlTop);
      pane()->add(buttonPnl,nAlTop);
      pane()->add(okBtn,nAlBottom);
}

void MacProp::onOKBtn(NButtonEvent *ev)
{
      pMGUI_->pMac()->SetEditName(nameEdit_->text());
      pMGUI_->repaint();
      onClose();
}

void MacProp::setVisible(bool on)
{
        if (on) {
                setTitle(pMGUI_->pMac()->GetEditName() + " Properties");
        }
        NWindow::setVisible(on);
}

int MacProp::onClose( )
{
  setVisible(false);
  return nHideWindow;
}


}}

