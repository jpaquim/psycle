#ifndef MIXERGUIELEMENT_H
#define MIXERGUIELEMENT_H

#include "MachineView/guimachineelement.h"

namespace qpsycle{

class MixerGuiElement: public GuiMachineElement
{
public:
    MixerGuiElement(const QPoint& pos,psycle::core::Machine* mac);
protected:
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event){GuiMachineElement::mouseMoveEvent(event);}
    void mousePressEvent(QGraphicsSceneMouseEvent *event){GuiMachineElement::mousePressEvent(event);}
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event){GuiMachineElement::mouseReleaseEvent(event);}
};

}
#endif // MIXERGUIELEMENT_H
