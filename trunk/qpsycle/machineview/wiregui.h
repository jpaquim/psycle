#ifndef WIREGUI_H
 #define WIREGUI_H

 #include <QGraphicsItem>

 class MachineGui;

 class WireGui : public QGraphicsItem
 {
 public:
     WireGui(MachineGui *sourceMacGui, MachineGui *destMacGui);
     ~WireGui();

     MachineGui *sourceMacGui() const;
     void setSourceMacGui(MachineGui *macGui);

     MachineGui *destMacGui() const;
     void setDestMacGui(MachineGui *macGui);

     void adjust();

     enum { Type = UserType + 2 };
     int type() const { return Type; }

 protected:
     QRectF boundingRect() const;
     void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

 private:
     MachineGui *source, *dest;

     QPointF sourcePoint;
     QPointF destPoint;
     qreal arrowSize;
 };

 #endif

