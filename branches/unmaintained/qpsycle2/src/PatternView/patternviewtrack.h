#ifndef PATTERNVIEWTRACK_H
#define PATTERNVIEWTRACK_H

#include <QGraphicsRectItem>
#include <QList>
#include <QFont>
#include <QPainter>
#include "psycle/core/patternevent.h"

/*!
 * Display class with the ability to mute/solo tracks.
 */
namespace qpsycle{

class PatternViewTrack : public QGraphicsRectItem
{
public:
    PatternViewTrack(QGraphicsItem *parent, int lines);
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

    void addEvent(int line, psycle::core::PatternEvent* event){ items[line] = event; }
    void drawLineText(int line, QPainter *painter);
    void makeSelection(int column, int row);
    void clearSelection();

private:
    QString generateHexString(uint8_t string, int length);

private:
    QVector<psycle::core::PatternEvent*> items;
    int charWidth = 0;
    int charHeight = 0;
    QFont font = QFont("Monospace");
    QGraphicsRectItem* selectionRect;

};

}

#endif // PATTERNVIEWTRACK_H
