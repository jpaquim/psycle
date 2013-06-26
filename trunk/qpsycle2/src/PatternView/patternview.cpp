#include "patternview.h"
#include "statics.h"

namespace qpsycle{

PatternView::PatternView(QObject *parent):
    QGraphicsScene(parent)
{
    reload();
}

void PatternView::reload(){
    this->clear();
    this->song = Statics::song();
    psycle::core::Sequence::patterns_type::iterator pattern = ++song->sequence().patterns_begin();
    psycle::core::Sequence::patterns_type::iterator patternsEnd = song->sequence().patterns_end();
    for(;pattern!=patternsEnd;pattern++){
        this->addItem(new PatternViewPattern(*pattern));
    }
    setSceneRect(itemsBoundingRect());
    setSceneRect(0,0,0,0);
}

}
