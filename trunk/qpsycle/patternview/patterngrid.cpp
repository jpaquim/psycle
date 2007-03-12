#include "patterngrid.h"
#include "psycore/singlepattern.h"
#include "psycore/global.h"
#include "psycore/player.h"

#include <iostream>
#include <iomanip>

template<class T> inline std::string toHex(T value , int nums = 2) {
    std::ostringstream buffer;
    buffer.setf(std::ios::uppercase);

    buffer.str("");
    buffer << std::setfill('0') << std::hex << std::setw( nums );
    buffer << (int) value;
    return buffer.str();
}

int d2i(double d)
{
		return (int) ( d<0?d-.5:d+.5);
}



PatternGrid::PatternGrid( PatternDraw *pDraw )
    : patDraw_( pDraw )
{
    setFlag(ItemIsFocusable);
    setupTrackGeometrics( patDraw_->patternView()->numberOfTracks() );

    addEvent( ColumnEvent::note );
    addEvent( ColumnEvent::hex2 );
    addEvent( ColumnEvent::hex2 );
    addEvent( ColumnEvent::hex2 );
    addEvent( ColumnEvent::hex4 );

    // just for test !
    addEvent( ColumnEvent::hex4 );
    addEvent( ColumnEvent::hex4 );
    addEvent( ColumnEvent::hex4 );
    // end of multi paraCmd
    
    // FIXME: hardcoding these for now.
     textColor_ = QColor( 255, 255, 255 );
     separatorColor_ = QColor( 145, 147, 147 );
     selectionColor_ = QColor( 149, 68, 41 );
     cursorColor_ = QColor( 179, 217, 34 );
     cursorTextColor_ = QColor( 0, 0, 0 );
     barColor_ = QColor( 70, 71, 69 );
     beatColor_ = QColor( 50, 51, 49 );
     beatTextColor_ = QColor( 199, 199, 199 );
     playBarColor_ = QColor( 182, 121, 88 );
     bigTrackSeparatorColor_ = QColor( 145, 147, 147 );
     smallTrackSeparatorColor_ = QColor( 105, 107, 107 );
     lineSepColor_ = QColor( 145, 147, 147 );
     restAreaColor_ = QColor( 24, 22, 25 );
}

void PatternGrid::addEvent( const ColumnEvent & event ) {
    events_.push_back( event );
}


QRectF PatternGrid::boundingRect() const
{
    // FIXME: should come from somewhere else (i.e. that not hard-coded.)
    return QRectF( 0, 0, 500, 500 );
}

void PatternGrid::paint( QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget )
{
    font_ = QFont( "courier", 9 );
    setFont( font_ );
    painter->setFont( font_ ); 
        if ( patDraw_->patternView()->pattern() ) {
            int numberOfTracks = patDraw_->patternView()->numberOfTracks();
            int trackWidth = patDraw_->patternView()->trackWidth();
            int numberOfLines = patDraw_->patternView()->numberOfLines();
            int rowHeight = patDraw_->patternView()->rowHeight();

            int startLine = 0; 
            int endLine = numberOfLines - 1;
            int startTrack = 0;
            int endTrack = numberOfTracks - 1;


            //drawSelBg(g,selection());


            drawGrid( painter, startLine, endLine, startTrack, endTrack );	
            //drawColumnGrid(g, startLine, endLine, startTrack, endTrack);
            drawPattern( painter, startLine, endLine, startTrack, endTrack );
        //drawRestArea(g, startLine, endLine, startTrack, endTrack);*/
        }
}

/** 
 * Get the width of the grid up until and including the given track.
 */
int PatternGrid::gridWidthByTrack( int track ) const 
{
    std::map<int, TrackGeometry>::const_iterator it;
    it = trackGeometrics().lower_bound( track );
    int gridWidth = 0;
    if ( it != trackGeometrics().end() ) {
        TrackGeometry trackGeom = it->second;
        gridWidth = trackGeom.left() + trackGeom.width();
    }
    return gridWidth;
}

/**
 * Draw the grid outline.
 */
