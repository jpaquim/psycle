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
			ngrs::NRect bgCoords;
			ngrs::NRect noCoords;
			ngrs::NRect sRecCoords;
			ngrs::NPoint dRecCoords;
			ngrs::NRect sMuteCoords;
			ngrs::NPoint dMuteCoords;
			ngrs::NRect sSoloCoords;
			ngrs::NPoint dSoloCoords;
			ngrs::NPoint dgX0Coords;
			ngrs::NPoint dg0XCoords;
		};

		class MachineCoordInfo {
		public:
			ngrs::NRect bgCoords;
			ngrs::NRect sPan;
			ngrs::NPoint dNameCoords;

			ngrs::NRect muteCoords;
			ngrs::NRect soloCoords;
			ngrs::NRect dSoloCoords;
			ngrs::NRect dMuteCoords;
			ngrs::NRect dPan;
			ngrs::NRect dByPass;

			ngrs::NRect sVuPeak;
			ngrs::NRect sVu0;
			ngrs::NRect dVu;
		};

		class MachineViewColorInfo {
		public:
			ngrs::NColor pane_bg_color;
			ngrs::NColor wire_bg_color;
			ngrs::NColor wire_poly_color;
			ngrs::NColor wire_arrow_border_color;
			ngrs::NColor sel_border_color;
		};

		class PatternViewColorInfo {
		public:
			ngrs::NColor cursor_bg_color;
			ngrs::NColor cursor_text_color;
			ngrs::NColor bar_bg_color;
			ngrs::NColor beat_bg_color;
			ngrs::NColor bg_color;
			ngrs::NColor text_color;
			ngrs::NColor sel_text_color;
			ngrs::NColor beat_text_color;
			ngrs::NColor playbar_bg_color;
			ngrs::NColor track_big_sep_color;
			ngrs::NColor track_small_sep_color;
			ngrs::NColor line_sep_color;
			ngrs::NColor col_sep_color;
			ngrs::NColor sel_cursor_bg_color;
			ngrs::NColor sel_bar_bg_color;
			ngrs::NColor sel_beat_bg_color;
			ngrs::NColor sel_bg_color;
			ngrs::NColor sel_beat_text_color;
			ngrs::NColor sel_playbar_bg_color;
			ngrs::NColor restarea_bg_color;
		};

		class SequencerViewInfo {
		public:
			ngrs::NColor pane_bg_color;
			ngrs::NColor pane_text_color;
			ngrs::NColor pane_grid_color;
			ngrs::NColor pane_move_line_color;
			ngrs::NColor pane_play_line_color;
		};

		class VuMeterInfo {
		public:
			ngrs::NColor vu1;
			ngrs::NColor vu2;
			ngrs::NColor vu3;
		};

		class FrameMachineInfo {
		public:
			ngrs::NColor machineGUITopColor;
			ngrs::NColor machineGUIFontTopColor;
			ngrs::NColor machineGUIBottomColor;
			ngrs::NColor machineGUIFontBottomColor;

			ngrs::NColor machineGUIHTopColor;
			ngrs::NColor machineGUIHFontTopColor;
			ngrs::NColor machineGUIHBottomColor;
			ngrs::NColor machineGUIHFontBottomColor;

			ngrs::NColor machineGUITitleColor;
			ngrs::NColor machineGUITitleFontColor;
		};

		class SkinReader : public ngrs::NObject {
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

			// loads a skin from a psycle_skin zip archive			
			bool loadSkin( const std::string & fileName );
			// sets an internal default skin, that doesn`t require an extra file
			void setDefaults();

			// patternview settings

			const PatternViewColorInfo & patternview_color_info() const;			

			const HeaderCoordInfo & headerCoordInfo() const;

			int patview_track_big_sep_width() const;
			bool patview_line_sep_enabled() const;
			bool patview_col_sep_enabled() const;
			int patview_track_left_ident() const;
			int patview_track_right_ident() const;

			ngrs::NBitmap & patview_header_bitmap();

			// machineview settings

			ngrs::NPixmap & machines_bitmap();

			const MachineCoordInfo & machineview_master_coords() const;
			const MachineCoordInfo & machineview_effect_coords() const;
			const MachineCoordInfo & machineview_generator_coords() const;

			const MachineViewColorInfo & machineview_color_info() const;

			// Sequencerview

			const SequencerViewInfo & sequencerview_info() const;

			// FrameMachine

			const FrameMachineInfo & framemachine_info() const;

			// bitmaps

			DefaultBitmaps & defaultBitmaps();

		private:

			void onTagParse(const ngrs::NXmlParser & parser, const std::string & tagName);

			// flags for the onTagParse, to know, in which parent tag we are

			bool parseSequencerView;
			bool parsePatView;
			bool parsePatHeader;
			bool parseMachineView;
			bool parseMacMaster;
			bool parseMacEffect;
			bool parseMacGenerator;

			// this method extracts a bitmap out of the opened zip file and loads it into a ngrs::NBitmap
			ngrs::NBitmap extractAndLoadBitmap( const std::string & zip_path );

			// patternview stuff

			// coordinates for putImage
			HeaderCoordInfo headerCoords_;

			// transforms a "00:00:00:00" str into a nrect "00" any int value
			ngrs::NRect getCoords( const std::string & coord ) const;

			bool patview_line_sep_enabled_;
			bool patview_col_sep_enabled_;

			int patview_track_left_ident_;
			int patview_track_right_ident_;
			int patview_track_big_sep_width_;

			PatternViewColorInfo patternview_color_info_;

			// machine_view stuff

			MachineCoordInfo machineview_master_coords_;
			MachineCoordInfo machineview_effect_coords_;
			MachineCoordInfo machineview_generator_coords_;		

			MachineViewColorInfo machineview_color_info_;


			// FrameMachine stuff

			FrameMachineInfo framemachine_info_;

			// default Bitmaps
			DefaultBitmaps defaultBitmaps_;

			ngrs::NBitmap patview_header_bitmap_;
			ngrs::NPixmap machines_bitmap_;

			// our zipreader handle
			zipreader *z;

			SequencerViewInfo sequencerview_info_;

		};
	}
}

#endif
