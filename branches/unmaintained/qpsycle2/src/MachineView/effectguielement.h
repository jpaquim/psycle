#ifndef EFFECTGUIELEMENT_H
#define EFFECTGUIELEMENT_H

#include "MachineView/guimachineelement.h"

#include "psycle/core/machinefactory.h"
#include "psycle/core/pluginfinder.h"

namespace qpsycle{

class EffectGuiElement:public GuiMachineElement
{
public:
    EffectGuiElement(const QPoint &pos, psycle::core::Machine *mach);
protected:
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event){GuiMachineElement::mouseMoveEvent(event);}
    void mousePressEvent(QGraphicsSceneMouseEvent *event){GuiMachineElement::mousePressEvent(event);}
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event){GuiMachineElement::mouseReleaseEvent(event);}
    bool onControl1(QPointF point);
    bool onControl2(QPointF point);
    bool onPan(QPointF point);
};
}

#endif // EFFECTGUIELEMENT_H
