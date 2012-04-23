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
/*
	to add some text to the logging console just call it's Add...(QString text) member
	you don't need to add \n at the end.... this void adds it automatically at the end :) MATTIAS
*/

#ifndef LOGCONSOLE_H
#define LOGCONSOLE_H

#include <QWidget>
#include <QString>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QToolBar>
#include <QAction>

namespace qpsycle {

class LogConsole : public QWidget
{
	Q_OBJECT
public: 
	LogConsole();
	void AddNormalText(QString text);
	void AddSuccessText(QString text);
	void AddErrorText(QString text);
	void CreateToolbar();

public slots:
	void Clear();
	void SaveToFile();
	
private:
	QVBoxLayout *layOut_;
	QToolBar *toolBar_;
	QAction *saveAct_;
	QAction *clearAct_;
	QTextEdit *logText;
};

}

#endif
