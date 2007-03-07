#include "patterngrid.h"
#include "psycore/singlepattern.h"

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



PatternGrid::PatternGrid( PatternView *pView )
    : patView_( pView )
{
    setupTrackGeometrics( patView_->numberOfTracks() );

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

}

void PatternGrid::addEvent( const ColumnEvent & event ) {
    events_.push_back( event );
}


QRectF PatternGrid::boundingRect() const
{
    int numberOfTracks = patView_->numberOfTracks();
    int trackWidth = patView_->trackWidth();
    int numberOfLines = patView_->numberOfLines();
    int gridWidth = trackWidth * numberOfTracks;//xEndByTrack( endTrack ) - dx();
    int gridHeight = lineHeight() * numberOfLines;//((endLine +1) * rowHeight()) - dy();
    return QRectF( 0, 0, gridWidth, gridHeight );
}

void PatternGrid::paint( QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget )
{
//        TimeSignature signature;
        int numberOfTracks = patView_->numberOfTracks();
        int trackWidth = patView_->trackWidth();
        int numberOfLines = patView_->numberOfLines();
        int rowHeight = patView_->rowHeight();

        int startLine = 0; 
        int endLine = numberOfLines;
        int startTrack = 0;
        int endTrack = numberOfTracks - 1;

        //int gridWidth = trackWidth * numberOfTracks;//xEndByTrack( endTrack ) - dx();
        int gridWidth = gridWidthByTrack( endTrack );// - dx();
        int gridHeight = rowHeight * numberOfLines;//((endLine +1) * rowHeight()) - dy();

        //drawSelBg(g,selection());


        drawGrid( painter, startLine, endLine, startTrack, endTrack );	
        //drawColumnGrid(g, startLine, endLine, startTrack, endTrack);
//        drawPattern( painter, startLine, endLine, startTrack, endTrack );
        //drawRestArea(g, startLine, endLine, startTrack, endTrack);*/
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
    std::map<int, TrackGeometry>::const_iterator it;
    it = trackGeometrics().lower_bound( endTrack );
    int gridWidth = 0;
    if ( it != trackGeometrics().end() ) {
        gridWidth = it->second.left() + it->second.width();
    }

    int gridHeight = ( (endLine+1) * patView_->rowHeight() );

    // Draw horizontal lines to demarcate the pattern lines.
    if ( lineGridEnabled() ) {
        painter->setPen( Qt::gray );
        for (int y = startLine; y <= endLine; y++)
            painter->drawLine( 0, y * lineHeight(), gridWidth, y* lineHeight() );
    }

    // Draw the vertical track separators.
    painter->setPen( Qt::gray );
    painter->setBrush( Qt::gray );
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
    if ( patView_->pattern() ) {
    //    drawCellBg( g, cursor()  );
        // find start iterator
        psy::core::SinglePattern::iterator it = patView_->pattern()->find_lower_nearest(startLine);
    //    TimeSignature signature;
/*

        int lastLine = -1;
        psy::core::PatternLine* line;
        psy::core::PatternLine emptyLine;

        for ( int y = startLine; y <= endLine; y++) {

            if ( it != patView_->pattern()->end() )	{
                int liney = d2i (it->first * patView_->pattern()->beatZoom());
                if (liney == y ) {
                    line = &it->second;
                    it++;
                } else line = &emptyLine;
            } else line = &emptyLine;


            if (y != lastLine) {
                QColor tColor = QColor( Qt::white );
                //QColor tColor = textColor();

                bool onBeat = false;
                bool onBar  = false;
/*                if ( !(y % beatZoom())) {
                    if (  it != patView_->pattern_->end() && pView->pattern()->barStart(it->first, signature) ) {
                        tColor = barColor();
                    } else {
                        onBeat = true;
                        tColor = beatTextColor();
                    }
                }

                // Check if this line is currently being played.
/*                if ((y == patView_->playPos() && Player::Instance()->playing() ) ) {
                    int trackWidth = xEndByTrack( endTrack ) - dx();
                    g.setForeground( playBarColor() );
                    g.fillRect(0, y*rowHeight() - dy(), trackWidth, rowHeight());
                }

                QColor stdColor = tColor;
                QColor crColor = QColor( Qt::red );
                //QColor crColor = patView_->colorInfo().cursor_text_color;

                std::map<int, psy::core::PatternEvent>::iterator eventIt = line->notes().lower_bound(startTrack);
                psy::core::PatternEvent emptyEvent;
                psy::core::PatternEvent* event;

                for ( int x = startTrack; x <= endTrack; x++ ) {

                    if ( eventIt != line->notes().end() && eventIt->first <= endTrack ) {
                        int trackx = eventIt->first;
                        if ( x == trackx) {
                            event = &eventIt->second;
                            eventIt++;
                        } else {
                            event = &emptyEvent;
                        }
                    } else event = &emptyEvent;


/*                    if ( x >= selection().left() && x < selection().right() &&
                        y >= selection().top() && y < selection().bottom() ) {
                            if ( !onBeat ) 
                                tColor = patView_->colorInfo().sel_text_color;
                            else 
                                tColor = patView_->colorInfo().sel_beat_text_color;
                    }	else tColor = stdColor;

                    drawData( painter, x, y, 0, event->note() ,event->isSharp(), tColor );
                    if (event->instrument() != 255) drawData( painter, x, y, 1, event->instrument(), 1, tColor );
                    if (event->machine() != 255) drawData( painter, x, y, 2, event->machine(), 1, tColor );
                    if (event->volume() != 255) drawData( painter, x, y, 3, event->volume(), 1, tColor );
                    if (event->command() != 0 || event->parameter() != 0) {
                        drawData( painter, x, y, 4, (event->command() << 8) | event->parameter(), 1, tColor );
                    }	else {
//                        drawString( g, x, y, 4, "....", tColor );
                    }									

                    psy::core::PatternEvent::PcmListType & pcList = event->paraCmdList();
                    psy::core::PatternEvent::PcmListType::iterator it = pcList.begin();
                    int count = 0;
                    for ( ; it < pcList.end(); it++, count++ ) {
                        psy::core::PatternEvent::PcmType & pc = *it;
                        int command = pc.first;
                        int parameter = pc.second;
                        if ( command != 0 || parameter != 0) {
                            drawData( painter, x, y, 5+count, ( command << 8) | parameter , 1, tColor );
                        }
                    }
                }
                lastLine = y;
            }
        }*/
    }
} // drawPattern

