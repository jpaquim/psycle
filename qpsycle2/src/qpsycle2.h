#ifndef QPSYCLE2_H
#define QPSYCLE2_H

#include <QMainWindow>

namespace qpsycle{
class MenuSignalHandler;
class qpsycle2 : public QMainWindow
{
    Q_OBJECT

    
public:
    qpsycle2(QWidget *parent = 0);
    ~qpsycle2();
    MenuSignalHandler* getSignalHandler();

private slots:

private:
    MenuSignalHandler* handler;
};

}
#endif // QPSYCLE2_H
