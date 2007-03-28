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
#include <QDial>
#include <QLabel>

class Knob : public QDial {
public:
    Knob( int param );
    QSize sizeHint() const;
    int param() { return param_; }

private:
    int param_;
};

class KnobGroup : public QWidget {
    Q_OBJECT
public:
    KnobGroup( int param );

    void setNameText( const QString & text );
    void setValueText( const QString & text );
    void setKnob( Knob *knob );
    Knob *knob();

public slots:
    void onKnobChanged();

signals:
    void changed( KnobGroup *kGroup );

private:
    Knob *knob_;
    QLabel *nameLbl;
    QLabel *valueLbl;
};

class FHeader : public QLabel {
public:
    FHeader( QWidget *parent = 0 );
};

class MachineTweakDlg : public QDialog {
    Q_OBJECT

public:
    MachineTweakDlg( psy::core::Machine *mac, QWidget *parent = 0 );

protected:
    void showEvent( QShowEvent *event );

public slots: 
    void onKnobGroupChanged( KnobGroup *kGroup );

private:
    void initParameterGui();
    void updateValues();

    QWidget *knobPanel;
    psy::core::Machine *pMachine_;

    std::map<int, KnobGroup*> knobGroupMap;
    std::map<int, FHeader*> headerMap;
};

#endif