void PatternGrid::drawData( QPainter *painter, int track, int line, int eventnr, int data, bool sharp, const QColor & color ) {

    std::map<int, TrackGeometry>::const_iterator it;
    it = trackGeometrics().lower_bound( track );
    if ( it == trackGeometrics().end() || eventnr >= it->second.visibleColumns()  ) return;

    //int xOff = it->second.left() + 5 + trackLeftIdent() - dx();			// 5= colIdent
    int xOff = 0;

    if ( eventnr < events_.size() ) {
        const ColumnEvent & event = events_.at( eventnr );
        switch ( event.type() ) {
                                case ColumnEvent::hex2 :
                                    drawBlockData( painter, xOff + eventOffset(eventnr,0), line, toHex(data,2), color );
                                    // check if cursor is on event and draw digit in cursortextColor
/*                                    if ( cursor().track() == track && cursor().line() == line && 
                                        cursor().eventNr() == eventnr && cursor().col() < 2 ) {
                                            drawBlockData( painter, xOff + eventOffset(eventnr,0) + cursor().col()*cellWidth() , line, toHex(data,2).substr(cursor().col(),1) ,cursorTextColor_ );
                                    }*/
                                    break;
                                case ColumnEvent::hex4 :
                                    drawBlockData( painter, xOff + eventOffset(eventnr,0), line, toHex(data,4), color );
/*                                    // check if cursor is on event and draw digit in cursortextColor
                                    if ( cursor().track() == track && cursor().line() == line && 
                                        cursor().eventNr() == eventnr && cursor().col() < 4 ) {
                                            drawBlockData( painter, xOff + eventOffset(eventnr,0) + cursor().col()*cellWidth(), line, toHex(data,4).substr(cursor().col(),1) ,cursorTextColor_ );
                                    }*/

                                    break;
                                case ColumnEvent::note :					
/*                                    if ( cursor().track() == track && cursor().line() == line && 
                                        cursor().eventNr() == eventnr ) {
                                            drawStringData( painter, xOff + eventOffset(eventnr,0), line, noteToString(data, sharp),cursorTextColor_ );
                                    } else*/
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
    int yOff = line  * lineHeight();
    int col = 0;
    for (int i = 0; i < text.length(); i++) {
        painter->drawText(xOff + col,yOff, QString::fromStdString( text.substr(i,1) ) );
        col += cellWidth();
    }
}

int PatternGrid::cellWidth( ) const {
/*    ngrs::FontMetrics metrics( font( ) );
    return metrics.maxCharWidth( );*/
    return 10;
}

int PatternGrid::eventOffset( int eventnr, int col ) const {
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

int PatternGrid::noteCellWidth( ) const {            	
    return cellWidth() * 3;
}

void PatternGrid::drawStringData( QPainter *painter, int xOff, int line, const std::string & text, const QColor & color )
{
//    int yp = ( rowHeight() - g.textHeight()) / 2  + g.textAscent();
//    int yOff = line  * rowHeight() + yp  - dy();
    int yOff = line  * lineHeight();

    painter->drawText(xOff,yOff, QString::fromStdString( text ) );
}

std::string PatternGrid::noteToString( int value, bool sharp )
{
/*    switch (value) {
        case cdefTweakM : return "twk"; break;
        case cdefTweakE : return "twf"; break;
        case cdefMIDICC : return "mcm"; break;
        case cdefTweakS : return "tws"; break;
        case 120        : return "off"; break;
        case 255        : return "---"; break;  // defaultNoteStr_; break;
    }*/

    int octave = value / 12;

/*    if (sharp) switch (value % 12) {
        case 0:   return "C-" + stringify( octave ); break;
        case 1:   return "C#" + stringify( octave ); break;
        case 2:   return "D-" + stringify( octave ); break;
        case 3:   return "D#" + stringify( octave ); break;
        case 4:   return "E-" + stringify( octave ); break;
        case 5:   return "F-" + stringify( octave ); break;
        case 6:   return "F#" + stringify( octave ); break;
        case 7:   return "G-" + stringify( octave ); break;
        case 8:   return "G#" + stringify( octave ); break;
        case 9:   return "A-" + stringify( octave ); break;
        case 10:  return "A#" + stringify( octave ); break;
        case 11:  return "B-" + stringify( octave ); break;
    } else
    switch (value % 12) {
        case 0:   return "C-" + stringify(octave); break;
        case 1:   return "Db" + stringify(octave); break;
        case 2:   return "D-" + stringify(octave); break;
        case 3:   return "Eb" + stringify(octave); break;
        case 4:   return "E-" + stringify(octave); break;
        case 5:   return "F-" + stringify(octave); break;
        case 6:   return "Gb" + stringify(octave); break;
        case 7:   return "G-" + stringify(octave); break;
        case 8:   return "Ab" + stringify(octave); break;
        case 9:   return "A-" + stringify(octave); break;
        case 10:  return "Bb" + stringify(octave); break;
        case 11:  return "B-" + stringify(octave); break;
    }*/
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
    return patView_->trackWidth();
}

int PatternGrid::lineHeight() const
{
    return patView_->rowHeight();
}

bool PatternGrid::lineGridEnabled() const
{
    return true;
}


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