void PatternGrid::drawGrid( QPainter *painter, int startLine, int endLine, int startTrack, int endTrack  ) 
{
    psy::core::TimeSignature signature;

    std::map<int, TrackGeometry>::const_iterator it;
    it = trackGeometrics().lower_bound( endTrack );
    int gridWidth = 0;
    if ( it != trackGeometrics().end() ) {
        gridWidth = it->second.left() + it->second.width();
    }

    int gridHeight = ( (endLine+1) * lineHeight() );

    // Draw horizontal lines to demarcate the pattern lines.
    if ( lineGridEnabled() ) {
        painter->setPen( separatorColor() );
        for (int y = startLine; y <= endLine; y++)
            painter->drawLine( 0, y * lineHeight(), gridWidth, y* lineHeight() );
    }
    for (int y = startLine; y <= endLine; y++) {
        float position = y / (float) beatZoom();
        if (!(true/*y == patDraw_->pView->playPos()*/) || !psy::core::Player::Instance()->playing() ) {
            if ( !(y % beatZoom())) {
                if ((patDraw_->patternView()->pattern()->barStart(position, signature) )) {

                    painter->setBrush( barColor() );//g.setForeground( barColor() );
                    painter->drawRect( 0, y*lineHeight() /*- dy()*/, gridWidth, lineHeight());

/*                    if ( y >= selection().top() && y < selection().bottom()) {
                        int left  = xOffByTrack( selection().left() );
                        int right = xOffByTrack( selection().right() );
                        g.setForeground( pView->colorInfo().sel_bar_bg_color );
                        g.fillRect( left - dx(), y*rowHeight() - dy(), right - left, rowHeight());
                    }*/

                } else {
                    painter->setBrush( beatColor() );//g.setForeground( beatColor() );
                    painter->drawRect( 0, y* lineHeight() /*- dy()*/, gridWidth, lineHeight() );

                    /*if ( y >= selection().top() && y < selection().bottom()) {
                        int left  = xOffByTrack( selection().left() );
                        int right = xOffByTrack( selection().right() );
                        g.setForeground( pView->colorInfo().sel_beat_bg_color );
                        g.fillRect( left - dx(), y*rowHeight() - dy(), right - left, rowHeight());
                    }*/
                }
            }
        }
    }


    // Draw the vertical track separators.
    painter->setPen( separatorColor() );
    painter->setBrush( separatorColor() );
    it = trackGeometrics().lower_bound( startTrack );
    for ( ; it != trackGeometrics().end() && it->first <= endTrack; it++) { //  oolIdent px space at begin of trackCol{
        TrackGeometry trackGeom = it->second;
        painter->drawRect( trackGeom.left(), 0, 5, gridHeight );
    }

/*    painter->setBrush( Qt::red );
    it = trackGeometrics().lower_bound( startTrack );
    for ( ; it != trackGeometrics().end() && it->first <= endTrack; it++) {
        // now refill the left and right ident areas
//        g.fillRect( it->second.left() - dx() + colIdent,0,trackLeftIdent(),lineHeight);
        // the right
        g.fillRect( it->second.left() + std::max( it->second.width(), trackMinWidth_ ) - trackRightIdent() - dx(),0,trackRightIdent(),lineHeight);
    }

    g.setForeground( smallTrackSeparatorColor() );
    it = trackGeometrics().lower_bound( startTrack );
    for ( ; it != trackGeometrics().end() && it->first <= endTrack; it++) // track small separators
        g.drawLine( it->second.left()-dx(),0, it->second.left()-dx(),lineHeight);

    g.setForeground( foreground( ) );
*/
}

