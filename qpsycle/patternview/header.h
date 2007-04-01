#ifndef HEADER_H
#define HEADER_H

#include "patterndraw.h"

#include <QGraphicsRectItem>

class Header: public QGraphicsRectItem {
public:
    Header(PatternDraw* pPatternDraw);
    ~Header();

    void paint( QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget );
    QRectF boundingRect() const;

   // virtual void onMousePress(int x, int y, int button);

private:

    int skinColWidth_;
    PatternDraw* pDraw;

//    void onSoloLedClick( int track );
 //   void onMuteLedClick( int track );
  //  void onRecLedClick( int track );

};

#endif
