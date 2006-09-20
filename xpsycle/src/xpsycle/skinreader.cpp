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
			z = 0;

			std::string mem;			

			mem +="<psyskin>";
			mem +="<info name='natti' year='2006' copyright='none' />";

			mem +="<patternview>";

			mem +="<cursor bgcolor='179:217:34' textcolor='0:0:0' />";
			mem +="<bar bgcolor='70:71:69' sel_bgcolor='162:101:68'/>";
			mem +="<beat bgcolor='50:51:49' sel_bgcolor='142:81:48' textcolor='199:199:199' sel_textcolor='216:154:120'/>";
			mem +="<lines bgcolor='34:32:35' sel_bgcolor='140:68:41' textcolor='255:255:255' sel_textcolor='239:175:140'/>";
			mem +="<playbar bgcolor='42:47:39' sel_bgcolor='0:0:200'/>";
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
			mem +="<solo_on_dest coord='97:3'/>";
			mem +="</header>";
			mem +="</patternview>";
			mem +="</psyskin>";

			patview_track_left_ident_ = 0;
			patview_track_right_ident_ = 0;

			NXmlParser parser;
			parser.tagParse.connect( this, &SkinReader::onTagParse );
			parser.parseString( mem );
		}
		
		bool SkinReader::loadSkin( const std::string & fileName )
		{
		 // open the zip file
			z = 0;
			zipreader_file *f;
			int fd = open( fileName.c_str(), O_RDONLY );
			z = zipreader_open( fd );

			if ( z) std::cout << "opened skin file :" << fileName << std::endl;

			// extract it to a temp file			
			int outFd = open(std::string("/home/natti/psyskintemp.xml").c_str(), O_RDWR|O_CREAT, 0666);
			f = zipreader_seek(z, "psycle_skin/xml/main.xml");

			if (!zipreader_extract(f, outFd )) {
				std::cout << "error at extracting" << std::endl;
				zipreader_close( z );	
				close( outFd );
				close( fd );
				return false;
			}			

			close( outFd );			

			// now start the xml parser

			parsePatView = false;
			parsePatHeader = false;

			NXmlParser parser;
			parser.tagParse.connect( this, &SkinReader::onTagParse );
			parser.parseFile("/home/natti/psyskintemp.xml");

			zipreader_close( z );
			close( fd );

			return true;
		}


		NRect SkinReader::getCoords( const std::string & coord ) const
		{
			int left = 0; 
			int top = 0;
			int width = 0;
			int height = 0;
			unsigned int i = 0;
			int start = 0;
			std::string substr;
			int c = 0;
			do {
				i = coord.find(':', i);
				if (i != std::string::npos) {
					substr = coord.substr(start,i-start);
					i+=1;
					start = i;
				} else substr = coord.substr(start);
				std::stringstream str; str << substr; int value = 0; str >> value;
				if (c==0) left  = value;
				if (c==1) top   = value;
				if (c==2) width = value;
				if (c==3) height = value;
				c++;
			} while (i != std::string::npos);

  		return NRect( left, top, width, height );
		}

		NBitmap SkinReader::extractAndLoadBitmap( const std::string & zip_path ) {
			
			std::cout << "extracting bitmap" << std::endl;
			// extract bitmap to a temp file			
			std::string fileName = NFile::extractFileNameFromPath( zip_path );

			std::string bitmapPath = zip_path;
			std::cout << "try to read bitmap with path :" << bitmapPath;

			int outFd = open(std::string( ("temp")+fileName).c_str(), O_RDWR|O_CREAT, 0666);
			zipreader_file *f = zipreader_seek(z, bitmapPath.c_str() );

			if (!zipreader_extract(f, outFd )) {
				std::cout << "bitmap path in skin zip not found" << std::endl;
				close( outFd );
				return NBitmap();
			}			
			close( outFd );			

			NBitmap bitmap;
			bitmap.loadFromFile( std::string("temp")+fileName );
			return bitmap;
		}

		void SkinReader::onTagParse( const NXmlParser & parser, const std::string & tagName )
		{
			if ( tagName == "patternview" ) {
				parsePatView = true;
			} else 
			if ( tagName == "header" && parsePatView ) {
				parsePatHeader = true;
				std::string src = parser.getAttribValue("src");
				if (src != "")	{
					patview_header_bitmap_ = extractAndLoadBitmap( src );
				}
			} else 
			if ( tagName == "background_source" && parsePatHeader ) {
				std::string coord = parser.getAttribValue("coord");
				headerCoords_.bgCoords = getCoords( coord );
				std::cout << "hre" << headerCoords_.bgCoords.height() << std::endl;
			} else
			if ( tagName == "number_0_source" && parsePatHeader ) {
				std::string coord = parser.getAttribValue("coord");
				headerCoords_.noCoords = getCoords( coord );
			} else
			if ( tagName == "record_on_source" && parsePatHeader ) {
				std::string coord = parser.getAttribValue("coord");
				headerCoords_.sRecCoords = getCoords( coord );
			} else
			if ( tagName == "mute_on_source" && parsePatHeader ) {
				std::string coord = parser.getAttribValue("coord");
				headerCoords_.sMuteCoords = getCoords( coord );
			} else
			if ( tagName == "solo_on_source" && parsePatHeader ) {
				std::string coord = parser.getAttribValue("coord");
				headerCoords_.sSoloCoords = getCoords( coord );
			} else
			if ( tagName == "digit_x0_dest" && parsePatHeader ) {
				std::string coord = parser.getAttribValue("coord");
				NRect rect = getCoords( coord );
				headerCoords_.dgX0Coords = NPoint( rect.left(), rect.top() );
			} else
			if ( tagName == "digit_0x_dest" && parsePatHeader ) {
				std::string coord = parser.getAttribValue("coord");
				NRect rect = getCoords( coord );
				headerCoords_.dg0XCoords = NPoint( rect.left(), rect.top() );
			} else
			if ( tagName == "record_on_dest" && parsePatHeader ) {
				std::string coord = parser.getAttribValue("coord");
				NRect rect = getCoords( coord );
				headerCoords_.dRecCoords = NPoint( rect.left(), rect.top() );
			} else
			if ( tagName == "mute_on_dest" && parsePatHeader ) {
				std::string coord = parser.getAttribValue("coord");
				NRect rect = getCoords( coord );
				headerCoords_.dMuteCoords = NPoint( rect.left(), rect.top() );
			} else
			if ( tagName == "solo_on_dest" && parsePatHeader ) {
				std::string coord = parser.getAttribValue("coord");
				NRect rect = getCoords( coord );
				headerCoords_.dSoloCoords = NPoint( rect.left(), rect.top() );
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
				bgcolor = parser.getAttribValue("textcolor");
				if ( bgcolor != "" ) {
					patview_cursor_text_color_ = NColor( bgcolor );
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
				bgcolor = parser.getAttribValue("sel_textcolor");
				if ( bgcolor != "" ) {
					patview_sel_beat_text_color_ = NColor( bgcolor );
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
				bgcolor = parser.getAttribValue("sel_textcolor");
				if ( bgcolor != "" ) {
					patview_sel_text_color_ = NColor( bgcolor );
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
			if ( tagName == "smalltrackseparator" && parsePatView ) {
				std::string bgcolor = parser.getAttribValue("bgcolor");
				if ( bgcolor != "" ) {
					patview_track_small_sep_color_ = NColor( bgcolor );
				}
			} else
			if ( tagName == "bigtrackseparator" && parsePatView ) {
				std::string bgcolor = parser.getAttribValue("bgcolor");
				if ( bgcolor != "" ) {
					patview_track_big_sep_color_ = NColor( bgcolor );
				}
				bgcolor = parser.getAttribValue("width");
				if ( bgcolor != "" ) {
					patview_track_big_sep_width_ = str<int>( bgcolor );
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
			if ( tagName == "trackident" && parsePatView ) {
				std::string leftIdent = parser.getAttribValue("left");
				if ( leftIdent != "") {
					patview_track_left_ident_ = str<int>( leftIdent );
				}
				std::string rightIdent = parser.getAttribValue("right");
				if ( rightIdent != "") {
					patview_track_right_ident_ = str<int>( rightIdent );
				}
			}
		}


		// Patternview

		const HeaderCoordInfo & SkinReader::headerCoordInfo() const {
				return headerCoords_;
		}

		// Patternview color`s
		
		const NColor & SkinReader::patview_cursor_bg_color( ) const
		{
			return patview_cursor_bg_color_;
		}

		const NColor & SkinReader::patview_cursor_text_color() const {
			return patview_cursor_text_color_;
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

		const NColor & SkinReader::patview_sel_beat_text_color() const {
			return patview_sel_beat_text_color_;
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

		const NColor & SkinReader::patview_track_small_sep_color() const {
			return patview_track_small_sep_color_;
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

		const NColor & SkinReader::patview_sel_text_color() const {
			return patview_sel_text_color_;
		}

		int SkinReader::patview_track_left_ident() const {
			return patview_track_left_ident_;
		}

		int SkinReader::patview_track_right_ident() const {
			return patview_track_right_ident_;
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

		int SkinReader::patview_track_big_sep_width() const {
			return patview_track_big_sep_width_;
		}

		NBitmap & SkinReader::patview_header_bitmap() {
			if ( patview_header_bitmap_.empty() )
				return defaultBitmaps.pattern_header_skin();
			else
				return patview_header_bitmap_;
		}

 }
}
