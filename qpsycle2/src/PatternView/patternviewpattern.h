#ifndef PATTERNVIEWPATTERN_H
#define PATTERNVIEWPATTERN_H

#include <QGraphicsItem>
#include <QList>
#include "patternviewtrack.h"
#include "psycle/core/pattern.h"

namespace qpsycle{

class PatternViewPattern : public QGraphicsItem
{
public:
    PatternViewPattern(psycle::core::Pattern* pattern, QGraphicsItem *parent = 0);
    ~PatternViewPattern();
    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

private:
    QVector<PatternViewTrack*> tracks;
    psycle::core::Pattern* pattern;
};

}
#endif // PATTERNVIEWPATTERN_H
