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
#include "zipwriter.h"
#include "zipwriterstream.h"
#include "zipreader.h"
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>
#include <ngrs/nfile.h>
#include <sys/types.h>
#include <sys/stat.h>


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
		{
			song_ = 0;
		}


		Psy4Filter::~Psy4Filter()
		{
		}

		int Psy4Filter::version( ) const
		{
			return 4;
		}

		bool Psy4Filter::testFormat( const std::string & fileName )
		{
			zipreader *z;
			zipreader_file *f;
			int fd = open( fileName.c_str(), O_RDONLY );
			z = zipreader_open( fd );
			int outFd = open(std::string("psytemp.xml").c_str(), O_RDWR|O_CREAT, 0666);
			f = zipreader_seek(z, "xml/song.xml");

			if (!zipreader_extract(f, outFd )) {
				zipreader_close( z );	
				close( outFd );
				close( fd );
				return false;
			}			
			close( outFd );

			f = zipreader_seek(z, "bin/song.bin");
			outFd = open(std::string("psytemp.bin").c_str(), O_RDWR|O_CREAT, 0666);
			if (!zipreader_extract(f, outFd )) {
				zipreader_close( z );	
				close( outFd );
				close( fd );
				return false;
			}			
			close( outFd );
			
			zipreader_close( z );
			close( fd );

			NXmlParser parser;
			isPsy4 = false;
			parser.tagParse.connect(this,&Psy4Filter::onDetectFilterTag);
			parser.parseFile("psytemp.xml");
			std::cout << "format is " << isPsy4 << std::endl;
			return isPsy4;		
		}

		void Psy4Filter::onDetectFilterTag( const NXmlParser & parser, const std::string & tagName )
		{
			if (tagName == "psy4") isPsy4 = true;
		}

		bool Psy4Filter::load( const std::string & fileName, Song & song )
		{
			NXmlParser parser;
			patMap.clear();

			song.patternSequence()->patternData()->removeAll();
			song.patternSequence()->removeAll();
			song.clear();
			song_ = &song;
			lastCategory = 0;
			lastPattern  = 0;
			lastSeqLine  = 0;
			lastMachine  = 0;
			std::cout << "psy4filter detected for load" << std::endl;
			parser.tagParse.connect(this,&Psy4Filter::onTagParse);
			parser.parseFile("psytemp.xml");


			if (true) {
				RiffFile file;
				file.Open("psytemp.bin");
				progress.emit(1,0,"");
				progress.emit(2,0,"Loading... psycle bin data ...");

				// skip header
				file.Skip(8);
				//\todo:
				size_t filesize = file.FileSize();
				std::uint32_t version = 0;
				std::uint32_t size = 0;
				char header[5];
				header[4]=0;
				std::uint32_t chunkcount = LoadSONGv0(&file,song);
				std::cout << chunkcount << std::endl;
			/* chunk_loop: */

				while(file.ReadChunk(&header, 4) && chunkcount)
				{
					file.Read(version);
					file.Read(size);

					int fileposition = file.GetPos();
					progress.emit(4,f2i((fileposition*16384.0f)/filesize),"");

					if(std::strcmp(header,"MACD") == 0)
					{
						progress.emit(2,0,"Loading... Song machines...");
						if ((version&0xFF00) == 0x0000) // chunkformat v0
						{
								LoadMACDv0(&file,song,version&0x00FF);
						}
						//else if ( (version&0xFF00) == 0x0100 ) //and so on
					}
					else if(std::strcmp(header,"INSD") == 0)
					{
						progress.emit(2,0,"Loading... Song instruments...");
						if ((version&0xFF00) == 0x0000) // chunkformat v0
						{
							LoadINSDv0(&file,song,version&0x00FF);
						}
						//else if ( (version&0xFF00) == 0x0100 ) //and so on
					}
					else
					{
//					loggers::warning("foreign chunk found. skipping it.");
						progress.emit(2,0,"Loading... foreign chunk found. skipping it...");
						std::ostringstream s;
							s << "foreign chunk: version: " << version << ", size: " << size;
//					loggers::trace(s.str());
						if(size && size < filesize-fileposition)
						{
							file.Skip(size);
						}
					}
					// For invalid version chunks, or chunks that haven't been read correctly/completely.
					if  (file.GetPos() != fileposition+size)
					{
					//\todo: verify how it works with invalid data.
//					if (file.GetPos() > fileposition+size) loggers::trace("Cursor ahead of size! resyncing with chunk size.");
//					else loggers::trace("Cursor still inside chunk, resyncing with chunk size.");
						file.Seek(fileposition+size);
					}
					--chunkcount;
				}
				progress.emit(4,16384,"");

				//\todo: Move this to something like "song.validate()" 

				// test all connections for invalid machines. disconnect invalid machines.
				for(int i(0) ; i < MAX_MACHINES ; ++i)
				{
					if(song._pMachine[i])
					{
						song._pMachine[i]->_connectedInputs = 0;
						song._pMachine[i]->_connectedOutputs = 0;
						for (int c(0) ; c < MAX_CONNECTIONS ; ++c)
						{
							if(song._pMachine[i]->_connection[c])
							{
								if(song._pMachine[i]->_outputMachines[c] < 0 || song._pMachine[i]->_outputMachines[c] >= MAX_MACHINES)
								{
									song._pMachine[i]->_connection[c] = false;
									song._pMachine[i]->_outputMachines[c] = -1;
								}
								else if(!song._pMachine[song._pMachine[i]->_outputMachines[c]])
								{
									song._pMachine[i]->_connection[c] = false;
									song._pMachine[i]->_outputMachines[c] = -1;
								}
								else 
								{
									song._pMachine[i]->_connectedOutputs++;
								}
							}
							else
							{
								song._pMachine[i]->_outputMachines[c] = -1;
							}

							if (song._pMachine[i]->_inputCon[c])
							{
								if (song._pMachine[i]->_inputMachines[c] < 0 || song._pMachine[i]->_inputMachines[c] >= MAX_MACHINES)
								{
									song._pMachine[i]->_inputCon[c] = false;
									song._pMachine[i]->_inputMachines[c] = -1;
								}
								else if (!song._pMachine[song._pMachine[i]->_inputMachines[c]])
								{
									song._pMachine[i]->_inputCon[c] = false;
									song._pMachine[i]->_inputMachines[c] = -1;
								}
								else
								{
									song._pMachine[i]->_connectedInputs++;
								}
							}
							else
							{
								song._pMachine[i]->_inputMachines[c] = -1;
							}
						}
					}
				}

				progress.emit(5,0,"");
				if(chunkcount)
				{
					std::ostringstream s;
					s << "Error reading from file '" << file.file_name() << "'" << std::endl;
					s << "some chunks were missing in the file";
					report.emit(s.str(), "Song Load Error.");
				}
				//\todo:
			}

			return isPsy4;
		}

		void Psy4Filter::onTagParse(const NXmlParser & parser, const std::string & tagName )
		{
			if ( tagName =="name" ) {
				std::string attrib = parser.getAttribValue( "text" );
				song_->setName( attrib );
			} else
			if ( tagName =="author" ) {
				std::string attrib = parser.getAttribValue( "text" );
				song_->setAuthor( attrib );
			} else
			if ( tagName =="comment" ) {
				std::string attrib = parser.getAttribValue( "text" );
				song_->setComment( attrib );
			} else
			if (tagName == "category") {
				std::string catName = parser.getAttribValue("name");
				std::string attrib = parser.getAttribValue("color");
				lastCategory = song_->patternSequence()->patternData()->createNewCategory(catName);
				lastCategory->setColor( str<long>( attrib ) );
			} else
			if (tagName == "pattern" && lastCategory) {
				std::string patName = parser.getAttribValue("name");
				int beatZoom = str_hex<int> (parser.getAttribValue("zoom"));
				lastPattern = lastCategory->createNewPattern(patName);
				lastPattern->clearBars();
				lastPattern->setBeatZoom(beatZoom);
				int pat_id  = str_hex<int> (parser.getAttribValue("id"));
				patMap[pat_id] = lastPattern;
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
					data.setSharp( str_hex<bool> (parser.getAttribValue("sharp")) );

					(*lastPattern)[lastPatternPos][trackNumber]=data;
			} else
			if (tagName == "seqline") {
				lastSeqLine = song_->patternSequence()->createNewLine();
				std::cout << "create seqline" << std::endl;
			} else 
			if (tagName == "seqentry" && lastSeqLine) {
				double pos =  str<double> (parser.getAttribValue("pos"));
				int pat_id  = str<int> (parser.getAttribValue("patid"));
				float startPos = str<float> (parser.getAttribValue("start"));
				float endPos = str<float> (parser.getAttribValue("end"));
				int transpose  = str_hex<int> (parser.getAttribValue("transpose"));
				std::map<int, SinglePattern*>::iterator it = patMap.begin();
				it = patMap.find( pat_id );
				if ( it != patMap.end() ) {
					SinglePattern* pattern = it->second; 
					if (pattern) {
						SequenceEntry* entry = lastSeqLine->createEntry(pattern, pos);
						entry->setStartPos(startPos);
						entry->setEndPos(endPos);
						entry->setTranspose(transpose);
					}
				}
			}
		}

		bool Psy4Filter::save( const std::string & file_Name, const Song & song )
		{
			std::string fileName = NFile::extractFileNameFromPath(file_Name);

			bool autosave = false;
			//\todo:
			if ( !autosave )
			{
				progress.emit(1,0,"");
				progress.emit(2,0,"Saving...");
			}

			// ideally, you should create a temporary file on the same physical
	  	// disk as the target zipfile... 

			zipwriter *z = zipwriter_start(open(fileName.c_str(), O_RDWR|O_CREAT, 0666));
			zipwriterfilestream xmlFile(z, "xml/song.xml" );

			std::ostringstream xml;
			xml << "<psy4>" << std::endl;
			xml << "<info>" << std::endl;
			xml << "<name   text='" << replaceIllegalXmlChr( song.name() ) << "' />" << std::endl;
			xml << "<author text='" << replaceIllegalXmlChr( song.author() ) << "' />" << std::endl;;
			xml << "<comment text='" << replaceIllegalXmlChr( song.comment() ) << "' />" << std::endl;;
			xml << "</info>" << std::endl;
			xml << song.patternSequence().patternData().toXml();
			xml << song.patternSequence().toXml();
			xml << "</psy4>" << std::endl;

			xmlFile << xml.str();
			xmlFile.close();


			zipwriter_file *f;


 			//\todo:
			if ( !autosave )
			{
				progress.emit(1,0,"");
				progress.emit(2,0,"Saving binary data...");
			}

			// we create here a temp file, cause our RiffFile is a fstream
			// and the zipwriter is only a ostream
			// modifiying the Rifffile makes it possible
			// to write direct into the zip without using a temp here

			RiffFile file;
			file.Create(std::string("psycle_tmp.bin").c_str(), true);

			file.WriteChunk("PSY4",4);
			saveSONGv0(&file,song);

			for(std::uint32_t index(0) ; index < MAX_MACHINES; ++index)
			{
				if (song._pMachine[index])
				{
					saveMACDv0(&file,song,index);
					if ( !autosave )
					{
						progress.emit(4,-1,"");
					}
				}
			}
			for(std::uint32_t index(0) ; index < MAX_INSTRUMENTS; ++index)
			{
				if (!song._pInstrument[index]->Empty())
				{
					saveINSDv0(&file,song,index);
					if ( !autosave )
					{
						progress.emit(4,-1,"");
					}
				}
			}
			//\todo:
			file.Close();

      // copy the bin data to the zip
      f = zipwriter_addfile(z, std::string("bin/song.bin").c_str(), 9);
	    zipwriter_copy(open("psycle_tmp.bin", O_RDONLY), f);

      if (!zipwriter_finish(z)) {
				return false;
			}

			// remove temp file
			 if( std::remove("psycle_tmp.bin") == -1 )
    			std::cerr << "Error deleting temp file" << std::endl;
  
         
		}

		int Psy4Filter::LoadSONGv0(RiffFile* file,Song& song)
		{
			std::uint32_t fileversion = 0;
			std::uint32_t size = 0;
			std::uint32_t chunkcount = 0;
			file->Read(fileversion);
			file->Read(size);
			if(fileversion > CURRENT_FILE_VERSION)
			{
				report.emit("This file is from a newer version of Psycle! This process will try to load it anyway.", "Load Warning");
			}
			file->Read(chunkcount);
			const int bytesread=4;
			if ( size > 4)
			{
				file->Skip(size - bytesread);// Size of the current header DATA // This ensures that any extra data is skipped.
			}
			return chunkcount;
		}

		bool Psy4Filter::saveSONGv0( RiffFile * file, const Song & song )
		{
			std::uint32_t chunkcount;
			std::uint32_t version, size;
			// chunk header;

			file->WriteChunk("SONG",4);
			version = FILE_VERSION;
			file->Write(version);
			size = sizeof chunkcount;
			file->Write(size);

			chunkcount = 3; // 3 chunks plus:
			for(unsigned int i(0) ; i < MAX_MACHINES    ; ++i)
					if(song._pMachine[i]) ++chunkcount;
			for(unsigned int i(0) ; i < MAX_INSTRUMENTS ; ++i)
				 if(!song._pInstrument[i]->Empty()) ++chunkcount;

			file->Write(chunkcount);

			return true;
		}

		bool Psy4Filter::saveMACDv0( RiffFile * file, const Song & song, int index )
		{
			std::uint32_t version, size;
			std::size_t pos;

			// chunk header

			file->WriteChunk("MACD",4);
			version = CURRENT_FILE_VERSION_MACD;
			file->Write(version);
			pos = file->GetPos();
			size = 0;
			file->Write(size);

			// chunk data

			file->Write(index);
			song._pMachine[index]->SaveFileChunk(file);

			// chunk size in header

			std::size_t const pos2 = file->GetPos();
			size = pos2 - pos - sizeof size;
			file->Seek(pos);
			file->Write(size);
			file->Seek(pos2);

			//\todo:
			return true;
		}

		bool Psy4Filter::saveINSDv0( RiffFile * file, const Song & song, int index )
		{
			std::uint32_t version, size;
			std::size_t pos;

			// chunk header

			file->WriteChunk("INSD",4);
			version = CURRENT_FILE_VERSION_INSD;
			file->Write(version);
			pos = file->GetPos();
			size = 0;
			file->Write(size);

			// chunk data

			file->Write(index);
			song._pInstrument[index]->SaveFileChunk(file);

			// chunk size in header

			std::size_t const pos2 = file->GetPos();
			size = pos2 - pos - sizeof size;
			file->Seek(pos);
			file->Write(size);
			file->Seek(pos2);

			//\todo:
			return true;
		}

		bool Psy4Filter::saveWAVEv0( RiffFile * file, const Song & song, int index )
		{
		}

	}
}


