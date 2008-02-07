/* -*- mode:c++, indent-tabs-mode:t -*- */
/**************************************************************************
*   Copyright 2007-2008 Psycledelics http://psycle.sourceforge.net        *
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
#include <psycle/core/psycleCorePch.hpp>

#include "psy4filter.h"

#include "file.h"
#include "fileio.h"
#include "internal_machines.h"
#include "song.h"
#include "zipwriter.h"
#include "zipwriterstream.h"
#include "zipreader.h"
#include "helpers/xml.h"

#include "signalslib.h"

#if defined PSYCLE__LIBXMLPP_AVAILABLE
	#include <libxml++/parsers/domparser.h>
#else
	#error none of the supported xml parser libs appear to be available
#endif

#if defined __unix__ || defined __APPLE__
	#include <unistd.h>
	#include <sys/types.h>
#elif defined _WIN64 || defined _WIN32
	#include <io.h>
#endif

#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <cerrno>
#include <sstream>
#include <iostream> // only for debug output

namespace psy { namespace core {

namespace {
	template<typename T>
	inline T str(const std::string &  value) {
		T result;

		std::stringstream str;
		str << value;
		str >> result;

		return result;
	}

	template<typename T>
	inline T str_hex(const std::string &  value) {
		T result;

		std::stringstream str;
		str << value;
		str >> std::hex >> result;

		return result;
	}
}

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
	int outFd = open(std::string("psytemp.xml").c_str(), O_RDWR|O_CREAT|O_TRUNC, 0644);
	f = zipreader_seek(z, "xml/song.xml");

	if (!zipreader_extract(f, outFd )) {
		zipreader_close( z );
		close( outFd );
		close( fd );
		return false;
	}
	close( outFd );

	f = zipreader_seek(z, "bin/song.bin");
	outFd = open(std::string("psytemp.bin").c_str(), O_RDWR|O_CREAT|O_TRUNC, 0644);
	if (!zipreader_extract(f, outFd )) {
		zipreader_close( z );
		close( outFd );
		close( fd );
		return false;
	}
	close( outFd );
	
	zipreader_close( z );
	close( fd );

	xmlpp::DomParser parser;
	parser.parse_file("psytemp.xml");
	if(!parser) return false;
	xmlpp::Element const & root_element(*parser.get_document()->get_root_node());
	return root_element.get_name() == "psy4";
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

	xmlpp::DomParser parser;
	parser.parse_file("psytemp.xml");
	if(!parser) return false;
	xmlpp::Document const & document(*parser.get_document());
	xmlpp::Element const & root_element(*document.get_root_node());

	std::string attrib;

	// Song info.
	{
		xmlpp::Node::NodeList const & info_nodes(root_element.get_children("info"));
		if(info_nodes.begin() != info_nodes.end()) {
			xmlpp::Element const & info(dynamic_cast<xmlpp::Element const &>(**info_nodes.begin()));
			{
				xmlpp::Node::NodeList const & names(info.get_children("name"));
				if(names.begin() != names.end()) {
					xmlpp::Element const & name(dynamic_cast<xmlpp::Element const &>(**names.begin()));
					xmlpp::Attribute const * const text_attribute(name.get_attribute("text"));
					if(!text_attribute) std::cerr << "expected text attribute in info/name element\n";
					else song_->setName(text_attribute->get_value());
				}
			}
			{
				xmlpp::Node::NodeList const & authors(info.get_children("author"));
				if(authors.begin() != authors.end()) {
					xmlpp::Element const & author(dynamic_cast<xmlpp::Element const &>(**authors.begin()));
					xmlpp::Attribute const * const text_attribute(author.get_attribute("text"));
					if(!text_attribute) std::cerr << "expected text attribute in info/author element\n";
					else song_->setAuthor(text_attribute->get_value());
				}
			}
			{
				xmlpp::Node::NodeList const & comments(info.get_children("comment"));
				if(comments.begin() != comments.end()) {
					xmlpp::Element const & comment(dynamic_cast<xmlpp::Element const &>(**comments.begin()));
					xmlpp::Attribute const * const text_attribute(comment.get_attribute("text"));
					if(!text_attribute) std::cerr << "expected text attribute in info/comment element\n";
					else song_->setComment(text_attribute->get_value());
				}
			}
		}
	}

	// Pattern data.
	{
		xmlpp::Node::NodeList const & pattern_data_nodes(root_element.get_children("patterndata"));
		if(pattern_data_nodes.begin() != pattern_data_nodes.end()) {
			xmlpp::Element const & pattern_data(dynamic_cast<xmlpp::Element const &>(**pattern_data_nodes.begin()));
			xmlpp::Node::NodeList const & categories(pattern_data.get_children("category"));
			for(xmlpp::Node::NodeList::const_iterator i = categories.begin(); i != categories.end(); ++i) {
				xmlpp::Element const & category(dynamic_cast<xmlpp::Element const &>(**i));
				xmlpp::Attribute const * const name_attribute(category.get_attribute("name"));
				if(!name_attribute) std::cerr << "expected name attribute in category element\n";
				else {
					lastCategory = song_->patternSequence()->patternData()->createNewCategory(name_attribute->get_value());
					xmlpp::Attribute const * const color_attribute(category.get_attribute("color"));
					if(color_attribute) lastCategory->setColor(str<long int>(color_attribute->get_value()));
				}
				xmlpp::Node::NodeList const & patterns(category.get_children("pattern"));
				for(xmlpp::Node::NodeList::const_iterator i = patterns.begin(); i != patterns.end(); ++i) {
					xmlpp::Element const & pattern(dynamic_cast<xmlpp::Element const &>(**i));
					xmlpp::Attribute const * const name_attribute(pattern.get_attribute("name"));
					if(!name_attribute) std::cerr << "expected name attribute in pattern element\n";
					else {
						lastPattern = lastCategory->createNewPattern(name_attribute->get_value());
						lastPattern->clearBars(); ///\todo we just created it.. why does it need cleaning?
						{
							xmlpp::Attribute const * const zoom_attribute(pattern.get_attribute("zoom"));
							if(zoom_attribute) lastPattern->setBeatZoom(str_hex<int>(zoom_attribute->get_value()));
						}
						{
							xmlpp::Attribute const * const id_attribute(pattern.get_attribute("id"));
							if(!id_attribute) std::cerr << "expected id attribute in pattern element\n";
							else patMap[str_hex<int>(id_attribute->get_value())] = lastPattern;
						}
						{
							xmlpp::Node::NodeList const & signatures(pattern.get_children("sign"));
							if(signatures.begin() == signatures.end()) std::cerr << "expected sign innert element in enclosing pattern element\n";
							{
								xmlpp::Element const & signature(dynamic_cast<xmlpp::Element const &>(**signatures.begin()));
								xmlpp::Attribute const * const free_attribute(signature.get_attribute("free"));
								if(free_attribute) {
									TimeSignature sig(str<float>(free_attribute->get_value()));
									lastPattern->addBar(sig);
								} else {
									xmlpp::Attribute const * const num_attribute(signature.get_attribute("num"));
									if(!num_attribute) std::cerr << "expected num attribute in sign element\n";
									else {
										xmlpp::Attribute const * const denom_attribute(signature.get_attribute("denom"));
										if(!denom_attribute) std::cerr << "expected denom attribute in sign element\n";
										else {
											xmlpp::Attribute const * const count_attribute(signature.get_attribute("count"));
											if(!count_attribute) std::cerr << "expected count attribute in sign element\n";
											else {
												TimeSignature sig(str<int>(num_attribute->get_value()), str<int>(denom_attribute->get_value()));
												sig.setCount(str<int>(count_attribute->get_value()));
												lastPattern->addBar(sig);
											}
										}
									}
								}
							}
						}
					}
					xmlpp::Node::NodeList const & pattern_lines(pattern.get_children("patline"));
					for(xmlpp::Node::NodeList::const_iterator i = pattern_lines.begin(); i != pattern_lines.end(); ++i) {
						xmlpp::Element const & pattern_line(dynamic_cast<xmlpp::Element const &>(**i));
						{
							xmlpp::Attribute const * const pos_attribute(pattern_line.get_attribute("pos"));
							if(!pos_attribute) std::cerr << "expected pos attribute in patline element\n";
							else lastPatternPos = str<float>(pos_attribute->get_value());
						}
						xmlpp::Node::NodeList const & pattern_events(pattern_line.get_children("patevent"));
						for(xmlpp::Node::NodeList::const_iterator i = pattern_events.begin(); i != pattern_events.end(); ++i) {
							xmlpp::Element const & pattern_event(dynamic_cast<xmlpp::Element const &>(**i));
							PatternEvent data;
							{
								xmlpp::Attribute const * const mac_attribute(pattern_event.get_attribute("mac"));
								if(!mac_attribute) std::cerr << "expected mac attribute in patevent element\n";
								else data.setMachine(str_hex<int>(mac_attribute->get_value()));
							}
							{
								xmlpp::Attribute const * const inst_attribute(pattern_event.get_attribute("inst"));
								if(!inst_attribute) std::cerr << "expected inst attribute in patevent element\n";
								else data.setInstrument(str_hex<int>(inst_attribute->get_value()));
							}
							{
								xmlpp::Attribute const * const note_attribute(pattern_event.get_attribute("note"));
								if(!note_attribute) std::cerr << "expected note attribute in patevent element\n";
								else data.setNote(str_hex<int>(note_attribute->get_value()));
							}
							{
								xmlpp::Attribute const * const param_attribute(pattern_event.get_attribute("param"));
								if(!param_attribute) std::cerr << "expected param attribute in patevent element\n";
								else data.setParameter(str_hex<int>(param_attribute->get_value()));
							}
							{
								xmlpp::Attribute const * const cmd_attribute(pattern_event.get_attribute("cmd"));
								if(!cmd_attribute) std::cerr << "expected cmd attribute in patevent element\n";
								else data.setParameter(str_hex<int>(cmd_attribute->get_value()));
							}
							{
								xmlpp::Attribute const * const track_attribute(pattern_event.get_attribute("track"));
								if(!track_attribute) std::cerr << "expected track attribute in patevent element\n";
								else (*lastPattern)[lastPatternPos].notes()[str_hex<int>(track_attribute->get_value())]= data;
							}
						}
					}
				}
			}
		}
	}

	// Sequence data.
	{
		xmlpp::Node::NodeList const & sequences(root_element.get_children("sequence"));
		if(sequences.begin() != sequences.end()) {
			xmlpp::Element const & sequence(dynamic_cast<xmlpp::Element const &>(**sequences.begin()));
			xmlpp::Node::NodeList const & sequencer_lines(sequence.get_children("seqline"));
			for(xmlpp::Node::NodeList::const_iterator i = sequencer_lines.begin(); i != sequencer_lines.end(); ++i) {
				xmlpp::Element const & sequencer_line(dynamic_cast<xmlpp::Element const &>(**i));
				lastSeqLine = song_->patternSequence()->createNewLine();
				xmlpp::Node::NodeList const & sequencer_entries(sequencer_line.get_children("seqentry"));
				for(xmlpp::Node::NodeList::const_iterator i = sequencer_entries.begin(); i != sequencer_entries.end(); ++i) {
					xmlpp::Element const & sequencer_entry(dynamic_cast<xmlpp::Element const &>(**i));
					xmlpp::Attribute const * const id_attribute(sequencer_entry.get_attribute("patid"));
					if(!id_attribute) std::cerr << "expected patid attribute in seqentry element\n";
					else {
							std::map<int, SinglePattern*>::iterator it = patMap.find(str<int>(id_attribute->get_value()));
							if(it != patMap.end()) {
								SinglePattern * pattern = it->second;
								if(pattern) {
									xmlpp::Attribute const * const pos_attribute(sequencer_entry.get_attribute("pos"));
									if(!pos_attribute) std::cerr << "expected pos attribute in seqentry element\n";
									else {
										SequenceEntry * entry = lastSeqLine->createEntry(pattern, str<double>(pos_attribute->get_value()));
										{
											xmlpp::Attribute const * const start_attribute(sequencer_entry.get_attribute("start"));
											if(start_attribute) entry->setStartPos(str<float>(start_attribute->get_value()));
										}
										{
											xmlpp::Attribute const * const end_attribute(sequencer_entry.get_attribute("end"));
											if(end_attribute) entry->setEndPos(str<float>(end_attribute->get_value()));
										}
										{
											xmlpp::Attribute const * const transpose_attribute(sequencer_entry.get_attribute("transpose"));
											if(transpose_attribute) entry->setTranspose(str<int>(transpose_attribute->get_value()));
										}
									}
								}
							}
					}
				}
			}
		}
	}

	if (true) {
		RiffFile file;
		file.Open("psytemp.bin");
		//progress.emit(1,0,"");
		//progress.emit(2,0,"Loading... psycle bin data ...");

		// skip header
		file.Skip(8);
		///\todo:
		size_t filesize = file.FileSize();
		std::uint32_t version = 0;
		std::uint32_t size = 0;
		char header[5];
		header[4]=0;
		std::uint32_t chunkcount = LoadSONGv0(&file,song);
		std::cout << chunkcount << std::endl;

		/* chunk_loop: */
		while(file.ReadArray(header, 4) && chunkcount)
		{
			file.Read(version);
			file.Read(size);

			int fileposition = file.GetPos();
			//progress.emit(4,f2i((fileposition*16384.0f)/filesize),"");

			if(std::strcmp(header,"MACD") == 0)
			{
				//progress.emit(2,0,"Loading... Song machines...");
				if ((version&0xFF00) == 0x0000) // chunkformat v0
				{
					LoadMACDv0(plugin_path, &file,song,version&0x00FF,callbacks);
				}
				//else if ( (version&0xFF00) == 0x0100 ) //and so on
			}
			else if(std::strcmp(header,"INSD") == 0)
			{
				//progress.emit(2,0,"Loading... Song instruments...");
				if ((version&0xFF00) == 0x0000) // chunkformat v0
				{
					LoadINSDv0(&file,song,version&0x00FF);
				}
				//else if ( (version&0xFF00) == 0x0100 ) //and so on
			}
			else
			{
				//loggers::warning("foreign chunk found. skipping it.");
				//progress.emit(2,0,"Loading... foreign chunk found. skipping it...");
				std::ostringstream s;
					s << "foreign chunk: version: " << version << ", size: " << size;
				//loggers::trace(s.str());
				if(size && size < filesize-fileposition)
				{
					file.Skip(size);
				}
			}
			// For invalid version chunks, or chunks that haven't been read correctly/completely.
			if  (file.GetPos() != fileposition+size)
			{
			///\todo: verify how it works with invalid data.
			//if (file.GetPos() > fileposition+size) loggers::trace("Cursor ahead of size! resyncing with chunk size.");
			//else loggers::trace("Cursor still inside chunk, resyncing with chunk size.");
				file.Seek(fileposition+size);
			}
			--chunkcount;
		}
		//progress.emit(4,16384,"");

		///\todo: Move this to something like "song.validate()" 

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
		///\todo:
	}

	return true;
} // load

