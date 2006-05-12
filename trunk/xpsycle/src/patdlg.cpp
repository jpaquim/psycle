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
#include "constants.h"
#include <nspinedit.h>
#include <nbutton.h>
#include <npanel.h>
#include <nflowlayout.h>

PatDlg::PatDlg()
 : NWindow()
{
  setPosition(0,0,200,100);
  setTitle("Pattern Lines");

  lineNumber_ = 0;
  lineNumEdit_ = new NSpinEdit();
    lineNumEdit_->setPosition(10,10,100,lineNumEdit_->preferredHeight());
    lineNumEdit_->decClicked.connect(this,&PatDlg::onDecBtnClicked);
    lineNumEdit_->incClicked.connect(this,&PatDlg::onIncBtnClicked);
  pane()->add(lineNumEdit_);

  NPanel* bPnl = new NPanel();
    bPnl->setAlign(nAlBottom);
    bPnl->setLayout(NFlowLayout(nAlRight));
    NButton* okBtn = new NButton("Change");
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
  str << lineNumEdit_->text();
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
  std::stringstream str; 
  str << lineNumEdit_->text();
  str >> lineNumber_;

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
  if (lineNumber_ > MAX_LINES && lineNumber_ < 0) lineNumber = 0;

  lineNumEdit_->setText(stringify(lineNumber));
  lineNumEdit_->repaint();
}

bool PatDlg::adaptSize() {
  return false;
}

void PatDlg::onIncBtnClicked( NButtonEvent * ev )
{
  std::stringstream str; 
  str << lineNumEdit_->text();
  str >> lineNumber_;
  if (lineNumber_ < MAX_LINES) lineNumber_++;
  lineNumEdit_->setText(stringify(lineNumber_));
  lineNumEdit_->repaint();
}

void PatDlg::onDecBtnClicked( NButtonEvent * ev )
{
  std::stringstream str; 
  str << lineNumEdit_->text();
  str >> lineNumber_;
  if (lineNumber_ > 0) lineNumber_--;
  lineNumEdit_->setText(stringify(lineNumber_));
  lineNumEdit_->repaint();
}

