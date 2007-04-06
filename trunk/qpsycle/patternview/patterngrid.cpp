/***************************************************************************
*   Copyright (C) 2007 by Psycledelics Community   *
*   psycle.sourceforge.net   *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
*   This program is distributed in the hope that it will be useful,       *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
*   GNU General Public License for more details.                          *
*                                                                         *
*   You should have received a copy of the GNU General Public License     *
*   along with this program; if not, write to the                         *
*   Free Software Foundation, Inc.,                                       *
*   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
***************************************************************************/
#include "psycore/singlepattern.h"
#include "psycore/global.h"
#include "psycore/player.h"

#include "patterngrid.h"

#include <iostream>
#include <iomanip>
#include <memory> // auto_ptr

#include <QApplication>
#include <QClipboard>
#include <QDomDocument>
#include <QGraphicsSceneMouseEvent>
#include <QScrollBar>

template<class T> inline std::string toHex(T value , int nums = 2) {
    std::ostringstream buffer;
    buffer.setf(std::ios::uppercase);

    buffer.str("");
    buffer << std::setfill('0') << std::hex << std::setw( nums );
    buffer << (int) value;
    return buffer.str();
}

template<class T> inline T str_hex(const std::string &  value) {
    T result;

    std::stringstream str;
    str << value;
    str >> std::hex >> result;

    return result;
}

PatternGrid::PatternGrid( PatternDraw *pDraw )
    : patDraw_( pDraw )
{
    setFlag(ItemIsFocusable);

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
    //
    selection_.set( 0,0,0,0 );
    doingKeybasedSelect_ = false;
    
    // FIXME: hardcoding these for now.
     textColor_ = QColor( 255, 255, 255 );
     separatorColor_ = QColor( 145, 147, 147 );
     selectionColor_ = QColor( 149, 68, 41, 70 );
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
    if ( patDraw_->patternView()->pattern() ) {
        int gridWidth = patDraw_->gridWidthByTrack( endTrackNumber() );
        int gridHeight = numberOfLines()*lineHeight();
        return QRectF( 0, 0, gridWidth, gridHeight ); 
    } else {
        return QRectF( 0, 0, patDraw_->width(), patDraw_->height() );
    }
}

void PatternGrid::paint( QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget )
{
    font_ = QFont( "courier", 9 );
    setFont( font_ );
    painter->setFont( font_ ); 
    if ( pattern() ) {
        int startLine = 0; 
        int endLine = endLineNumber();
        int startTrack = 0;
        int endTrack = endTrackNumber();

        patDraw_->alignTracks();
        drawGrid( painter, startLine, endLine, startTrack, endTrack );	
        //drawColumnGrid(g, startLine, endLine, startTrack, endTrack);
        drawPattern( painter, startLine, endLine, startTrack, endTrack );
        drawSelBg( painter, selection() );
    //drawRestArea(g, startLine, endLine, startTrack, endTrack);*/
    }
}

/**
 * Draw the grid outline.
 */
