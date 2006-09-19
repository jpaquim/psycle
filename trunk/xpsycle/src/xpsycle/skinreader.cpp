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
#include "skinreader.h"
#include "zipreader.h"
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>
#include <ngrs/nfile.h>
#include <sys/types.h>
#include <sys/stat.h>

namespace psycle { 
	namespace host {


		// skin default for memparse




		SkinReader::SkinReader()
		{
		}


		SkinReader::~SkinReader()
		{
		}


		void SkinReader::setDefaults() {
			std::string mem;			

			mem +="<psyskin>";
			mem +="<info name='natti' year='2006' copyright='none' />";

			mem +="<patternview>";

			mem +="<cursor bgcolor='179:217:34' sel_bgcolor='100:100:100' />";
			mem +="<bar bgcolor='70:71:69' sel_bgcolor='200:200:200'/>";
			mem +="<beat bgcolor='50:51:49' sel_bgcolor='10:220:10' textcolor='199:199:199'/>";
			mem +="<lines bgcolor='34:32:35' sel_bgcolor='140:68:41' textcolor='255:255:255'/>";
			mem +="<playbar bgcolor='132:162:30' sel_bgcolor='0:0:200'/>";
			mem +="<bigtrackseparator bgcolor='145:147:147' />";
			mem +="<lineseparator enable='0' bgcolor='145:147:147'/>";
			mem +="<colseparator enable='0' bgcolor='145:147:147'/>";
			mem +="</patternview>";
			mem +="</psyskin>";

			NXmlParser parser;
			parser.tagParse.connect( this, &SkinReader::onTagParse );
			parser.parseString( mem );
		
		}
		
		bool SkinReader::loadSkin( const std::string & fileName )
		{
		/*	// open the zip file
			zipreader *z;
			zipreader_file *f;
			int fd = open( fileName.c_str(), O_RDONLY );
			z = zipreader_open( fd );

			// extract it to a temp file			
			int outFd = open(std::string("psyskintemp.xml").c_str(), O_RDWR|O_CREAT, 0666);
			f = zipreader_seek(z, "xml/skin.xml");

			if (!zipreader_extract(f, outFd )) {
				zipreader_close( z );	
				close( outFd );
				close( fd );
				return false;
			}			
			close( outFd );			
			zipreader_close( z );
			close( fd );*/

			// now start the xml parser

			parsePatView = false;

			NXmlParser parser;
			parser.tagParse.connect( this, &SkinReader::onTagParse );
			parser.parseFile("/home/natti/psyskintemp.xml");
			return true;
		}


