#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    Q_INIT_RESOURCE(p3b);

    MainWindow mainWin;
    mainWin.show();
    return app.exec();
}
