#include <packageneric/pre-compiled.private.hpp>
#include PACKAGENERIC
#include <psycle/engine/psy3loader.hpp>
#include <psycle/engine/FileIO.hpp>
#include <psycle/engine/song.hpp>
#include <psycle/helpers/DataCompression.hpp>

namespace psycle {
	namespace host {

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

		std::uint32_t const Psy3Filter::FILE_VERSION      = Psy3Filter::VERSION_INFO + Psy3Filter::VERSION_SNGI + Psy3Filter::VERSION_SEQD
			+ Psy3Filter::VERSION_PATD	+ Psy3Filter::VERSION_PATD + Psy3Filter::VERSION_MACD +	Psy3Filter::VERSION_INSD + Psy3Filter::VERSION_WAVE;

		Psy3Filter::Psy3Filter()
		{
		}
		Psy3Filter::~Psy3Filter()
		{
		}


		bool Psy3Filter::Test(std::string fourcc)
		{
			return fourcc == FILE_FOURCC;
		}
		bool Psy3Filter::Load(RiffFile* file, Song& song)
		{
//			loggers::trace("file header: PSY3SONG");
			progress.emit(1,0,"");
			progress.emit(2,0,"Loading... psycle song fileformat version 3...");

			//\todo:
			song.New();
			std::fpos_t filesize = file->FileSize();
			std::uint32_t version = 0;
			std::uint32_t size = 0;
			char header[5];
			header[4]=0;
			std::uint32_t chunkcount = LoadSONGv0(file,song);
			/* chunk_loop: */
			while(file->ReadChunk(&header, 4) && chunkcount)
			{
//				std::string readchunk = "Reading chunk: "; readchunk += header;
//				loggers::trace(readchunk);

				file->Read(version);
				file->Read(size);

				int fileposition = file->GetPos();
				progress.emit(4,f2i((fileposition*16384.0f)/filesize),"");

				if(std::strcmp(header,"INFO") == 0)
				{
					progress.emit(2,0,"Loading... Song authorship information...");
					if ((version&0xFF00) == 0x0000) // chunkformat v0
					{
						LoadINFOv0(file,song,version&0x00FF);
					}
					//else if ( (version&0xFF00) == 0x0100 ) //and so on
				}
				else if(std::strcmp(header,"SNGI")==0)
				{
					progress.emit(2,0,"Loading... Song properties information...");
					if ((version&0xFF00) == 0x0000) // chunkformat v0
					{
						LoadSNGIv0(file,song,version&0x00FF);
						//\ Fix for a bug existing in the Song Saver in the 1.7.x series
						if (version == 0x0000) size = 76; 
					}
					//else if ( (version&0xFF00) == 0x0100 ) //and so on
				}
				else if(std::strcmp(header,"SEQD")==0)
				{
					progress.emit(2,0,"Loading... Song sequence...");
					if ((version&0xFF00) == 0x0000) // chunkformat v0
					{
						LoadSEQDv0(file,song,version&0x00FF);
					}
					//else if ( (version&0xFF00) == 0x0100 ) //and so on
				}
				else if(std::strcmp(header,"PATD") == 0)
				{
					progress.emit(2,0,"Loading... Song patterns...");
					if ((version&0xFF00) == 0x0000) // chunkformat v0
					{
						LoadPATDv0(file,song,version&0x00FF);
						//\ Fix for a bug existing in the Song Saver in the 1.7.x series
						if (version == 0x0000) size += 4; 
					}
					//else if ( (version&0xFF00) == 0x0100 ) //and so on
				}
				else if(std::strcmp(header,"MACD") == 0)
				{
					progress.emit(2,0,"Loading... Song machines...");
					if ((version&0xFF00) == 0x0000) // chunkformat v0
					{
						LoadMACDv0(file,song,version&0x00FF);
					}
					//else if ( (version&0xFF00) == 0x0100 ) //and so on
				}
				else if(std::strcmp(header,"INSD") == 0)
				{
					progress.emit(2,0,"Loading... Song instruments...");
					if ((version&0xFF00) == 0x0000) // chunkformat v0
					{
						LoadINSDv0(file,song,version&0x00FF);
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
						file->Skip(size);
					}
				}

				// For invalid version chunks, or chunks that haven't been read correctly/completely.
				if  (file->GetPos() != fileposition+size)
				{
					//\todo: verify how it works with invalid data.
//					if (file->GetPos() > fileposition+size) loggers::trace("Cursor ahead of size! resyncing with chunk size.");
//					else loggers::trace("Cursor still inside chunk, resyncing with chunk size.");
					file->Seek(fileposition+size);
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
				s << "Error reading from file '" << file->file_name() << "'" << std::endl;
				s << "some chunks were missing in the file";
				report.emit(s.str(), "Song Load Error.");
			}
			//\todo:
			return true;
		}
		bool Psy3Filter::Save(RiffFile* file,const Song& song)
		{
			if ( !autosave ) 
			{
				progress.emit(1,0,"");
				progress.emit(2,0,"Saving...");
			}

			file->WriteChunk("PSY3",4);
			if ( !autosave ) 
			{
				SaveSONGv0(file,song);
				progress.emit(4,-1,"");
				SaveINFOv0(file,song);
				progress.emit(4,-1,"");
				SaveSNGIv0(file,song);
				progress.emit(4,-1,"");
				SaveSEQDv0(file,song);
				progress.emit(4,-1,"");
			}
			else
			{
				SaveSONGv0(file,song);
				SaveINFOv0(file,song);
				SaveSNGIv0(file,song);
				SaveSEQDv0(file,song);
			}

			for(std::uint32_t index(0) ; index < MAX_PATTERNS; ++index)
			{
				// check every pattern for validity
				if (IsPatternUsed(index))
				{
					SavePATDv0(file,song);
					if ( !autosave ) 
					{
						progress.emit(4,-1,"");
					}
				}
			}
			for(std::uint32_t index(0) ; index < MAX_MACHINES; ++index)
			{
				if (_pMachine[index])
				{
					SaveMACDv0(file,song);
					if ( !autosave ) 
					{
						progress.emit(4,-1,"");
					}
				}
			}
			for(std::uint32_t index(0) ; index < MAX_INSTRUMENTS; ++index)
			{
				if (!_pInstrument[index]->Empty())
				{
					SaveINSDv0(file,song);
					if ( !autosave ) 
					{
						progress.emit(4,-1,"");
					}
				}
			}
			//\todo:
			return true;
		}


		int Psy3Filter::LoadSONGv0(RiffFile* file,Song& song)
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



		bool Psy3Filter::LoadINFOv0(RiffFile* file,Song& song,int minorversion)
		{
			file->ReadString(song.Name, 64);
			file->ReadString(song.Author, 64);
			return file->ReadString(song.Comment, 256);
		}
		bool Psy3Filter::LoadSNGIv0(RiffFile* file,Song& song,int minorversion)
		{
			std::uint32_t temp(0);
			std::uint16_t temp16(0);
			bool fileread = false;

			// why all these temps?  to make sure if someone changes the defs of
			// any of these members, the rest of the file reads ok.  assume 
			// everything is 32-bit, when we write we do the same thing.

			// # of tracks for whole song
			file->Read(temp);
			song.tracks(temp);
			// bpm
			file->Read(temp16);
			int BPMCoarse = temp16;
			file->Read(temp16);
			song.m_BeatsPerMin = BPMCoarse + temp16/100.0f;
			// tpb
			file->Read(temp);
			song.m_LinesPerBeat = temp;
			// current octave
			file->Read(temp);
			song.currentOctave = temp;
			// machineSoloed
			//\todo:
			// we need fix destroy machine, because it clears machineSoloed
			file->Read(temp);
			song.machineSoloed = temp;
			// trackSoloed
			file->Read(temp);
			song._trackSoloed = temp;
			file->Read(temp);
			song.seqBus = temp;
			file->Read(temp);
			song.midiSelected = temp;
			file->Read(temp);
			song.auxcolSelected = temp;
			file->Read(temp);
			song.instSelected = temp;
			// sequence width, for multipattern
			file->Read(temp);
			song._trackArmedCount = 0;
			for(int i(0) ; i < song.tracks(); ++i)
			{
				file->Read(song._trackMuted[i]);
				// remember to count them
				fileread = file->Read(song._trackArmed[i]);
				if(song._trackArmed[i]) ++song._trackArmedCount;
			}
//			Global::player().SetBPM(song.m_BeatsPerMin,song.m_LinesPerBeat);
			return fileread;
		}


		bool Psy3Filter::LoadSEQDv0(RiffFile* file,Song& song,int minorversion)
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
				song.playLength = temp;
				// name, for multipattern, for now unused
				file->ReadString(pTemp, sizeof pTemp);
				for (int i(0) ; i < song.playLength; ++i)
				{
					fileread = file->Read(temp);
					song.playOrder[i] = temp;
				}
			}
			return fileread;
		}