		void SkinReader::onTagParse( const NXmlParser & parser, const std::string & tagName )
		{
			if ( tagName == "patternview" ) {
				parsePatView = true;
			} else 
			if ( tagName == "selection" && parsePatView ) {
				std::string selcolor = parser.getAttribValue("bgcolor");
				if ( selcolor!= "") {
					patview_sel_bg_color_ = NColor( selcolor );
				}
			} else
			if ( tagName == "cursor" && parsePatView ) {
				std::string bgcolor = parser.getAttribValue("bgcolor");
				if ( bgcolor != "" ) {
					patview_cursor_bg_color_ = NColor( bgcolor );
				}
				bgcolor = parser.getAttribValue("sel_bgcolor");
				if ( bgcolor != "" ) {
					patview_sel_cursor_bg_color_ = NColor( bgcolor );
				}
			} else
			if ( tagName == "bar" && parsePatView ) {
				std::string bgcolor = parser.getAttribValue("bgcolor");
				if ( bgcolor != "" ) {
					patview_bar_bg_color_ = NColor( bgcolor );
				}
				bgcolor = parser.getAttribValue("sel_bgcolor");
				if ( bgcolor != "" ) {
					patview_sel_bar_bg_color_ = NColor( bgcolor );
				}
			} else
			if ( tagName == "beat" && parsePatView ) {
				std::string bgcolor = parser.getAttribValue("bgcolor");
				if ( bgcolor != "" ) {
					patview_beat_bg_color_ = NColor( bgcolor );
				}
				bgcolor = parser.getAttribValue("sel_bgcolor");
				if ( bgcolor != "" ) {
					patview_sel_beat_bg_color_ = NColor( bgcolor );
				}
				bgcolor = parser.getAttribValue("textcolor");
				if ( bgcolor != "" ) {
					patview_beat_text_color_ = NColor( bgcolor );
				}
			} else
			if ( tagName == "lines" && parsePatView ) {
				std::string bgcolor = parser.getAttribValue("bgcolor");
				if ( bgcolor != "" ) {
					patview_bg_color_ = NColor( bgcolor );
				}
				bgcolor = parser.getAttribValue("sel_bgcolor");
				if ( bgcolor != "" ) {
					patview_sel_bg_color_ = NColor( bgcolor );
				}
				bgcolor = parser.getAttribValue("textcolor");
				if ( bgcolor != "" ) {
					patview_text_color_ = NColor( bgcolor );
				}
			} else
			if ( tagName == "playbar" && parsePatView ) {
				std::string bgcolor = parser.getAttribValue("bgcolor");
				if ( bgcolor != "" ) {
					patview_playbar_bg_color_ = NColor( bgcolor );
				}
				bgcolor = parser.getAttribValue("sel_bgcolor");
				if ( bgcolor != "" ) {
					patview_sel_playbar_bg_color_ = NColor( bgcolor );
				}
			} else
			if ( tagName == "bigtrackseparator" && parsePatView ) {
				std::string bgcolor = parser.getAttribValue("bgcolor");
				if ( bgcolor != "" ) {
					patview_track_big_sep_color_ = NColor( bgcolor );
				}
			} else
			if ( tagName == "lineseparator" && parsePatView ) {
				std::string bgcolor = parser.getAttribValue("bgcolor");
				if ( bgcolor != "" ) {
					patview_line_sep_color_ = NColor( bgcolor );
				}

				std::string enable = parser.getAttribValue("enable");
				if ( enable == "1" ) 
					patview_line_sep_enabled_ = 1; 
				else
					patview_line_sep_enabled_ = 0;
			}
			if ( tagName == "colseparator" && parsePatView ) {
				std::string bgcolor = parser.getAttribValue("bgcolor");
				if ( bgcolor != "" ) {
					patview_col_sep_color_ = NColor( bgcolor );
				}
				std::string enable = parser.getAttribValue("enable");
				if ( enable == "1" ) 
					patview_col_sep_enabled_ = 1; 
				else
					patview_col_sep_enabled_ = 0;
			}
		}

		// Patternview color`s
		
		const NColor & SkinReader::patview_cursor_bg_color( ) const
		{
			return patview_cursor_bg_color_;
		}

		const NColor & SkinReader::patview_bar_bg_color() const {
			return patview_bar_bg_color_;
		}

		const NColor & SkinReader::patview_beat_bg_color() const {
			return patview_beat_bg_color_;
		}

		const NColor & SkinReader::patview_beat_text_color() const {
			return patview_beat_text_color_;
		}

		const NColor & SkinReader::patview_bg_color() const {
			return patview_bg_color_;
		}

		const NColor & SkinReader::patview_playbar_bg_color() const {
			return patview_playbar_bg_color_;
		}

		const NColor & SkinReader::patview_track_big_sep_color() const {
			return patview_track_big_sep_color_;
		}

		const NColor & SkinReader::patview_line_sep_color() const {
			return patview_line_sep_color_;
		}

		const NColor & SkinReader::patview_col_sep_color() const {
			return patview_col_sep_color_;
		}

		bool SkinReader::patview_line_sep_enabled() const {
			return patview_line_sep_enabled_;
		}

		bool SkinReader::patview_col_sep_enabled() const {
			return patview_col_sep_enabled_;
		}

		const NColor & SkinReader::patview_text_color() const {
			return patview_text_color_;
		}

		// with selection

		const NColor & SkinReader::patview_sel_cursor_bg_color( ) const
		{
			return patview_sel_cursor_bg_color_;
		}

		const NColor & SkinReader::patview_sel_bar_bg_color() const {
			return patview_sel_bar_bg_color_;
		}

		const NColor & SkinReader::patview_sel_beat_bg_color() const {
			return patview_sel_beat_bg_color_;
		}

		const NColor & SkinReader::patview_sel_bg_color() const {
			return patview_sel_bg_color_;
		}

		const NColor & SkinReader::patview_sel_playbar_bg_color() const {
			return patview_sel_playbar_bg_color_;
		}


 }
}
