#include "patternmanager.h"
#include "ui_patternmanager.h"

#include <QStringList>

#include "psycle/core/song.h"
#include "psycle/core/sequence.h"
#include "psycle/core/pattern.h"
#include "statics.h"


#include "statics.h"
namespace qpsycle {

PatternManager::PatternManager(QWidget *parent) :
    QDockWidget(parent),
    ui(new Ui::PatternManager)
{
    setAllowedAreas(Qt::LeftDockWidgetArea|Qt::RightDockWidgetArea);


    QWidget *centralWidget = new QWidget(this);
    this->setWidget(centralWidget);
    ui->setupUi(centralWidget);
    connect(ui->patternList, &QListWidget::currentRowChanged,
            this, &PatternManager::patternSelected);

}

PatternManager::~PatternManager()
{
    delete ui;
}

void PatternManager::selectedPatternChanged(int selectedPattern)
{
    ui->patternList->setCurrentRow( selectedPattern );
}

void PatternManager::patternsChanged()
{
    psycle::core::Song* Song = Globals::song();
    QStringList patternNames;
    //skip pattern 0, the "master" pattern used internally by the sequencer.
    for( auto pat = ++Song->sequence().patterns_begin(), end = Song->sequence().patterns_end(); pat != end; ++pat){
        patternNames.append( QString::fromStdString( (*pat)->name() ) );
    }
    ui->patternList->clear();
    ui->patternList->addItems( patternNames );
}

void PatternManager::newButtonPressed()
{
    psycle::core::Song* song = Globals::song();
    psycle::core::Pattern newPattern ;
    song->sequence().Add( newPattern );
    patternsChanged();
}

void PatternManager::deleteButtonPressed()
{
    int patternNum = ui->patternList->currentRow();
    psycle::core::Song* song = Globals::song();
    song->sequence().removePattern( **(song->sequence().patterns_begin()+patternNum) );
}



}
