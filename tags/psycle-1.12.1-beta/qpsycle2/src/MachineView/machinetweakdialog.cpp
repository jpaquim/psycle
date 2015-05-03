#include "machinetweakdialog.h"
#include <QGridLayout>
#include <QPushButton>
#include <QDial>
#include <QSizePolicy>

namespace qpsycle{

MachineTweakDialog::MachineTweakDialog(QWidget* parent, GuiMachineElement *mach):QDialog(parent)
{
    machine = mach;
    QGridLayout* layout = new QGridLayout(this);
    setLayout(layout);
    setWindowTitle(QString::fromStdString(machine->getMachine()->GetName()));
    int columns = machine->getMachine()->GetNumCols();
    int paramNumber = machine->getMachine()->GetNumParams();
    for (int i = 0;i<paramNumber;i++){
        layout->addLayout(new dialElement(machine->getMachine(),i),i%columns,i/columns);

    }
    //make it not huge by default.
    resize(minimumSize());
}

dialElement::dialElement(psycle::core::Machine *mach, int paramNum)
{
    machine = mach;
    param = paramNum;
    machine->GetParamName(param,paramName);
    machine->GetParamValue(param,paramValue);
    machine->GetParamRange(param,min,max);
    max++;
    dial = new QDial();
    dial->resize(dial->minimumSize());
    dial->setValue(machine->GetParamValue(param));
    dial->setMinimum(min);
    dial->setMaximum(max);
    addWidget(dial,0,0,0,1);
    nameLabel = new QLabel();
    nameLabel->setText(QString(paramName));
    addWidget(nameLabel,0,1);
    valueLabel = new QLabel();
    valueLabel->setText(QString(paramValue));
    addWidget(valueLabel,1,1);
    QObject::connect(dial,SIGNAL(valueChanged(int)),this,SLOT(update()));
}

void dialElement::update(){
    machine->SetParameter(param,dial->value());
    machine->GetParamValue(param,paramValue);
    valueLabel->setText(QString(paramValue));

}

}
