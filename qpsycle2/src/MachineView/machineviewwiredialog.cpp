#include "machineviewwiredialog.h"

#include "statics.h"
#include <cmath>
#include <QSlider>
#include <QPushButton>
#include <QGridLayout>

namespace qpsycle{

const int SliderSize = 1024;
const int MaxVolume = 4; //multiplier, not dB
const double scale = (SliderSize*SliderSize)/(float)MaxVolume;  //262144.0f


MachineViewWireDialog::MachineViewWireDialog(MachineViewLine* line)
{
    wire = line;
    this->setWindowTitle("Wire settings");
    this->setWindowFlags(Qt::Dialog);
    QGridLayout* layout = new QGridLayout(this);
    this->setLayout(layout);
    QSlider* slider = new QSlider();
    slider->setTickPosition(QSlider::TicksBothSides);
    slider->setRange(0, SliderSize);
    slider->setTickInterval(SliderSize/16);
    slider->setPageStep(SliderSize/16);
    slider->setSingleStep(SliderSize/128);
    slider->setToolTip("Volume");
    slider->setValue((int)sqrtf(wire->getDest()->getMachine()->GetWireVolume(wire->getWire())*scale));
    QObject::connect(slider, SIGNAL(sliderMoved(int)),this,SLOT(updateVolume(int)));
    layout->addWidget(slider);
    QPushButton* button = new QPushButton("Delete Wire");
    layout->addWidget(button);
    QObject::connect(button,SIGNAL(clicked()),this,SLOT(deleteWire()));
    button = new QPushButton("Close");
    layout->addWidget(button);
    QObject::connect(button,SIGNAL(clicked()),this,SLOT(close()));
}

void MachineViewWireDialog::updateVolume(int i){
    double newvol = (i * i) / scale;
    float f;
    wire->getDest()->getMachine()->GetWireVolume( wire->getWire(), f );
    if (f != newvol)
    {
        ///\todo undo code
        //m_pParent->AddMacViewUndo();
        wire->getDest()->getMachine()->SetWireVolume( wire->getWire(), newvol );
    }
}

void MachineViewWireDialog::deleteWire(){
    wire->getSource()->getMachine()->Disconnect(*wire->getDest()->getMachine());
    wire->getDest()->removeInLine(wire);
    wire->getSource()->removeOutLine(wire);
    delete wire;
    this->close();
}

}
