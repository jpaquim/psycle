#include <QPainter>

 #include "wiregui.h"
 #include "machinegui.h"

 #include <math.h>

 static const double Pi = 3.14159265358979323846264338327950288419717;
 static double TwoPi = 2.0 * Pi;

 WireGui::WireGui(MachineGui *sourceMacGui, MachineGui *destMacGui)
     : arrowSize(10)
 {
     setAcceptedMouseButtons(0);
     source = sourceMacGui;
     dest = destMacGui;
     source->addWireGui(this);
     dest->addWireGui(this);
     adjust();
 }

 WireGui::~WireGui()
 {
 }

 MachineGui *WireGui::sourceMacGui() const
 {
     return source;
 }

 void WireGui::setSourceMacGui(MachineGui *macGui)
 {
     source = macGui;
     adjust();
 }

 MachineGui *WireGui::destMacGui() const
 {
     return dest;
 }

 void WireGui::setDestMacGui(MachineGui *macGui)
 {
     dest = macGui;
     adjust();
 }

 void WireGui::adjust()
 {
     if (!source || !dest)
         return;

     QLineF line(mapFromItem(source, 0, 0), mapFromItem(dest, 0, 0));
     qreal length = line.length();
     QPointF wireGuiOffset((line.dx() * 10) / length, (line.dy() * 10) / length);

     removeFromIndex();
     sourcePoint = line.p1() + wireGuiOffset;
     destPoint = line.p2() - wireGuiOffset;
     addToIndex();
 }

 QRectF WireGui::boundingRect() const
 {
     if (!source || !dest)
         return QRectF();

     qreal penWidth = 1;
     qreal extra = (penWidth + arrowSize) / 2.0;

     return QRectF(sourcePoint, QSizeF(destPoint.x() - sourcePoint.x(),
                                       destPoint.y() - sourcePoint.y()))
         .normalized()
         .adjusted(-extra, -extra, extra, extra);
 }

 void WireGui::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
 {
     if (!source || !dest)
         return;

     // Draw the line itself
     QLineF line(sourcePoint, destPoint);
     painter->setPen(QPen(Qt::white, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
     painter->drawLine(line);

     // Draw the arrows if there's enough room
     double angle = ::acos(line.dx() / line.length());
     if (line.dy() >= 0)
         angle = TwoPi - angle;

     QPointF sourceArrowP1 = sourcePoint + QPointF(sin(angle + Pi / 3) * arrowSize,
                                                   cos(angle + Pi / 3) * arrowSize);
     QPointF sourceArrowP2 = sourcePoint + QPointF(sin(angle + Pi - Pi / 3) * arrowSize,
                                                   cos(angle + Pi - Pi / 3) * arrowSize);
     QPointF destArrowP1 = destPoint + QPointF(sin(angle - Pi / 3) * arrowSize,
                                               cos(angle - Pi / 3) * arrowSize);
     QPointF destArrowP2 = destPoint + QPointF(sin(angle - Pi + Pi / 3) * arrowSize,
                                               cos(angle - Pi + Pi / 3) * arrowSize);

     painter->setBrush(Qt::black);
     painter->drawPolygon(QPolygonF() << line.p1() << sourceArrowP1 << sourceArrowP2);
     painter->drawPolygon(QPolygonF() << line.p2() << destArrowP1 << destArrowP2);
 }

