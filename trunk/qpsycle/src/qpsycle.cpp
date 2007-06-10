#include "gui/mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
  QApplication app(argc, argv);
  Q_INIT_RESOURCE(qpsycle);
  app.setWindowIcon ( QIcon(":images/psycle.png") );
  app.setStyle("plastique");
  MainWindow mainWin;
  mainWin.show();
  return app.exec();
}
