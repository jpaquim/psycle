// -*- mode:c++; indent-tabs-mode:t -*-
/***************************************************************************
*   Copyright (C) 2007 by Psycledelics Community   *
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

#ifndef BEHAVIOURPAGE_H
#define BEHAVIOURPAGE_H

#include <QString>
#include <QWidget>

class QComboBox;
class QPushButton;
class QCheckBox;

namespace qpsycle {

class Configuration;

class BehaviourPage : public QWidget {
Q_OBJECT
public:
	BehaviourPage( QWidget *parent = 0 );

public slots:
	void onSaveButtonClicked();
	void onSettingsChanged();

private:
	Configuration *config_;
	QComboBox *knobBehaviourCombo_;
	QPushButton *saveBtn_;
	QPushButton *closeBtn_;

	QCheckBox *homeEndChk;
	QCheckBox *shiftChk;
	QCheckBox *wrapChk;
	QCheckBox *centerCursorChk;
};

} // namespace qpsycle

#endif
