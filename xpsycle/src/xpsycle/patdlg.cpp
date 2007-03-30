/***************************************************************************
	*   Copyright (C) 2006 by  Stefan   *
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
#include <ngrs/spinedit.h>
#include <ngrs/button.h>
#include <ngrs/panel.h>
#include <ngrs/flowlayout.h>

namespace psycle { namespace host {

PatDlg::PatDlg()
	: ngrs::Window()
{
	setPosition(0,0,200,100);
	setTitle("Pattern Beats");

	lineNumber_ = 0;
	lineNumEdit_ = new ngrs::SpinEdit();
		lineNumEdit_->setPosition(10,10,100,lineNumEdit_->preferredHeight());
		lineNumEdit_->decClick.connect(this,&PatDlg::onDecBtnClick);
		lineNumEdit_->incClick.connect(this,&PatDlg::onIncBtnClick);
	pane()->add(lineNumEdit_);

	ngrs::Panel* bPnl = new ngrs::Panel();
		bPnl->setAlign(ngrs::nAlBottom);
		bPnl->setLayout( ngrs::FlowLayout(ngrs::nAlRight) );
		ngrs::Button* okBtn = new ngrs::Button("Change");
			okBtn->clicked.connect(this,&PatDlg::onOkBtn);
			okBtn->setFlat(false);
		bPnl->add(okBtn);
		ngrs::Button* cancelBtn = new ngrs::Button("Cancel");
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
	return ngrs::nHideWindow;
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


void PatDlg::onOkBtn( ngrs::ButtonEvent * sender )
{
	std::stringstream str; 
	str << lineNumEdit_->text();
	str >> lineNumber_;

	do_Execute = true;
	setVisible(false);
	setExitLoop( ngrs::nDestroyWindow );
}

void PatDlg::onCancelBtn( ngrs::ButtonEvent* sender )
{
	do_Execute = false;
	setVisible( false );
	setExitLoop( ngrs::nDestroyWindow );
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

void PatDlg::onIncBtnClick( ngrs::ButtonEvent * ev )
{
	std::stringstream str; 
	str << lineNumEdit_->text();
	str >> lineNumber_;
	if (lineNumber_ < MAX_LINES) lineNumber_++;
	lineNumEdit_->setText(stringify(lineNumber_));
	lineNumEdit_->repaint();
}

void PatDlg::onDecBtnClick( ngrs::ButtonEvent * ev )
{
	std::stringstream str; 
	str << lineNumEdit_->text();
	str >> lineNumber_;
	if (lineNumber_ > 0) lineNumber_--;
	lineNumEdit_->setText(stringify(lineNumber_));
	lineNumEdit_->repaint();
}

}}
