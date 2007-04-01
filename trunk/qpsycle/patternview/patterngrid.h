#ifndef PATTERNGRID_H
#define PATTERNGRID_H

#include "patterndraw.h"
#include "psycore/singlepattern.h"

#include <map>

#include <QGraphicsItem>
#include <QKeyEvent>
#include <QFont>
#include <QFontMetrics>


class PatternDraw;
class TrackGeometry;
class ColumnEvent;

class Selection {
public:
    Selection() {}

    void set( int left, int right, int top, int bottom )
    { left_ = left; right_ = right; top_ = top; bottom_ = bottom; }
    int left() { return left_; }
    int right() { return right_; }
    int top() { return top_; }
    int bottom() { return bottom_; }
    void setLeft( int left ) { left_ = left; }
    void setRight( int right ) { right_ = right; };
    void setTop( int top ) { top_ = top; }
    void setBottom( int bottom ){ bottom_ = bottom; }
    void clear() { left_=0; right_=0; top_=0; bottom_=0; }

private:
    int left_, right_, top_, bottom_;

};

class PatCursor {
public:
    PatCursor();
    PatCursor( int track, int line, int eventNr, int col );
    ~PatCursor();

    void setPosition( int track, int line, int eventNr, int col );

    void setTrack( int x );
    int track() const;
    void setLine( int y );
    int line() const;
    void setEventNr( int event );
    int eventNr() const;
    void setCol( int col );
    int col() const;

private:

    int track_;
    int line_;
    int eventNr_;
    int col_;

};



class PatternGrid : public QGraphicsItem {

public:
    PatternGrid( PatternDraw *pDraw );

    QRectF boundingRect() const;
    void paint( QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget );

    void drawGrid( QPainter *painter, int startLine, int endLine, int startTrack, int endTrack  );
    void drawPattern( QPainter *painter, int startLine, int endLine, int startTrack, int endTrack  );
    void drawData( QPainter *painter, int startLine, int endLine, int startTrack, int endTrack, bool sharp, const QColor & color );
    void drawSelBg( QPainter *painter, Selection selArea );

    const std::map<int, TrackGeometry> & trackGeometrics() const;
    int trackWidth() const;
    int lineHeight() const;
    bool lineGridEnabled() const;
    int gridWidthByTrack( int track ) const;
    bool isNote( int key );
    bool isHex( QKeyEvent *ev );
    unsigned char convertDigit( int defaultValue, int scanCode, unsigned char oldByte, int col ) const;

    void drawBlockData( QPainter *painter, int xOff, int line, const std::string & text, const QColor & color);
    int cellWidth( ) const;
    int eventOffset( int eventnr, int col ) const;
    int eventColWidth( int eventnr ) const;
    int noteCellWidth( ) const;
    void drawStringData( QPainter *painter, int xOff, int line, const std::string & text, const QColor & color );
    std::string noteToString( int value, bool sharp );
    void drawString( QPainter *painter, int track, int line, int eventnr, const std::string & data , const QColor & color );
    void drawCellBg( QPainter *painter, const PatCursor& cursor );

    void addEvent( const ColumnEvent & event );
    const PatCursor & cursor() const;
    void setCursor( const PatCursor & cursor );
    void moveCursor( int dx, int dy );
    int visibleEvents( int track ) const;
    const QFont & font() const;
    void setFont( QFont font );

    int numberOfTracks() const;
    int numberOfLines() const;

    int beatZoom() const;
    psy::core::SinglePattern *pattern();

    void setSeparatorColor( const QColor & color );
    const QColor & separatorColor() const;

    void setLineSeparatorColor( const QColor & color );
    const QColor & lineSeparatorColor() const;

    void setRestAreaColor( const QColor & color );
    const QColor & restArea() const;

    void setBigTrackSeparatorColor( const QColor & selColor );
    const QColor & bigTrackSeparatorColor() const;

