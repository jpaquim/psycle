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
#include "psy4filter.h"
#include "fileio.h"

template<class T> inline T str(const std::string &  value) {
   T result;

   std::stringstream str;
   str << value;
   str >> result;

   return result;
}

template<class T> inline T str_hex(const std::string &  value) {
   T result;

   std::stringstream str;
   str << value;
   str >> std::hex >> result;

   return result;
}


namespace psycle {
	namespace host {


		Psy4Filter::Psy4Filter()
			: PsyFilter()
		{
			song_ = 0;
		}


		Psy4Filter::~Psy4Filter()
		{
		}

		int psycle::host::Psy4Filter::version( ) const
		{
			return 4;
		}

		bool Psy4Filter::testFormat( const std::string & fileName )
		{
			NXmlParser parser;
			isPsy4 = false;
			parser.tagParse.connect(this,&Psy4Filter::onDetectFilterTag);
			parser.parseFile(fileName);
			return isPsy4;
		}

		void Psy4Filter::onDetectFilterTag( const std::string & tagName )
		{
			if (tagName == "psy4") isPsy4 = true;
		}

		bool Psy4Filter::load( const std::string & fileName, Song & song )
		{
			song.patternSequence()->patternData()->removeAll();
			song.patternSequence()->removeAll();

			song_ = &song;
			lastCategory = 0;
			lastPattern  = 0;
			lastSeqLine  = 0;
			std::cout << "psy4filter detected for load" << std::endl;
			parser.tagParse.connect(this,&Psy4Filter::onTagParse);
			parser.parseFile(fileName);
			return isPsy4;
			return false;
		}

		void Psy4Filter::onTagParse( const std::string & tagName )
		{
			if (tagName == "category") {
				std::string catName = parser.getAttribValue(tagName);
				lastCategory = song_->patternSequence()->patternData()->createNewCategory("name");
			} else
			if (tagName == "pattern" && lastCategory) {
				std::string patName = parser.getAttribValue("name");
				int beatZoom = str_hex<int> (parser.getAttribValue("zoom"));
				lastPattern = lastCategory->createNewPattern(patName);
				lastPattern->clearBars();
				lastPattern->setBeatZoom(beatZoom);
				int pat_id  = str_hex<int> (parser.getAttribValue("id"));
				lastPattern->setID(pat_id);
			} else
			if (tagName == "patline" && lastPattern) {
				lastPatternPos = str<float> (parser.getAttribValue("pos"));
			} else
			if (tagName == "sign" && lastPattern) {
				std::string freeStr = parser.getAttribValue("free");
				if (freeStr != "") {
					float free = str<float> (freeStr);
					TimeSignature sig(free);
					lastPattern->addBar(sig);
				} else {
					int num = str<int> (parser.getAttribValue("num"));
					int denom = str<int> (parser.getAttribValue("denom"));
					int count = str<int> (parser.getAttribValue("count"));
					TimeSignature sig(num,denom);
					sig.setCount(count);
					lastPattern->addBar(sig);
				}
			} else
			if (tagName == "patevent" && lastPattern) {
					int trackNumber = str_hex<int> (parser.getAttribValue("track"));

					PatternEvent data;
					data.setMachine( str_hex<int> (parser.getAttribValue("mac")) );
					data.setInstrument( str_hex<int> (parser.getAttribValue("inst")) );
					data.setNote( str_hex<int> (parser.getAttribValue("note")) );
					data.setParameter( str_hex<int> (parser.getAttribValue("param")) );
					data.setParameter( str_hex<int> (parser.getAttribValue("cmd")) );

					(*lastPattern)[lastPatternPos][trackNumber]=data;
			} else
			if (tagName == "seqline") {
				lastSeqLine = song_->patternSequence()->createNewLine();
			} else 
			if (tagName == "seqentry" && lastSeqLine) {
				double pos =  str<double> (parser.getAttribValue("pos"));
				int pat_id  = str_hex<int> (parser.getAttribValue("patid"));
				float startPos = str<float> (parser.getAttribValue("start"));
				float endPos = str<float> (parser.getAttribValue("end"));
				SinglePattern* pattern = song_->patternSequence()->patternData()->findById(pat_id);
				if (pattern) {
					SequenceEntry* entry = lastSeqLine->createEntry(pattern, pos);
					entry->setStartPos(startPos);
					entry->setEndPos(endPos);
				}
			}
		}

		bool Psy4Filter::save( const std::string & fileName, const Song & song )
		{
			bool autosave = false;

			_stream.open(fileName.c_str (), std::ios_base::out | std::ios_base::trunc |std::ios_base::binary);
			if (!_stream.is_open ()) return false;
			_stream.seekg (0, std::ios::beg);
			

			std::ostringstream xml;
			xml << "<psy4>" << std::endl;
			xml << "<info>" << std::endl;
			xml << "<name   text='" << song.name()    << "' />" << std::endl;
			xml << "<author text='" << song.author()  << "' />" << std::endl;;
			xml << "<coment text='" << song.comment() << "' />" << std::endl;;
			xml << "</info>" << std::endl;
			xml << "<global>" << std::endl;
			xml << "</global>" << std::endl;
			xml << song.patternSequence().patternData().toXml();
			xml << song.patternSequence().toXml();
			xml << "<machines>" << std::endl;
			for(std::uint32_t index(0) ; index < MAX_MACHINES; ++index)
			{
				if (song._pMachine[index])
				{
					xml << song._pMachine[index]->toXml();
					if ( !autosave )
					{
						progress.emit(4,-1,"");
					}
				}
			}
			xml << "</machines>" << std::endl;
			xml << "<instruments>" << std::endl;
			xml << "</instruments>" << std::endl;
			xml << "</psy4>" << std::endl;

			_stream << xml.str() << std::endl;
			_stream.close();
		}

	}
}
