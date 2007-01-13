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
  : ngrs::NWindow()
{
  pMach_ = machine;
  setPosition(0,0,260,260);

  init();
}

MacPropDlg::~MacPropDlg() { }

void MacPropDlg::init( )
{
      setTitle(pMach_->GetEditName() + " Properties");
      ngrs::NLabel* machNameLbl = new ngrs::NLabel("Name:");
      nameEdit_ = new ngrs::NEdit();
      nameEdit_->setText( pMach_->GetEditName() );
      ngrs::NPanel* buttonPnl = new ngrs::NPanel();
      buttonPnl->setLayout( ngrs::NAlignLayout( 5, 5) );
              ngrs::NButton* deleteBtn = new ngrs::NButton("Delete Machine");
                      deleteBtn->setFlat(false);
                      deleteBtn->clicked.connect(this,&MacPropDlg::onDeleteBtn);
              buttonPnl->add(deleteBtn,ngrs::nAlTop);
              ngrs::NButton* cloneBtn = new ngrs::NButton("Clone Machine");
                      cloneBtn->setFlat(false);
                      cloneBtn->clicked.connect(this,&MacPropDlg::onCloneBtn);
              buttonPnl->add(cloneBtn,ngrs::nAlTop);
      ngrs::NButton* okBtn = new ngrs::NButton("OK");
              okBtn->setFlat( false );
              okBtn->clicked.connect( this, &MacPropDlg::onOKBtn );

      pane()->add( machNameLbl, ngrs::nAlTop );
      pane()->add( nameEdit_, ngrs::nAlTop );
      pane()->add( buttonPnl, ngrs::nAlTop );
      pane()->add( okBtn, ngrs::nAlBottom );
}

void MacPropDlg::onOKBtn( ngrs::NButtonEvent *ev )
{
      pMach_->SetEditName(nameEdit_->text()); // good design to set this here?
      updateMachineProperties.emit(pMach_);
      onClose();
}

void MacPropDlg::onCloneBtn( ngrs::NButtonEvent *ev )
{
}

void MacPropDlg::onDeleteBtn( ngrs::NButtonEvent *ev )
{
        deleteMachine.emit();
}

void MacPropDlg::setVisible(bool on)
{
        if (on) {
                setTitle(pMach_->GetEditName() + " Properties");
        }
        ngrs::NWindow::setVisible(on);
}

int MacPropDlg::onClose( )
{
  setVisible(false);
  return ngrs::nHideWindow;
}


}}

