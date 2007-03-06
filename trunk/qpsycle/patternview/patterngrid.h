#ifndef PATTERNGRID_H
#define PATTERNGRID_H

#include <QGraphicsItem>

#include "patternview.h"

#include <map>

class PatternView;
class TrackGeometry;

class PatternGrid : public QGraphicsItem {

public:
    PatternGrid( PatternView *patView );

    QRectF boundingRect() const;
    void paint( QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget );

    int dy() { return 0; }
    void drawTrackGrid( QPainter *painter, int startLine, int endLine, int startTrack, int endTrack  );
    const std::map<int, TrackGeometry> & trackGeometrics() const;

    int trackWidth() const;
    int lineHeight() const;
    bool lineGridEnabled() const;


private:
    void setupTrackGeometrics( int numberOfTracks );
    void alignTracks();
    PatternView *patView_;
    std::map<int, TrackGeometry> trackGeometryMap;


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


#endif
