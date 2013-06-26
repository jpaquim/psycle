#include <QGraphicsSceneMouseEvent>

#include "effectguielement.h"
#include "MachineView/machineview.h"
#include "statics.h"
#include "MachineView/machinetweakdialog.h"

#include "psycle/core/machinefactory.h"
#include "psycle/core/pluginfinder.h"


namespace qpsycle{

EffectGuiElement::EffectGuiElement(const QPoint& pos,psycle::core::Machine* mach)
{
    canAccept = true;
    canOutput = true;
    this->setPos(pos);
    background=new QGraphicsPixmapItem(this);
    background->setPixmap(Statics::theme()->effect);
    machine = mach;
    text= new QGraphicsSimpleTextItem(QString::fromStdString(mach->GetEditName()), this);
    text->setPos(Statics::theme()->effectNameDest.x(),Statics::theme()->effectNameDest.y());
    GuiMachineElement();

}

void EffectGuiElement::mouseDoubleClickEvent(QGraphicsSceneMouseEvent */*event*/){
    MachineTweakDialog* tweakWindow = new MachineTweakDialog(0,this);
    tweakWindow->show();
}
}
