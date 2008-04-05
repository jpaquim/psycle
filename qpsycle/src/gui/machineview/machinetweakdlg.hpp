// -*- mode:c++; indent-tabs-mode:t -*-
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

/**
 * QSynth style knob behaviour based on sf.net/projects/qsynth
 * original (c) Rui Nuno Capela
 * Any dodgy modifications not his fault!
 */


#ifndef MACHINETWEAKDLG_H
#define MACHINETWEAKDLG_H

namespace psy { namespace core {
	class Machine;
}}

#include <psycle/core/preset.h>

#include "configuration.hpp"

#include <map>

#include <QDialog>
#include <QDial>
#include <QLabel>
#include <QMessageBox>
#include <QModelIndex>
#include <QListWidget>

class QGridLayout;
class QVBoxLayout;
class QMenuBar;
class QAction;
class QMenu;
class QCheckBox;
class QLineEdit;
class QListWidget;
class QListWidgetItem;

namespace qpsycle {

class MachineGui;
class MachineView;

const int K_XSIZE = 28;
const int K_YSIZE = 28;
const int K_NUMFRAMES = 63;
const int W_ROWWIDTH = 150;
const int LABEL_WIDTH = 150;

class Knob : public QDial {
public:
	Knob( int param );
	QSize sizeHint() const;
	int param() const { return m_paramIndex; }

protected: 
	void paintEvent( QPaintEvent *ev );
	void mousePressEvent( QMouseEvent *ev );
	void mouseMoveEvent( QMouseEvent *ev );
	void mouseReleaseEvent( QMouseEvent *ev );

	double mouseAngle(const QPoint& pos) const;

private:
	int m_paramIndex;
	QPoint cursorTriggerPoint_;

	double m_accumulator;

// Knob dial mode behavior.
	KnobMode m_knobMode;

// Alternate mouse behavior tracking.
	bool   m_bMousePressed;
	QPoint m_posMousePressed;

	bool m_fineTweak;
	bool m_ultraFineTweak;
	int m_initialValue;

// Just for more precission on the movement
	double m_lastDragValue;
};

class KnobGroup : public QWidget {
	Q_OBJECT
public:
	KnobGroup( int param );

	void setNameText( const QString & text );
	void setValueText( const QString & text );
	void setKnob( Knob *knob );
	Knob *knob() const;
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

class SectionHeader : public QLabel {
public:
	SectionHeader( QWidget *parent = 0 );

	QSize sizeHint() const;
protected:
	void paintEvent( QPaintEvent *ev );

};

class KnobHole : public QLabel {
public:
	KnobHole( QWidget *parent = 0 );
protected:
	void paintEvent( QPaintEvent *ev );
};



/**
	* PresetsDlg
	*/
class PresetsDialog : public QDialog {
Q_OBJECT
public:
	PresetsDialog( MachineGui *macGui, QWidget *parent = 0 );

	bool loadPresets();

public slots:
	void usePreset();
	void onCompletionActivated( const QString &text );
	void onItemClicked( QListWidgetItem *item );
	void onSavePreset();

private:
	std::map< QListWidgetItem*, psy::core::Preset > presetMap;
	psy::core::Preset m_selectedPreset;
	QGridLayout *lay;
	QLabel *label;
	QLineEdit *lineEdit;
	QListWidget *prsList;
	
	QPushButton *saveBtn;
	QPushButton *delBtn;
	QPushButton *impBtn;
	QPushButton *expBtn;

	QCheckBox *prevChk;
	

	QPushButton *useBtn;
	QPushButton *clsBtn;

	MachineGui *m_macGui;

	int const static MAX_PRESETS = 256;

	psy::core::Preset iniPreset;
};


/**
 * MachineTweakDlg
 */
class MachineTweakDlg : public QDialog {
Q_OBJECT
	public:
	MachineTweakDlg( MachineGui *macGui, QWidget *parent );

	void setupLayout();

protected:
	void showEvent( QShowEvent *event );
	void keyPressEvent( QKeyEvent *event );
	void keyReleaseEvent( QKeyEvent *event );


public slots: 
	virtual void onKnobGroupChanged( KnobGroup *kGroup );
	void randomiseParameters();
	void resetParameters();
	void showAboutDialog();
	void showPresetsDialog();

signals:
	void notePress( int note, psy::core::Machine *mac );
	void noteRelease( int note, psy::core::Machine *mac );

protected:
	void createActions();
	void createMenus();
	virtual void initParameterGui();
	virtual void updateValues();

	QAction *aboutAction_;
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

	PresetsDialog *prsDlg;

	std::map<int, KnobGroup*> knobGroupMap;
	std::map<int, SectionHeader*> headerMap;
};

} // namespace qpsycle

#endif
