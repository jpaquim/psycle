#ifndef PATTERNGRID_H
#define PATTERNGRID_H

#include <QGraphicsItem>
#include <QKeyEvent>
#include <QFont>
#include <QFontMetrics>

#include "patterndraw.h"

#include <map>

class PatternDraw;
class TrackGeometry;
class ColumnEvent;

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

    const std::map<int, TrackGeometry> & trackGeometrics() const;
    int trackWidth() const;
    int lineHeight() const;
    bool lineGridEnabled() const;
    int gridWidthByTrack( int track ) const;
    bool isNote( int key );

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
void moveCursor( int dx, int dy );
int visibleEvents( int track ) const;
const QFont & font() const;
void setFont( QFont font );


public slots:
    void keyPressEvent( QKeyEvent *event );


private:
    void setupTrackGeometrics( int numberOfTracks );
    void alignTracks();
    PatternDraw *patDraw_;
    std::map<int, TrackGeometry> trackGeometryMap;

    std::vector<ColumnEvent> events_;

    PatCursor cursor_;
    QFont font_;
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
