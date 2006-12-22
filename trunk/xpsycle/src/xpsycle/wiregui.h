/***************************************************************************
  *   Copyright (C) 2006 by Stefan Nattkemper   *
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
#ifndef WIREGUI_H
#define WIREGUI_H

#include <ngrs/nvisualcomponent.h>

/**
@author Stefan
*/

class BendedLineShape;
class NPopupMenu;

namespace psycle 
{ 
	namespace host
	{
		
		class WireDlg;

		class WireGUI : public NVisualComponent
		{
		public:
				WireGUI();

				~WireGUI();

				signal1<WireGUI*> bendAdded;

                const NPoint & p1() const;
                const NPoint & p2() const;

                void setPoints( const NPoint & p1, const NPoint & p2 );
                void addBend( const NPoint & pts );

				virtual void paint(NGraphics* g);
			
				void updateSkin();

				virtual void onMousePress  (int x, int y, int button);
				virtual void onMouseDoublePress (int x, int y, int button);
   
				WireDlg* dialog();
        
		private:

                BendedLineShape* lineShape;

				NPopupMenu* menu_;
				NPoint newBendPos_;
                
				NColor polyColor_;
				NColor borderColor_;

				WireDlg* dlg;

				float deltaColR;
				float deltaColG;
				float deltaColB;

				double triangle_size_center ;
				double triangle_size_tall;
				double triangle_size_wide;
				double triangle_size_indent;
				
				void drawArrow( NGraphics * g );
				void initPopupMenu( );

				void onAddBend( NButtonEvent* ev );  

		};
	}
}
#endif
