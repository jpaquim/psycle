#ifndef TRACKHEADER_H
#define TRACKHEADE_H

#include "patterndraw.h"

#include <QGraphicsRectItem>

class TrackHeader: public QGraphicsItem {
public:
    TrackHeader(PatternDraw* pPatternDraw);
    ~TrackHeader();

    void paint( QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0 );
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