bool Psy4Filter::save( const std::string & file_Name, const CoreSong & song )
{
	std::string fileName = File::extractFileNameFromPath(file_Name);

	bool autosave = false;
	///\todo:
	if ( !autosave )
	{
		//progress.emit(1,0,"");
		//progress.emit(2,0,"Saving...");
	}

	// ideally, you should create a temporary file on the same physical
	// disk as the target zipfile... 

	zipwriter *z = zipwriter_start(open(fileName.c_str(), O_RDWR|O_CREAT, 0644));
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


	///\todo:
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

	file.WriteArray("PSY4",4);
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
	///\todo:
	file.Close();

	// copy the bin data to the zip
	f = zipwriter_addfile(z, std::string("bin/song.bin").c_str(), 9);
	zipwriter_copy(open("psycle_tmp.bin", O_RDONLY), f);

	if (!zipwriter_finish(z)) {
		std::cerr << "Zipwriter failed." << std::endl;
		return false;
	}

	// remove temp file
	if( std::remove("psycle_tmp.bin") == -1 ) std::cerr << "Error deleting temp file" << std::endl;
			
	return true;
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
		file->Skip(size - bytesread); // Size of the current header DATA // This ensures that any extra data is skipped.
	}
	return chunkcount;
}

bool Psy4Filter::saveSONGv0( RiffFile * file, const CoreSong & song )
{
	std::uint32_t chunkcount;
	std::uint32_t version, size;
	// chunk header;

	file->WriteArray("SONG",4);
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

	file->WriteArray("MACD",4);
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

	///\todo:
	return true;
}

bool Psy4Filter::saveINSDv0( RiffFile * file, const CoreSong & song, int index )
{
	std::uint32_t version, size;
	std::size_t pos;

	// chunk header

	file->WriteArray("INSD",4);
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

	///\todo:
	return true;
}

bool Psy4Filter::saveWAVEv0( RiffFile * file, const CoreSong & song, int index )
{
	return false;
}

}}
