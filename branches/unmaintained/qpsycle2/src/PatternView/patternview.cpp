#include "patternview.h"
#include "statics.h"

#include "qheaderview.h"

namespace qpsycle{

PatternView::PatternView(QWidget *parent):
    QTableView(parent)
{
    reload();
    this->setModel( model );
}

void PatternView::reload(){
    this->setShowGrid( false );
    this->horizontalHeader()->setVisible( true );
    this->verticalHeader()->setVisible( true );
    this->song = Globals::song();
    showPattern( 1 );
}

void PatternView::showPattern(int patternNum){
    this->currentPatternNum = patternNum;
    psycle::core::Pattern *pattern = *(song->sequence().patterns_begin()+patternNum);
    this->setModel( nullptr );
    delete model;
    model = new PatternViewPattern( pattern, this);
    this->setModel( model );
    this->resizeColumnsToContents();
}

void PatternView::paintEvent(QPaintEvent *event)
{
    QTableView::paintEvent( event );
}

void PatternView::keyPressEvent(QKeyEvent *event)
{
    QTableView::keyPressEvent( event );
}

}
