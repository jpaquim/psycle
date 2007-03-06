#include "header.h"

Header::Header( PatternView * pPatternView ) : pView(pPatternView)
{
    setRect( 0, 0, pView->width(), 20 );
}

Header::~ Header( )
{
}

void Header::paint( QPainter *painter,
                    const QStyleOptionGraphicsItem *option,
                    QWidget *widget )
{
    painter->drawRect( 0, 0, pView->width(), 20 );

    int trackWidth = 100;
    int trackHeight = 20;
    int numTracks = pView->numberOfTracks();

    for ( int i = 0; i < numTracks; i++ )
    {
        painter->setPen( QPen( Qt::black ) );
        painter->setBrush( QBrush( Qt::transparent ) );
        painter->drawRect( i*trackWidth, 0, trackWidth, trackHeight ); 
        painter->setPen( QPen( Qt::white ) );
        painter->drawText( i*trackWidth+5, 15, QString::number(i) );
        painter->setPen( QPen( Qt::gray ) );
        painter->setBrush( QBrush( Qt::red ) );
        painter->drawEllipse( (i+1)*trackWidth - 15, 5, 10, 10 ); 
        painter->setBrush( QBrush( Qt::yellow ) );
        painter->drawEllipse( (i+1)*trackWidth - 30, 5, 10, 10 ); 
        painter->setBrush( QBrush( Qt::green ) );
        painter->drawEllipse( (i+1)*trackWidth - 45, 5, 10, 10 ); 
    }
    /*DefaultBitmaps & icons = SkinReader::Instance()->bitmaps();

    ngrs::Bitmap & bitmap = icons.pattern_header_skin();
    ngrs::Bitmap & patNav = icons.patNav();

    int startTrack = pView->drawArea->findTrackByScreenX( scrollDx() );

    g.setForeground(pView->separatorColor());

    std::map<int, TrackGeometry>::const_iterator it;
    it = pView->trackGeometrics().lower_bound( startTrack );

    for ( ; it != pView->trackGeometrics().end() && it->first <= pView->trackNumber() - 1; it++) {
        const TrackGeometry & trackGeometry = it->second;

        const int trackX0 = it->first / 10;
        const int track0X = it->first % 10;
        int xOff = trackGeometry.left();
        if (xOff - scrollDx() > spacingWidth() ) break;
        int center =  ( trackGeometry.width() - patNav.width() - skinColWidth()) / 2;		
        if ( center > -patNav.width() ) {
            int xOffc = xOff + patNav.width()  + center;
            g.putBitmap(xOffc,0,std::min((int) coords_.bgCoords.width(),trackGeometry.width()) ,coords_.bgCoords.height(), bitmap, 
                coords_.bgCoords.left(), coords_.bgCoords.top());
            g.putBitmap(xOffc+coords_.dgX0Coords.x(),0+coords_.dgX0Coords.y(),coords_.noCoords.width(),coords_.noCoords.height(), bitmap,
                trackX0*coords_.noCoords.width(), coords_.noCoords.top());
            g.putBitmap(xOffc+coords_.dg0XCoords.x(),0+coords_.dg0XCoords.y(),coords_.noCoords.width(),coords_.noCoords.height(), bitmap,
                track0X*coords_.noCoords.width(), coords_.noCoords.top());

            // blit the mute LED
            if (  pView->pSong()->_trackMuted[it->first]) {
                g.putBitmap(xOffc+coords_.dMuteCoords.x(),0+coords_.dMuteCoords.y(),coords_.sMuteCoords.width(),coords_.sMuteCoords.height(), bitmap,
                    coords_.sMuteCoords.left(), coords_.sMuteCoords.top());
            }

            // blit the solo LED
            if ( pView->pSong()->_trackSoloed == it->first) {
                g.putBitmap(xOffc+coords_.dSoloCoords.x(),0+coords_.dSoloCoords.y(),coords_.sSoloCoords.width(),coords_.sSoloCoords.height(), bitmap,
                    coords_.sSoloCoords.left(), coords_.sSoloCoords.top());
            }

            // blit the record LED
            if ( pView->pSong()->_trackArmed[it->first]) {
                g.putBitmap(xOffc+coords_.dRecCoords.x(),0+coords_.dRecCoords.y(),coords_.sRecCoords.width(),coords_.sRecCoords.height(), bitmap,
                    coords_.sRecCoords.left(), coords_.sRecCoords.top());
            }
        }
        g.putBitmap(xOff, 0,patNav.width(),patNav.height(), patNav, 
            0, 0);

        if (it->first!=0) g.drawLine( xOff, 0, xOff, clientHeight()); // col seperator
    }*/
}

