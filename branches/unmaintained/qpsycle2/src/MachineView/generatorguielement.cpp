#include "generatorguielement.h"
#include "MachineView/machinethemeloader.h"
#include "MachineView/machineview.h"
#include "MachineView/machinetweakdialog.h"
#include "statics.h"


#include "psycle/core/machinefactory.h"
#include "psycle/core/pluginfinder.h"

namespace qpsycle{

GeneratorGuiElement::GeneratorGuiElement(const QPoint& pos, psycle::core::Machine* mach)
{
    canAccept=false;
    canOutput=true;
    this->setPos(pos);
    background = new QGraphicsPixmapItem(this);
    background->setPixmap(Globals::theme()->generator);
    machine = mach;
    text= new QGraphicsSimpleTextItem(QString::fromStdString(mach->GetEditName()), this);
    text->setPos(Globals::theme()->generatorNameDest.x(),Globals::theme()->generatorNameDest.y());
    GuiMachineElement();

}

void GeneratorGuiElement::mouseDoubleClickEvent(QGraphicsSceneMouseEvent */*event*/){
    MachineTweakDialog* tweakWindow = new MachineTweakDialog(0,this);
    tweakWindow->show();
}

}