void PatternGrid::drawPattern( QPainter *painter, int startLine, int endLine, int startTrack, int endTrack )
{
    // do we have a pattern ?
    if ( patDraw_->patternView()->pattern() ) {
        drawCellBg( painter, cursor()  );
        // find start iterator
        psy::core::SinglePattern::iterator it = patDraw_->patternView()->pattern()->find_lower_nearest(startLine);
        psy::core::TimeSignature signature;


        int lastLinenum = -1;
        psy::core::PatternLine* line;
        psy::core::PatternLine emptyLine;

        for ( int curLinenum = startLine; curLinenum <= endLine; curLinenum++ ) {

            if ( it != patDraw_->patternView()->pattern()->end() )	{
                int liney = d2i (it->first * beatZoom() );
                if (liney == curLinenum ) {
                    line = &it->second;
                    it++;
                } else line = &emptyLine;
            } else line = &emptyLine;


            if (curLinenum != lastLinenum) {
                QColor tColor = textColor();

                bool onBeat = false;
                bool onBar  = false;
                if ( !(curLinenum % beatZoom())) {
                    if (  it != patDraw_->patternView()->pattern()->end() && patDraw_->patternView()->pattern()->barStart(it->first, signature) ) {
                        tColor = QColor( barColor() );
                    } else {
                        onBeat = true;
                        tColor = QColor( beatTextColor() );
                    }
                }

                // Check if this line is currently being played.
/*                if ((curLinenum == patView_->playPos() && Player::Instance()->playing() ) ) {
                    int trackWidth = xEndByTrack( endTrack ) - dx();
                    g.setForeground( playBarColor() );
                    g.fillRect(0, y*rowHeight() - dy(), trackWidth, rowHeight());
                }*/

                QColor stdColor = tColor;
                QColor crColor = cursorTextColor();

                std::map<int, psy::core::PatternEvent>::iterator eventIt = line->notes().lower_bound(startTrack);
                psy::core::PatternEvent emptyEvent;
                psy::core::PatternEvent* event;

                for ( int curTracknum = startTrack; curTracknum <= endTrack; curTracknum++ ) 
                {
                    if ( eventIt != line->notes().end() && eventIt->first <= endTrack ) {
                        int trackx = eventIt->first;
                        if ( curTracknum == trackx ) {
                            event = &eventIt->second;
                            eventIt++;
                        } else {
                            event = &emptyEvent;
                        }
                    } else {
                        event = &emptyEvent;
                    }


/*                    if ( x >= selection().left() && x < selection().right() &&
                        curLinenum >= selection().top() && currentLine < selection().bottom() ) {
                            if ( !onBeat ) 
                                tColor = patView_->colorInfo().sel_text_color;
                            else 
                                tColor = patView_->colorInfo().sel_beat_text_color;
                    }	else tColor = stdColor;*/
                    painter->setPen( tColor );
                    drawData( painter, curTracknum, curLinenum, 0, event->note() ,event->isSharp(), tColor );
                    if (event->instrument() != 255) drawData( painter, curTracknum, curLinenum, 1, event->instrument(), 1, tColor );
                    if (event->machine() != 255) drawData( painter, curTracknum, curLinenum, 2, event->machine(), 1, tColor );
                    if (event->volume() != 255) drawData( painter, curTracknum, curLinenum, 3, event->volume(), 1, tColor );
                    if (event->command() != 0 || event->parameter() != 0) {
                        drawData( painter, curTracknum, curLinenum, 4, (event->command() << 8) | event->parameter(), 1, tColor );
                    }	else {
                        drawString( painter, curTracknum, curLinenum, 4, "....", tColor );
                    }									

                    psy::core::PatternEvent::PcmListType & pcList = event->paraCmdList();
                    psy::core::PatternEvent::PcmListType::iterator it = pcList.begin();
                    int count = 0;
                    for ( ; it < pcList.end(); it++, count++ ) {
                        psy::core::PatternEvent::PcmType & pc = *it;
                        int command = pc.first;
                        int parameter = pc.second;
                        if ( command != 0 || parameter != 0) {
                            drawData( painter, curTracknum, curLinenum, 5+count, ( command << 8) | parameter , 1, tColor );
                        }
                    }
                }
                lastLinenum = curLinenum;
            }
        }
    }
} // drawPattern

void PatternGrid::drawData( QPainter *painter, int track, int line, int eventnr, int data, bool sharp, const QColor & color ) {

    std::map<int, TrackGeometry>::const_iterator it;
    it = trackGeometrics().lower_bound( track );
    if ( it == trackGeometrics().end() || eventnr >= it->second.visibleColumns()  ) return;

    int xOff = it->second.left() + 5 + 5;/*trackLeftIdent() - dx()*/;			// 5= colIdent

    if ( eventnr < events_.size() ) {
        const ColumnEvent & event = events_.at( eventnr );
        switch ( event.type() ) {
                                case ColumnEvent::hex2 :
                                    drawBlockData( painter, xOff + eventOffset(eventnr,0), line, toHex(data,2), color );
                                    // check if cursor is on event and draw digit in cursortextColor
                                    if ( cursor().track() == track && cursor().line() == line && 
                                        cursor().eventNr() == eventnr && cursor().col() < 2 ) {
                                            drawBlockData( painter, xOff + eventOffset(eventnr,0) + cursor().col()*cellWidth() , line, toHex(data,2).substr(cursor().col(),1) ,cursorTextColor() );
                                    }
                                    break;
                                case ColumnEvent::hex4 :
                                    drawBlockData( painter, xOff + eventOffset(eventnr,0), line, toHex(data,4), color );
                                    // check if cursor is on event and draw digit in cursortextColor
                                    if ( cursor().track() == track && cursor().line() == line && 
                                        cursor().eventNr() == eventnr && cursor().col() < 4 ) {
                                            drawBlockData( painter, xOff + eventOffset(eventnr,0) + cursor().col()*cellWidth(), line, toHex(data,4).substr(cursor().col(),1) ,cursorTextColor() );
                                    }

                                    break;
                                case ColumnEvent::note :					
                                    if ( cursor().track() == track && cursor().line() == line && 
                                        cursor().eventNr() == eventnr ) {
                                            drawStringData( painter, xOff + eventOffset(eventnr,0), line, noteToString(data, sharp),cursorTextColor() );
                                    } else
                                        drawStringData( painter, xOff + eventOffset(eventnr,0), line, noteToString(data, sharp),color );
                                    break;
                                default: ;
        }
    }
} // drawData


void PatternGrid::drawBlockData( QPainter *painter, int xOff, int line, const std::string & text, const QColor & color)
{					
//    int yp = ( rowHeight() - g.textHeight()) / 2  + g.textAscent();
//    int yOff = line  * rowHeight() + yp  - dy();
    int yOff = (line+1)  * lineHeight();
    int col = 0;
    for (int i = 0; i < text.length(); i++) {
        painter->drawText(xOff + col,yOff, QString::fromStdString( text.substr(i,1) ) );
        col += cellWidth();
    }
}

