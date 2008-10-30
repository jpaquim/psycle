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

#ifndef LOOKSPAGE_H
#define LOOKSPAGE_H

#include <QString>
#include <QWidget>

class QComboBox;
class QPushButton;
class QCheckBox;

namespace qpsycle {

class Configuration;

class LooksPage : public QWidget {
Q_OBJECT

private:
	QComboBox *themeCombo;
public:
	LooksPage( QWidget *parent = 0 );

public slots:
	void oncolorSchemeComboChanged( const QString &sheetName );
	void onthemeComboChanged( const QString &themeName );

};

} // namespace qpsycle

#endif
