#ifndef GENERATORGUIELEMENT_H
#define GENERATORGUIELEMENT_H

#include "MachineView/guimachineelement.h"
#include <QGraphicsItem>
#include "psycle/core/machinefactory.h"
#include "psycle/core/pluginfinder.h"


/*
 * A generator item for the machine view.
 */
namespace qpsycle{

class GeneratorGuiElement:  public GuiMachineElement
{
public:
    GeneratorGuiElement(const QPoint& pos,psycle::core::Machine* mach);
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
#endif // GENERATORGUIELEMENT_H
