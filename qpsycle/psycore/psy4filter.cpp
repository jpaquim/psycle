/***************************************************************************
*   Copyright (C) 2007 Psycledelics Community
*   psycle.sf.net
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
#include "internal_machines.h"
#include "song.h"
#include "helpers/xml.h"

// FIXME: remove Qt usage.
#include <QFile>
#include <QDomDocument>
#include <QDomElement>
#include <QXmlInputSource>

#if defined __unix__
	#include <unistd.h>
	#include <sys/types.h>
#elif defined _WIN64 || defined _WIN32
	#include <io.h>
#endif

#include <fcntl.h>
#include <cstdlib>
#include <cerrno>
#include "file.h"
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


namespace psy {
	namespace core {
		Psy4Filter::Psy4Filter()
		:
			song_()
		{}

		bool Psy4Filter::testFormat( const std::string & fileName )
		{
			///\todo this creates a temporary file. need to find a way for all operations to be performed in ram
		
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

            QFile *file = new QFile( "psytemp.xml" );
            QDomDocument *doc = new QDomDocument();
            doc->setContent( file );
			isPsy4 = false;
            QDomElement psy4El = doc->documentElement();
			if ( psy4El.tagName() == "psy4" ) isPsy4 = true;
			return isPsy4;		
		}

		bool Psy4Filter::load(std::string const & plugin_path, const std::string & fileName, CoreSong & song, MachineCallbacks* callbacks )
		{
			///\todo this creates a temporary file. need to find a way for all operations to be performed in ram

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

            QFile *file = new QFile( "psytemp.xml" );
            if (!file->open(QIODevice::ReadOnly | QIODevice::Text)) return 0;
            QDomDocument *doc = new QDomDocument();
            doc->setContent( file );

            std::string attrib;

            // Song info.
            QDomElement root = doc->firstChildElement();
            QDomElement infoElm = root.firstChildElement( "info" );
            QDomElement nameElm = infoElm.firstChildElement( "name" );
				attrib = nameElm.attribute( "text" ).toStdString();
                std::cout << attrib << std::endl;
				song_->setName( attrib );
            QDomElement authorElm = infoElm.firstChildElement( "author" );
				attrib = authorElm.attribute( "text" ).toStdString();
				song_->setAuthor( attrib );
            QDomElement commentElm = infoElm.firstChildElement( "comment" );
				attrib = commentElm.attribute( "text" ).toStdString();
				song_->setComment( attrib );

            // Pattern data.
            QDomElement patData = root.firstChildElement( "patterndata" );
            QDomNodeList categories = patData.elementsByTagName( "category" );
            for ( int i = 0; i < categories.count(); i++ )
            {
                QDomElement category = categories.item( i ).toElement();
				std::string catName = category.attribute("name").toStdString();
				std::string attrib = category.attribute("color").toStdString();
				lastCategory = song_->patternSequence()->patternData()->createNewCategory(catName);
				lastCategory->setColor( str<long>( attrib ) );

                QDomNodeList patterns = category.elementsByTagName( "pattern" );
                for ( int i = 0; i < patterns.count(); i++ )
                {
                    QDomElement pattern = patterns.item( i ).toElement();
                
                    std::string patName = pattern.attribute("name").toStdString();
                    int beatZoom = str_hex<int> (pattern.attribute("zoom").toStdString());
                    lastPattern = lastCategory->createNewPattern(patName);
                    lastPattern->clearBars();
                    lastPattern->setBeatZoom(beatZoom);
                    int pat_id  = str_hex<int> (pattern.attribute("id").toStdString());
                    patMap[pat_id] = lastPattern;

                    QDomElement sign = pattern.firstChildElement( "sign" );
                    std::string freeStr = sign.attribute("free").toStdString();
                    if (freeStr != "") {
                        float free = str<float> (freeStr);
                        TimeSignature sig(free);
                        lastPattern->addBar(sig);
                    } else {
                        int num = str<int> (sign.attribute("num").toStdString());
                        int denom = str<int> (sign.attribute("denom").toStdString());
                        int count = str<int> (sign.attribute("count").toStdString());
                        TimeSignature sig(num,denom);
                        sig.setCount(count);
                        lastPattern->addBar(sig);
                    }

                    QDomNodeList patlines = pattern.elementsByTagName( "patline" );
                    for ( int i = 0; i < patlines.count(); i++ )
                    {
                        QDomElement patline = patlines.item( i ).toElement();
                        lastPatternPos = str<float> (patline.attribute("pos").toStdString());

                        QDomNodeList patevents = patline.elementsByTagName( "patevent" );
                        for ( int i = 0; i < patevents.count(); i++ )
                        {
                            QDomElement patevent = patevents.item( i ).toElement();
                            int trackNumber = str_hex<int> (patevent.attribute("track").toStdString());

                            PatternEvent data;
                            data.setMachine( str_hex<int> (patevent.attribute("mac").toStdString()) );
                            data.setInstrument( str_hex<int> (patevent.attribute("inst").toStdString()) );
                            data.setNote( str_hex<int> (patevent.attribute("note").toStdString()) );
                            data.setParameter( str_hex<int> (patevent.attribute("param").toStdString()) );
                            data.setParameter( str_hex<int> (patevent.attribute("cmd").toStdString()) );
                            data.setSharp( str_hex<bool> (patevent.attribute("sharp").toStdString()) );

                            (*lastPattern)[lastPatternPos].notes()[trackNumber]=data;
                        } // patevents
                    } // patlines
                } // patterns
            } // categories

            // Sequence data.
            QDomElement seqData = root.firstChildElement( "sequence" );
            QDomNodeList seqlines = seqData.elementsByTagName( "seqline" );
            for ( int i = 0; i < seqlines.count(); i++ )
            {
                QDomElement seqline = seqlines.item( i ).toElement();
				lastSeqLine = song_->patternSequence()->createNewLine();
				std::cout << "create seqline" << std::endl;

                QDomNodeList seqentries = seqline.elementsByTagName( "seqentry" );
                for ( int i = 0; i < seqentries.count(); i++ )
                {
                    QDomElement seqentry = seqentries.item( i ).toElement();
                    double pos =  str<double> (seqentry.attribute("pos").toStdString());
                    int pat_id  = str<int> (seqentry.attribute("patid").toStdString());
                    float startPos = str<float> (seqentry.attribute("start").toStdString());
                    float endPos = str<float> (seqentry.attribute("end").toStdString());
                    int transpose  = str_hex<int> (seqentry.attribute("transpose").toStdString());
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
                } // seqentries
			} // seqlines

			if (true) {
				RiffFile file;
				file.Open("psytemp.bin");
//				//progress.emit(1,0,"");
//				//progress.emit(2,0,"Loading... psycle bin data ...");

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
//					//progress.emit(4,f2i((fileposition*16384.0f)/filesize),"");

					if(std::strcmp(header,"MACD") == 0)
					{
//						//progress.emit(2,0,"Loading... Song machines...");
						if ((version&0xFF00) == 0x0000) // chunkformat v0
						{
              LoadMACDv0(plugin_path, &file,song,version&0x00FF,callbacks);
						}
						//else if ( (version&0xFF00) == 0x0100 ) //and so on
					}
					else if(std::strcmp(header,"INSD") == 0)
					{
//						//progress.emit(2,0,"Loading... Song instruments...");
						if ((version&0xFF00) == 0x0000) // chunkformat v0
						{
							LoadINSDv0(&file,song,version&0x00FF);
						}
						//else if ( (version&0xFF00) == 0x0100 ) //and so on
					}
					else
					{
//					loggers::warning("foreign chunk found. skipping it.");
						//progress.emit(2,0,"Loading... foreign chunk found. skipping it...");
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
				//progress.emit(4,16384,"");

				//\todo: Move this to something like "song.validate()" 

				// test all connections for invalid machines. disconnect invalid machines.
				for(int i(0) ; i < MAX_MACHINES ; ++i)
				{
					if(song.machine(i))
					{
						song.machine(i)->_connectedInputs = 0;
						song.machine(i)->_connectedOutputs = 0;
						for (int c(0) ; c < MAX_CONNECTIONS ; ++c)
						{
							if(song.machine(i)->_connection[c])
							{
								if(song.machine(i)->_outputMachines[c] < 0 || song.machine(i)->_outputMachines[c] >= MAX_MACHINES)
								{
									song.machine(i)->_connection[c] = false;
									song.machine(i)->_outputMachines[c] = -1;
								}
								else if(!song.machine(song.machine(i)->_outputMachines[c]))
								{
									song.machine(i)->_connection[c] = false;
									song.machine(i)->_outputMachines[c] = -1;
								}
								else 
								{
									song.machine(i)->_connectedOutputs++;
								}
							}
							else
							{
								song.machine(i)->_outputMachines[c] = -1;
							}

							if (song.machine(i)->_inputCon[c])
							{
								if (song.machine(i)->_inputMachines[c] < 0 || song.machine(i)->_inputMachines[c] >= MAX_MACHINES)
								{
									song.machine(i)->_inputCon[c] = false;
									song.machine(i)->_inputMachines[c] = -1;
								}
								else if (!song.machine(song.machine(i)->_inputMachines[c]))
								{
									song.machine(i)->_inputCon[c] = false;
									song.machine(i)->_inputMachines[c] = -1;
								}
								else
								{
									song.machine(i)->_connectedInputs++;
								}
							}
							else
							{
								song.machine(i)->_inputMachines[c] = -1;
							}
						}
					}
				}

				//progress.emit(5,0,"");
				if(chunkcount)
				{
					if (!song.machine(MASTER_INDEX) )
					{
						song.machine(MASTER_INDEX, new Master( callbacks, MASTER_INDEX, &song ));
						song.machine(MASTER_INDEX)->Init();
					}
					std::ostringstream s;
					s << "Error reading from file '" << file.file_name() << "'" << std::endl;
					s << "some chunks were missing in the file";
					//report.emit(s.str(), "Song Load Error.");
				}
				//\todo:
			}

			return isPsy4;
		} // load


		bool Psy4Filter::save( const std::string & file_Name, const CoreSong & song )
		{
			std::string fileName = File::extractFileNameFromPath(file_Name);

			bool autosave = false;
			//\todo:
			if ( !autosave )
			{
				//progress.emit(1,0,"");
				//progress.emit(2,0,"Saving...");
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
				//progress.emit(1,0,"");
				//progress.emit(2,0,"Saving binary data...");
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
				if (song.machine(index))
				{
					saveMACDv0(&file,song,index);
					if ( !autosave )
					{
						//progress.emit(4,-1,"");
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
						//progress.emit(4,-1,"");
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

		int Psy4Filter::LoadSONGv0(RiffFile* file,CoreSong& song)
		{
			std::uint32_t fileversion = 0;
			std::uint32_t size = 0;
			std::uint32_t chunkcount = 0;
			file->Read(fileversion);
			file->Read(size);
			if(fileversion > CURRENT_FILE_VERSION)
			{
				//report.emit("This file is from a newer version of Psycle! This process will try to load it anyway.", "Load Warning");
			}
			file->Read(chunkcount);
			const int bytesread=4;
			if ( size > 4)
			{
				file->Skip(size - bytesread);// Size of the current header DATA // This ensures that any extra data is skipped.
			}
			return chunkcount;
		}

		bool Psy4Filter::saveSONGv0( RiffFile * file, const CoreSong & song )
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
					if(song.machine(i)) ++chunkcount;
			for(unsigned int i(0) ; i < MAX_INSTRUMENTS ; ++i)
				 if(!song._pInstrument[i]->Empty()) ++chunkcount;

			file->Write(chunkcount);

			return true;
		}

		bool Psy4Filter::saveMACDv0( RiffFile * file, const CoreSong & song, int index )
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
			song.machine(index)->SaveFileChunk(file);

			// chunk size in header

			std::size_t const pos2 = file->GetPos();
			size = pos2 - pos - sizeof size;
			file->Seek(pos);
			file->Write(size);
			file->Seek(pos2);

			//\todo:
			return true;
		}

		bool Psy4Filter::saveINSDv0( RiffFile * file, const CoreSong & song, int index )
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

		bool Psy4Filter::saveWAVEv0( RiffFile * file, const CoreSong & song, int index )
		{
			return false;
		}

	}
}