/*void Header::onMousePress( int x, int y, int button )
{
    DefaultBitmaps & icons = SkinReader::Instance()->bitmaps();
    // check if left mouse button pressed
    if (button == 1)
    {
        // determine the track column, the mousepress occured on
        int track = pView->drawArea->findTrackByScreenX( x );

        // find out the start offset of the header bitmap
        ngrs::Bitmap & patNav = icons.patNav();
        ngrs::Point off( patNav.width()+ pView->drawArea->xOffByTrack( track ) + (pView->drawArea->trackWidth( track ) - patNav.width() - skinColWidth()) / 2,0);
        // find out the start offset of the nav buttons
        ngrs::Point navOff( pView->drawArea->xOffByTrack( track ),0); 

        ngrs::Rect decCol( navOff.x() , navOff.y(), 10, patNav.height() );
        ngrs::Rect incCol( navOff.x() +10, navOff.y(), 10, patNav.height() );
        ngrs::Rect xCol( navOff.x() +20, navOff.y(), 10, patNav.height() );

        // the rect area of the solo led
        ngrs::Rect solo(off.x() + coords_.dSoloCoords.x(), off.y() + coords_.dSoloCoords.y(), coords_.sSoloCoords.width(), coords_.sSoloCoords.height());

        std::map<int, TrackGeometry>::const_iterator it;
        it = pView->trackGeometrics().lower_bound( track );

        if (incCol.intersects(x,y)) {
            int visibleEvents = pView->drawArea->visibleEvents( track );
            pView->drawArea->setVisibleEvents( track,  visibleEvents +1);
            pView->updateRange();
            pView->repaint();
        } else 
            if (decCol.intersects(x,y)) {
                int visibleEvents = pView->drawArea->visibleEvents( track );
                pView->drawArea->setVisibleEvents( track, std::max(1,visibleEvents -1));
                pView->updateRange();
                pView->repaint();
            } else
                if (xCol.intersects(x,y)) {
                    int visibleEvents = pView->drawArea->visibleEvents( track );
                    pView->drawArea->setVisibleEvents( track, 1);
                    pView->updateRange();
                    pView->repaint();
                }

                // now check point intersection for solo
                if (solo.intersects(x,y)) {
                    onSoloLedClick(track);
                } else {
                    // the rect area of the solo led
                    ngrs::Rect mute(off.x() + coords_.dMuteCoords.x(), off.y() + coords_.dMuteCoords.y(), coords_.sMuteCoords.width(), coords_.sMuteCoords.height());
                    // now check point intersection for solo
                    if (mute.intersects(x,y)) {
                        onMuteLedClick(track);
                    } else
                    {
                        // the rect area of the record led
                        ngrs::Rect record(off.x() + coords_.dRecCoords.x(), off.y() + coords_.dRecCoords.y(), coords_.sRecCoords.width(), coords_.sRecCoords.height());
                        // now check point intersection for solo
                        if (record.intersects(x,y)) {
                            onRecLedClick(track);
                        }
                    }
                }
    }
}

void Header::onSoloLedClick( int track )
{
    if ( pView->pSong()->_trackSoloed != track )
    {
        for ( int i=0;i<MAX_TRACKS;i++ ) {
            pView->pSong()->_trackMuted[i] = true;
        }
        pView->pSong()->_trackMuted[track] = false;
        pView->pSong()->_trackSoloed = track;
    }
    else
    {
        for ( int i=0;i<MAX_TRACKS;i++ )
        {
            pView->pSong()->_trackMuted[i] = false;
        }
        pView->pSong()->_trackSoloed = -1;
    }
    repaint();
}

void Header::onMuteLedClick( int track )
{
    pView->pSong()->_trackMuted[track] = !(pView->pSong()->_trackMuted[track]);
    repaint();
}

void Header::onRecLedClick(int track) {
    pView->pSong()->_trackArmed[track] = ! pView->pSong()->_trackArmed[track];
    pView->pSong()->_trackArmedCount = 0;
    for ( int i=0;i<MAX_TRACKS;i++ ) {
        if ( pView->pSong()->_trackArmed[i] )
        {
            pView->pSong()->_trackArmedCount++;
        }
    }
    repaint();
}*/
