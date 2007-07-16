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
#include "logconsole.h"
LogConsole::LogConsole()
{
	setAttribute(Qt::WA_QuitOnClose, false);
	layOut_ = new QVBoxLayout(this);
	logText = new QTextEdit(this);
	logText->setReadOnly(true);
	logText->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
	layOut_->addWidget(logText);
	setLayout(layOut_);
	logText->append("Logging Console is active\n");
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
