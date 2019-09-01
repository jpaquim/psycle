#ifndef MACHINEVIEWLINE_H
#define MACHINEVIEWLINE_H

#include <QGraphicsLineItem>
#include <iostream>

#include "MachineView/guimachineelement.h"
#include "MachineView/machineviewguielement.h"

#include "psycle/core/song.h"

/*
 * A line in the machine view.
 */
namespace qpsycle{

class GuiMachineElement;

class MachineViewLine : public QGraphicsLineItem, public MachineViewGuiElement
{
public:
    MachineViewLine(qreal x1, qreal y1, qreal x2, qreal y2);
    ~MachineViewLine();
    void addInLine(MachineViewLine* /*line*/){ std::cerr<<"input added to line.  This shouldn't happen"<<std::endl;}
    bool canAcceptConnections() const { return false;}
    qreal x1() const {return x1_;}
    qreal x2() const {return x2_;}
    qreal y1() const {return y1_;}
    qreal y2() const {return y2_;}
    void setX1(qreal x){this->setLine(x,y1_,x2_,y2_);updateTriangle();x1_=x;}
    void setX2(qreal x){this->setLine(x1_,y1_,x,y2_);updateTriangle();x2_=x;}
    void setY1(qreal y){this->setLine(x1_,y,x2_,y2_);updateTriangle();y1_=y;}
    void setY2(qreal y){this->setLine(x1_,y1_,x2_,y);updateTriangle();y2_=y;}
    void setSource(GuiMachineElement* src);
    void setDest(GuiMachineElement* dst);
    GuiMachineElement* getSource() const {return source;}
    GuiMachineElement* getDest() const {return dest;}
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    QRectF boundingRect() const;
    QPainterPath shape() const;
    void updateTriangle();
    void setWire(psycle::core::Wire::id_type w){wire = w;}
    psycle::core::Wire::id_type getWire(){return wire;}

protected:
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);

private:

    GuiMachineElement* source;
    GuiMachineElement* dest;
    QPolygonF triangle;
    int arrowSize;
    qreal x1_,x2_,y1_,y2_;
    psycle::core::Wire::id_type wire;
};

}
#endif // MACHINEVIEWLINE_H
