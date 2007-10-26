#include <iostream>

#include "mainwindow.h"

#include <QApplication>
#include <QCoreApplication>
#include <QSettings>

int main(int argc, char *argv[])
{
	if ( argc != 2 ) {
		std::cout << "Usage: ./p3b plugin-dll-name\n"
			  << "\t(e.g. ./p3b phantom.dll)" << std::endl;
		exit(1);
	}
	QString pluginLibName( argv[1] );

	QApplication app(argc, argv);
	Q_INIT_RESOURCE(p3b);

	QCoreApplication::setOrganizationName("Psycle Software Krew");
	QCoreApplication::setOrganizationDomain("psycle.pastnotecut.org");
	QCoreApplication::setApplicationName("p3b");

	QSettings settings;
	settings.setValue( "plugins/psyclePath", "/home/neil/code/psycle.plugins/" );
	settings.setValue( "plugins/ladspaPath", "/usr/lib/ladspa/" );
	settings.setValue( "mainPlugin/libName", pluginLibName );

	MainWindow mainWin;
	mainWin.show();
	return app.exec();
}
