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
#ifndef SKINREADER_H
#define SKINREADER_H

/**
@author Stefan Nattkemper
*/

#include <ngrs/nobject.h>
#include <ngrs/nxmlparser.h>
#include <ngrs/ncolor.h>

namespace psycle { 
	namespace host {

		class SkinReader : public NObject {
		// Singleton Pattern
		private:
	  	SkinReader();          
  		~SkinReader();
			SkinReader( SkinReader const & );
  		SkinReader& operator=(SkinReader const&);

		public:

			static SkinReader* Instance() {
					// use single threaded only
					static SkinReader s;
 					return &s; 
			}
		// Singleton pattern end

			bool loadSkin( const std::string & fileName );
			void setDefaults();

			// patternview settings
			
			const NColor & patview_cursor_bg_color() const;
			const NColor & patview_bar_bg_color() const;
			const NColor & patview_beat_bg_color() const;
			const NColor & patview_beat_text_color() const;
			const NColor & patview_bg_color() const;
			const NColor & patview_text_color() const;
			const NColor & patview_playbar_bg_color() const;
			const NColor & patview_track_big_sep_color() const;
			const NColor & patview_line_sep_color() const;
			bool patview_line_sep_enabled() const;
			const NColor & patview_col_sep_color() const;
			bool patview_col_sep_enabled() const;

			const NColor & patview_sel_cursor_bg_color() const;
			const NColor & patview_sel_bar_bg_color() const;
			const NColor & patview_sel_beat_bg_color() const;
			const NColor & patview_sel_bg_color() const;
			const NColor & patview_sel_playbar_bg_color() const;
			

		private:

			void onTagParse(const NXmlParser & parser, const std::string & tagName);

			bool parsePatView;

			// patternview stuff
			NColor patview_cursor_bg_color_;
			NColor patview_bar_bg_color_;
			NColor patview_beat_bg_color_;
			NColor patview_bg_color_;
			NColor patview_text_color_;
			NColor patview_beat_text_color_;
			NColor patview_playbar_bg_color_;
			NColor patview_track_big_sep_color_;
			NColor patview_line_sep_color_;
			bool patview_line_sep_enabled_;
			NColor patview_col_sep_color_;
			bool patview_col_sep_enabled_;

			NColor patview_sel_cursor_bg_color_;
			NColor patview_sel_bar_bg_color_;
			NColor patview_sel_beat_bg_color_;
			NColor patview_sel_bg_color_;
			NColor patview_sel_playbar_bg_color_;

		};
	}
}

#endif
