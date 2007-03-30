/***************************************************************************
*   Copyright (C) 2006 by  Stefan Nattkemper   *
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
@author  Stefan Nattkemper
*/

#include "defaultbitmaps.h"
#include "zipreader.h"
#include <ngrs/rect.h>
#include <ngrs/point.h>
#include <ngrs/object.h>
#include <ngrs/xmlparser.h>
#include <ngrs/color.h>
#include <ngrs/bitmap.h>
#include <ngrs/pixmap.h>


namespace psycle { 
	namespace host {

		class HeaderCoordInfo {
		public:
			ngrs::Rect bgCoords;
			ngrs::Rect noCoords;
			ngrs::Rect sRecCoords;
			ngrs::NPoint dRecCoords;
			ngrs::Rect sMuteCoords;
			ngrs::NPoint dMuteCoords;
			ngrs::Rect sSoloCoords;
			ngrs::NPoint dSoloCoords;
			ngrs::NPoint dgX0Coords;
			ngrs::NPoint dg0XCoords;
		};

		class MachineCoordInfo {
		public:
			ngrs::Rect bgCoords;
			ngrs::Rect sPan;
			ngrs::NPoint dNameCoords;

			ngrs::Rect muteCoords;
			ngrs::Rect soloCoords;
			ngrs::Rect dSoloCoords;
			ngrs::Rect dMuteCoords;
			ngrs::Rect dPan;
			ngrs::Rect dByPass;

			ngrs::Rect sVuPeak;
			ngrs::Rect sVu0;
			ngrs::Rect dVu;
		};

		class MachineViewColorInfo {
		public:
			ngrs::Color pane_bg_color;
			ngrs::Color wire_bg_color;
			ngrs::Color wire_poly_color;
			ngrs::Color wire_arrow_border_color;
			ngrs::Color sel_border_color;
		};

		class PatternViewColorInfo {
		public:
			ngrs::Color cursor_bg_color;
			ngrs::Color cursor_text_color;
			ngrs::Color bar_bg_color;
			ngrs::Color beat_bg_color;
			ngrs::Color bg_color;
			ngrs::Color text_color;
			ngrs::Color sel_text_color;
			ngrs::Color beat_text_color;
			ngrs::Color playbar_bg_color;
			ngrs::Color track_big_sep_color;
			ngrs::Color track_small_sep_color;
			ngrs::Color line_sep_color;
			ngrs::Color col_sep_color;
			ngrs::Color sel_cursor_bg_color;
			ngrs::Color sel_bar_bg_color;
			ngrs::Color sel_beat_bg_color;
			ngrs::Color sel_bg_color;
			ngrs::Color sel_beat_text_color;
			ngrs::Color sel_playbar_bg_color;
			ngrs::Color restarea_bg_color;
		};

		class SequencerViewInfo {
		public:
			ngrs::Color pane_bg_color;
			ngrs::Color pane_text_color;
			ngrs::Color pane_grid_color;
			ngrs::Color pane_move_line_color;
			ngrs::Color pane_play_line_color;
		};

		class VuMeterInfo {
		public:
			ngrs::Color vu1;
			ngrs::Color vu2;
			ngrs::Color vu3;
		};

		class FrameMachineInfo {
		public:
			ngrs::Color machineGUITopColor;
			ngrs::Color machineGUIFontTopColor;
			ngrs::Color machineGUIBottomColor;
			ngrs::Color machineGUIFontBottomColor;

			ngrs::Color machineGUIHTopColor;
			ngrs::Color machineGUIHFontTopColor;
			ngrs::Color machineGUIHBottomColor;
			ngrs::Color machineGUIHFontBottomColor;

			ngrs::Color machineGUITitleColor;
			ngrs::Color machineGUITitleFontColor;
		};

		class SkinReader : public ngrs::Object {
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

			// machineview settings

			const MachineCoordInfo & machineview_master_coords() const;
			const MachineCoordInfo & machineview_effect_coords() const;
			const MachineCoordInfo & machineview_generator_coords() const;

			const MachineViewColorInfo & machineview_color_info() const;

			// Sequencerview

			const SequencerViewInfo & sequencerview_info() const;

			// FrameMachine

			const FrameMachineInfo & framemachine_info() const;

			// bitmaps

			DefaultBitmaps & bitmaps();

		private:

			void onTagParse(const ngrs::XmlParser & parser, const std::string & tagName);

			// flags for the onTagParse, to know, in which parent tag we are

			bool parseSequencerView;
			bool parsePatView;
			bool parsePatHeader;
			bool parseMachineView;
			bool parseMacMaster;
			bool parseMacEffect;
			bool parseMacGenerator;

			// this method extracts a bitmap out of the opened zip file and loads it into a ngrs::Bitmap
			ngrs::Bitmap extractAndLoadBitmap( const std::string & zip_path );

			// patternview stuff

			// coordinates for putImage
			HeaderCoordInfo headerCoords_;

			// transforms a "00:00:00:00" str into a nrect "00" any int value
			ngrs::Rect getCoords( const std::string & coord ) const;

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

			// our zipreader handle
			zipreader *z;

			SequencerViewInfo sequencerview_info_;

		};
	}
}

#endif
