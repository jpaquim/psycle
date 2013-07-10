#include <QGraphicsScene>
#include "patternviewtrack.h"
#include "notelabels.h"

namespace qpsycle{

PatternViewTrack::PatternViewTrack(QGraphicsItem *parent, int lines) :
    QGraphicsItem(parent)
{
    items.resize(lines);
    items.fill(NULL);
    noteColumn.resize(lines);

    for(int i = 0 ; i< noteColumn.size() ; i++){
            noteColumn[i] = new QGraphicsTextItem(" - ", this);
    }
    updateStrings();
}

PatternViewTrack::~PatternViewTrack(){
    Q_FOREACH(QGraphicsTextItem* item, noteColumn){
        delete item;
    }
}

void PatternViewTrack::paint(QPainter */*painter*/, const QStyleOptionGraphicsItem */*option*/, QWidget */*widget*/){
}

void PatternViewTrack::updateString(int line){
    QString str = "";
    psycle::core::PatternEvent* event = items[line];
    QGraphicsTextItem* textItem = noteColumn[line];
    str = (" %1 | %2 | %3 | %4 | %5 ");
    str = str.arg(event?(notes_tab_a220[event->note()]):"-- ", 4, ' ');


    str = str.arg(event?(QString::number(event->machine(), 16)):"00", 2, '0');

    str = str.arg(event?(QString::number(event->instrument(), 16)):"00", 2, '0');

    str = str.arg(event?(QString::number(event->volume(), 16)):"00", 2, '0');

    str = str.arg(event?(QString::number(event->command(), 16)):"00", 2, '0');

    textItem->setPlainText(str);
    QFont font("Monospace");
    font.setStyleHint(QFont::TypeWriter);
    font.setPixelSize(7);
    textItem->setFont(font);
    textItem->setPos(0, line*12);

}

void PatternViewTrack::updateStrings(){

    for(int i = 0 ; i<items.size() ; i++){
        updateString(i);

    }
}

}
