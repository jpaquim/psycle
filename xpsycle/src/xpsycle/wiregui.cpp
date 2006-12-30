/***************************************************************************
  *   Copyright (C) 2006 by Stefan   *
  *   natti@linux   *
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
#include "wiregui.h"
#include "skinreader.h"
#include "bendedlineshape.h"
#include "wiredlg.h"
#include <ngrs/npopupmenu.h>
#include <ngrs/nmenuitem.h>
#include <ngrs/napp.h>
#include <cmath>
#include <cstdio>

#ifdef _MSC_VER
#undef min 
#undef max
#endif

namespace psycle { 
	namespace host 
	{

		template<typename X, typename Y>
			inline double min (X x,Y y) {
    	return std::min((double)(x),(double)(y));
		}

		template<typename X, typename Y>
		inline double max (X x,Y y) { 
			return std::max((double)(x),(double)(y));
		}


		WireGUI::WireGUI()
			: NVisualComponent()
		{
            lineShape = new BendedLineShape();
            lineShape->setClippingDistance(12);
            setGeometry( lineShape );  

			triangle_size_tall = 22+((23*2)/16);

			triangle_size_center = triangle_size_tall/2;
			triangle_size_wide = triangle_size_tall/2;
			triangle_size_indent = triangle_size_tall/6;

			updateSkin();		

			dlg = new WireDlg();
				dlg->setLine(this);
			add( dlg );

			initPopupMenu();
		}


		WireGUI::~WireGUI()
		{
         delete lineShape;
		}
				

        const NPoint & WireGUI::p1( ) const {
          return lineShape->p1();
        }

        const NPoint & WireGUI::p2( ) const
        {
          return lineShape->p2();
        }
                
		void WireGUI::insertBend( const NPoint & pt ) {
		  lineShape->insertBend( pt );		
		}                

        void WireGUI::setPoints( const NPoint & p1, const NPoint & p2 )
        {
          lineShape->setPoints(p1,p2);
        }

		void WireGUI::paint( NGraphics * g )
		{
			NPen pen;
			pen.setLineWidth(2);
			g->setPen(pen);

            g->setTranslation( g->xTranslation()-left(), g->yTranslation()-top() );
              
            // draw the bended lines           
            NPoint startPt = lineShape->p1();                        
            std::vector<NPoint>::const_iterator it = lineShape->bendPts().begin();
            for ( ; it < lineShape->bendPts().end(); it++ ) {
              NPoint pt = *it;
              g->drawLine( startPt.x(), startPt.y(), pt.x(), pt.y() );              
              startPt = pt;                         
            }                                                                                    
            g->drawLine( startPt.x(), startPt.y(), lineShape->p2().x(), lineShape->p2().y() );
            
            // draw the arrows with resetted pen ( normal line width )
            g->resetPen();
            startPt = lineShape->p1();                        
            it = lineShape->bendPts().begin();
            for ( ; it < lineShape->bendPts().end(); it++ ) {
              NPoint pt = *it;
              drawArrow( g, startPt, pt );             
              startPt = pt;                         
            }                                                                           		
			drawArrow( g, startPt, lineShape->p2() );

			g->setTranslation(g->xTranslation()+left(),g->yTranslation()+top());
		}

		void WireGUI::drawArrow( NGraphics * g , const NPoint & p1, const NPoint & p2 )
		{
			// Spaces between the end and startPoint of the Line

			double  ankathede    = ( p1.x() - p2.x() );
			double  gegenkathede = ( p1.y() - p2.y() );
			double  hypetenuse   = std::sqrt( ankathede*ankathede + gegenkathede*gegenkathede);


			double cos = ankathede / hypetenuse;
			double sin = gegenkathede / hypetenuse;

			int middleX = ( p1.x() + p2.x() ) / 2;
			int middleY = ( p1.y() + p2.y() ) / 2;

			double slope = atan2( sin, cos );
			int rtcol = 140 + abs( static_cast<int>( slope * 32 ) );

			double altslope=slope;
			if(altslope<-1.05)  altslope -= 2 * (altslope + 1.05);
			if(altslope>2.10) altslope -= 2 * (altslope - 2.10);
			int ltcol = 140 + abs((int)((altslope - 2.10) * 32));
			altslope=slope;
			if(altslope>0.79)  altslope -= 2 * (altslope - 0.79);
			if(altslope<-2.36)  altslope -= 2 * (altslope + 2.36);
			int btcol = 240 - abs((int)((altslope-0.79) * 32));


			NColor rtBrush((int) max(0, min(255, rtcol * deltaColR)),
										(int) max(0, min(255, rtcol * deltaColG)),
										(int) max(0, min(255, rtcol * deltaColB)));
			NColor ltBrush( (int) max(0, min(255, ltcol * deltaColR)),
									(int) max(0, min(255, ltcol * deltaColG)),
									(int) max(0, min(255, ltcol * deltaColB)));
			NColor btBrush( (int) max(0, min(255, btcol * deltaColR)),
									(int) max(0, min(255, btcol * deltaColG)),
									(int) max(0, min(255, btcol * deltaColB)));

			NColor polyInnardsColor((int) ( 192 * deltaColR ), (int) (192 * deltaColG) , (int) (192 * deltaColB) );
			NPoint pol[5];

			pol[0].setX( middleX -  (int) (cos    * triangle_size_center) );
			pol[0].setY( middleY -  (int) (sin    * triangle_size_center) );
			pol[1].setX( pol[0].x() + (int) (cos    * triangle_size_tall) );
			pol[1].setY( pol[0].y() + (int) (sin    * triangle_size_tall) );
			pol[2].setX( pol[0].x() - (int) (sin    * triangle_size_wide) );
			pol[2].setY( pol[0].y() + (int) (cos    * triangle_size_wide) );
			pol[3].setX( pol[0].x() + (int) (cos    * triangle_size_indent) );
			pol[3].setY( pol[0].y() + (int) (sin    * triangle_size_indent) );
			pol[4].setX( pol[0].x() + (int) (sin    * triangle_size_wide) );
			pol[4].setY( pol[0].y() - (int) (cos    * triangle_size_wide) );

			NPoint fillPoly[7];

			fillPoly[2].setX( pol[0].x() + (int) (2* cos * triangle_size_indent) );
			fillPoly[2].setY( pol[0].y() + (int) (2* sin * triangle_size_indent) );
			fillPoly[6].setX( fillPoly[2].x() );
			fillPoly[6].setY( fillPoly[2].y() );
			fillPoly[1].setX( pol[1].x() );
			fillPoly[1].setY( pol[1].y() );
			fillPoly[0].setX( pol[2].x() );
			fillPoly[0].setY( pol[2].y() );
			fillPoly[5].setX( pol[2].x() );
			fillPoly[5].setY( pol[2].y() );
			fillPoly[4].setX( pol[3].x() );
			fillPoly[4].setY( pol[3].y() );
			fillPoly[3].setX( pol[4].x() );
			fillPoly[3].setY( pol[4].y() );

			g->setForeground(rtBrush);
			g->fillPolygon(fillPoly,3);
			g->setForeground(ltBrush);
			g->fillPolygon(&fillPoly[1],3);
			g->setForeground(btBrush);
			g->fillPolygon(&fillPoly[3], 4);

			g->setForeground(polyInnardsColor);
			g->drawPolygon(fillPoly,3);
			g->drawPolygon(&fillPoly[1],3);
			g->drawPolygon(&fillPoly[3], 4);

			g->setForeground( borderColor_ );
			g->drawPolygon(&pol[1], 4);
		}

		void WireGUI::updateSkin( )
		{
			setForeground( SkinReader::Instance()->machineview_color_info().wire_bg_color );

			polyColor_ = SkinReader::Instance()->machineview_color_info().wire_poly_color;

			deltaColR = ( polyColor_.red()   / 510.0) + .45;
			deltaColG = ( polyColor_.green() / 510.0) + .45;
			deltaColB = ( polyColor_.blue()  / 510.0) + .45;

			borderColor_ = SkinReader::Instance()->machineview_color_info().wire_arrow_border_color;
		}


		void WireGUI::initPopupMenu() {
			menu_ = new NPopupMenu();
			NMenuItem* item;
			item = new NMenuItem("add Bend");
			item->click.connect(this,&WireGUI::onAddBend);
			menu_->add( item );
			item = new NMenuItem("remove Connection");
			item->click.connect(this,&WireGUI::onRemoveMe);
			menu_->add( item );
			add( menu_ );                                 
		}

		void WireGUI::onAddBend( NButtonEvent* ev ) {
			insertBend( newBendPos_ );
			repaint();
			bendAdded.emit( this );
			setMoveable( NMoveable( nMvPolygonPicker ) );
		}
		
		void WireGUI::onRemoveMe( NButtonEvent* ev ) {
            removeMe.emit( this );
        }

		void WireGUI::onMousePress( int x, int y, int button ) {
      		int shift = NApp::system().shiftState();      
			if ( (shift &  nsRight) && !(shift & nsCtrl) ) {
				// display right click popup menu
				newBendPos_.setXY( left() + x, top() + y );

				menu_->setPosition( x + absoluteLeft() + window()->left(), y + absoluteTop() + window()->top(), 100,100);
				menu_->setVisible( true ); 
			} else
			if ( (shift &  nsRight) && (shift & nsCtrl) ) {
				// start here rewire 
				int distToSquareP1 = ( left() + x - p1().x() )*( left() + x - p1().x() ) + ( top() + y - p1().y() )*( top() + y - p1().y() );
				int distToSquareP2 = ( left() + x - p2().x() )*( left() + x - p2().x() ) + ( top() + y - p2().y() )*( top() + y - p2().y() );
				if ( distToSquareP1 < distToSquareP2 )
                  setMoveFocus(0);
				else
				  setMoveFocus(1);				
			}
			setMoveable( NMoveable( nMvPolygonPicker ) );        
			repaint();        
		}
                                        
		void WireGUI::onMouseDoublePress (int x, int y, int button) {
			if ( button==1 ) {
				dlg->setVisible(true);
			}
		}
                      
		WireDlg* WireGUI::dialog() {
			return dlg;
		}

	} // end of host namespace
} // end of psycle namespace

