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

namespace psy { namespace core {
class Machine;
}}

class MachineGui;

#include <map>

#include <QDialog>
#include <QDial>
#include <QLabel>

class QGridLayout;
class QVBoxLayout;
class QMenuBar;
class QAction;
class QMenu;

const int K_XSIZE = 28;
const int K_YSIZE = 28;
const int K_NUMFRAMES = 63;
const int W_ROWWIDTH = 150;
const int LABEL_WIDTH = 150;

class Knob : public QDial {
public:
    Knob( int param );
    QSize sizeHint() const;
    int param() { return param_; }

protected: 
    void paintEvent( QPaintEvent *ev );

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
    QSize sizeHint() const;

public slots:
    void onKnobChanged();
    void onKnobPressed();
    void onKnobReleased();

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
protected:
    void paintEvent( QPaintEvent *ev );
};

/**
 * MachineTweakDlg
 */
class MachineTweakDlg : public QDialog {
Q_OBJECT

	public:
	MachineTweakDlg( MachineGui *macGui, QWidget *parent = 0 );

protected:
	void showEvent( QShowEvent *event );
	void keyPressEvent( QKeyEvent *event );
	void keyReleaseEvent( QKeyEvent *event );


public slots: 
	void onKnobGroupChanged( KnobGroup *kGroup );
	void randomiseParameters();

signals:
	void notePress( int note, psy::core::Machine *mac );
	void noteRelease( int note );

private:
	void createActions();
	void createMenus();
	void initParameterGui();
	void updateValues();

	QAction *paramsResetAction_;
	QAction *paramsRandomAction_;
	QAction *paramsOpenPrsAction_;
	QMenuBar *menuBar;
	QMenu *aboutMenu;
	QMenu *paramsMenu;
	QWidget *knobPanel;
	QVBoxLayout *mainLayout;
	QGridLayout *knobPanelLayout;
	psy::core::Machine *pMachine_;
	MachineGui *m_macGui;

	std::map<int, KnobGroup*> knobGroupMap;
	std::map<int, FHeader*> headerMap;
};

#endif
