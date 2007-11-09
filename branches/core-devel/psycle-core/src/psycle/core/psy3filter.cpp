/**************************************************************************
*   Copyright 2007 Psycledelics http://psycle.sourceforge.net             *
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

#include "psy3filter.h"

#include "commands.h"
#include "datacompression.h"
#include "fileio.h"
#include "internal_machines.h"
#include "song.h"

#include <sstream>
#include <iostream>

namespace psy { namespace core {

std::string const Psy3Filter::FILE_FOURCC = "PSY3";
/// Current version of the Song file and its chunks.
/// format: 0xAABB
/// A = Major version. It can't be loaded, skip the whole chunk. (Right now the loader does it, so simply do nothing)
/// B = minor version. It can be loaded with the existing loader, but not all information will be available.
std::uint32_t const Psy3Filter::VERSION_INFO = 0x0000;
std::uint32_t const Psy3Filter::VERSION_SNGI = 0x0000;
std::uint32_t const Psy3Filter::VERSION_SEQD = 0x0000;
std::uint32_t const Psy3Filter::VERSION_PATD = 0x0000;
std::uint32_t const Psy3Filter::VERSION_MACD = 0x0000;
std::uint32_t const Psy3Filter::VERSION_INSD = 0x0000;
std::uint32_t const Psy3Filter::VERSION_WAVE = 0x0000;

std::uint32_t const Psy3Filter::FILE_VERSION =
	Psy3Filter::VERSION_INFO +
	Psy3Filter::VERSION_SNGI +
	Psy3Filter::VERSION_SEQD +
	Psy3Filter::VERSION_PATD +
	Psy3Filter::VERSION_PATD + 
	Psy3Filter::VERSION_MACD +
	Psy3Filter::VERSION_INSD +
	Psy3Filter::VERSION_WAVE;

Psy3Filter::Psy3Filter()
:
	singleCat(),
	singleLine()
{}

bool Psy3Filter::testFormat( const std::string & fileName )
{
	RiffFile file;
	file.Open(fileName);
	char Header[9];
	file.ReadArray(Header, 8);
	Header[8]=0;
	file.Close();
	if (strcmp(Header,"PSY3SONG")==0) return true;
	return false;
}


void Psy3Filter::preparePatternSequence( CoreSong & song )
{
	seqList.clear();
	song.patternSequence()->removeAll();
	// creatse a single Pattern Category
	singleCat = song.patternSequence()-> patternData()->createNewCategory("SinglePattern");
	// here we add in one single Line the patterns
	singleLine = song.patternSequence()->createNewLine();
}

bool Psy3Filter::load(std::string const & plugin_path, const std::string & fileName, CoreSong & song, MachineCallbacks* callbacks )
{
	RiffFile file;
	file.Open(fileName);
	//progress.emit(1,0,"");
	//progress.emit(2,0,"Loading... psycle song fileformat version 3...");
	std::cout << "PSY3 format"<< std::endl;
	
	// skip header
	file.Skip(8);
	//char Header[9];
	//file.ReadChunk(&Header, 8);
	//Header[8]=0;

	///\todo:
	song.clear();
	preparePatternSequence(song);
	size_t filesize = file.FileSize();
	std::uint32_t version = 0;
	std::uint32_t size = 0;
	char header[5];
	header[4]=0;
	std::uint32_t chunkcount = LoadSONGv0(&file,song);
	/* chunk_loop: */

	while(file.ReadArray(header, 4) && chunkcount)
	{
		file.Read(version);
		file.Read(size);

		int fileposition = file.GetPos();
		//song.progress.emit(4,f2i((fileposition*16384.0f)/filesize),"");

		if(std::strcmp(header,"INFO") == 0)
		{
			//song.progress.emit(2,0,"Loading... Song authorship information...");
			if ((version&0xFF00) == 0x0000) // chunkformat v0
			{
				LoadINFOv0(&file,song,version&0x00FF);
				//bug in psycle 1.8.5, writes size as 12bytes more!
				if ( version == 0x0000 ) size = song.name().length()+song.author().length()+song.comment().length() + 3;
			}
			//else if ( (version&0xFF00) == 0x0100 ) //and so on
		}
		else if(std::strcmp(header,"SNGI")==0)
		{
			//song.progress.emit(2,0,"Loading... Song properties information...");
			if ((version&0xFF00) == 0x0000) // chunkformat v0
			{
				LoadSNGIv0(&file,song,version&0x00FF,callbacks);
				//\ Fix for a bug existing in the Song Saver in the 1.7.x series
				if (version == 0x0000) size = 11*sizeof(std::uint32_t)+song.tracks()*2*sizeof(bool); 
			}
			//else if ( (version&0xFF00) == 0x0100 ) //and so on
		}
		else if(std::strcmp(header,"SEQD")==0)
		{
			//song.progress.emit(2,0,"Loading... Song sequence...");
			if ((version&0xFF00) == 0x0000) // chunkformat v0
			{
				LoadSEQDv0(&file,song,version&0x00FF);
			}
			//else if ( (version&0xFF00) == 0x0100 ) //and so on
		}
		else if(std::strcmp(header,"PATD") == 0)
		{
			//progress.emit(2,0,"Loading... Song patterns...");
			if ((version&0xFF00) == 0x0000) // chunkformat v0
			{
				LoadPATDv0(&file,song,version&0x00FF);
				//\ Fix for a bug existing in the Song Saver in the 1.7.x series
				if ((version == 0x0000) &&( file.GetPos() == fileposition+size+4)) size += 4; 
			}
			//else if ( (version&0xFF00) == 0x0100 ) //and so on
		}
		else if(std::strcmp(header,"MACD") == 0)
		{
			//song.progress.emit(2,0,"Loading... Song machines...");
			if ((version&0xFF00) == 0x0000) // chunkformat v0
			{
				LoadMACDv0(plugin_path, &file,song,version&0x00FF,callbacks);
			}
			//else if ( (version&0xFF00) == 0x0100 ) //and so on
		}
		else if(std::strcmp(header,"INSD") == 0)
		{
			//song.progress.emit(2,0,"Loading... Song instruments...");
			if ((version&0xFF00) == 0x0000) // chunkformat v0
			{
				LoadINSDv0(&file,song,version&0x00FF);
			}
			//else if ( (version&0xFF00) == 0x0100 ) //and so on
		}
		else
		{
			//loggers::warning("foreign chunk found. skipping it.");
			//song.progress.emit(2,0,"Loading... foreign chunk found. skipping it...");
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

	//song.progress.emit(4,16384,"");

	///\todo: Move this to something like "song.validate()" 

	// now that we have loaded all the patterns, time to prepare them.
	double pos = 0;
	std::vector<int>::iterator it = seqList.begin();
	for ( ; it < seqList.end(); ++it)
	{
		SinglePattern* pat = song.patternSequence()->patternData()->findById(*it);
		singleLine->createEntry(pat,pos);
		pos+=pat->beats();
	}

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

	//song.progress.emit(5,0,"");
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
	return true;
}

int Psy3Filter::LoadSONGv0(RiffFile* file,CoreSong& song)
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
		// This is left here if someday, extra data is added to the file version chunk.
		// update "bytesread" accordingly.
		
		//file->Read(chunkversion);
		//if (chunkversion == x)
		//{}
		//else if (...)
		//{}

		file->Skip(size - bytesread);// Size of the current header DATA // This ensures that any extra data is skipped.
	}
	return chunkcount;
}

