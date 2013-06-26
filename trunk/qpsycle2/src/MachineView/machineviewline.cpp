#include "MachineView/machineviewline.h"
#include "MachineView/machineviewwiredialog.h"
#include <QGraphicsLineItem>
#include <QPainter>
#include <QGraphicsScene>
#include <QLine>
#include <qmath.h>

namespace qpsycle{

const double Pi = 3.14159265358979323846264338327950288419717;
const double TwoPi = 2.0 * Pi;

MachineViewLine::MachineViewLine(qreal x1, qreal y1, qreal x2, qreal y2)
{
    QGraphicsLineItem(x1,y1,x2,y2);
    x1_=x1;
    y1_=y1;
    x2_=x2;
    y2_=y2;
    setZValue(-2);

    //set up the triangle
    arrowSize = 20;
    updateTriangle();
}

MachineViewLine::~MachineViewLine(){

}

void MachineViewLine::setSource(GuiMachineElement *src){
    x1_=src->scenePos().x()+src->boundingRect().center().x();
    y1_=src->scenePos().y()+src->boundingRect().center().y();
    setLine(x1_,y1_,x2_,y2_);
    updateTriangle();


    source = src;

}

void MachineViewLine::setDest(GuiMachineElement *dst){
    x2_=dst->scenePos().x()+dst->boundingRect().center().x();
    y2_=dst->scenePos().y()+dst->boundingRect().center().y();
    setLine(x1_,y1_,x2_,y2_);
    updateTriangle();

    dest = dst;


}

QRectF MachineViewLine::boundingRect() const{
   QRectF ret = QGraphicsLineItem::boundingRect();
   ret.adjusted(-arrowSize,-arrowSize,arrowSize,arrowSize);
   return ret;
}


void MachineViewLine::paint(QPainter *painter, const QStyleOptionGraphicsItem */*option*/, QWidget */*widget*/){

    painter->setPen(QPen(QColor(0, 0, 0), 1, Qt::SolidLine,
                         Qt::FlatCap, Qt::MiterJoin));
    painter->setBrush(QColor(122, 163, 39));
    painter->setRenderHint(QPainter::Antialiasing,true);

    painter->drawPath(shape());
    scene()->update();


//    QGraphicsLineItem::paint(painter,option,widget);

//    painter->setBrush(Qt::black);
//    painter->drawPolygon(triangle);

}

void MachineViewLine::mouseDoubleClickEvent(QGraphicsSceneMouseEvent */*event*/){
    MachineViewWireDialog* dialog = new MachineViewWireDialog(this);
    dialog->show();
}

QPainterPath MachineViewLine::shape() const{
    QPainterPath ret;
    ret = QGraphicsLineItem::shape();
    ret.addPolygon(triangle);
    return ret.simplified();

}

void MachineViewLine::updateTriangle(){

    triangle.clear();
    QPointF midPoint = QPointF(x1_,y1_) + QPointF((x2_-x1_)/2 + qCos(line().angle()*Pi/180) * arrowSize/2,
                                                      (y2_-y1_)/2 - qSin(line().angle()*Pi/180) * arrowSize/2);
    triangle.append(midPoint);
    triangle.append(midPoint + QPointF(qSin(line().angle()*Pi/180 - Pi / 3) * arrowSize,
                                       qCos(line().angle()*Pi/180 - Pi / 3) * arrowSize));
    triangle.append(midPoint + QPointF(qSin(line().angle()*Pi/180 - Pi + Pi / 3) * arrowSize,
                                       qCos(line().angle()*Pi/180 - Pi + Pi / 3) * arrowSize));
    triangle.append(midPoint);
}


}
