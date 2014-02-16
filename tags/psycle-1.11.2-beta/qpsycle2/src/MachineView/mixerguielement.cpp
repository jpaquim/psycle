#include "mixerguielement.h"
#include "MachineView/machineview.h"
#include "MachineView/machinethemeloader.h"
#include "statics.h"

namespace qpsycle{

MixerGuiElement::MixerGuiElement(const QPoint& pos, psycle::core::Machine *mac)
{
    canAccept=true;
    canOutput=false;
    this->setPos(pos);
    background=new QGraphicsPixmapItem(this);
    background->setPixmap(Statics::theme()->master);
    machine = mac;
}

void MixerGuiElement::mouseDoubleClickEvent(QGraphicsSceneMouseEvent */*event*/){

}
}
