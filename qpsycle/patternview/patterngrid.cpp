#include "patterngrid.h"

PatternGrid::PatternGrid( PatternView *pView )
    : patView_( pView )
{
    setupTrackGeometrics( patView_->numberOfTracks() );
}

QRectF PatternGrid::boundingRect() const
{
    int numberOfTracks = patView_->numberOfTracks();
    int trackWidth = patView_->trackWidth();
    int numberOfLines = patView_->numberOfLines();
    int rowHeight = patView_->rowHeight();
    int gridWidth = trackWidth * numberOfTracks;//xEndByTrack( endTrack ) - dx();
    int gridHeight = rowHeight * numberOfLines;//((endLine +1) * rowHeight()) - dy();
    return QRectF( 0, 0, gridWidth, gridHeight );
}

void PatternGrid::paint( QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget )
{
//        TimeSignature signature;
        int numberOfTracks = patView_->numberOfTracks();
        int trackWidth = patView_->trackWidth();
        int numberOfLines = patView_->numberOfLines();
        int rowHeight = patView_->rowHeight();

        int gridWidth = trackWidth * numberOfTracks;//xEndByTrack( endTrack ) - dx();
        int gridHeight = rowHeight * numberOfLines;//((endLine +1) * rowHeight()) - dy();

        //drawSelBg(g,selection());

        int startLine = 0; 
        int endLine = numberOfLines;
        int startTrack = 0;
        int endTrack = numberOfTracks - 1;

        drawTrackGrid( painter, startLine, endLine, startTrack, endTrack );	
        //drawColumnGrid(g, startLine, endLine, startTrack, endTrack);
        //drawPattern(g, startLine, endLine, startTrack, endTrack);
        //drawRestArea(g, startLine, endLine, startTrack, endTrack);
}

/*int PatternGrid::xEndByTrack( int track ) const {
    std::map<int, TrackGeometry>::const_iterator it;
    it = trackGeometrics().lower_bound( track );
    int trackOff = 0;
    if ( it != trackGeometrics().end() )
        trackOff = it->second.left() + it->second.width();
    return trackOff;
}

const std::map<int, TrackGeometry> & PatternGrid::trackGeometrics() const {
    return trackGeometryMap;
}*/

void PatternGrid::drawTrackGrid( QPainter *painter, int startLine, int endLine, int startTrack, int endTrack  ) {
    std::map<int, TrackGeometry>::const_iterator it;
    it = trackGeometrics().lower_bound( endTrack );
    int gridWidth = 0;
    if ( it != trackGeometrics().end() ) {
        gridWidth = it->second.left() + it->second.width();
    }
//    gridWidth -= dx();
//    int gridWidth = patView_->trackWidth() * patView_->numberOfTracks();
    std::cout << "grid " << gridWidth << std::endl;

    int gridHeight = ( (endLine+1) * patView_->rowHeight() );

    if ( lineGridEnabled() ) {
        painter->setPen( Qt::red );
        for (int y = startLine; y <= endLine; y++)
            painter->drawLine( 0,y * lineHeight() - dy(), gridWidth, y* lineHeight()-dy() );
    }

    painter->setPen( Qt::white );//bigTrackSeparatorColor() );
    painter->setBrush( Qt::green );//bigTrackSeparatorColor() );
    it = trackGeometrics().lower_bound( startTrack );
    for ( ; it != trackGeometrics().end() && it->first <= endTrack; it++) { //  oolIdent px space at begin of trackCol{
        TrackGeometry trackGeom = it->second;
    std::cout << trackGeom.left() << std::endl;
        painter->drawRect( it->second.left(),0, 5, gridHeight );
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

