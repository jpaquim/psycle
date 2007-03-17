#ifndef PATTERNGRID_H
#define PATTERNGRID_H

#include <QGraphicsItem>
#include <QKeyEvent>
#include <QFont>
#include <QFontMetrics>

#include "patterndraw.h"
#include "psycore/singlepattern.h"

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
    bool isHex( int c );
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

/*			mem +="<cursor bgcolor='179:217:34' textcolor='0:0:0' />";
			mem +="<restarea bgcolor='24:22:25' />";
			mem +="<bar bgcolor='70:71:69' sel_bgcolor='162:101:68'/>";
			mem +="<beat bgcolor='50:51:49' sel_bgcolor='142:81:48' textcolor='199:199:199' sel_textcolor='216:154:120'/>";
			mem +="<lines bgcolor='34:32:35' sel_bgcolor='140:68:41' textcolor='255:255:255' sel_textcolor='239:175:140'/>";
			mem +="<playbar bgcolor='182:121:88' sel_bgcolor='0:0:200'/>";
			mem +="<bigtrackseparator bgcolor='145:147:147' width='2' />";
			mem +="<smalltrackseparator bgcolor='105:107:107' />";
			mem +="<lineseparator enable='0' bgcolor='145:147:147'/>";
			mem +="<colseparator enable='0' bgcolor='145:147:147'/>";
			mem +="<trackident left ='2' right='2'/>";
			mem +="<header src='/bitmaps/alk_orange_header_small.xpm'>";
			mem +="<background_source coord='0:0:109:18'/>";
			mem +="<number_0_source coord='0:18:7:12'/>";
			mem +="<record_on_source coord='70:18:11:11'/>";
			mem +="<mute_on_source coord='81:18:11:11'/>";
			mem +="<solo_on_source coord='92:18:11:11'/>";
			mem +="<digit_x0_dest coord='24:3'/>";
			mem +="<digit_0x_dest coord='31:3'/>";
			mem +="<record_on_dest coord='52:3'/>";
			mem +="<mute_on_dest coord='75:3'/>";
			mem +="<solo_on_dest coord='97:3'/>";*/

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
