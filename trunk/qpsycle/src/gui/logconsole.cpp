/***************************************************************************
*   Copyright (C) 2007 Psycledelics Community   *
*   psycle.sourceforge.net   *
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
#include <qpsyclePch.hpp>

#include "logconsole.hpp"

namespace qpsycle {

LogConsole::LogConsole()
{
	setAttribute(Qt::WA_QuitOnClose, false);
	CreateToolbar();
	layOut_ = new QVBoxLayout(this);
	logText = new QTextEdit(this);
	logText->setReadOnly(true);
	logText->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
	layOut_->addWidget(logText);
	layOut_->addWidget(toolBar_);
	setLayout(layOut_);
	logText->append("Logging Console is active");
}
void LogConsole::AddNormalText(QString text)
{
	text += "\n";
	logText->append(text);
}

void LogConsole::AddSuccessText(QString text)
{
	text = "<font color=green><i>" + text + "\n</i></font>";
	logText->append(text);
}

void LogConsole::AddErrorText(QString text)
{
	text = "<font color=red><b>" + text + "\n</b></font>";
	logText->append(text);
}

void LogConsole::Clear()
{
	logText->clear();
}

void LogConsole::SaveToFile()
{

}

void LogConsole::CreateToolbar()
{
	toolBar_ = new QToolBar(this);
	
	saveAct_ = new QAction(QIcon(":/images/save.png"), "Save To File", this);

	clearAct_ = new QAction("Clear Buffer", this);
	
	connect(clearAct_, SIGNAL(triggered()),this, SLOT(Clear()));
	
	toolBar_->addAction(saveAct_);
	toolBar_->addAction(clearAct_);
}

}
