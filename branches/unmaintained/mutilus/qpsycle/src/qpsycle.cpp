/*
	Copyright 2007 Psycledelics http://psycle.sourceforge.net
	
	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the GNU General Public License for more details.
	
	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the
	Free Software Foundation, Inc.,
	59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/
#include <qpsyclePch.hpp>

#include <QApplication>
#include <QtGui/qicon.h>

#include "gui/mainwindow.h"
#include "gui/global.h"

int main(int argc, char *argv[])
{
	QApplication *app = new QApplication(argc, argv);
	Q_INIT_RESOURCE(qpsycle);
	app->setWindowIcon ( QIcon(":images/psycle.png") );
	MainWindow mainWin;
	mainWin.show();

	Global::Instance().setApplication( app );

	return app->exec();
}