int PatternGrid::cellWidth( ) const 
{
    QFontMetrics metrics( font() );
    return metrics.maxWidth();
}

int PatternGrid::eventOffset( int eventnr, int col ) const 
{
    std::vector<ColumnEvent>::const_iterator it = events_.begin();
    int nr = 0;
    int offset = 0;
    for ( ; it < events_.end(); it++, nr++ ) {
        const ColumnEvent & event = *it;
        if (nr == eventnr) {
            int colOff = col * cellWidth();
            return offset + colOff;
        }
        switch ( event.type() ) {
                                case ColumnEvent::hex2 : offset+= 2*cellWidth(); 	break;
                                case ColumnEvent::hex4 : offset+= 4*cellWidth(); 	break;
                                case ColumnEvent::note : offset+= noteCellWidth(); break;
                                default: ;
        }
    }
    return -1;
}

int PatternGrid::eventColWidth( int eventnr ) const 
{
    int eventColWidth_ = 0;
    if ( eventnr < events_.size() ) {
        const ColumnEvent & event = events_.at(eventnr);

        switch ( event.type() ) {
                                case ColumnEvent::hex2 : eventColWidth_= cellWidth(); 	break;
                                case ColumnEvent::hex4 : eventColWidth_= cellWidth(); 	break;
                                case ColumnEvent::note : eventColWidth_= noteCellWidth(); break;
                                default: ;
        }
    }
    return eventColWidth_;
}


int PatternGrid::noteCellWidth( ) const {            	
    return cellWidth() * 3;
}

void PatternGrid::drawStringData( QPainter *painter, int xOff, int line, const std::string & text, const QColor & color )
{
//    int yp = ( rowHeight() - g.textHeight()) / 2  + g.textAscent();
//    int yOff = line  * rowHeight() + yp  - dy();
    int yOff = (line+1)  * lineHeight();

    painter->drawText(xOff,yOff, QString::fromStdString( text ) );
}

void PatternGrid::drawString( QPainter *painter, int track, int line, int eventnr, const std::string & data , const QColor & color ) 
{
    std::map<int, TrackGeometry>::const_iterator it;
    it = trackGeometrics().lower_bound( track );
    if ( it == trackGeometrics().end() || eventnr >= it->second.visibleColumns()  ) return;

// ORIG    int xOff = it->second.left() + colIdent + trackLeftIdent() - dx();
    int xOff = it->second.left() + 5 + 5 + 0;

    drawStringData( painter, xOff + eventOffset(eventnr,0), line, data, color );
}

std::string PatternGrid::noteToString( int value, bool sharp )
{
    switch (value) {
        case psy::core::cdefTweakM : return "twk"; break;
        case psy::core::cdefTweakE : return "twf"; break;
        case psy::core::cdefMIDICC : return "mcm"; break;
        case psy::core::cdefTweakS : return "tws"; break;
        case 120        : return "off"; break;
        case 255        : return "---"; break;  // defaultNoteStr_; break;
    }

    int octave = value / 12;
    std::string octStr = QString::number( octave ).toStdString();

    if (sharp) switch (value % 12) {
        case 0:   return "C-" + octStr; break;
        case 1:   return "C#" + octStr; break;
        case 2:   return "D-" + octStr; break;
        case 3:   return "D#" + octStr; break;
        case 4:   return "E-" + octStr; break;
        case 5:   return "F-" + octStr; break;
        case 6:   return "F#" + octStr; break;
        case 7:   return "G-" + octStr; break;
        case 8:   return "G#" + octStr; break;
        case 9:   return "A-" + octStr; break;
        case 10:  return "A#" + octStr; break;
        case 11:  return "B-" + octStr; break;
    } else
    switch (value % 12) {
        case 0:   return "C-" + octStr; break;
        case 1:   return "Db" + octStr; break;
        case 2:   return "D-" + octStr; break;
        case 3:   return "Eb" + octStr; break;
        case 4:   return "E-" + octStr; break;
        case 5:   return "F-" + octStr; break;
        case 6:   return "Gb" + octStr; break;
        case 7:   return "G-" + octStr; break;
        case 8:   return "Ab" + octStr; break;
        case 9:   return "A-" + octStr; break;
        case 10:  return "Bb" + octStr; break;
        case 11:  return "B-" + octStr; break;
    }
    return "err";
}




