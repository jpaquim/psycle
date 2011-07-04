/** @file 
 *  @brief implementation file
 *  $Date: 2004/10/12 01:09:29 $
 *  $Revision: 1.4 $
 */
#include "stdafx.h"
#include <boost/format.hpp>
#include "NewMachine.h"
#include "MainFrm.h"
#include "PsycleWTLView.h"
#include "ProgressDialog.h"

//	extern CPsycleApp theApp;
#include "Song.h"
#include "Instrument.h"
#include "Machine.h" // It wouldn't be needed, since it is already included in "song.h"
#include "Sampler.h"
#include "Plugin.h"
#include "VSTHost.h"
#include "DataCompression.h"

//#include <sstream>

#ifdef CONVERT_INTERNAL_MACHINES
	#include "convert_internal_machines.h" // conversion
#endif

#include "Riff.h"	 // For Wave file loading.

#if defined(_MSC_VER) && defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <cstdlib>
#include "crtdbg.h"
#define malloc(a) _malloc_dbg(a,_NORMAL_BLOCK,__FILE__,__LINE__)
    inline void*  operator new(size_t size, LPCSTR strFileName, INT iLine)
        {return _malloc_dbg(size, _NORMAL_BLOCK, strFileName, iLine);}
    inline void operator delete(void *pVoid, LPCSTR strFileName, INT iLine)
        {_free_dbg(pVoid, _NORMAL_BLOCK);}
#define new  ::new(_NORMAL_BLOCK, __FILE__, __LINE__)
#endif
#include ".\psyFsongsaver.h"

namespace SF {
	PsyFSongSaver::PsyFSongSaver(void)
	{
	}

	PsyFSongSaver::~PsyFSongSaver(void)
	{
	}

	void PsyFSongSaver::Save(RiffFile& riffFile,Song &song){
			// NEW FILE FORMAT!!!
		// this is much more flexible, making maintenance a breeze compared to that old hell.
		// now you can just update one module without breaking the whole thing.

		// header, this has to be at the top of the file

		m_Progress.Create(NULL);
		m_Progress.SetWindowText(SF::CResourceString(IDS_MSG0011));
		m_Progress.ShowWindow(SW_SHOW);

		try {
		SaveFileInfo(riffFile,song);

		// the rest of the modules can be arranged in any order

		SaveSongInfo(riffFile,song);

		m_Progress.m_Progress.StepIt();
		::Sleep(1);

		SaveSequenceData(riffFile,song);

		m_Progress.m_Progress.StepIt();
		::Sleep(1);

		SavePatternData(riffFile,song);

		SaveMachineData(riffFile,song);

		SaveInstrumentData(riffFile,song);

//		m_Progress.m_Progress.SetPos(chunkcount);
//		::Sleep(1);
		} catch(RiffFile::Exception & e){
			MessageBox(NULL,e.what(),SF::CResourceString(IDS_ERR_MSG0075),NULL);
		} catch (...)
		{
			m_Progress.OnCancel();
			throw;
		}

		m_Progress.OnCancel();

		if (!riffFile.Close())
		{
			throw Exception(
				(SF::tformat(SF::CResourceString(IDS_ERR_MSG0051)) % riffFile.szName).str());
		}
	}
	
