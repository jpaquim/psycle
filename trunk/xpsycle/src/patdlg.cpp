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
#include "patdlg.h"

#include "nedit.h"
#include "nbutton.h"
#include "npanel.h"
#include "nflowlayout.h"

PatDlg::PatDlg()
 : NWindow()
{
  lineNumber_ = 0;
  lineNumEdit = new NEdit();
    lineNumEdit->setPosition(10,10,100,lineNumEdit->preferredHeight());
  pane()->add(lineNumEdit);

  NPanel* bPnl = new NPanel();
    bPnl->setAlign(nAlBottom);
    bPnl->setLayout(new NFlowLayout(nAlRight));
    NButton* okBtn = new NButton("Open");
      okBtn->clicked.connect(this,&PatDlg::onOkBtn);
      okBtn->setFlat(false);
    bPnl->add(okBtn);
    NButton* cancelBtn = new NButton("Cancel");
      cancelBtn->clicked.connect(this,&PatDlg::onCancelBtn);
      cancelBtn->setFlat(false);
    bPnl->add(cancelBtn);
  pane()->add(bPnl);

  setModal(true);
  do_Execute = false;
}

PatDlg::~PatDlg()
{
}

int PatDlg::onClose( )
{
  std::stringstream str; 
  str << lineNumEdit->text();
  str >> lineNumber_;

  setVisible(false);
  return nHideWindow;
}

int PatDlg::lineNumber( )
{
  return lineNumber_;
}

bool PatDlg::execute( )
{
  setVisible(true);
  return do_Execute;
}


void PatDlg::onOkBtn( NButtonEvent * sender )
{
  do_Execute = true;
  setVisible(false);
  setExitLoop(nDestroyWindow);
}

void PatDlg::onCancelBtn( NButtonEvent * sender )
{
  do_Execute = false;
  setVisible(false);
  setExitLoop(nDestroyWindow);
}

void PatDlg::setLineNumber(int lineNumber )
{
  lineNumber_ = lineNumber;
}

bool PatDlg::adaptSize() {
  return false;
}
