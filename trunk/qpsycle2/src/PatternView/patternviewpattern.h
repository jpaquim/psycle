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

public slots:
    void mousePressEvent(QGraphicsSceneMouseEvent *event);

private:
    QVector<PatternViewTrack*> tracks;
    psycle::core::Pattern* pattern;
    QFont* font;
    int charHeight;
    int charWidth;
    int columnSpacerSize;
    int trackSpacerSize;

};

}
#endif // PATTERNVIEWPATTERN_H
