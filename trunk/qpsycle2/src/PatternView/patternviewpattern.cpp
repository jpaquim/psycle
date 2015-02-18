#include "patternviewpattern.h"

#include <QGraphicsSceneMouseEvent>
#include <QFont>
#include <QFontMetrics>
#include <QBrush>
#include <QPainter>
#include <QList>
#include <QKeyEvent>


#include "psycle/core/pattern.h"
#include "statics.h"
#include "notelabels.h"
#include "psycle/core/song.h"


namespace qpsycle{

using psycle::core::TimeSignature;

PatternViewPattern::PatternViewPattern(psycle::core::Pattern *pattern, QObject *parent):
    QAbstractTableModel(parent)
{

    this->pattern = pattern;
    TimeSignature timesig = pattern->timeSignatures()[0];
    float lines = pattern->beats();
    lines *=timesig.numerator();
    int numTracks = Globals::song()->sequence().numTracks();
    pattern->playPosTimeSignature(0);

//    const int width2Char = 2 * charWidth + 2 * textPadSize;
//    const int width4Char = 4 * charWidth + 2 * textPadSize;
//    for(int i = 0; i < numTracks; i++){
//        int trackStart = trackBounds.last().second + trackSpacerSize;
//        columnBounds.append( { trackStart + columnSpacerSize, trackStart + columnSpacerSize+ width4Char } ); //NoteColumn
//        columnBounds.append( { columnBounds.last().second + columnSpacerSize, columnBounds.last().second + columnSpacerSize+width2Char } ); //machine
//        columnBounds.append( { columnBounds.last().second + columnSpacerSize, columnBounds.last().second + columnSpacerSize+width2Char } ); //instrument
//        columnBounds.append( { columnBounds.last().second + columnSpacerSize, columnBounds.last().second + columnSpacerSize+width2Char } ); //volume
//        columnBounds.append( { columnBounds.last().second + columnSpacerSize, columnBounds.last().second + columnSpacerSize+width2Char } ); //command
//        trackBounds.append( { trackStart, columnBounds.last().second } );
//    }
//    int trackStart = trackBounds.last().second + trackSpacerSize;
//    columnBounds.append( { trackStart, trackStart } );


//    for(int i = 0; i < lines; i++){
//        rowBounds.append( { rowBounds.last().second+rowSpacerSize, rowBounds.last().second+rowSpacerSize+charHeight} );
//    }
//    trackStart = rowBounds.last().second + trackSpacerSize;
//    rowBounds.append( { trackStart, trackStart } );


//    playbackRect = new QGraphicsRectItem(this);
//    playbackRect->setFlag(QGraphicsItem::ItemStacksBehindParent, true);
//    playbackRect->setBrush(Qt::blue);
//    playbackRect->setRect(0, rowBounds[playbackRow].first, columnBounds.last().second, rowBounds[playbackRow].second - rowBounds[playbackRow].first);

//    selectionRect = new QGraphicsRectItem(this);
//    selectionRect->setFlag(QGraphicsItem::ItemStacksBehindParent, true);
//    selectionRect->setBrush(Qt::blue);
//    selectionRect->setOpacity(.4);
}


//void PatternViewPattern::paint(QPainter *painter, const QStyleOptionGraphicsItem */*option*/, QWidget */*widget*/){
//    painter->setBrush(Qt::darkGray);
//    painter->setPen(Qt::gray);
//    for (int i = 0; i<columnBounds.size()-1; i++){
//        painter->drawRect(columnBounds[i].second, 0, columnBounds[i+1].first-columnBounds[i].second, rowBounds.last().second);
//    }
//    for (int i = 0; i<rowBounds.size()-1; i++){
//        painter->drawRect(0, rowBounds[i].second, columnBounds.last().second, rowBounds[i+1].first - rowBounds[i].second);
//    }

//    painter->setPen(Qt::black);
//    painter->setFont( *font );
//    for ( auto eventIter  = pattern->begin(); eventIter!=pattern->end(); eventIter++ ) {
//        int track = eventIter->second.track();
//        int row  = eventIter->first * pattern->timeSignatures()[0].numerator();
//        painter->drawText(columnBounds[track*5 + 1].first, rowBounds[row + 1].second, notes_tab_a220[eventIter->second.note()] );
//        painter->drawText(columnBounds[track*5 + 2].first, rowBounds[row + 1].second, generateHexString(eventIter->second.machine()) );
//        painter->drawText(columnBounds[track*5 + 3].first, rowBounds[row + 1].second, generateHexString(eventIter->second.instrument()) );
//        painter->drawText(columnBounds[track*5 + 4].first, rowBounds[row + 1].second, generateHexString(eventIter->second.volume()) );
//        painter->drawText(columnBounds[track*5 + 5].first, rowBounds[row + 1].second, generateHexString(eventIter->second.command()) );
//    }
//}


QString PatternViewPattern::generateHexString(uint8_t string) const {
    QString str("");
    str+=QString::number(string,16);
    str = str.rightJustified(2, '0' ,true).toUpper();
    return str;
}

int PatternViewPattern::rowCount(const QModelIndex &parent) const
{
    TimeSignature timesig = pattern->timeSignatures()[0];
    float lines = pattern->beats();
    lines *=timesig.numerator();
    return lines;
}

int PatternViewPattern::columnCount(const QModelIndex &parent) const
{
    int numTracks = Globals::song()->sequence().numTracks();
    return numTracks * NColumns;
}

QVariant PatternViewPattern::data(const QModelIndex &index, int role) const
{
    QVariant retVal;
    if ( role != Qt::DisplayRole ) return retVal;
    int track = index.column()/NColumns;
    ColumnType type = static_cast<ColumnType> ( index.column()%NColumns );
    double row = index.row();

    double pos = row/pattern->timeSignatures()[0].numerator();
    const psycle::core::PatternEvent& event = pattern->getPatternEvent(row, track);
    switch( type ){
        case Note:
            retVal = notes_tab_a220[event.note()];
            break;
        case Instrument:
            retVal = generateHexString( event.instrument() );
            break;
        case Machine:
            retVal = generateHexString( event.machine() );
            break;
        case Command:
            retVal = generateHexString( event.command() );
            break;
        case Param:
            retVal = generateHexString( event.parameter() );
            break;
        case Volume:
            retVal = generateHexString( event.volume() );
            break;
        case NColumns:
            std::cerr<<"Invalid column requested."<<std::cerr;
            break;
    }

    return retVal;
}

} // namespace qpsycle
