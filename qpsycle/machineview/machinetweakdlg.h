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

#ifndef MACHINETWEAKDLG_H
#define MACHINETWEAKDLG_H

#include "psycore/machine.h"

#include <QDialog>
#include <QWidget>

class Knob : public QWidget {
public:
    Knob( QWidget *parent = 0 );
};
class FHeader : public QWidget {
public:
    FHeader( QWidget *parent = 0 );
};

class MachineTweakDlg : public QDialog {
    Q_OBJECT

public:
    MachineTweakDlg( psy::core::Machine *mac, QWidget *parent = 0 );

};

#endif