void PatternGrid::setupTrackGeometrics( int numberOfTracks ) 
{
    for ( int newTrack = 0; newTrack < numberOfTracks; newTrack++ ) {
        TrackGeometry trackGeometry( *this );
        trackGeometry.setVisibleColumns( 6 );
        trackGeometryMap[ newTrack ] = trackGeometry;
    }

    std::map<int, TrackGeometry>::iterator it;
    it = trackGeometryMap.lower_bound( numberOfTracks );
    while ( it != trackGeometryMap.end() ) {
        trackGeometryMap.erase( it++ );
    }			

    alignTracks();
}
void PatternGrid::alignTracks() 
{
    std::map<int, TrackGeometry>::iterator it = trackGeometryMap.begin();
    int offset = 0;
    for ( ; it != trackGeometryMap.end(); it++ ) {
        TrackGeometry & geometry = it->second;
        geometry.setLeft( offset );
        offset+= std::max( 10, geometry.width() );		// 50 is track min width
    }
}


const std::map<int, TrackGeometry> & PatternGrid::trackGeometrics() const {
    return trackGeometryMap;
}

int PatternGrid::trackWidth() const
{
    return patDraw_->patternView()->trackWidth();
}

int PatternGrid::lineHeight() const
{
    return patDraw_->patternView()->rowHeight();
}

bool PatternGrid::lineGridEnabled() const
{
    return false;
}

void PatternGrid::drawCellBg( QPainter *painter, const PatCursor& cursor ) 
{
    std::map<int, TrackGeometry>::const_iterator it;
    it = trackGeometrics().lower_bound( cursor.track() );
    if ( it == trackGeometrics().end() ) return;

    //int xOff = it->second.left() + colIdent + trackLeftIdent() - dx();
    int xOff = it->second.left() + 5 + 5 - 0;
    //int yOff = cursor.line()  * lineHeight()  - dy();
    int yOff = cursor.line()  * lineHeight()  - 0;
    int colOffset = eventOffset( cursor.eventNr(), cursor.col() );
    painter->setBrush( cursorColor() );
    painter->setPen( cursorTextColor() );
    painter->drawRect( xOff + colOffset, yOff, eventColWidth( cursor.eventNr() ), lineHeight() );
}

const PatCursor & PatternGrid::cursor() const {
    return cursor_;
}

void PatternGrid::keyPressEvent( QKeyEvent *event )
{
    int command = psy::core::Global::pConfig()->inputHandler().getEnumCodeByKey( psy::core::Key( event->modifiers() , event->key() ) );
    int keyChar = QChar( event->text().at(0) ).toAscii();

    if ( cursor().eventNr() == 0 && isNote( command ) ) {
        // A note event.
        std::cout << "event #0 - note event" << std::endl;
        int note = command; // the cdefs for the keys correspond to the correct notes.
        if ( note == psy::core::cdefKeyStop ) {
            //pView->noteOffAny( cursor() );
        } else
            if (note >=0 && note < 120) {
    //            pView->undoManager().addUndo( cursor() );
                patDraw_->patternView()->enterNote( cursor(), note ); // FIXME: better to emit a signal here?
                moveCursor(0,1);
     //           pView->checkDownScroll( cursor() );
            }
    } else
        if ( isHex( keyChar ) ) {
            if ( cursor().eventNr() == 1 ) {
                // inst select
                // i.e. a keyChar is pressed in the instrument column.
                std::cout << "eventnr 1 - inst select" << std::endl;

                // Add the new data...
                psy::core::PatternEvent patEvent = pattern()->event( cursor().line(), cursor().track() );
                unsigned char newByte = convertDigit( 0xFF, keyChar, patEvent.instrument(), cursor().col() );
                patEvent.setInstrument( newByte );
                pattern()->setEvent( cursor().line(), cursor().track(), patEvent );
                // ...and move the cursor.
                if (cursor().col() == 0) {
                    moveCursor(1,0);			
                } else {
                    moveCursor(-1,1);
//                    pView->checkDownScroll( cursor() );
                }
            } else 
                if ( cursor().eventNr() == 2) {
                    // mac select
                    // i.e. a keyChar is pressed in the machine column.
                    std::cout << "event nr 2 - mach select" << std::endl;
                    psy::core::PatternEvent patEvent = pattern()->event( cursor().line(), cursor().track() );
                    unsigned char newByte = convertDigit( 0xFF, keyChar, patEvent.machine(), cursor().col() );
                    patEvent.setMachine( newByte );
                    pattern()->setEvent( cursor().line(), cursor().track(), patEvent );
                    if (cursor().col() == 0) {
                        moveCursor(1,0);			
                    } else {
                        moveCursor(-1,1);
  //                      pView->checkDownScroll( cursor() );
                    }
                } else
                    if ( cursor().eventNr() == 3) {
                        // mac select
                        std::cout << "event nr 3 - volume column" << std::endl;
                        psy::core::PatternEvent patEvent = pattern()->event( cursor().line(), cursor().track() );
                        unsigned char newByte = convertDigit( 0xFF, keyChar, patEvent.volume(), cursor().col() );
                        patEvent.setVolume( newByte );
                        pattern()->setEvent( cursor().line(), cursor().track(), patEvent );
                        if (cursor().col() == 0) {
                            moveCursor(1,0);			
                        } else {
                            moveCursor(-1,1);
                        }
 //                       pView->checkDownScroll( cursor() );
                    } else
                        if ( cursor().eventNr() >= 4) {
                            // comand or parameter
                            std::cout << "event nr >=4 - command or parameter" << std::endl;
                            psy::core::PatternEvent patEvent = pattern()->event( cursor().line(), cursor().track() );
                            if (cursor().col() < 2 ) {
                                int cmdValue;
                                if (cursor().eventNr() == 4) {
                                    cmdValue = patEvent.command();
                                } else {
                                    psy::core::PatternEvent::PcmType & pc = patEvent.paraCmdList()[cursor().eventNr() - 5];
                                    cmdValue = pc.first;
                                }
                                unsigned char newByte = convertDigit( 0x00, keyChar, cmdValue, cursor().col() );
                                if (cursor().eventNr() == 4) {
                                    patEvent.setCommand( newByte );
                                } else {
                                    psy::core::PatternEvent::PcmType & pc = patEvent.paraCmdList()[cursor().eventNr() - 5];
                                    pc.first = newByte;					
                                }
                                pattern()->setEvent( cursor().line(), cursor().track(), patEvent );
                                moveCursor(1,0);
                            }
                            else {
                                int paraValue;
                                if (cursor().eventNr() == 4) {
                                    paraValue = patEvent.parameter();
                                } else {
                                    psy::core::PatternEvent::PcmType & pc = patEvent.paraCmdList()[cursor().eventNr() - 5];
                                    paraValue = pc.second;
                                }
                                unsigned char newByte = convertDigit( 0x00, keyChar, paraValue, cursor().col() - 2 );
                                if (cursor().eventNr() == 4) {
                                    patEvent.setParameter( newByte );
                                } else {
                                    psy::core::PatternEvent::PcmType & pc = patEvent.paraCmdList()[cursor().eventNr() - 5];
                                    pc.second = newByte;					
                                }
                                pattern()->setEvent( cursor().line(), cursor().track(), patEvent );
                                if (cursor().col() < 3) {
                                    moveCursor(1,0);			
                                } else {
                                    moveCursor(-3,1);
//                                    pView->checkDownScroll( cursor() );
                                }
                            }			
                        }
        }

    switch ( command ) {
        case psy::core::cdefNavUp:
            moveCursor( 0, -1 );/*-patternStep()*/ ;
            return;
        break;
        case psy::core::cdefNavDown:
            moveCursor( 0, 1 );/*-patternStep()*/ ;
            return;
        break;
        case psy::core::cdefNavLeft:
            moveCursor( -1, 0 );/*-patternStep()*/ ;
            return;
        break;
        case psy::core::cdefNavRight:
            moveCursor( 1, 0 );/*-patternStep()*/ ;
            return;
        break;
        default:
            // If we got here, we didn't do anything with it, so pass to parent.
            event->ignore();
    }

}

