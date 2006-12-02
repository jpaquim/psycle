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
#include "macpropdlg.h"
#include "machine.h"

#include <ngrs/npanel.h>
#include <ngrs/nedit.h>
#include <ngrs/nbutton.h>
#include <ngrs/nlabel.h>
#include <ngrs/nalignlayout.h>

namespace psycle {
namespace host {

MacPropDlg::MacPropDlg(Machine* machine)
  : NWindow()
{
  pMach_ = machine;
  setPosition(0,0,260,260);

  init();
}

MacPropDlg::~MacPropDlg() { }

void MacPropDlg::init( )
{
      setTitle(pMach_->GetEditName() + " Properties");
      NLabel* machNameLbl = new NLabel("Name:");
      nameEdit_ = new NEdit();
      nameEdit_->setText(pMach_->GetEditName());
      NPanel* buttonPnl = new NPanel();
      buttonPnl->setLayout(NAlignLayout(5,5));
              NButton* deleteBtn = new NButton("Delete Machine");
                      deleteBtn->setFlat(false);
                      deleteBtn->clicked.connect(this,&MacPropDlg::onDeleteBtn);
              buttonPnl->add(deleteBtn,nAlTop);
              NButton* cloneBtn = new NButton("Clone Machine");
                      cloneBtn->setFlat(false);
                      cloneBtn->clicked.connect(this,&MacPropDlg::onCloneBtn);
              buttonPnl->add(cloneBtn,nAlTop);
      NButton* okBtn = new NButton("OK");
              okBtn->setFlat(false);
              okBtn->clicked.connect(this,&MacPropDlg::onOKBtn);

      pane()->add(machNameLbl,nAlTop);
      pane()->add(nameEdit_,nAlTop);
      pane()->add(buttonPnl,nAlTop);
      pane()->add(okBtn,nAlBottom);
}

void MacPropDlg::onOKBtn(NButtonEvent *ev)
{
      pMach_->SetEditName(nameEdit_->text()); // good design to set this here?
      updateMachineProperties.emit(pMach_);
      onClose();
}

void MacPropDlg::onCloneBtn(NButtonEvent *ev)
{
}

void MacPropDlg::onDeleteBtn(NButtonEvent *ev)
{
        deleteMachine.emit();
}

void MacPropDlg::setVisible(bool on)
{
        if (on) {
                setTitle(pMach_->GetEditName() + " Properties");
        }
        NWindow::setVisible(on);
}

int MacPropDlg::onClose( )
{
  setVisible(false);
  return nHideWindow;
}


}}