void PatternGrid::drawGrid( QPainter *painter, int startLine, int endLine, int startTrack, int endTrack  ) 
{
    psy::core::TimeSignature signature;

    int gridWidth = boundingRect().width();
    int gridHeight = boundingRect().height();

    // Draw horizontal lines to demarcate the pattern lines.
    if ( lineGridEnabled() ) {
        painter->setPen( separatorColor() );
        for (int y = startLine; y <= endLine; y++)
            painter->drawLine( 0, y * lineHeight(), gridWidth, y* lineHeight() );
    }

    for (int y = startLine; y <= endLine; y++) {
        float position = y / (float) beatZoom();
        if (!(y == patDraw_->patternView()->playPos() ) || !psy::core::Player::Instance()->playing() ) {
            if ( !(y % beatZoom())) {
                if ((patDraw_->patternView()->pattern()->barStart(position, signature) )) {

                    painter->setBrush( barColor() );
                    painter->drawRect( 0, y*lineHeight() /*- dy()*/, gridWidth, lineHeight());

/*                    if ( y >= selection().top() && y < selection().bottom()) {
                        int left  = patDraw_->xOffByTrack( selection().left() );
                        int right = patDraw_->xOffByTrack( selection().right() );
                        g.setForeground( pView->colorInfo().sel_bar_bg_color );
                        g.fillRect( left - dx(), y*rowHeight() - dy(), right - left, rowHeight());
                    }*/

                } else {
                    painter->setBrush( beatColor() );
                    painter->drawRect( 0, y* lineHeight() /*- dy()*/, gridWidth, lineHeight() );

                    /*if ( y >= selection().top() && y < selection().bottom()) {
                        int left  = patDraw_->xOffByTrack( selection().left() );
                        int right = patDraw_->xOffByTrack( selection().right() );
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
    std::map<int, TrackGeometry>::const_iterator it;
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
    if ( pattern() ) {
        drawCellBg( painter, cursor()  );
        // find start iterator
        psy::core::SinglePattern::iterator it = pattern()->find_lower_nearest(startLine);
        psy::core::TimeSignature signature;


        int lastLinenum = -1;
        psy::core::PatternLine* line;
        psy::core::PatternLine emptyLine;

        for ( int curLinenum = startLine; curLinenum <= endLine; curLinenum++ ) {

            if ( it != pattern()->end() )	{
                int liney = it->first * beatZoom();
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
                    if (  it != pattern()->end() && pattern()->barStart(it->first, signature) ) {
                        tColor = QColor( barColor() );
                    } else {
                        onBeat = true;
                        tColor = QColor( beatTextColor() );
                    }
                }

                // Check if this line is currently being played.
                if ((curLinenum == patDraw_->patternView()->playPos() && psy::core::Player::Instance()->playing() ) ) {
                    int trackWidth = patDraw_->xEndByTrack( endTrack ) /*- dx()*/;
                    painter->setPen( playBarColor() );
                    painter->setBrush( playBarColor() );
                    painter->drawRect( 0, curLinenum*lineHeight() /*- dy()*/, trackWidth, lineHeight() );
                }

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

    int xOff = it->second.left() + patDraw_->trackPaddingLeft() + 5;/*dx()*/;		

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
    for ( ; it < events_.end(); it++, nr++ ) 
    {
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

void PatternGrid::setCursor( const PatCursor & cursor ) {
    cursor_ = cursor;
}

void PatternGrid::keyPressEvent( QKeyEvent *event )
{
    int command = psy::core::Global::pConfig()->inputHandler().getEnumCodeByKey( psy::core::Key( event->modifiers() , event->key() ) );

    if ( cursor().eventNr() == 0 && isNote( command ) ) 
    {
        int note = command; // The cdefs for the keys correspond to the correct notes.
        doNoteEvent( note );
    } else if ( isHex( event ) ) {
        int keyChar = QChar( event->text().at(0) ).toAscii();
        if ( cursor().eventNr() == 1 ) {
            doInstrumentEvent( keyChar );
        } else if ( cursor().eventNr() == 2 ) {
            doMachineSelectionEvent( keyChar );
        } else if ( cursor().eventNr() == 3 ) {
            doVolumeEvent( keyChar );
        } else if ( cursor().eventNr() >= 4) {
            doCommandOrParameterEvent( keyChar );
        }
    }

    switch ( command ) {
        case psy::core::cdefNavUp:
            moveCursor( 0, -navStep() ); 
            checkUpScroll( cursor() );
            return;
        break;
        case psy::core::cdefNavDown:
            moveCursor( 0, navStep() );
            checkDownScroll( cursor() );
            return;
        break;
        case psy::core::cdefNavLeft:
            moveCursor( -1, 0 );
            checkLeftScroll( cursor() );
            return;
        break;
        case psy::core::cdefNavRight:
            moveCursor( 1, 0 );
            checkRightScroll( cursor() );
            return;
        break;
        case psy::core::cdefTrackPrev:
            if ( cursor().track() > 0 ) {
                PatCursor oldCursor = cursor();
                setCursor( PatCursor( cursor().track()-1, cursor().line(),0,0 ) );
            }
            repaintCursor();
            checkLeftScroll( cursor() );
            break;
            return;
        break;
        case psy::core::cdefTrackNext:
            if ( cursor().track()+1 < numberOfTracks() ) {
                PatCursor oldCursor = cursor();
                setCursor( PatCursor( cursor().track()+1, cursor().line(),0,0 ) );
            }
            repaintCursor(); 
            checkRightScroll( cursor() );
            break;
            return;
        break;
        case psy::core::cdefNavPageDn:
            moveCursor( 0, 16 );
            checkDownScroll( cursor() );
            break;
        case psy::core::cdefNavPageUp:
            moveCursor( 0, -16 );
            checkUpScroll( cursor() );
            break;
        case psy::core::cdefSelectUp:
            selectUp();
            break;
        case psy::core::cdefSelectDn:
            selectDown();
            break;
        case psy::core::cdefSelectLeft:
            selectLeft();
            break;
        case psy::core::cdefSelectRight:
            selectRight();
        break;
        case psy::core::cdefSelectTrack:
            selectTrack();
        break;
        case psy::core::cdefSelectAll:
            selectAll();
        break;
        case psy::core::cdefBlockCopy: 
            copyBlock( false );
            break;
        case psy::core::cdefBlockCut: 
            copyBlock( true );
            break;
        case psy::core::cdefBlockPaste: 
            qDebug( "hi" );
            pasteBlock( cursor().track(), cursor().line(), false );
            break;
        case psy::core::cdefBlockDelete: 
            deleteBlock();
            break;
        default:
            // If we got here, we didn't do anything with it, so officially ignore it.
            event->ignore();
    }

}

void PatternGrid::doNoteEvent( int note )
{
    if ( note == psy::core::cdefKeyStop ) {
        //pView->noteOffAny( cursor() );
    } else if (note >=0 && note < 120) {
            patDraw_->patternView()->enterNote( cursor(), note ); // FIXME: better to emit a signal here?
            moveCursor( 0, patternStep() );
            checkDownScroll( cursor() );
    }
}

void PatternGrid::doInstrumentEvent( int keyChar )
{
    // Add the new data.
    psy::core::PatternEvent patEvent = pattern()->event( cursor().line(), cursor().track() );
    unsigned char newByte = convertDigit( 0xFF, keyChar, patEvent.instrument(), cursor().col() );
    patEvent.setInstrument( newByte );
    pattern()->setEvent( cursor().line(), cursor().track(), patEvent );

    // Move the cursor.
    if ( cursor().col() == 0 ) {
        moveCursor( 1, 0 );			
    } else {
        moveCursor( -1, patternStep() );
        checkDownScroll( cursor() );
    }
}

void PatternGrid::doMachineSelectionEvent( int keyChar )
{
    psy::core::PatternEvent patEvent = pattern()->event( cursor().line(), cursor().track() );
    unsigned char newByte = convertDigit( 0xFF, keyChar, patEvent.machine(), cursor().col() );
    patEvent.setMachine( newByte );
    pattern()->setEvent( cursor().line(), cursor().track(), patEvent );
    if ( cursor().col() == 0 ) {
        moveCursor(1,0);			
    } else {
        moveCursor( -1, patternStep() );
        checkDownScroll( cursor() );
    }
}

void PatternGrid::doVolumeEvent( int keyChar )
{
    psy::core::PatternEvent patEvent = pattern()->event( cursor().line(), cursor().track() );
    unsigned char newByte = convertDigit( 0xFF, keyChar, patEvent.volume(), cursor().col() );
    patEvent.setVolume( newByte );
    pattern()->setEvent( cursor().line(), cursor().track(), patEvent );
    if (cursor().col() == 0) {
        moveCursor(1,0);			
    } else {
        moveCursor(-1, patternStep() );
    }
    checkDownScroll( cursor() );
}

void PatternGrid::doCommandOrParameterEvent( int keyChar )
{
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
    } else {
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
            moveCursor(-3, patternStep() );
            checkDownScroll( cursor() );
        }
    }			
}

void PatternGrid::drawSelBg( QPainter *painter, Selection selArea )
{			
    if ( !selArea.isEmpty() ) {
        int xStart = patDraw_->xOffByTrack( selArea.left() );
        int yStart = selArea.top()  * lineHeight() ;

        int xEnd = patDraw_->xEndByTrack( selArea.right() );
        int yEnd = (selArea.bottom()+1) * lineHeight();

        painter->setBrush( selectionColor() );
        painter->drawRect( xStart, yStart, xEnd-xStart, yEnd-yStart );
    }
}

void PatternGrid::selectUp()
{
    int step = navStep();

    oldSelection_ = selection_;
    PatCursor crs = cursor();
    int newLeft, newRight, newTop, newBottom;
    int newCursorTrack = cursor().track();
    int newCursorLine = cursor().line();
    int newCursorCol = cursor().col();
    if ( doingKeybasedSelect() ) {
        // if above line is not already selected then select it...
        if ( !lineAlreadySelected( crs.line() ) ) {
            // don't set selection out of bounds of grid...
            newTop = std::max( oldSelection_.top()-step, 0 );
            newBottom = oldSelection_.bottom();
        } else { // else if it is selected, deselect it...
            newTop = oldSelection_.top();
            newBottom = oldSelection_.bottom() - step;
        }
        newLeft = oldSelection_.left(); // left&right stay the same.
        newRight = oldSelection_.right();
        selection_.set( newLeft,newRight, newTop, newBottom ); 
    } else {
        startKeybasedSelection( crs.track(), crs.track(),
                                std::max( 0, crs.line() - step ), crs.line() );
    }
    newCursorLine = std::max( 0, cursor().line() - step );
    setCursor( PatCursor( newCursorTrack, newCursorLine, 0, 0 ) );
    checkUpScroll( cursor() );
    repaintSelection();
}

void PatternGrid::selectDown()
{
    int step = navStep();

    oldSelection_ = selection_;
    PatCursor crs = cursor();
    int newLeft, newRight, newTop, newBottom;
    int newCursorTrack = cursor().track();
    int newCursorLine = cursor().line();
    int newCursorCol = cursor().col();
    if ( doingKeybasedSelect() ) {
        // if line beneath is not selected...
        if ( !lineAlreadySelected( crs.line()+1 ) ) {
            // select line beneath.
            newTop = oldSelection_.top();
            newBottom = std::min( oldSelection_.bottom() + step, endLineNumber() );
        } else { // line beneath is selected...
            // deselect line beneath.
            newTop = oldSelection_.top() + step;
            newBottom = oldSelection_.bottom();
        }
        newLeft = oldSelection_.left(); // left&right stay the same.
        newRight = oldSelection_.right();
        selection_.set( newLeft, newRight, newTop, newBottom ); 

    } else {
        startKeybasedSelection( crs.track(), crs.track(),
            crs.line(),
            std::min( endLineNumber(), crs.line() + step ) );
    }
    newCursorLine = std::min( endLineNumber(), cursor().line() + step );
    setCursor( PatCursor( newCursorTrack, newCursorLine, 0, 0 ) );
    checkDownScroll( cursor() );
    repaintSelection();
}

void PatternGrid::selectLeft()
{
    oldSelection_ = selection_;
    PatCursor crs = cursor();
    int newLeft, newRight, newTop, newBottom;
    int newCursorTrack = cursor().track();
    int newCursorLine = cursor().line();
    int newCursorCol = cursor().col();
    if (doingKeybasedSelect()) {
        // if track to left is not selected...
        if (!trackAlreadySelected(crs.track()-1)) {
            // select track to left.
            newLeft = std::max( 0, oldSelection_.left()-1 );
            newRight = oldSelection_.right();
        } else { // track to left is selected...
            // deselect current track.
            newLeft = oldSelection_.left();
            newRight = oldSelection_.right()-1;
        }
        newTop = oldSelection_.top(); // top&bottom stay the same.
        newBottom = oldSelection_.bottom();
        selection_.set(newLeft,newRight,newTop,newBottom); 

    } else { // start a keyboard-based selection. 
        startKeybasedSelection(std::max(0,crs.track()-1),
            crs.track(),
            crs.line(), crs.line());
    }
    newCursorTrack = std::max(0,cursor().track()-1);
    newCursorLine = cursor().line(); 
    setCursor(PatCursor(newCursorTrack, newCursorLine, 0, 0));
    repaintSelection();
    checkLeftScroll( cursor() );
    //        newCursorCol = cursor().col()+1;
}

void PatternGrid::selectRight()
{
    oldSelection_ = selection_;
    PatCursor crs = cursor();
    int newLeft, newRight, newTop, newBottom;
    int newCursorTrack = cursor().track();
    int newCursorLine = cursor().line();
    int newCursorCol = cursor().col();
    if (doingKeybasedSelect()) {
        if (!trackAlreadySelected(crs.track()+1)) { // if track to right is not selected...
            newLeft = oldSelection_.left();         // select track to right.
            newRight = std::min(oldSelection_.right()+1, endTrackNumber() );
        } else { // track to right is selected... so deselect current track.
            newLeft = oldSelection_.left()+1;
            newRight = oldSelection_.right();
        }
        newTop = oldSelection_.top(); // top&bottom stay the same.
        newBottom = oldSelection_.bottom();
        selection_.set(newLeft,newRight,newTop,newBottom); 
    } else {
        startKeybasedSelection(crs.track(), 
            std::min(numberOfTracks(),crs.track()+1),
            crs.line(), crs.line());
    }
    newCursorTrack = std::min( endTrackNumber(), cursor().track()+1 );
    newCursorLine = cursor().line(); 
    setCursor(PatCursor(newCursorTrack, newCursorLine, 0, 0));
    checkRightScroll( cursor() );
    repaintSelection();
}

void PatternGrid::selectAll() 
{
    doingKeybasedSelect_ = true;
    startKeybasedSelection( 0, endTrackNumber(), 0, endLineNumber() );

    repaintSelection();
    selCursor_ = cursor();
}

void PatternGrid::selectTrack() {
    startKeybasedSelection( cursor().track(), cursor().track(), 0, endLineNumber() );
    repaintSelection();
}

int PatternGrid::navStep() {
    return ignorePatStepForNav() ? 1 : patternStep(); 
}

bool PatternGrid::ignorePatStepForNav() {
    return false; // FIXME: will come from a configuration option in future.
}

void PatternGrid::repaintSelection() {
    // FIXME: do this much better.
    int selLeft = selection_.left();
    int selRight = selection_.right();
    int selTop = selection_.top();
    int selBottom = selection_.bottom();
    int selWidth = patDraw_->xEndByTrack( selRight ) - patDraw_->xOffByTrack( selLeft );
    int selHeight = lineHeight() * (selBottom-selTop);
    int oldLeft = oldSelection_.left();
    int oldRight = oldSelection_.right();
    int oldTop = oldSelection_.top();
    int oldBottom = oldSelection_.bottom();
    int oldWidth = patDraw_->xEndByTrack( oldRight ) - patDraw_->xOffByTrack( oldLeft );
    int oldHeight = lineHeight() * (oldBottom-oldTop);
    update( patDraw_->xOffByTrack(selLeft), selTop*lineHeight(), selWidth, selHeight );
    update( patDraw_->xOffByTrack(oldLeft), oldTop*lineHeight(), oldWidth, oldHeight );
    update( boundingRect() );
    oldSelection_ = selection_;
}

void PatternGrid::repaintCursor() {
    // FIXME: could be done more accurately.
    update( patDraw_->xOffByTrack(cursor_.track()), cursor_.line()*lineHeight(), 
            patDraw_->xEndByTrack(cursor_.track()), lineHeight() );
    update( patDraw_->xOffByTrack(oldCursor_.track()), oldCursor_.line()*lineHeight(), 
            patDraw_->xEndByTrack(cursor_.track()), lineHeight() );
}


bool PatternGrid::isNote( int key )
{
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
    ) { return true; }

    return false;
}

void PatternGrid::moveCursor( int dx, int dy) 
{
    // dx -1 left hex digit move
    // dx +1 rigth hex digit move
    // dy in lines
    oldCursor_ = cursor_;
    int eventnr = cursor().eventNr();
    if ( dx > 0 ) {			
        if ( eventnr < events_.size() ) 
        {
            const ColumnEvent & event = events_.at( eventnr );
            int maxCols = event.cols();
            if ( cursor_.col() + dx < maxCols ) {
                cursor_.setCol( cursor_.col() + dx);
            } else if (eventnr + 1 < visibleEvents( cursor_.track()) ) {
                cursor_.setCol( 0 );
                cursor_.setEventNr( eventnr + 1);
            } else if (cursor_.track()+1 < numberOfTracks() ) {
                cursor_.setTrack( cursor_.track() + 1 );
                cursor_.setEventNr(0);
                cursor_.setCol(0);
            }
        }
    } else if ( dx < 0 ) {
        if ( cursor_.col() + dx >= 0 ) {
            cursor_.setCol( cursor_.col() + dx);
        } else if ( cursor_.eventNr() > 0 ) {
            cursor_.setEventNr( cursor_.eventNr() - 1 );
            const ColumnEvent & event = events_.at( cursor_.eventNr() );
            cursor_.setCol( event.cols() - 1 );					
        } else if ( cursor_.track() > 0 ) {
                cursor_.setTrack( cursor_.track() -1 );
                cursor_.setEventNr( visibleEvents( cursor_.track() -1 )-1 );
                const ColumnEvent & event = events_.at( cursor_.eventNr() );
                cursor_.setCol( event.cols() - 1 );
        }
    }

    if ( dy != 0 && (dy + cursor_.line() >= 0) ) {
        cursor_.setLine( std::min(cursor_.line() + dy, endLineNumber()));
    } else if ( dy != 0 && (dy + cursor_.line() < 0) ) {
        cursor_.setLine( std::max(cursor_.line() + dy, 0));
    } else if (dy!=0) {
    }
    repaintCursor(); 
    if ( doingKeybasedSelect() ) {
        doingKeybasedSelect_ = false;
        selection_.clear();
        repaintSelection(); 
    }
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

bool PatternGrid::isHex( QKeyEvent *ev ) {
    if ( ev->modifiers() != Qt::NoModifier ) return false;

    int key = ev->key();
    if ( key == Qt::Key_A ||
         key == Qt::Key_B ||
         key == Qt::Key_C ||
         key == Qt::Key_D ||
         key == Qt::Key_E ||
         key == Qt::Key_F ||
         key == Qt::Key_0 ||
         key == Qt::Key_1 ||
         key == Qt::Key_2 ||
         key == Qt::Key_3 ||
         key == Qt::Key_4 ||
         key == Qt::Key_5 ||
         key == Qt::Key_6 ||
         key == Qt::Key_7 ||
         key == Qt::Key_8 ||
         key == Qt::Key_9 
       ) return true;

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

int PatternGrid::numberOfTracks() const {
    return patDraw_->patternView()->numberOfTracks();
}

int PatternGrid::endTrackNumber() const {
    return patDraw_->patternView()->numberOfTracks() - 1;
}

int PatternGrid::numberOfLines() const {
    return patDraw_->patternView()->numberOfLines();
}

int PatternGrid::endLineNumber() const {
    return patDraw_->patternView()->numberOfLines() - 1;
}

Selection PatternGrid::selection() const 
{
    return selection_;
}


bool PatternGrid::lineAlreadySelected( int lineNumber ) 
{
    std::cout << "line " << lineNumber << std::endl;
    if ( lineNumber > selection_.top() && lineNumber <= selection_.bottom() ) {
        return true;
    } else {
        return false;
    }
}

bool PatternGrid::trackAlreadySelected( int trackNumber ) 
{
    if ( trackNumber >= selection_.left() && trackNumber < selection_.right() ) {
        return true;
    } else {
        return false;
    }
}

void PatternGrid::copyBlock( bool cutit )
{  
    isBlockCopied_=true;
    pasteBuffer.clear();
    std::auto_ptr<psy::core::SinglePattern> copyPattern(pattern()->block( selection().left(), selection().right()+1, selection().top(), selection().bottom()+1 ));

    float start = selection().top()    / (float) pattern()->beatZoom();
    float end   = selection().bottom() / (float) pattern()->beatZoom();

    std::string xml = "<patsel beats='" + QString::number( end - start ).toStdString(); 
    xml+= "' tracks='"+ QString::number( selection().right()+1 - selection().left() ).toStdString();
    xml+= "'>"; 
    xml+= copyPattern->toXml();
    xml+= "</patsel>";

    QApplication::clipboard()->setText( QString::fromStdString( xml ) );

    if (cutit) {
        pattern()->deleteBlock( selection().left(), selection().right()+1, selection().top(), selection().bottom()+1 );
        update( boundingRect() ); // FIXME: just update the selection.
    }
}

void PatternGrid::pasteBlock(int tx,int lx,bool mix )
{
    // If the clipboard isn't empty...
    if ( QApplication::clipboard()->text() != "" ) 
    {
        // Make a parser.
        QDomDocument *doc = new QDomDocument();
        doc->setContent( QApplication::clipboard()->text() );

        lastXmlLineBeatPos = 0.0;
        xmlTracks = 0;
        xmlBeats = 0;

        // Parse the clipboard text...
        QDomElement patselEl = doc->firstChildElement( "patsel" );
            xmlTracks = patselEl.attribute("tracks").toInt();
            xmlBeats = patselEl.attribute("beats").toFloat();

        QDomNodeList patlines = patselEl.elementsByTagName( "patline" );
        for ( int i = 0; i < patlines.count(); i++ )
        {
            QDomElement patLineElm = patlines.item( i ).toElement();
            lastXmlLineBeatPos = patLineElm.attribute("pos").toFloat();     

            QDomNodeList patEvents = patLineElm.elementsByTagName( "patevent" );
            for ( int j = 0; j < patEvents.count(); j++ )
            {
                QDomElement patEventElm = patEvents.item( j ).toElement();
                int trackNumber = str_hex<int> ( patEventElm.attribute("track").toStdString() );

                psy::core::PatternEvent data;
                data.setMachine( str_hex<int> (patEventElm.attribute("mac").toStdString() ) );
                data.setInstrument( str_hex<int> (patEventElm.attribute("inst").toStdString() ) );
                data.setNote( str_hex<int> (patEventElm.attribute("note").toStdString() ) );
                data.setParameter( str_hex<int> (patEventElm.attribute("param").toStdString() ) );
                data.setParameter( str_hex<int> (patEventElm.attribute("cmd").toStdString() ) );

                pasteBuffer[lastXmlLineBeatPos].notes()[trackNumber]=data;
            }
        }

        if (!mix)
            pattern()->copyBlock(tx,lx,pasteBuffer,xmlTracks,xmlBeats);
        else
            pattern()->mixBlock(tx,lx,pasteBuffer,xmlTracks,xmlBeats);
    }
    update( boundingRect() ); // FIXME: inefficient, be more specific.
}

void PatternGrid::deleteBlock( )
{
    int right = selection().right();
    int left = selection().left();
    double top = selection().top() / (double) beatZoom();
    double bottom = selection().bottom() / (double) beatZoom();

    pattern()->deleteBlock(left, right, top, bottom);
    update( boundingRect() ); // FIXME: be more specific.
}

QRectF PatternGrid::repaintTrackArea(int startLine,int endLine,int startTrack, int endTrack) const {
    int top    = startLine    * lineHeight()  + 0;//absoluteTop() - dy_;
    int bottom = (endLine+1)  * lineHeight()  + 0;//absoluteTop();//  - dy_;
    int left   = patDraw_->xOffByTrack( startTrack)  + 0;//absoluteLeft() - dx_;
    int right  = patDraw_->xEndByTrack( endTrack  )  + 0;//absoluteLeft() - dx_;

    return QRectF( left, top, right - left, bottom - top );
}

int PatternGrid::patternStep()
{
    return patDraw_->patternView()->patternStep();
}

// Start a new block selection using the keyboard.
void PatternGrid::startKeybasedSelection( int leftPos, int rightPos, int topPos, int bottomPos ) {
    PatCursor crs = cursor();
    selStartPoint_ = crs;
    selCursor_ = crs;
    doingKeybasedSelect_ = true;
    selection_.set( leftPos, rightPos, topPos, bottomPos );
}

PatCursor PatternGrid::intersectCell( int x, int y ) 
{
    int track = patDraw_->findTrackByXPos( x );
    int line  = y / lineHeight();
    int colOff   = x - patDraw_->xOffByTrack(track);

    std::vector<ColumnEvent>::const_iterator it = events_.begin();
    int nr = 0;
    int offset = 3/*colIdent*/ + patDraw_->trackPaddingLeft();
    int lastOffset = 3/*colIdent*/ + patDraw_->trackPaddingLeft();
    for ( ; it < events_.end(); it++, nr++ ) 
    {				
        const ColumnEvent & event = *it;				
        switch ( event.type() ) 
        {
            case ColumnEvent::hex2 : offset+= 2*cellWidth(); 	break;
            case ColumnEvent::hex4 : offset+= 4*cellWidth(); 	break;
            case ColumnEvent::note : offset+= noteCellWidth(); break;
            default: ;
        }
        if (offset > colOff) {
            // found our event
            if ( event.type() == ColumnEvent::note ) 
                return PatCursor(track,line,nr,0);
            else {
                int cellStart = colOff - lastOffset;
                int col = cellStart  / cellWidth();
                return PatCursor(track,line,nr,col);
            }			
        }
        lastOffset = offset;
    }
    return PatCursor();
}

void PatternGrid::startMouseSelection( const PatCursor & p )
{
    selStartPoint_ = p;
    selCursor_ = p;
    selection_.set( p.track(), p.track(), p.line(), p.line() );

    oldSelection_ = selection_;
    doingMouseSelect_ = true;
    doingKeybasedSelect_ = false;
}


void PatternGrid::mousePressEvent( QGraphicsSceneMouseEvent *event )
{
    if ( event->button() == Qt::LeftButton ) {
        oldCursor_ = cursor();
        PatCursor crs = intersectCell( event->lastPos().x(), event->lastPos().y() );
        setCursor( crs );
        repaintCursor();
    }
}

void PatternGrid::mouseMoveEvent( QGraphicsSceneMouseEvent *event )
{
    if ( event->buttons() == Qt::LeftButton ) {
        if ( !doingMouseSelect_ ) {
            startMouseSelection( intersectCell( event->buttonDownPos( Qt::LeftButton ).x(), event->buttonDownPos( Qt::LeftButton ).y() ) );
        } else {
            PatCursor p = intersectCell( event->pos().x(), event->pos().y() );

			if ( p.track() < selStartPoint().track() ) {
				selection_.setLeft( std::max(p.track(),0) ); 
			} else if (p.track() == selStartPoint_.track()) {
					selection_.setLeft (std::max(p.track(),0));
					selection_.setRight(std::min(p.track(), endTrackNumber()));
            } else if (p.track() > selStartPoint_.track()) {
                selection_.setRight(std::min(p.track(), endTrackNumber()));
            } 

            if (p.line() < selStartPoint_.line()) {
                selection_.setTop(std::max(p.line(),0));
                selection_.setBottom( std::min(selection_.bottom(), selStartPoint_.line()));
            } else if (p.line() > selStartPoint_.line()) {
                selection_.setBottom(std::min(p.line(), endLineNumber()));
                selection_.setTop( std::max(selection_.top(), selStartPoint_.line()));
            }

            update( boundingRect() );
        }
    }
}

void PatternGrid::mouseReleaseEvent( QGraphicsSceneMouseEvent *event )
{
    if ( event->button() == Qt::LeftButton ) {
        PatCursor mouseCurrentCrs = intersectCell( event->scenePos().x(), event->scenePos().y() );
        doingMouseSelect_ = false;
    }
}

int PatternGrid::visibleColWidth( int maxEvents ) const 
{
    std::vector<ColumnEvent>::const_iterator it = events_.begin();
    int offset = 0;
    int eventCount = 0;
    for ( ; it < events_.end() && eventCount < maxEvents; it++, eventCount++ ) {
        const ColumnEvent & event = *it;

        switch ( event.type() ) {
            case ColumnEvent::hex2 : offset+= 2*cellWidth(); 	break;
            case ColumnEvent::hex4 : offset+= 4*cellWidth(); 	break;
            case ColumnEvent::note : offset+= noteCellWidth(); break;
            default: ;
        }
    }

    return offset /* + colIdent*/ + patDraw_->trackPaddingLeft() + patDraw_->trackPaddingRight();
}

const std::map<int, TrackGeometry> & PatternGrid::trackGeometrics() const
{
    return patDraw_->trackGeometrics();
}

void PatternGrid::checkLeftScroll( const PatCursor & cursor ) 
{
    int sceneX = patDraw_->xOffByTrack( cursor.track() );
    QPoint foo = patDraw_->mapFromScene( sceneX, 0 ); 
    int viewX = foo.x();
    if ( viewX < 0 ) {
        patDraw_->horizontalScrollBar()->setValue( patDraw_->xOffByTrack( std::max( cursor.track(), 0 ) ) );
    }	
}

void PatternGrid::checkRightScroll( const PatCursor & cursor ) 
{
    if ( patDraw_->xOffByTrack( std::max( cursor.track()+1, 0 ) ) > patDraw_->width() ) {
        patDraw_->horizontalScrollBar()->setValue( patDraw_->xEndByTrack( std::min( cursor.track()+1 , endTrackNumber())) - patDraw_->width() );
    }
}

void PatternGrid::checkUpScroll( const PatCursor & cursor ) 
{
    int sceneY = cursor.line() * lineHeight(); 
    QPoint foo = patDraw_->mapFromScene( 0, sceneY ); 
    int viewY = foo.y();
    if ( viewY * lineHeight() < 0 ) {
        patDraw_->verticalScrollBar()->setValue( std::max( 0, cursor.line() ) * lineHeight() );
    }
}

void PatternGrid::checkDownScroll( const PatCursor & cursor ) 
{
    int sceneY = cursor.line() * lineHeight(); 
    QPoint foo = patDraw_->mapFromScene( 0, sceneY ); 
    int viewY = foo.y();
    if ( viewY > patDraw_->height() ) {
        patDraw_->verticalScrollBar()->setValue( ( std::min( cursor.line(),endLineNumber()) +1 ) * lineHeight() );
    }
}

const PatCursor & PatternGrid::selStartPoint() const {
    return selStartPoint_;
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


