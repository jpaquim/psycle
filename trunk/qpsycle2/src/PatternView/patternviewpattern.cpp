#include <QGraphicsSceneMouseEvent>

#include "patternviewpattern.h"
#include "statics.h"

namespace qpsycle{

using psycle::core::TimeSignature;

PatternViewPattern::PatternViewPattern(psycle::core::Pattern *pattern, QGraphicsItem *parent):
    QGraphicsItem(parent)
{
    this->pattern = pattern;
    TimeSignature timesig = pattern->timeSignatures()[0];
    float lines = pattern->beats();
    lines *=timesig.numerator();
    int numTracks = Statics::song()->sequence().numTracks();
    pattern->playPosTimeSignature(0);
    for(int i = 0; i < numTracks; i++){
        tracks.append(new PatternViewTrack(this, lines));
        tracks[i]->setX((i==0)?0:(tracks[i-1]->x()+tracks[i-1]->rect().width()+2));
    }

    for(psycle::core::Pattern::iterator patternItr = pattern->begin() ;patternItr!=pattern->end(); ++patternItr){
        tracks[patternItr->second.track()]->addEvent(patternItr->first*timesig.denominator(), &patternItr->second);
    }

    this->setPos(0,pattern->id()*this->boundingRect().height());

}

PatternViewPattern::~PatternViewPattern(){
    foreach(PatternViewTrack* track, tracks){
        delete track;
    }
}

QRectF PatternViewPattern::boundingRect() const {
    return childrenBoundingRect();
}

void PatternViewPattern::paint(QPainter */*painter*/, const QStyleOptionGraphicsItem */*option*/, QWidget */*widget*/){
}


void PatternViewPattern::mousePressEvent(QGraphicsSceneMouseEvent *event){
    font = new QFont("Monospace");
    font->setStyleHint(QFont::TypeWriter);
    font->setPixelSize(12);

    QFontMetrics fm(*font);
    charWidth = fm.maxWidth();
    charHeight = fm.height();
    int columnSpacerSize = 6;
    delete font;
    if(event->button() == Qt::LeftButton ){
        int track = 0;
        int seenLength = tracks[track]->boundingRect().width()+2;
        while( event->pos().x() > seenLength && track<tracks.size()-1){
            track++;
            seenLength += tracks[track]->boundingRect().width()+2;
        }
        int xPos = event->pos().x() - tracks[track]->x();
        int selectedRow = std::floor(event->pos().y()/charHeight) +1;
        int selectedColumn = 0;
        if ( xPos < charWidth*4+columnSpacerSize/2 ){
            selectedColumn = 0;
        } else if (xPos < charWidth * 6 + columnSpacerSize*1.5){
            selectedColumn = 1;
        } else if (xPos < charWidth * 8 + columnSpacerSize*2.5){
            selectedColumn = 2;
        } else if (xPos < charWidth * 10 + columnSpacerSize*3.5){
            selectedColumn = 3;
        } else if (xPos < charWidth * 14 + columnSpacerSize + 5){
            selectedColumn = 4;
        }
        foreach(PatternViewTrack* i,tracks){
            i->clearSelection();
        }
        tracks[track]->makeSelection(selectedColumn, selectedRow);
    }

}

}
