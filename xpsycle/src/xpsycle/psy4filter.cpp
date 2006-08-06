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

namespace psycle {
	namespace host {

		std::uint32_t const Psy4Filter::VERSION_SNGI = 0x0001;

		Psy4Filter::Psy4Filter()
			: Psy3Filter()
		{
		}


		Psy4Filter::~Psy4Filter()
		{
		}

		bool Psy4Filter::testFormat( const std::string & fileName )
		{
			RiffFile file;
			file.Open(fileName);
			char Header[9];
			file.ReadChunk(&Header, 8);
			Header[8]=0;
			file.Close();
			if (strcmp(Header,"PSY4SONG")==0) return true;
			return false;
		}

		bool Psy4Filter::LoadSNGIv0( RiffFile * file, Song & song, int minorversion )
		{
			std::uint32_t temp   = 0;
			std::uint16_t temp16 = 0;
			bool fileread = false;

			// # of tracks for whole song
			file->Read(temp);
			song.setTracks(temp);
			// bpm
			file->Read(temp16);
			int BPMCoarse = temp16;
			file->Read(temp16);
			song.setBpm( BPMCoarse + temp16/100.0f );
			// tpb
			song.m_LinesPerBeat = 4;
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

			return fileread;

		}

		void Psy4Filter::save( const std::string & fileName, const Song & song ) const
		{
		}

		bool Psy4Filter::SaveSONGv0( RiffFile * file, const Song & song )
		{
			//\todo:
			bool autosave=false;
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
			}
			else
			{
				SaveSONGv0(file,song);
				SaveINFOv0(file,song);
				SaveSNGIv0(file,song);
			}

			SaveSEQDv0(file,song);
			SavePATDv0(file,song);

			for(std::uint32_t index(0) ; index < MAX_MACHINES; ++index)
			{
				if (song._pMachine[index])
				{
					SaveMACDv0(file,song,index);
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
					SaveINSDv0(file,song,index);
					if ( !autosave )
					{
						progress.emit(4,-1,"");
					}
				}
			}
			//\todo:
			return true;
		}

		bool Psy4Filter::SaveINFOv0( RiffFile * file, const Song & song )
		{
			uint32_t version, size;
			// chunk header

			file->WriteChunk("INFO",4);

			version = VERSION_INFO;
			file->Write(version);

			size = song.name().length() + song.author().length()+song.comment().length()+3;
			file->Write(size);

			// chunk data

			file->WriteChunk(song.name().c_str(),song.name().length()+1);
			file->WriteChunk(song.author().c_str(),song.author().length()+1);
			file->WriteChunk(song.comment().c_str(),song.comment().length()+1);

			//\todo:
			return true;
		}

		bool Psy4Filter::SaveSNGIv0( RiffFile * file, const Song & song )
		{
			std::uint32_t version, size, temp;
			std::uint16_t temp16;
			// chunk header

			file->WriteChunk("SNGI",4);

			version = VERSION_SNGI;
			file->Write(version);

			size = 10;
			file->Write(size*sizeof(temp));

			// chunk data

			temp = song.tracks();
			file->Write(temp);
			temp16 = int(floor(song.bpm()));
			file->Write(temp16);
			temp16 = int((song.bpm()-floor(song.bpm()))*100);
			file->Write(temp16);
			temp = song.currentOctave;
			file->Write(temp);
			temp = song.machineSoloed;
			file->Write(temp);
			temp = song._trackSoloed;
			file->Write(temp);

			temp = song.seqBus;
			file->Write(temp);

			temp = song.midiSelected;
			file->Write(temp);
			temp = song.auxcolSelected;
			file->Write(temp);
			temp = song.instSelected;
			file->Write(temp);

			return true;
		}

		bool Psy4Filter::SaveSEQDv0( RiffFile * file, const Song & song )
		{
			return true;
		}

		bool Psy4Filter::SavePATDv0( RiffFile * file, const Song & song )
		{
			return true;
		}

		bool Psy4Filter::SaveMACDv0( RiffFile * file, const Song & song, int index )
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

			size_t const pos2(file->GetPos());
			size = pos2 - pos - sizeof size;
			file->Seek(pos);
			file->Write(size);
			file->Seek(pos2);
		
			return true;
		}

		bool Psy4Filter::SaveINSDv0( RiffFile * file, const Song & song, int index )
		{
			std::uint32_t version, size;
			size_t pos;

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

			size_t const pos2(file->GetPos());
			size = pos2 - pos - sizeof size;
			file->Seek(pos);
			file->Write(size);
			file->Seek(pos2);

			return true;
		}

		bool Psy4Filter::SaveWAVEv0( RiffFile * file, const Song & song, int index )
		{
			return true;
		}

		int Psy4Filter::version( ) const
		{
			return 4;
		}


	}
}


