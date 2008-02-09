/* -*- mode:c++, indent-tabs-mode:t -*- */
/******************************************************************************
*  copyright 2007 members of the psycle project http://psycle.sourceforge.net *
*                                                                             *
*  This program is free software; you can redistribute it and/or modify       *
*  it under the terms of the GNU General Public License as published by       *
*  the Free Software Foundation; either version 2 of the License, or          *
*  (at your option) any later version.                                        *
*                                                                             *
*  This program is distributed in the hope that it will be useful,            *
*  but WITHOUT ANY WARRANTY; without even the implied warranty of             *
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the              *
*  GNU General Public License for more details.                               *
*                                                                             *
*  You should have received a copy of the GNU General Public License          *
*  along with this program; if not, write to the                              *
*  Free Software Foundation, Inc.,                                            *
*  59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.                  *
******************************************************************************/

#include <QApplication>
#include <QIcon>

#include "gui/mainwindow.hpp"

int main(int argc, char *argv[])
{
	QApplication *app = new QApplication(argc, argv);
	QCoreApplication::setOrganizationName( "Psycledelics" );
	QCoreApplication::setOrganizationDomain( "psycle.pastnotecut.org" );
	QCoreApplication::setApplicationName( "qpsycle" );

	Q_INIT_RESOURCE(qpsycle);

	app->setWindowIcon ( QIcon(":images/psycle.png") );

	qpsycle::MainWindow mainWin;
	mainWin.show();

	return app->exec();
}
