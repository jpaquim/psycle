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

#include <ngrs/panel.h>
#include <ngrs/edit.h>
#include <ngrs/button.h>
#include <ngrs/label.h>
#include <ngrs/alignlayout.h>

namespace psycle {
namespace host {

MacPropDlg::MacPropDlg(Machine* machine)
  : ngrs::Window()
{
  pMach_ = machine;
  setPosition(0,0,260,260);

  init();
}

MacPropDlg::~MacPropDlg() { }

void MacPropDlg::init( )
{
      setTitle(pMach_->GetEditName() + " Properties");
      ngrs::Label* machNameLbl = new ngrs::Label("Name:");
      nameEdit_ = new ngrs::Edit();
      nameEdit_->setText( pMach_->GetEditName() );
      ngrs::Panel* buttonPnl = new ngrs::Panel();
      buttonPnl->setLayout( ngrs::AlignLayout( 5, 5) );
              ngrs::Button* deleteBtn = new ngrs::Button("Delete Machine");
                      deleteBtn->setFlat(false);
                      deleteBtn->clicked.connect(this,&MacPropDlg::onDeleteBtn);
              buttonPnl->add(deleteBtn,ngrs::nAlTop);
              ngrs::Button* cloneBtn = new ngrs::Button("Clone Machine");
                      cloneBtn->setFlat(false);
                      cloneBtn->clicked.connect(this,&MacPropDlg::onCloneBtn);
              buttonPnl->add(cloneBtn,ngrs::nAlTop);
      ngrs::Button* okBtn = new ngrs::Button("OK");
              okBtn->setFlat( false );
              okBtn->clicked.connect( this, &MacPropDlg::onOKBtn );

      pane()->add( machNameLbl, ngrs::nAlTop );
      pane()->add( nameEdit_, ngrs::nAlTop );
      pane()->add( buttonPnl, ngrs::nAlTop );
      pane()->add( okBtn, ngrs::nAlBottom );
}

void MacPropDlg::onOKBtn( ngrs::ButtonEvent *ev )
{
      pMach_->SetEditName(nameEdit_->text()); // good design to set this here?
      updateMachineProperties.emit(pMach_);
      onClose();
}

void MacPropDlg::onCloneBtn( ngrs::ButtonEvent *ev )
{
}

void MacPropDlg::onDeleteBtn( ngrs::ButtonEvent *ev )
{
        deleteMachine.emit();
}

void MacPropDlg::setVisible(bool on)
{
        if (on) {
                setTitle(pMach_->GetEditName() + " Properties");
        }
        ngrs::Window::setVisible(on);
}

int MacPropDlg::onClose( )
{
  setVisible(false);
  return ngrs::nHideWindow;
}


}}

