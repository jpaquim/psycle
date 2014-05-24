#include "patternview.h"
#include "statics.h"

namespace qpsycle{

PatternView::PatternView(QObject *parent):
    QGraphicsScene(parent)
{
    reload();
    QBrush backgroundBrush(Qt::black);
    setBackgroundBrush(backgroundBrush);
}

void PatternView::reload(){
    foreach(PatternViewPattern* pattern,patternList){
        delete pattern;
    }
    patternList.clear();
    this->clear();
    this->song = Statics::song();
    psycle::core::Sequence::patterns_type::iterator pattern = ++song->sequence().patterns_begin();
    psycle::core::Sequence::patterns_type::iterator patternsEnd = song->sequence().patterns_end();
    for(;pattern!=patternsEnd;pattern++){
        patternList.push_back(new PatternViewPattern(*pattern));
        this->addItem(patternList.last());
    }

    setSceneRect(itemsBoundingRect());
}

}
