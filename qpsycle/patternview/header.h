#ifndef HEADER_H
#define HEADER_H

#include <QGraphicsRectItem>

#include "patternview.h"

class Header: public QGraphicsRectItem {
public:
    Header(PatternView* pPatternView);
    ~Header();

    void paint( QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget );

   // virtual void onMousePress(int x, int y, int button);

private:

    int skinColWidth_;
    PatternView* pView;

//    void onSoloLedClick( int track );
 //   void onMuteLedClick( int track );
  //  void onRecLedClick( int track );

};

#endif
