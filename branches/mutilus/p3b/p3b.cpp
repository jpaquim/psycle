#include "mainwindow.h"

#include <QApplication>
#include <QCoreApplication>
#include <QSettings>

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);
	Q_INIT_RESOURCE(p3b);

	QCoreApplication::setOrganizationName("Psycle Software Krew");
	QCoreApplication::setOrganizationDomain("psycle.pastnotecut.org");
	QCoreApplication::setApplicationName("p3b");

	QSettings settings;
	settings.setValue( "plugins/psyclePath", "/home/neil/code/psycle.plugins/" );
	settings.setValue( "plugins/ladspaPath", "/usr/lib/ladspa/" );

	MainWindow mainWin;
	mainWin.show();
	return app.exec();
}
