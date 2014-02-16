#include <QApplication>
#include "qpsycle2.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    qpsycle::qpsycle2 w;
    w.showMaximized();
    
    return a.exec();
}