		bool Psy3Filter::LoadPATDv0(RiffFile* file,Song& song,int minorversion)
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
				song.patternLines[index] = temp;
				// num tracks per pattern // eventually this may be variable per pattern, like when we get multipattern
				file->Read(temp);
				file->ReadString(song.patternName[index], sizeof *song.patternName);
				file->Read(size);
				unsigned char * pSource = new unsigned char[size];
				fileread = file->ReadChunk(pSource, size);
				unsigned char * pDest;
				BEERZ77Decomp2(pSource, &pDest);
				zapArray(pSource,pDest);
				for(int y(0) ; y < song.patternLines[index] ; ++y)
				{
					unsigned char* pData(song._ppattern(index) + (y * MULTIPLY));
					std::memcpy(pData, pSource, song.tracks() * EVENT_SIZE);
					pSource += song.tracks() * EVENT_SIZE;
				}
				zapArray(pDest);
			}
			return fileread;
		}

		bool Psy3Filter::LoadMACDv0(RiffFile* file,Song& song,int minorversion)
		{
			std::uint32_t index = 0;

			file->Read(index);
			if(index < MAX_MACHINES)
			{
				Machine::id_type const id(index);
				song._pMachine[index] = Machine::LoadFileChunk(file, id, minorversion, true);
			}
			return (bool)song._pMachine[index];
		}
		bool Psy3Filter::LoadINSDv0(RiffFile* file,Song& song,int minorversion)
		{
			std::uint32_t index = 0;
			file->Read(index);
			if(index < MAX_INSTRUMENTS)
			{
				song._pInstrument[index]->LoadFileChunk(file, minorversion, true);
			}
			//\todo:
			return true;
		}



		bool Psy3Filter::SaveSONGv0(RiffFile* file,const Song& song)
		{
			std::uint32_t chunkcount;
			// chunk header;
			{
				file->WriteChunk("SONG",4);
				version = FILE_VERSION;
				file->Write(version);
				size = sizeof chunkcount;
				file->Write(size);

			}
			chunkcount = 3; // 3 chunks plus:
			for(unsigned int i(0) ; i < MAX_PATTERNS    ; ++i) if(song.IsPatternUsed(i))          ++chunkcount; // check every pattern for validity
			for(unsigned int i(0) ; i < MAX_MACHINES    ; ++i) if(song._pMachine[i])              ++chunkcount;
			for(unsigned int i(0) ; i < MAX_INSTRUMENTS ; ++i) if(!song._pInstrument[i]->Empty()) ++chunkcount;

			if ( !autosave ) 
			{
				progress.emit(3,chunkcount,"");
			}
			// chunk data
			{
				file->Write(chunkcount);
			}
		}
		bool Psy3Filter::SaveINFOv0(RiffFile* file,const Song& song)
		{
			// chunk header
			{
				pFile->WriteChunk("INFO",4);

				version = VERSION_INFO;
				pFile->Write(version);

				size = strlen(Name)+strlen(Author)+strlen(Comment)+3; // [bohan] since those are variable length, we could change from fixed size arrays to std::string
				pFile->Write(size);
			}
			// chunk data
			{
				pFile->WriteChunk(Name,strlen(Name)+1);
				pFile->WriteChunk(Author,strlen(Author)+1);
				pFile->WriteChunk(Comment,strlen(Comment)+1);
			}
		}
		bool Psy3Filter::SaveSNGIv0(RiffFile* file,const Song& song)
		{
			// chunk header
			{
				pFile->WriteChunk("SNGI",4);

				version = VERSION_SNGI;
				pFile->Write(version);

				size = (11*sizeof(temp))+(tracks()*(sizeof(_trackMuted[0])+sizeof(_trackArmed[0])));
				pFile->Write(size);
			}
			// chunk data
			{
				temp = tracks();     pFile->Write(temp);
				temp16 = int(floor(m_BeatsPerMin));							pFile->Write(temp16);
				temp16 = int((m_BeatsPerMin-floor(m_BeatsPerMin))*100);		pFile->Write(temp16);
				temp = m_LinesPerBeat; pFile->Write(temp);
				temp = currentOctave;  pFile->Write(temp);
				temp = machineSoloed;  pFile->Write(temp);
				temp = _trackSoloed;   pFile->Write(temp);

				temp = seqBus; pFile->Write(temp);

				temp = midiSelected; pFile->Write(temp);
				temp = auxcolSelected; pFile->Write(temp);
				temp = instSelected; pFile->Write(temp);

				temp = 1;  pFile->Write(temp); // sequence width

				for(int i = 0; i < tracks(); i++)
				{
					pFile->Write(_trackMuted[i]);
					pFile->Write(_trackArmed[i]); // remember to count them
				}
			}

		}
		bool Psy3Filter::SaveSEQDv0(RiffFile* file,const Song& song)
		{
			for(std::uint32_t index(0) ; index < MAX_SEQUENCES ; ++index)
			{
				char* pSequenceName = "seq0\0"; // This needs to be replaced when converting to Multisequence.

				// chunk header
				{
					pFile->WriteChunk("SEQD",4);

					version = CURRENT_FILE_VERSION_SEQD;
					pFile->Write(version);

					size = ((playLength+2)*sizeof(temp))+strlen(pSequenceName)+1;
					pFile->Write(size);
				}
				// chunk data
				{
					pFile->Write(index); // Sequence Track number
					temp = playLength; pFile->Write(temp); // Sequence length

					pFile->WriteChunk(pSequenceName,strlen(pSequenceName)+1); // Sequence Name

					for (int i = 0; i < playLength; i++)
					{
						temp = playOrder[i]; pFile->Write(temp); // Sequence data.
					}
				}
			}
		}
		bool Psy3Filter::SavePATDv0(RiffFile* file,const Song& song)
		{
			unsigned char * pSource = new unsigned char[tracks()*patternLines[index]*EVENT_SIZE];
			unsigned char * pCopy = pSource;

			for (int y = 0; y < patternLines[index]; y++)
			{
				unsigned char * pData = ppPatternData[index]+(y*MULTIPLY);
				std::memcpy(pCopy,pData,EVENT_SIZE*tracks());
				pCopy+=EVENT_SIZE*tracks();
			}

			std::uint32_t sizez77 = BEERZ77Comp2(pSource, &pCopy, tracks()*patternLines[index]*EVENT_SIZE);
			delete[] pSource;

			// chunk header
			{
				pFile->WriteChunk("PATD",4);

				version = CURRENT_FILE_VERSION_PATD;
				pFile->Write(version);

				size = sizez77 + 4 * sizeof temp + strlen(patternName[index]) + 1;
				pFile->Write(size);
			}
			// chunk data
			{
				pFile->Write(index);
				temp = patternLines[index]; pFile->Write(temp);
				temp = tracks(); pFile->Write(temp); // eventually this may be variable per pattern

				pFile->WriteChunk(&patternName[index],strlen(patternName[index])+1);

				pFile->Write(sizez77);
				pFile->WriteChunk(pCopy,sizez77);
			}

			delete[] pCopy;
		}
		bool Psy3Filter::SaveMACDv0(RiffFile* file,const Song& song)
		{
			std::fpos_t pos;

			// chunk header
			{
				pFile->WriteChunk("MACD",4);

				version = CURRENT_FILE_VERSION_MACD;
				pFile->Write(version);

				pos = pFile->GetPos();

				size = 0;
				pFile->Write(size);
			}
			// chunk data
			{
				pFile->Write(index);
				_pMachine[index]->SaveFileChunk(pFile);
			}
			// chunk size in header
			{
				std::fpos_t const pos2(pFile->GetPos());
				size = pos2 - pos - sizeof size;
				pFile->Seek(pos);
				pFile->Write(size);
				pFile->Seek(pos2);
			}
		}
		bool Psy3Filter::SaveINSDv0(RiffFile* file,const Song& song)
		{
			std::fpos_t pos;

			// chunk header
			{
				pFile->WriteChunk("INSD",4);

				version = CURRENT_FILE_VERSION_INSD;
				pFile->Write(version);

				pos = pFile->GetPos();

				size = 0;
				pFile->Write(size);
			}
			// chunk data
			{
				pFile->Write(index);
				_pInstrument[index]->SaveFileChunk(pFile);
			}
			// chunk size in header
			{
				std::fpos_t const pos2(pFile->GetPos());
				size = pos2 - pos - sizeof size;
				pFile->Seek(pos);
				pFile->Write(size);
				pFile->Seek(pos2);
			}
		}








	}
}