	/// ファイル情報
	void PsyFSongSaver::SaveFileInfo(RiffFile& riffFile,Song& song)
	{
		int chunkcount = 3; // 3 chunks plus:
		for (int i = 0; i < MAX_PATTERNS; i++)
		{
			// check every pattern for validity
			if (song.IsPatternUsed(i))
			{
				chunkcount++;
			}
		}

		for (i = 0; i < MAX_MACHINES; i++)
		{
			// check every pattern for validity
			if (song.pMachine(i))
			{
				chunkcount++;
			}
		}

		for (i = 0; i < MAX_INSTRUMENTS; i++)
		{
			if (!song.pInstrument(i)->Empty())
			{
				chunkcount++;
			}
		}

		m_Progress.m_Progress.SetRange(0,chunkcount);
		m_Progress.m_Progress.SetStep(1);

		/*
		===================
		FILE HEADER
		===================
		id = "PSY3SONG"; // PSY2 was 1.66
		*/

		riffFile.Write("PSYFSONG", 8);

		UINT version = CURRENT_FILE_VERSION;
		UINT size = sizeof(chunkcount);
		UINT index = 0;

		riffFile.Write(version);
		riffFile.Write(size);
		riffFile.Write(chunkcount);

		m_Progress.m_Progress.StepIt();
		::Sleep(1);

	}
	/// ソング情報
	void PsyFSongSaver::SaveSongInfo(RiffFile& riffFile,Song& song)
	{
		/*
		===================
		SONG INFO TEXT
		===================
		id = "INFO"; 
		*/

		riffFile.Write("INFO",4);
		
		UINT version = CURRENT_FILE_VERSION_INFO;
		
		CT2A _name(const_cast<TCHAR*>(song.Name().data())),
			_author(const_cast<TCHAR*>(song.Author().data())),
			_comment(const_cast<TCHAR*>(song.Comment().data()));

//		UINT size = (song.Name().length() + song.Author().length() + song.Comment().length());
		UINT size = strlen(_name) + strlen(_author) + strlen(_comment) + 3;

		riffFile.Write(version);
		riffFile.Write(size);
		
		//# ここはUniCode化したら変える必要があります。

		riffFile.Write(_name,strlen(_name) + 1);
		riffFile.Write(_author,strlen(_author) + 1);
		riffFile.Write(_comment,strlen(_comment) + 1);

		
		//riffFile.Write(&Name,strlen(Name)+1);
		//riffFile.Write(&Author,strlen(Author)+1);
		//riffFile.Write(&Comment,strlen(Comment)+1);

		/*
		===================
		SONG INFO
		===================
		id = "SNGI"; 
		*/

		riffFile.Write("SNGI",4);
		version = CURRENT_FILE_VERSION_SNGI;
		size = (6 * sizeof(int));
		riffFile.Write(version);
		riffFile.Write(size);

		riffFile.Write(song.SongTracks());
		riffFile.Write(song.BeatsPerMin());
		riffFile.Write(song.TicksPerBeat());
		riffFile.Write(song.CurrentOctave());
		riffFile.Write(song.MachineSoloed());
		riffFile.Write(song.TrackSoloed());
		riffFile.Write(song.SeqBus());
		riffFile.Write(song.MidiSelected());
		riffFile.Write(song.AuxcolSelected());
		riffFile.Write(song.InstSelected());
		int _temp = 1;
		riffFile.Write(_temp);

		for (int i = 0; i < song.SongTracks(); i++)
		{
			riffFile.Write(song.IsTrackMuted(i));
			riffFile.Write(song.IsTrackArmed(i)); // remember to count them
		}


	}
	/// シーケンスデータ
	void PsyFSongSaver::SaveSequenceData(RiffFile& riffFile,Song& song){
		/*
		===================
		SEQUENCE DATA
		===================
		id = "SEQD"; 
		*/

		char* pSequenceName = "seq0\0";

		riffFile.Write("SEQD",4);
		UINT version = CURRENT_FILE_VERSION_SEQD;
		UINT size = ((song.PlayLength() + 2) * sizeof(int)) + strlen(pSequenceName) + 1;
		riffFile.Write(version);
		riffFile.Write(size);

		UINT index = 0; // index
		riffFile.Write(index);
		riffFile.Write(song.PlayLength());

		riffFile.Write((void *)pSequenceName,strlen(pSequenceName) + 1);

		for (int i = 0; i < song.PlayLength(); i++)
		{
			riffFile.Write(song.PlayOrder(i));
		}
	}
	/// パターンデータ
	void PsyFSongSaver::SavePatternData(RiffFile& riffFile,Song& song)
	{
		for (int i = 0; i < MAX_PATTERNS; i++)
		{
			// check every pattern for validity
			if (song.IsPatternUsed(i))
			{
				// ok save it
				byte* pSource=new byte[song.SongTracks() * song.PatternLines(i) * EVENT_SIZE];
				byte* pCopy = pSource;

				for (int y = 0; y < song.PatternLines(i); y++)
				{
					unsigned char* pData = song.pPatternData(i) + ( y * MULTIPLY);
					memcpy(pCopy,pData,EVENT_SIZE * song.SongTracks());
					pCopy += EVENT_SIZE * song.SongTracks();
				}

				int size = BEERZ77Comp2(pSource,
					&pCopy, 
					song.SongTracks() * song.PatternLines(i) * EVENT_SIZE)
					+(3 * sizeof(int)) + (_tcslen(song.PatternName(i)) + 1);
				delete pSource;

				riffFile.Write("PATD",4);
				UINT version = CURRENT_FILE_VERSION_PATD;

				riffFile.Write(version);
				riffFile.Write(size);

				UINT index = i; // index
				riffFile.Write(index);
				riffFile.Write(song.PatternLines(i));
				riffFile.Write(song.SongTracks());// eventually this may be variable per pattern
				
				CT2A _pattern_name(song.PatternName(i));
				riffFile.Write(_pattern_name,strlen(_pattern_name) + 1);

				size -= (3 * sizeof(int)) + strlen(_pattern_name) + 1;
				riffFile.Write(size);
				riffFile.Write(pCopy,size);
				delete pCopy;

				m_Progress.m_Progress.StepIt();
				::Sleep(0);

			}
	

		}
		

	}
	/// マシンデータの読み込み
	void PsyFSongSaver::SaveMachineData(RiffFile& riffFile,Song& song)
	{
		// machine and instruments handle their save and load in their respective classes

		for (int i = 0; i < MAX_MACHINES; i++)
		{
			if (song.pMachine(i))
			{
				riffFile.Write("MACD",4);
				UINT version = CURRENT_FILE_VERSION_MACD;
				riffFile.Write(version);
				long pos = riffFile.GetPos();
				UINT size = 0;
				riffFile.Write(size);

				UINT index = i; // index
				riffFile.Write(index);

				song.pMachine(i)->SaveFileChunk(riffFile);

				long pos2 = riffFile.GetPos(); 
				size = pos2 - pos - sizeof(size);
				riffFile.Seek(pos);
				riffFile.Write(size);
				riffFile.Seek(pos2);
				m_Progress.m_Progress.StepIt();
				::Sleep(0);


			}
		}

	}
	/// インストルメントデータの読み込み
	void PsyFSongSaver::SaveInstrumentData(RiffFile& riffFile,Song& song)
	{
		for (int i = 0; i < MAX_INSTRUMENTS; i++)
		{
			if (!song.pInstrument(i)->Empty())
			{
				riffFile.Write("INSD",4);
				UINT version = CURRENT_FILE_VERSION_INSD;
				riffFile.Write(&version,sizeof(version));
				long pos = riffFile.GetPos();
				UINT size = 0;
				riffFile.Write(size);

				UINT index = i; // index
				riffFile.Write(index);

				song.pInstrument(i)->SaveFileChunk(riffFile);

				long pos2 = riffFile.GetPos(); 
				size = pos2 - pos - sizeof(size);
				riffFile.Seek(pos);
				riffFile.Write(size);
				riffFile.Seek(pos2);

				m_Progress.m_Progress.StepIt();
				::Sleep(0);
			}
		}

	}
}
