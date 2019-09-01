#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QFontMetrics>
#include <QFont>
#include <QPainter>
#include "patternviewtrack.h"
#include "notelabels.h"

namespace qpsycle{

static const int spacerSize = 6;

PatternViewTrack::PatternViewTrack(QGraphicsItem *parent, int lines) :
    QGraphicsRectItem(parent)
{
    items.resize(lines);
    items.fill(NULL);

    font.setStyleHint(QFont::TypeWriter);
    font.setPixelSize(12);

    QFontMetrics fm(font);
    charWidth = fm.maxWidth();
    charHeight = fm.height();

    QBrush selectionBrush(Qt::blue);
    selectionRect = new QGraphicsRectItem(this);
    selectionRect->setBrush(selectionBrush);

    setRect(x(),y(), charWidth *12 + spacerSize*7 , charHeight*lines + spacerSize*2);
    setZValue(-1);
    setBrush(QBrush(Qt::white));
}


void PatternViewTrack::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget){
    QGraphicsRectItem::paint(painter,option,widget);
    painter->setFont(font);
    for(int i = 0 ; i<items.size() ; i++){
        drawLineText(i,painter);
    }
}

void PatternViewTrack::drawLineText(int line,QPainter* painter){
    const psycle::core::PatternEvent *const event = items[line];
    int linePos =  (line+1) * charHeight + spacerSize;
    int columnPos = spacerSize;
    painter->drawText(columnPos, linePos ,event?(notes_tab_a220[event->note()]):"----");
    columnPos += 4*charWidth+spacerSize;
    painter->drawText(columnPos, linePos ,event?(generateHexString(event->machine(), 2)):"--");
    columnPos += 2*charWidth+spacerSize;
    painter->drawText(columnPos, linePos ,event?(generateHexString(event->instrument(), 2)):"--");
    columnPos += 2*charWidth+spacerSize;
    painter->drawText(columnPos, linePos ,event?(generateHexString(event->volume(), 2)):"--");
    columnPos += 2*charWidth+spacerSize;
    painter->drawText(columnPos, linePos ,event?(generateHexString(event->command(), 2)):"--");



}

QString PatternViewTrack::generateHexString(uint8_t string,int length){
    QString str("");
    str+=QString::number(string,16);
    str = str.rightJustified(length, '0' ,true);
    return str;
}


void PatternViewTrack::makeSelection(int column, int row){
    int selectedRow = row;
    int xPos = 0;
    int xSize = 0;
    if ( column == 0 ){
        xPos = 0;
        xSize = 4 * charWidth + spacerSize*1.5;
    } else if (column == 1){
        xPos = 4 * charWidth + spacerSize/1.5;
        xSize = 2 * charWidth + spacerSize;
    } else if (column == 2){
        xPos = 6 * charWidth + spacerSize*2.5;
        xSize = 2 * charWidth + spacerSize;
    } else if (column == 3){
        xPos = 8 * charWidth + spacerSize*3.5;
        xSize = 2 * charWidth + spacerSize;
    } else if (column == 4){
        xPos = 10 * charWidth + spacerSize*4.5;
        xSize = 2 * charWidth + spacerSize*1.5;
    }
    selectionRect->setRect(xPos ,(selectedRow-1)*charHeight,xSize,charHeight);
    selectionRect->setZValue(1);
    selectionRect->setOpacity(.5);
    selectionRect->show();

}

void PatternViewTrack::clearSelection(){
    selectionRect->hide();
}

}