bool Psy3Filter::LoadINFOv0(RiffFile* file,CoreSong& song,int minorversion)
{
		char Name[129]; char Author[65]; char Comment[65536];
		
		file->ReadString(Name, 128);
		song.setName(Name);
		file->ReadString(Author, 64);
		song.setAuthor(Author);
		bool result = file->ReadString(Comment, 65535);
		song.setComment(Comment);
		return result;
}

bool Psy3Filter::LoadSNGIv0(RiffFile* file,CoreSong& song,int minorversion, MachineCallbacks* callbacks)
{
	std::int32_t temp(0);
	std::int16_t temp16(0);
	bool fileread = false;

	// why all these temps?  to make sure if someone changes the defs of
	// any of these members, the rest of the file reads ok.  assume 
	// everything is 32-bit, when we write we do the same thing.

	// # of tracks for whole song
	file->Read(temp);
	song.setTracks(temp);
	// bpm
	///\todo: This was a hack added in 1.9alpha to allow decimal BPM values
	//{
		file->Read(temp16);
		int BPMCoarse = temp16;
		file->Read(temp16);
		song.setBpm( BPMCoarse + temp16/100.0f );
	//}
	// linesperbeat
	file->Read(temp);
	song.setTicksSpeed(temp);
	linesPerBeat= song.ticksSpeed();

	// current octave
	file->Read(temp);
	octave = static_cast<unsigned char>(temp);
	// machineSoloed
	file->Read(machineSoloed);
	// trackSoloed
	file->Read(trackSoloed);
	file->Read(seqBus);
	file->Read(midiSelected);
	file->Read(auxcolSelected);
	file->Read(instSelected);
	
	// sequence width, for multipattern
	file->Read(temp);
	for(int i(0) ; i < song.tracks(); ++i)
	{
		bool tmp;
		file->Read(tmp);
		song.patternSequence()->setMutedTrack(i,tmp);
		fileread = file->Read(tmp);
		song.patternSequence()->setArmedTrack(i,tmp);
	}
	callbacks->timeInfo().setBpm(song.bpm());
	callbacks->timeInfo().setTicksSpeed(song.ticksSpeed(),song.isTicks());
	return fileread;
}