    void setSmallTrackSeparatorColor( const QColor & color );
    const QColor & smallTrackSeparatorColor() const;

    void setSelectionColor( const QColor & selColor );
    const QColor & selectionColor() const;

    void setCursorColor( const QColor & cursorColor );
    const QColor & cursorColor() const;

    void setCursorTextColor( const QColor & cursorTextColor );
    const QColor & cursorTextColor() const;

    void setBarColor( const QColor & barColor );
    const QColor & barColor() const;

    void setBeatColor( const QColor & beatColor );
    const QColor & beatColor() const;

    void setPlayBarColor( const QColor & playBarColor );
    const QColor & playBarColor() const;

    void setBeatTextColor( const QColor & color );
    const QColor & beatTextColor();

    void setTextColor( const QColor & color);
    const QColor & textColor() const;

    void setBigTrackSeparatorWidth( int ident );
    int bigTrackSeparatorWidth() const;

    bool doingKeybasedSelect() { return doingKeybasedSelect_; }
    bool lineAlreadySelected( int lineNumber );
    bool trackAlreadySelected( int trackNumber );
    void startKeybasedSelection(int leftPos, int rightPos, int topPos, int bottomPos);
    void startMouseSelection( const PatCursor & p );
    void repaintSelection();
    void repaintCursor();
    PatCursor intersectCell( int x, int y );
    int findTrackByXPos( int x ) const;

    int patternStep(); 

    void copyBlock( bool cutit );
    void pasteBlock( int tx,int lx,bool mix );

    QRectF repaintTrackArea(int startLine,int endLine,int startTrack, int endTrack) const;


protected:
    void mousePressEvent( QGraphicsSceneMouseEvent *event );
    void mouseReleaseEvent( QGraphicsSceneMouseEvent *event );
    void mouseMoveEvent( QGraphicsSceneMouseEvent *event );
    void keyPressEvent( QKeyEvent *event );


private:
    Selection selection_;
    Selection oldSelection_;
    Selection selection() const;
    PatCursor selStartPoint_;
    PatCursor selCursor_;

    bool doingKeybasedSelect_;
    bool doingMouseSelect_;

    bool isBlockCopied_;
    psy::core::SinglePattern pasteBuffer;
    float lastXmlLineBeatPos;
    int xmlTracks;
    float xmlBeats;

    void setupTrackGeometrics( int numberOfTracks );
    void alignTracks();
    int xOffByTrack( int track ) const;
    int xEndByTrack( int track ) const;

    PatternDraw *patDraw_;
    std::map<int, TrackGeometry> trackGeometryMap;

    std::vector<ColumnEvent> events_;

    PatCursor cursor_;
    PatCursor oldCursor_;
    QFont font_;

    QColor textColor_;
    QColor separatorColor_;
    QColor selectionColor_;
    QColor cursorColor_;
    QColor cursorTextColor_ ;
    QColor barColor_ ;
    QColor beatColor_;
    QColor beatTextColor_; 
    QColor playBarColor_;
    QColor bigTrackSeparatorColor_; 
    QColor smallTrackSeparatorColor_;
    QColor lineSepColor_;
    QColor restAreaColor_ ;

};

class TrackGeometry {
public:			
    TrackGeometry();

    TrackGeometry( PatternGrid & patternGrid );

    ~TrackGeometry();

    void setLeft( int left );
    int left() const;

    void setWidth( int width );
    int width() const;			

    void setVisibleColumns( int cols );
    int visibleColumns() const;

    void setVisible( bool on);
    bool visible() const;

private:

    PatternGrid *pGrid;
    int left_;
    int width_;
    int visibleColumns_;
    bool visible_;

};

class ColumnEvent {			
public:
    enum ColType { hex2 = 0, hex4 = 1, note = 2 };

    ColumnEvent( ColType type );

    ~ColumnEvent();

    ColType type() const;
    int cols() const;

private:

    ColType type_;
};


#endif
