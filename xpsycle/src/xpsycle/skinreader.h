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

#include "defaultbitmaps.h"
#include "zipreader.h"

#include <ngrs/nrect.h>
#include <ngrs/npoint.h>
#include <ngrs/nobject.h>
#include <ngrs/nxmlparser.h>
#include <ngrs/ncolor.h>
#include <ngrs/nbitmap.h>
#include <ngrs/npixmap.h>


namespace psycle { 
	namespace host {

		class HeaderCoordInfo {
		public:
			NRect bgCoords;
			NRect noCoords;
			NRect sRecCoords;
			NPoint dRecCoords;
			NRect sMuteCoords;
			NPoint dMuteCoords;
			NRect sSoloCoords;
			NPoint dSoloCoords;
			NPoint dgX0Coords;
			NPoint dg0XCoords;
		};

		class MachineCoordInfo {
		public:
			NRect bgCoords;
			NRect sPan;
			NPoint dNameCoords;

			NRect muteCoords;
			NRect soloCoords;
			NRect dSoloCoords;
			NRect dMuteCoords;
			NRect dPan;
			NRect dByPass;

			NRect sVuPeak;
			NRect sVu0;
			NRect dVu;

		};


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
		
			const HeaderCoordInfo & headerCoordInfo() const;
	
			const NColor & patview_cursor_bg_color() const;
			const NColor & patview_cursor_text_color() const;
			const NColor & patview_bar_bg_color() const;
			const NColor & patview_beat_bg_color() const;
			const NColor & patview_sel_beat_text_color() const;
			const NColor & patview_beat_text_color() const;
			const NColor & patview_bg_color() const;
			const NColor & patview_text_color() const;
			const NColor & patview_sel_text_color() const;
			const NColor & patview_playbar_bg_color() const;
			const NColor & patview_track_big_sep_color() const;
			const NColor & patview_track_small_sep_color() const;
			int patview_track_big_sep_width() const;
			const NColor & patview_line_sep_color() const;
			bool patview_line_sep_enabled() const;
			const NColor & patview_col_sep_color() const;
			bool patview_col_sep_enabled() const;

			const NColor & patview_sel_cursor_bg_color() const;
			const NColor & patview_sel_bar_bg_color() const;
			const NColor & patview_sel_beat_bg_color() const;
			const NColor & patview_sel_bg_color() const;
			const NColor & patview_sel_playbar_bg_color() const;

			int patview_track_left_ident() const;
			int patview_track_right_ident() const;

			NBitmap & patview_header_bitmap();

			NPixmap & machines_bitmap();

			// machineview settings

			const MachineCoordInfo & machineview_master_coords() const;
			const MachineCoordInfo & machineview_effect_coords() const;
			const MachineCoordInfo & machineview_generator_coords() const;

			
		private:

			void onTagParse(const NXmlParser & parser, const std::string & tagName);

			bool parsePatView;
			bool parsePatHeader;
			bool parseMachineView;
			bool parseMacMaster;
			bool parseMacEffect;
			bool parseMacGenerator;

			NBitmap extractAndLoadBitmap( const std::string & zip_path );

			// patternview stuff

			HeaderCoordInfo headerCoords_;
			// transforms a "00:00:00:00" str into a nrect "00" any int value
			NRect SkinReader::getCoords( const std::string & coord ) const;

			NColor patview_cursor_bg_color_;
			NColor patview_cursor_text_color_;
			NColor patview_bar_bg_color_;
			NColor patview_beat_bg_color_;
			NColor patview_bg_color_;
			NColor patview_text_color_;
			NColor patview_sel_text_color_;
			NColor patview_beat_text_color_;
			NColor patview_playbar_bg_color_;
			NColor patview_track_big_sep_color_;
			NColor patview_track_small_sep_color_;
			NColor patview_line_sep_color_;
			bool patview_line_sep_enabled_;
			NColor patview_col_sep_color_;
			bool patview_col_sep_enabled_;

			int patview_track_left_ident_;
			int patview_track_right_ident_;
			int patview_track_big_sep_width_;

			NColor patview_sel_cursor_bg_color_;
			NColor patview_sel_bar_bg_color_;
			NColor patview_sel_beat_bg_color_;
			NColor patview_sel_bg_color_;
			NColor patview_sel_beat_text_color_;
			NColor patview_sel_playbar_bg_color_;


			// machine_view stuff

			MachineCoordInfo machineview_master_coords_;
			MachineCoordInfo machineview_effect_coords_;
			MachineCoordInfo machineview_generator_coords_;

			// default Bitmaps
			DefaultBitmaps defaultBitmaps;

			NBitmap patview_header_bitmap_;
			NPixmap machines_bitmap_;

			// our zipreader handle
			zipreader *z;

		};
	}
}

#endif
