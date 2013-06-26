#ifndef MACHINETWEAKDIALOG_H
#define MACHINETWEAKDIALOG_H

#include "MachineView/guimachineelement.h"

#include <QGridLayout>
#include <QDialog>
#include <QDial>
#include <QLabel>
#include <psycle/core/machine.h>

namespace qpsycle{

class MachineTweakDialog : public QDialog
{
public:
    MachineTweakDialog(QWidget *parent, GuiMachineElement *mach);

private:
    GuiMachineElement* machine;

};



class dialElement : public QGridLayout
{ 
Q_OBJECT

public:
    dialElement(psycle::core::Machine *mach,int paramNum);

private:
    psycle::core::Machine* machine;
    int param;
    QDial* dial;
    char paramName[100];
    char paramValue[100];
    QLabel* nameLabel;
    QLabel* valueLabel;
    int max,min = 0;

public Q_SLOTS:
    void update();
};


}
#endif // MACHINETWEAKDIALOG_H
