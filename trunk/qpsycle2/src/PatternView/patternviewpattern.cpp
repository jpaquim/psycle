#include "patternviewpattern.h"
#include "statics.h"

namespace qpsycle{

using psycle::core::TimeSignature;

PatternViewPattern::PatternViewPattern(psycle::core::Pattern *pattern, QGraphicsItem *parent):
    QGraphicsItem(parent)
{
    this->pattern = pattern;
    this->setFlag(QGraphicsItem::ItemIsMovable, true);
    TimeSignature timesig = pattern->timeSignatures()[0];
    float lines = pattern->beats();
    lines *=timesig.numerator();
    int numTracks = Statics::song()->sequence().numTracks();
    pattern->playPosTimeSignature(0);
    for(int i = 0; i < numTracks; i++){
        tracks.append(new PatternViewTrack(this, lines));
        tracks[i]->setX((i==0)?0:tracks[i-1]->x()+tracks[i-1]->boundingRect().width()+5);
    }

    for(psycle::core::Pattern::iterator patternItr = pattern->begin() ;patternItr!=pattern->end(); ++patternItr){
        tracks[patternItr->second.track()]->addEvent(patternItr->first*timesig.denominator(), &patternItr->second);
    }

    this->setPos(0,pattern->id()*this->boundingRect().height());

}

PatternViewPattern::~PatternViewPattern(){
    Q_FOREACH(PatternViewTrack* track, tracks){
        delete track;
    }
}

QRectF PatternViewPattern::boundingRect() const {
    return childrenBoundingRect();
}

void PatternViewPattern::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget){
}

}