bool PatternGrid::isNote( int key )
{
    // FIXME: this will be different one the input handler is set up.
    if ( 
         key == psy::core::cdefKeyC_0 ||
         key == psy::core::cdefKeyCS0 ||
         key == psy::core::cdefKeyD_0 ||
         key == psy::core::cdefKeyDS0 ||
         key == psy::core::cdefKeyE_0 ||
         key == psy::core::cdefKeyF_0 ||
         key == psy::core::cdefKeyFS0 ||
         key == psy::core::cdefKeyG_0 ||
         key == psy::core::cdefKeyGS0 ||
         key == psy::core::cdefKeyA_0 ||
         key == psy::core::cdefKeyAS0 ||
         key == psy::core::cdefKeyB_0 ||
         key == psy::core::cdefKeyC_1 ||
         key == psy::core::cdefKeyCS1 ||
         key == psy::core::cdefKeyD_1 ||
         key == psy::core::cdefKeyDS1 ||
         key == psy::core::cdefKeyE_1 ||
         key == psy::core::cdefKeyF_1 ||
         key == psy::core::cdefKeyFS1 ||
         key == psy::core::cdefKeyG_1 ||
         key == psy::core::cdefKeyGS1 ||
         key == psy::core::cdefKeyA_1 ||
         key == psy::core::cdefKeyAS1 ||
         key == psy::core::cdefKeyB_1 ||
         key == psy::core::cdefKeyC_2 ||
         key == psy::core::cdefKeyCS2 ||
         key == psy::core::cdefKeyD_2 ||
         key == psy::core::cdefKeyDS2 ||
         key == psy::core::cdefKeyE_2 
    )
    {
        return true;    
    }

    return false;
}

