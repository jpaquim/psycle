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

#ifndef MIXERTWEAKDLG_H
#define MIXERTWEAKDLG_H

namespace psy { namespace core {
	class Machine;
}}

#include "../configuration.hpp"
#include "machinetweakdlg.hpp"

#include <map>

#include <QDial>
#include <QLabel>
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

/**
 * MixerTweakDlg
 */
class MixerTweakDlg : public MachineTweakDlg {
Q_OBJECT
public:
	MixerTweakDlg( MachineGui *macGui, QWidget *parent );

public slots: 
	virtual void onKnobGroupChanged( KnobGroup *kGroup );

private:
	virtual void initParameterGui();

};

} // namespace qpsycle

#endif
