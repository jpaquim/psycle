/***************************************************************************
*   Copyright (C) 2007 Psycledelics Community   *
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
#include <qpsyclePch.hpp>

#include "global.hpp"
#include "configuration.hpp"

#include <QApplication>
#include <QDesktopWidget>

namespace qpsycle {

Global::Global(){};

Configuration* Global::pConfig() {
	static Configuration* c = new Configuration();
	return c;
}

Configuration const & Global::configuration() {
	return *pConfig();
}

int Global::screenHeight()
{
	// qApp is a handy global pointer referring to the
	// unique application object (i.e. qpsycle's QApplication.) 
	QDesktopWidget *desktopWidget = qApp->desktop();
	// We're assuming for now psycle doesn't run dual-head...
	// so access default, primary screen.
	return desktopWidget->screenGeometry().height();
}

}