bool Psy3Filter::LoadSEQDv0(RiffFile* file,CoreSong& song,int minorversion)
{
	std::uint32_t index = 0;
	std::uint32_t temp;
	bool fileread = false;
	// index, for multipattern - for now always 0
	file->Read(index);
	if (index < MAX_SEQUENCES)
	{
		char pTemp[256];
		// play length for this sequence
		file->Read(temp);
		int playLength = temp;
		// name, for multipattern, for now unused
		file->ReadString(pTemp, sizeof pTemp);
		for (int i(0) ; i < playLength; ++i)
		{
			fileread = file->Read(temp);
			seqList.push_back(temp);
		}
	}
	return fileread;
}

PatternEvent Psy3Filter::convertEntry( unsigned char * data ) const
{
	PatternEvent event;
	event.setNote(*data); data++;
	event.setInstrument(*data); data++;
	event.setMachine(*data); data++;
	event.setCommand(*data); data++;
	event.setParameter(*data); data++;
	return event;
}

bool Psy3Filter::LoadPATDv0(RiffFile* file,CoreSong& song,int minorversion)
{
	std::uint32_t index = 0;
	std::uint32_t temp;
	std::uint32_t size;
	bool fileread=false;

	// index
	file->Read(index);
	if(index < MAX_PATTERNS)
	{
		// num lines
		file->Read(temp);
		// clear it out if it already exists
		int numLines = temp;
		// num tracks per pattern // eventually this may be variable per pattern, like when we get multipattern
		file->Read(temp);
		char patternName[32];
		file->ReadString(patternName, sizeof patternName);
		file->Read(size);
		unsigned char * pSource = new unsigned char[size];
		fileread = file->ReadArray(pSource, size);
		unsigned char * pDest;
		DataCompression::BEERZ77Decomp2(pSource, &pDest);
		delete[] pSource; pSource = pDest;
		// create a SinglePattern
		std::string indexStr;
		std::ostringstream o;
		if (!(o << index))
			indexStr = "error";
		else
			indexStr = o.str();
		SinglePattern* pat = singleCat->createNewPattern(std::string(patternName)+indexStr);
		pat->setBeatZoom(song.ticksSpeed());
		pat->setID(index);
		float beatpos=0;
		for(int y(0) ; y < numLines ; ++y) // lines
		{
			for (int x = 0; x < song.tracks(); x++) {
				unsigned char entry[5] ;
				std::memcpy( &entry, pSource, 5);
				PatternEvent event = convertEntry(entry);
				if (!event.empty()) {
					if (event.note() == commands::tweak) {
						(*pat)[beatpos].tweaks()[pat->tweakTrack(TweakTrackInfo(event.machine(),event.parameter(),TweakTrackInfo::twk))] = event;
					}
					else if (event.note() == commands::tweak_slide) {
						(*pat)[beatpos].tweaks()[pat->tweakTrack(TweakTrackInfo(event.machine(),event.parameter(),TweakTrackInfo::tws))] = event;
					}
					else if (event.note() == commands::midi_cc) {
						(*pat)[beatpos].tweaks()[pat->tweakTrack(TweakTrackInfo(event.machine(),event.parameter(),TweakTrackInfo::mdi))] = event;
					///\todo: Also, move the Global commands (tempo, mute..) out of the pattern.
					}
					else {
						if(event.command() == PatternCmd::NOTE_DELAY)
						{
							/// Convert old value (part of line) to new value (part of beat)
							event.setParameter(event.parameter()/linesPerBeat);
						}
						(*pat)[beatpos].notes()[x] = event;
					}

					if ( (event.note() <= commands::release || event.note() == commands::empty) && (event.command() == 0xFE) && (event.parameter() < 0x20 ))
					{
						linesPerBeat= event.parameter()&0x1F;
					}
				}
				pSource += EVENT_SIZE;
			}
			beatpos += 1 / (float) linesPerBeat;
		}
		delete[] pDest; pDest = 0;
		TimeSignature & sig =  pat->timeSignatures().back();
		sig.setCount((int) (beatpos / 4) );
		float uebertrag = beatpos - ((int) beatpos);
		if ( uebertrag != 0) {
			TimeSignature uebertragSig(uebertrag);
			if ( sig.count() == 0 ) pat->timeSignatures().pop_back();
			pat->addBar(uebertragSig);
		}

	}
	return fileread;
}

bool Psy3Filter::LoadMACDv0(std::string const & plugin_path, RiffFile* file,CoreSong& song,int minorversion, MachineCallbacks* callbacks)
{
	std::uint32_t index = 0;

	file->Read(index);
	if(index < MAX_MACHINES)
	{
		Machine::id_type const id(index);
		///\todo: song.clear() creates an empty song with a Master Machine. This loader doesn't
		// try to free that allocated machine.
		song.machine(index, Machine::LoadFileChunk(plugin_path, &song,file, callbacks, id, minorversion, true));
	}
	return song.machine(index) != 0;
}

bool Psy3Filter::LoadINSDv0(RiffFile* file,CoreSong& song,int minorversion)
{
	std::uint32_t index = 0;
	file->Read(index);
	if(index < MAX_INSTRUMENTS)
	{
		song._pInstrument[index]->LoadFileChunk(file, minorversion, true);
	}
	///\todo:
	return true;
}
		
}}