void PatternGrid::moveCursor( int dx, int dy) {
    // dx -1 left hex digit move
    // dx +1 rigth hex digit move
    // dy in lines
    PatCursor oldCursor = cursor_;
    int eventnr = cursor().eventNr();
    if ( dx > 0 ) {			
        if ( eventnr < events_.size() ) {
            const ColumnEvent & event = events_.at( eventnr );
            int maxCols = event.cols();
            if ( cursor_.col() + dx < maxCols ) {
                cursor_.setCol( cursor_.col() + dx);
            } else
                if (eventnr + 1 < visibleEvents( cursor_.track()) ) {
                    cursor_.setCol( 0 );
                    cursor_.setEventNr( eventnr + 1);
                } else 
                    if (cursor_.track()+1 < patDraw_->patternView()->numberOfTracks() ) {
                        cursor_.setTrack( cursor_.track() + 1 );
                        cursor_.setEventNr(0);
                        cursor_.setCol(0);
                    }
               //     window()->repaint(this,repaintTrackArea( cursor_.line(), cursor_.line(), oldCursor.track(), cursor_.track()) );
        }
    } else 
        if ( dx < 0 ) {
            if ( cursor_.col() + dx >= 0 ) {
                cursor_.setCol( cursor_.col() + dx);
            } else 
                if ( cursor_.eventNr() > 0 ) {
                    cursor_.setEventNr( cursor_.eventNr() - 1 );
                    const ColumnEvent & event = events_.at( cursor_.eventNr() );
                    cursor_.setCol( event.cols() - 1 );					
                } else {
                    if ( cursor_.track() > 0 ) {
                        cursor_.setTrack( cursor_.track() -1 );
                        cursor_.setEventNr( visibleEvents( cursor_.track() -1 )-1 );
                        const ColumnEvent & event = events_.at( cursor_.eventNr() );
                        cursor_.setCol( event.cols() - 1 );
                    }		
                }
                //window()->repaint(this,repaintTrackArea( cursor_.line(), cursor_.line(), cursor_.track(), oldCursor.track()) );
        }

        if ( dy != 0 && (dy + cursor_.line() >= 0) ) {
            cursor_.setLine( std::min(cursor_.line() + dy, patDraw_->patternView()->numberOfLines()-1));
//            window()->repaint(this,repaintTrackArea( oldCursor.line(), oldCursor.line(), oldCursor.track(), oldCursor.track()) );
//            window()->repaint(this,repaintTrackArea( cursor_.line(), cursor_.line(), cursor_.track(), cursor_.track()) );
        } else if (dy!=0) {
//            window()->repaint(this,repaintTrackArea( cursor_.line(), cursor_.line(), cursor_.track(), cursor_.track()) );
        }
        update(); // FIXME: inefficient to update the whole grid here.
//        updateStatusBar();
}

int PatternGrid::visibleEvents( int track ) const 
{
    std::map<int, TrackGeometry>::const_iterator it;
    it = trackGeometrics().lower_bound( track );

    if ( it != trackGeometrics().end() )
        return it->second.visibleColumns();
    return 0;
}

const QFont & PatternGrid::font() const { return font_; }
void PatternGrid::setFont( QFont font ) { font_ = font; };

int PatternGrid::beatZoom() const {
    return patDraw_->patternView()->beatZoom();
}

void PatternGrid::setSeparatorColor( const QColor & color ) {
    separatorColor_ = color;
}

const QColor & PatternGrid::separatorColor() const {
    return separatorColor_;
}

void PatternGrid::setSelectionColor( const QColor & selColor ) {
    selectionColor_ = selColor;
}

const QColor & PatternGrid::selectionColor() const {
    return selectionColor_;
}

void PatternGrid::setCursorColor( const QColor & cursorColor )
{
    cursorColor_ = cursorColor;
}

const QColor & PatternGrid::cursorTextColor() const {
    return cursorTextColor_;
}

void PatternGrid::setCursorTextColor( const QColor & cursorTextColor )
{
    cursorTextColor_ = cursorTextColor;
}

void PatternGrid::setRestAreaColor( const QColor & color ) {
    restAreaColor_ = color;
}

const QColor & PatternGrid::restArea() const {
    return restAreaColor_;
}

const QColor & PatternGrid::cursorColor() const {
    return cursorColor_;
}

void PatternGrid::setBarColor( const QColor & barColor ) {
    barColor_ = barColor;
}

const QColor & PatternGrid::barColor() const {
    return barColor_;
}

void PatternGrid::setBeatColor( const QColor & beatColor ) {
    beatColor_ = beatColor;
}

const QColor & PatternGrid::beatColor() const {
    return beatColor_;
}

void PatternGrid::setPlayBarColor( const QColor & playBarColor ) {
    playBarColor_ = playBarColor;
}

const QColor & PatternGrid::playBarColor() const {
    return playBarColor_;
}

void PatternGrid::setBigTrackSeparatorColor( const QColor & color ) {
    bigTrackSeparatorColor_ = color;
}

const QColor & PatternGrid::bigTrackSeparatorColor() const {
    return bigTrackSeparatorColor_;
}

