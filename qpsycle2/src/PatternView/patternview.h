#ifndef PATTERNVIEW_H
#define PATTERNVIEW_H

#include <QGraphicsScene>
#include <QList>
#include "patternviewpattern.h"
#include "psycle/core/song.h"

namespace qpsycle {


class PatternView : public QGraphicsScene
{
Q_OBJECT

public:
    PatternView(QObject *parent = 0);

public Q_SLOTS:
    void reload();

private:
    QVector<PatternViewPattern*> patternList;
    psycle::core::Song *song;
};

}
#endif // PATTERNVIEW_H
