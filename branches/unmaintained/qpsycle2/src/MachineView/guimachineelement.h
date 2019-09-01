#ifndef GUIMACHINEELEMENT_H
#define GUIMACHINEELEMENT_H

#include <QGraphicsItem>
#include <QList>
#include "machineviewline.h"
#include "machineviewguielement.h"
#include "psycle/core/machine.h"


/*
 * A generic machine for the machine view, should not be used directly.
 */
namespace qpsycle{
class GuiMachineElement :public MachineViewGuiElement, public QGraphicsItem
{
public:
    ~GuiMachineElement();
    GuiMachineElement();
    void paint(QPainter *painter,const QStyleOptionGraphicsItem *option, QWidget *widget = 0);
    QRectF boundingRect() const;
    void addInLine(MachineViewLine* line);
    bool canAcceptConnections() const { return machine->acceptsConnections();}
    bool canHaveOutputs() const {return machine->emitsConnections();}
    void addInLineInitial(MachineViewLine* line);
    void addOutLineInitial(MachineViewLine *line);
    psycle::core::Machine* getMachine() const {return machine;}
    void removeInLine(MachineViewLine* l){inlines.removeOne(l);}
    void removeOutLine(MachineViewLine* l){outlines.removeOne(l);}

protected:
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    QGraphicsPixmapItem* background;
    bool canOutput;
    bool canAccept;
    bool movingPan = false;
    QGraphicsSimpleTextItem* text;
    MachineViewLine* activeLine;
    bool moving= false;
    bool lineDrawing = false;
    QList<MachineViewLine*> inlines;
    QList<MachineViewLine*> outlines;
    psycle::core::Machine* machine;
    int machineNumber;

private:
};

}

#endif // GUIMACHINEELEMENT_H
