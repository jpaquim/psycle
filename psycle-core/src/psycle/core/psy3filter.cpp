// This program is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
// You should have received a copy of the GNU General Public License along with this program; if not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
// copyright 2007-2009 members of the psycle project http://psycle.sourceforge.net

#include <psycle/core/config.private.hpp>
#include "psy3filter.h"
#include "fileio.h"
#include "song.h"
#include "machinefactory.h"
#include "mixer.h"
#include "vstplugin.h"
#include <psycle/helpers/datacompression.hpp>
#include <diversalis/os.hpp>
#include <sstream>
#include <iostream>

namespace psycle { namespace core {

	using namespace psycle::helpers;

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

Psy3Filter * Psy3Filter::getInstance() {
	///\todo
	static Psy3Filter s;
	return &s;
}

Psy3Filter::Psy3Filter()
:
	singleCat(),
	singleLine()
{}

bool Psy3Filter::testFormat(const std::string & fileName) {
	RiffFile file;
	file.Open(fileName);
	char header[9];
	file.ReadArray(header, 8);
	header[8] = 0;
	file.Close(); ///\todo RiffFile's dtor doesn't close!!!
	return !strcmp(header, "PSY3SONG");
}


void Psy3Filter::preparePatternSequence(CoreSong & song) {
	seqList.clear();
	song.patternSequence().removeAll();
	// here we add in one single Line the patterns
	singleLine = song.patternSequence().createNewLine();
}

bool Psy3Filter::load(const std::string & fileName, CoreSong & song) {
	//progress.emit(1,0,"");
	//progress.emit(2,0,"Loading... psycle song fileformat version 3...");
	std::cout << "psycle: core: psy3 loader: loading psycle song fileformat version 3: " << fileName << '\n';

	RiffFile file;
	file.Open(fileName);

	// skip header
	file.Skip(8);
	//char Header[9];
	//file.ReadChunk(&Header, 8);
	//Header[8]=0;

	///\todo:
	song.clear();
	preparePatternSequence(song);
	//size_t filesize = file.FileSize();
	std::uint32_t version = 0;
	std::uint32_t size = 0;
	bool problemfound = false;
	int fileposition = 0;
	char header[5];
	header[4]=0;
	std::uint32_t chunkcount = LoadSONGv0(&file,song);
	/* chunk_loop: */

	while(file.ReadArray(header, 4) && chunkcount) {
		//song.progress.emit(4, static_cast<int>(file.GetPos() * 16384.0f / filesize), "");

		if(!std::strcmp(header,"INFO")) {
			//song.progress.emit(2, 0, "Loading... Song authorship information...");
			--chunkcount;
			problemfound=false;
			file.Read(version);
			file.Read(size);
			fileposition = file.GetPos();
			if((version & 0xff00) == 0) { // chunkformat v0
				LoadINFOv0(&file, song, version & 0xff);
				//bug in psycle 1.8.5, writes size as 12bytes more!
				if(version == 0) size = song.name().length()+song.author().length()+song.comment().length() + 3;
			}
			//else if((version & 0xff00) == 0x0100) // and so on
		} else if(!std::strcmp(header,"SNGI")) {
			//song.progress.emit(2, 0, "Loading... Song properties information...");
			--chunkcount;
			problemfound=false;
			file.Read(version);
			file.Read(size);
			fileposition = file.GetPos();
			if((version & 0xff00) == 0) { // chunkformat v0
				LoadSNGIv0(&file, song, version & 0xff);
				// fix for a bug existing in the song saver in the 1.7.x series
				if(version == 0) size = 11 * sizeof(std::uint32_t) + song.tracks() * 2 * sizeof(bool);
			}
			//else if((version & 0xff00) == 0x0100) // and so on
		} else if(!std::strcmp(header,"SEQD")) {
			//song.progress.emit(2, 0, "Loading... Song sequence...");
			--chunkcount;
			problemfound=false;
			file.Read(version);
			file.Read(size);
			fileposition = file.GetPos();
			if ((version & 0xff00) == 0) { // chunkformat v0
				LoadSEQDv0(&file, song, version & 0xff);
			}
			//else if((version & 0xff00) == 0x0100) // and so on
		} else if(!std::strcmp(header,"PATD")) {
			//progress.emit(2, 0, "Loading... Song patterns...");
			--chunkcount;
			problemfound=false;
			file.Read(version);
			file.Read(size);
			fileposition = file.GetPos();
			if((version  & 0xff00) == 0) { // chunkformat v0
				LoadPATDv0(&file, song, version & 0xff);
				//\ Fix for a bug existing in the Song Saver in the 1.7.x series
				if((version == 0x0000) &&( file.GetPos() == fileposition+size+4)) size += 4;
			}
			//else if((version & 0xff00) == 0x0100) // and so on
		} else if(!std::strcmp(header,"MACD")) {
			//song.progress.emit(2, 0, "Loading... Song machines...");
			--chunkcount;
			problemfound=false;
			file.Read(version);
			file.Read(size);
			fileposition = file.GetPos();
			if((version & 0xff00) == 0) {// chunkformat v0
				Machine* mac = LoadMACDv0(&file, song, version & 0xff);
				//Bugfix.
				#if defined DIVERSALIS__OS__MICROSOFT
					if (mac->getMachineKey().host() == Hosts::VST
						&& ((vst::plugin*)mac)->ProgramIsChunk() == false) {
							size = file.GetPos() - fileposition;
					}
				#else
					///\todo
				#endif
			}
			//else if((version & 0xff00) == 0x0100 ) //and so on
		} else if(!std::strcmp(header,"INSD")) {
			//song.progress.emit(2, 0, "Loading... Song instruments...");
			--chunkcount;
			problemfound=false;
			file.Read(version);
			file.Read(size);
			fileposition = file.GetPos();
			if((version & 0xff00) == 0) {// chunkformat v0
				LoadINSDv0(&file,song,version&0x00FF);
			}
			//else if((version & 0xff00) == 0x0100) // and so on
		} else if(!std::strcmp(header,"EINS")) {
			//song.progress.emit(2, 0, "Loading... Extended Song instruments...");
			--chunkcount;
			problemfound=false;
			file.Read(version);
			file.Read(size);
			fileposition = file.GetPos();
			if((version & 0xffff0000) == FILE_VERSION_XMSAMPLER_ONE) {// chunkformat v1
				LoadEINSv1(&file,song,version&0xFFFF, size);
			}
			//else if((version & 0xff00) == 0x0100) // and so on
		} else {
			if (!problemfound) {
				//loggers::warning("invalid position or unknown chunk found. skipping it.");
				//song.progress.emit(2, 0, "Loading... invalid position or unknown chunk found. skipping it...");
				std::ostringstream s;
				s << "invalid position or unknown chunk found:" << header << ", position:" << file.GetPos();
				//loggers::trace(s.str());
				std::cerr << "psycle: core: psy3 loader: " << s << '\n';
			}
			problemfound = true;
			// shift back 3 bytes and try again
			fileposition = file.GetPos()-3;
			size = 0;
		}
		// For invalid version chunks, or chunks that haven't been read correctly/completely.
		if(file.GetPos() != fileposition+size) {
			///\todo: verify how it works with invalid data.
			//if (file.GetPos() > fileposition+size) loggers::trace("Cursor ahead of size! resyncing with chunk size.");
			//else loggers::trace("Cursor still inside chunk, resyncing with chunk size.");
			file.Seek(fileposition+size);
		}
	}

	//song.progress.emit(4,16384,"");

	///\todo: Move this to something like "song.validate()"

	// now that we have loaded all the patterns, time to prepare them.
	double pos = 0;
	std::vector<int>::iterator it = seqList.begin();
	for(; it < seqList.end(); ++it) {
		#if 0
			Pattern* pat = song.patternSequence().PatternPool()->findById(*it);
		#else
			Pattern* pat = song.patternSequence().FindPattern(*it);
		#endif
		singleLine->createEntry(pat, pos);
		pos += pat->beats();
	}

	// test all connections for invalid machines. disconnect invalid machines.
	for(int i(0) ; i < MAX_MACHINES ; ++i) if(song.machine(i)) {
		Machine* mac = song.machine(i);
		mac->_connectedInputs = 0;
		mac->_connectedOutputs = 0;
		for(int c(0) ; c < MAX_CONNECTIONS ; ++c) {
			if(mac->_connection[c]) {
				if(mac->_outputMachines[c] < 0 || mac->_outputMachines[c] >= MAX_MACHINES) {
					mac->_connection[c] = false;
					mac->_outputMachines[c] = -1;
				} else if(!song.machine(mac->_outputMachines[c])) {
					mac->_connection[c] = false;
					mac->_outputMachines[c] = -1;
				} else ++mac->_connectedOutputs;
			} else mac->_outputMachines[c] = -1;

			if(mac->_inputCon[c]) {
				if(mac->_inputMachines[c] < 0 || mac->_inputMachines[c] >= MAX_MACHINES) {
					mac->_inputCon[c] = false;
					mac->_inputMachines[c] = -1;
				} else if (!song.machine(mac->_inputMachines[c])) {
					mac->_inputCon[c] = false;
					mac->_inputMachines[c] = -1;
				} else {
					++mac->_connectedInputs;
					mac->_wireMultiplier[c]=song.machine(mac->_inputMachines[c])->GetAudioRange() / mac->GetAudioRange();
				}
			} else song.machine(i)->_inputMachines[c] = -1;
		}
	}
	RestoreMixerSendFlags(song);
	//song.progress.emit(5,0,"");
	if(chunkcount) {
		if(!song.machine(MASTER_INDEX)) {
			Machine* mac = MachineFactory::getInstance().CreateMachine(MachineKey::master(), MASTER_INDEX);
			song.AddMachine(mac);
		}
		std::ostringstream s;
		s << "Error reading from file '" << file.file_name() << "'" << std::endl;
		s << "some chunks were missing in the file";
		//loggers::trace(s.str());
		std::cerr << "psycle: core: psy3 loader: " << s << '\n';
		//report.emit(s.str(), "Song Load Error.");
	}
	///\todo:
	return true;
}

int Psy3Filter::LoadSONGv0(RiffFile* file,CoreSong& /*song*/) {
	std::int32_t fileversion = 0;
	std::uint32_t size = 0;
	std::uint32_t chunkcount = 0;

	file->Read(fileversion);
	file->Read(size);
	if(fileversion > CURRENT_FILE_VERSION) {
		//report.emit("This file is from a newer version of Psycle! This process will try to load it anyway.", "Load Warning");
	}

	file->Read(chunkcount);
	int bytesread(4);
	if(size > 4) {
		// This is left here if someday, extra data is added to the file version chunk.
		// update "bytesread" accordingly.

		//file->Read(chunkversion);
		//if((chunkversion&0xFF00) ) == x) {} else if(...) {}

		file->Skip(size - bytesread);// Size of the current header DATA // This ensures that any extra data is skipped.
	}
	return chunkcount;
}

bool Psy3Filter::LoadINFOv0(RiffFile* file,CoreSong& song,int /*minorversion*/) {
	char Name[129]; char Author[65]; char Comment[65536];

	file->ReadString(Name, 128);
	song.setName(Name);
	file->ReadString(Author, 64);
	song.setAuthor(Author);
	bool result = file->ReadString(Comment, 65535);
	song.setComment(Comment);
	return result;
}

bool Psy3Filter::LoadSNGIv0(RiffFile* file,CoreSong& song,int /*minorversion*/) {
	MachineCallbacks* callbacks = MachineFactory::getInstance().getCallbacks();
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
	{ ///\todo: This was a hack added in 1.9alpha to allow decimal BPM values
		file->Read(temp16);
		int BPMCoarse = temp16;
		file->Read(temp16);
		song.setBpm(BPMCoarse + temp16 / 100.0f );
	}
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
	for(unsigned int i(0) ; i < song.tracks(); ++i) {
		bool tmp=0;
		file->Read(tmp);
		song.patternSequence().setMutedTrack(i,tmp);
		fileread = file->Read(tmp);
		song.patternSequence().setArmedTrack(i,tmp);
	}
	callbacks->timeInfo().setBpm(song.bpm());
	callbacks->timeInfo().setTicksSpeed(song.ticksSpeed(),song.isTicks());
	return fileread;
}

bool Psy3Filter::LoadSEQDv0(RiffFile* file,CoreSong& /*song*/,int /*minorversion*/) {
	std::int32_t index = 0;
	std::uint32_t temp;
	bool fileread = false;
	// index, for multipattern - for now always 0
	file->Read(index);
	if(index < MAX_SEQUENCES) {
		char pTemp[256];
		// play length for this sequence
		file->Read(temp);
		int playLength = temp;
		// name, for multipattern, for now unused
		file->ReadString(pTemp, sizeof pTemp);
		for(int i(0) ; i < playLength; ++i) {
			fileread = file->Read(temp);
			seqList.push_back(temp);
		}
	}
	return fileread;
}

PatternEvent Psy3Filter::convertEntry(unsigned char * data) const {
	PatternEvent event;
	event.setNote(*data); data++;
	event.setInstrument(*data); data++;
	event.setMachine(*data); data++;
	event.setCommand(*data); data++;
	event.setParameter(*data); data++;
	return event;
}

bool Psy3Filter::LoadPATDv0(RiffFile* file,CoreSong& song,int /*minorversion*/) {
	std::int32_t index = 0;
	std::uint32_t temp = 0;
	std::uint32_t size = 0;
	bool fileread=false;

	// index
	file->Read(index);
	//todo: This loading is quite slow now. Needs an improvement
	if(index < MAX_PATTERNS) {
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
		// create a Pattern
		std::string indexStr;
		std::ostringstream o;
		if(!(o << index))
			indexStr = "error";
		else
			indexStr = o.str();
		Pattern* pat = new Pattern();
		pat->setName(patternName+indexStr);
		pat->setID(index);
		song.patternSequence().Add(pat);
		float beatpos=0;
		for(int y(0) ; y < numLines ; ++y) { // lines
			for (unsigned int x = 0; x < song.tracks(); ++x) {
				unsigned char entry[5] ;
				std::memcpy( &entry, pSource, 5);
				PatternEvent event = convertEntry(entry);
				if(!event.empty()) {
					if(event.note() == notetypes::tweak) {
						event.set_track(x);
						pat->insert(beatpos, event);
					} else if(event.note() == notetypes::tweak_slide) {
						event.set_track(x);
						pat->insert(beatpos, event);
					} else if(event.note() == notetypes::midi_cc) {
						event.set_track(x);
						pat->insert(beatpos, event);
					///\todo: Also, move the Global commands (tempo, mute..) out of the pattern.
					} else {
						if(event.command() == commandtypes::NOTE_DELAY)
							/// Convert old value (part of line) to new value (part of beat)
							event.setParameter(event.parameter()/linesPerBeat);
						event.set_track(x);
						pat->insert(beatpos, event);
					}
					if((event.note() <= notetypes::release || event.note() == notetypes::empty) && event.command() == 0xfe && event.parameter() < 0x20)
						linesPerBeat= event.parameter() & 0x1f;
				}
				pSource += EVENT_SIZE;
			}
			beatpos += 1 / static_cast<float>(linesPerBeat);
		}
		delete[] pDest; pDest = 0;
		pat->timeSignatures().clear();
		pat->timeSignatures().push_back(TimeSignature(beatpos));
	}
	return fileread;
}

Machine* Psy3Filter::LoadMACDv0(RiffFile * file, CoreSong & song, int minorversion) {
	MachineFactory& factory = MachineFactory::getInstance();
	std::int32_t index = 0;

	file->Read(index);
	if(index < MAX_MACHINES) {
		Machine::id_type const id(index);
		// assume version 0 for now
		std::int32_t type;
		Machine* mac=0;
		char sDllName[256];
		file->Read(type);
		file->ReadString(sDllName, 256);
		bool failedLoad = false;
		switch(type) {
			case MACH_MASTER:
				mac = factory.CreateMachine(MachineKey::master(), MASTER_INDEX);
				break;
			case MACH_SAMPLER:
				mac = factory.CreateMachine(MachineKey::sampler(), id);
				break;
			case MACH_MIXER:
				mac = factory.CreateMachine(MachineKey::mixer(), id);
				break;
			case MACH_XMSAMPLER:
				mac = factory.CreateMachine(MachineKey::sampulse(), id);
				break;
			case MACH_DUPLICATOR:
				mac = factory.CreateMachine(MachineKey::duplicator(), id);
				break;
			case MACH_AUDIOINPUT:
				mac = factory.CreateMachine(MachineKey::audioinput(), id);
				break;
			//case MACH_LFO:
				//mac = factory.CreateMachine(MachineKey::lfo(), id);
				//break;
			//case MACH_SCOPE:
			case MACH_DUMMY:
				mac = factory.CreateMachine(MachineKey::dummy(), id);
				break;
			case MACH_PLUGIN:
			{
				mac = factory.CreateMachine(MachineKey(Hosts::NATIVE, sDllName, 0), id);
				break;
			}
			case MACH_VST:
			case MACH_VSTFX:
			{
				mac = factory.CreateMachine(MachineKey(Hosts::VST, sDllName, 0), id);
				break;
			}
			default: ;
		}
		if(!mac) {
			std::ostringstream s;
			s << "Problem loading machine!" << std::endl << "type: " << type << ", dllName: " << sDllName;
			//MessageBox(0, s.str().c_str(), "Loading old song", MB_ICONERROR);
			mac = factory.CreateMachine(MachineKey::dummy(),id);
			failedLoad = true;
		}
		song.AddMachine(mac);
		mac->LoadFileChunk(file,minorversion);
		if(failedLoad) mac->SetEditName(mac->GetEditName() + " (replaced)");
	}
	return song.machine(index);
}

bool Psy3Filter::LoadINSDv0(RiffFile* file,CoreSong& song,int minorversion) {
	std::int32_t index = 0;
	file->Read(index);
	if(index < MAX_INSTRUMENTS) song._pInstrument[index]->LoadFileChunk(file, minorversion, true);
	///\todo:
	return true;
}

bool Psy3Filter::LoadEINSv1(RiffFile* file, CoreSong& song, int minorversion, std::uint32_t size) {

	// Instrument Data Load
	int numInstruments;
	long begins=file->GetPos();
	unsigned long filepos=file->GetPos();
	file->Read(numInstruments);
	int idx;
	for(int i = 0;i < numInstruments && filepos < begins+size;i++)
	{
		file->Read(idx);
		filepos = file->GetPos();
		unsigned int sizeIns = song.rInstrument(idx).Load(*file);
		if ((minorversion&0xFFFF) > 0) {
			//Version 0 doesn't write the chunk size correctly
			//so we cannot correct it in case of error
			file->Seek(filepos+sizeIns);
			filepos=file->GetPos();
		}
	}
	int numSamples;
	file->Read(numSamples);
	for(int i = 0;i < numSamples && filepos < begins+size;i++)
	{
		file->Read(idx);
		filepos = file->GetPos();
		unsigned int sizeSamp = song.SampleData(idx).Load(*file);
		if ((minorversion&0xFFFF) > 0) {
			//Version 0 doesn't write the chunk size correctly
			//so we cannot correct it in case of error
			file->Seek(filepos+sizeSamp);
			filepos=file->GetPos();
		}
	}
	filepos=file->GetPos();
	return begins+size == filepos;

}

void Psy3Filter::RestoreMixerSendFlags(CoreSong& song) {
	for(int i(0);i < MAX_MACHINES; ++i) if(song.machine(i) && song.machine(i)->getMachineKey() == MachineKey::mixer()) {
		Mixer & mac = static_cast<Mixer&>(*song.machine(i));
		for(int j(0); j < mac.numreturns(); ++j) if(mac.Return(j).IsValid())
			song.machine(mac.Return(j).Wire().machine_)->SetMixerSendFlag(&song);
	}
}

}}
