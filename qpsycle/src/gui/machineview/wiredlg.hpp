/* -*- mode:c++, indent-tabs-mode:t -*- */
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
#ifndef WIREDLG_H
#define WIREDLG_H

namespace psy {
	namespace core {
		class Machine;
	}
}

#include <QDialog>

class QSlider;
class WireGui;
class QLabel;

namespace qpsycle {

class WireDlg : public QDialog
{
	Q_OBJECT

public:
	WireDlg(WireGui *wire, QWidget *parent, QPoint where);
	void wireChanged();

public slots:
	void sliderMoved(int newval);

signals:
	void volumeChanged(float newval);

private:
	WireGui* m_wireGui;

	//<dw> m_wireId should be a psy::core::Wire::id_type, but i'd have to
	//#include machine.h here to use it.  Wire::id_type is actually listed
	//as legacy..  looking at machine.h, it seems like we should have a
	//pointer to a Wire object in WireGui.  is this ready to implement yet,
	//or are changes needed in Machine first?
	int m_wireId; 
	psy::core::Machine *m_sourceMac, *m_destMac;

	float m_volume;

	QSlider* m_volSlider;
	QLabel* m_volLabel;
};

} // namespace qpsycle

#endif