void PatternGrid::setSmallTrackSeparatorColor( const QColor & color ) {
    smallTrackSeparatorColor_ = color;
}

const QColor & PatternGrid::smallTrackSeparatorColor() const {
    return smallTrackSeparatorColor_;
}

void PatternGrid::setLineSeparatorColor( const QColor & color ) {
    lineSepColor_ = color;
}

const QColor & PatternGrid::lineSeparatorColor() const {
    return lineSepColor_;
}

void PatternGrid::setTextColor( const QColor & color ) {
    textColor_ = color;
}

const QColor & PatternGrid::textColor() const {
    return textColor_;
}

void PatternGrid::setBeatTextColor( const QColor & color ) {
    beatTextColor_ = color;
}

const QColor & PatternGrid::beatTextColor() {
    return beatTextColor_;
}

bool PatternGrid::isHex( int c ) {
    if ( ( (c >= 'a') && (c <='f') ) || ( (c>='0' && c <='9') ) ) return true;
    return false;
}

char hex_value(char c) { 
    if(c >= 'A') return 10 + c - 'A'; else return c - '0'; 
}

unsigned char PatternGrid::convertDigit( int defaultValue, int scanCode, unsigned char oldByte, int col ) const 
{
    unsigned char newByte = 0;
    if (col == 0) {
        if (oldByte == defaultValue)
            newByte = ( 0 & 0x0F ) | ( 0xF0 & (hex_value(scanCode) << 4 ) );
        else
            newByte = ( oldByte & 0x0F ) | ( 0xF0 & ( hex_value(scanCode) << 4) );
    }
    else {
        if (oldByte == defaultValue)
            newByte = ( 0 & 0xF0 ) | ( 0x0F & (hex_value(scanCode)) );
        else
            newByte = ( oldByte & 0xF0 ) | ( 0x0F & (hex_value(scanCode) ) );
    }

    return newByte;
}

psy::core::SinglePattern* PatternGrid::pattern() {
    return patDraw_->patternView()->pattern();
}







//
//
// TrackGeometry
//

TrackGeometry::TrackGeometry( ) :
pGrid( 0 ),
    left_(0),
    width_(0),
    visibleColumns_(0),
    visible_(1)
{ }

TrackGeometry::TrackGeometry( PatternGrid & patternGrid ) :
pGrid( &patternGrid ),
    left_(0),
    width_(0),
    visibleColumns_(0),
    visible_(1)
{ }

TrackGeometry::~TrackGeometry() { }

void TrackGeometry::setLeft( int left ) {
    left_ = left;
}

int TrackGeometry::left() const {
    return left_;
}

int TrackGeometry::width() const {
    //return pView->visibleColWidth( visibleColumns() );
    return pGrid->trackWidth();
}

void TrackGeometry::setVisibleColumns( int cols ) {
    visibleColumns_= cols;
}

int TrackGeometry::visibleColumns() const {
    return visibleColumns_;
}

void TrackGeometry::setVisible( bool on) {
    visible_ = on;
}

bool TrackGeometry::visible() const {
    return visible_;
}

ColumnEvent::ColumnEvent( ColumnEvent::ColType type ) {
    type_ = type;
}

ColumnEvent::~ColumnEvent() {
}

ColumnEvent::ColType ColumnEvent::type() const {
    return type_;
}

int ColumnEvent::cols() const {
    int cols_ = 1;
    switch ( type_ ) {
                        case ColumnEvent::hex2 : 
                            cols_ = 2; 
                            break;
                        case ColumnEvent::hex4 : 
                            cols_ = 4; 
                            break;
                        case ColumnEvent::note : 
                            cols_ = 1; 
                            break;
                        default: ;
    }
    return cols_;
}		
// end of ColumnEvent

//
// start of PatCursor class
//
PatCursor::PatCursor() :
    track_(0), 
    line_(0), 
    eventNr_(0), 
    col_(0) 
{
}

PatCursor::PatCursor(int track, int line, int eventNr, int col) :
track_( track ), 
    line_( line ), 
    eventNr_( eventNr ), 
    col_( col ) 
{
}

PatCursor::~PatCursor() {

}

void PatCursor::setPosition( int track, int line, int eventNr, int col ) {
    track_ = track;
    line_  = line;
    eventNr_ = eventNr;
    col_ = col;
}

void PatCursor::setTrack( int track ) {
    track_ = track;
}

int PatCursor::track() const {
    return track_;
}

void PatCursor::setLine( int line ) {
    line_ = line;
}

int PatCursor::line() const {
    return line_;
}

void PatCursor::setEventNr( int eventNr ) {
    eventNr_ = eventNr;
}

int PatCursor::eventNr() const {
    return eventNr_;
}

void PatCursor::setCol( int col) {
    col_ = col;
}

int PatCursor::col() const {
    return col_;
}


