#ifndef PATTERNVIEWTRACK_H
#define PATTERNVIEWTRACK_H

#include <QGraphicsItem>
#include <QList>
#include "psycle/core/patternevent.h"


/*!
 * Display class with the ability to mute/solo tracks.
 */
namespace qpsycle{

class PatternViewTrack : public QGraphicsItem
{
public:
    PatternViewTrack(QGraphicsItem *parent, int lines);
    ~PatternViewTrack();
    QRectF boundingRect() const {return childrenBoundingRect();}
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

    void addEvent(int line, psycle::core::PatternEvent* event){ items[line] = event;updateString(line); }
    void updateString(int line);

private:
    void updateStrings();

private:
    QVector<psycle::core::PatternEvent*> items;
    QVector<QGraphicsTextItem*> noteColumn;


};

}

#endif // PATTERNVIEWTRACK_H
