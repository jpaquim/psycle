/** @file 
 *  @brief implementation file
 *  $Date$
 *  $Revision$
 */
#include "stdafx.h"
#include <boost/format.hpp>
#include "NewMachine.h"
#include "MainFrm.h"
#include "PsycleWTLView.h"
#include "ProgressDialog.h"

//	extern CPsycleApp theApp;
#include "Song.h"
#include "IPsySongLoader.h"
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
#include ".\psy3songloader.h"

namespace SF {
	Psy3SongLoader::Psy3SongLoader(void)
	{
		m_LoadFuncMap["INFO"] = &Psy3SongLoader::LoadFileInfo;
		m_LoadFuncMap["SNGI"] = &Psy3SongLoader::LoadSongInfo;
		m_LoadFuncMap["SEQD"] = &Psy3SongLoader::LoadSequenceData;
		m_LoadFuncMap["PATD"] = &Psy3SongLoader::LoadPatternData;
		m_LoadFuncMap["MACD"] = &Psy3SongLoader::LoadMachineData;
		m_LoadFuncMap["INSD"] = &Psy3SongLoader::LoadInstrumentData;

	}

	Psy3SongLoader::~Psy3SongLoader(void)
	{
	}

	void Psy3SongLoader::Load(RiffFile& riffFile,Song& song,const bool fullopen)
	{
		CProgressDialog Progress;
		Progress.Create(NULL);
		Progress.SetWindowText(_T("読み込み中..."));
		Progress.ShowWindow(SW_SHOW);

		UINT version = 0;
		UINT size = 0;
		UINT index = 0;

		int chunkcount;

		char Header[5];
		Header[4] = 0;

		long filesize = riffFile.FileSize();

		riffFile.Read(&version,sizeof(version));
		riffFile.Read(&size,sizeof(size));
		riffFile.Read(&chunkcount,sizeof(chunkcount));

		if (version > CURRENT_FILE_VERSION)
		{
			// there is an error, this file is newer than this build of psycle
			//MessageBox(NULL,_T("このﾌｧｲﾙは、使用中の Psycle よりも新しい Psycle で作成されています。新しいﾊﾞｰｼﾞｮﾝの Psycle をｹﾞｯﾄしましょう!"),NULL,NULL);
			Progress.OnCancel();
			throw SF::IPsySongLoader::Exception(
				std::string(_T("このﾌｧｲﾙは、使用中の Psycle よりも新しい Psycle で作成されています。新しいﾊﾞｰｼﾞｮﾝの Psycle をｹﾞｯﾄしましょう!")));
		}
		riffFile.Skip(size - sizeof(chunkcount));
		/*
		else
		{
			// there is currently no data in this segment
		}
		*/
		song.DestroyAllMachines();
		song.IsMachineLock(true);
		song.DeleteInstruments();
		song.DeleteAllPatterns();

		while(chunkcount > 0 && riffFile.Read(&Header, 4))
		{
			Progress.m_Progress.SetPos(f2i((riffFile.GetPos()*16384.0f)/filesize));
			::Sleep(1);
			// we should use the size to update the index, but for now we will skip it
			LoadFuncMap::iterator it = m_LoadFuncMap.find(std::string(Header));
			if(it != m_LoadFuncMap.end())
			{
				/// ヘッダに応じた読み出しメソッドを呼ぶ
				(this->*(it->second))(riffFile,song,fullopen);
				chunkcount--;
			}
			else 
			{
				// we are not at a valid header for some weird reason.  
				// probably there is some extra data.
				// shift back 3 bytes and try again
				riffFile.Skip(-3);
			}
		}
		// now that we have loaded all the modules, time to prepare them.
		
		Progress.m_Progress.SetPos(16384);
		::Sleep(1);
		// test all connections for invalid machines. disconnect invalid machines.
		for (int i = 0; i < MAX_MACHINES; i++)
		{
			if (song.pMachine(i))
			{
				song.pMachine(i)->_numInputs = 0;
				song.pMachine(i)->_numOutputs = 0;

				for (int c = 0; c < MAX_CONNECTIONS; c++)
				{
					if (song.pMachine(i)->_connection[c])
					{
						if (song.pMachine(i)->_outputMachines[c] < 0 || 
							song.pMachine(i)->_outputMachines[c] >= MAX_MACHINES)
						{
							song.pMachine(i)->_connection[c] = FALSE;
							song.pMachine(i)->_outputMachines[c] = 255;
						}
						else if (!song.pMachine(song.pMachine(i)->_outputMachines[c]))
						{
							song.pMachine(i)->_connection[c] = FALSE;
							song.pMachine(i)->_outputMachines[c] = 255;
						}
						else 
						{
							song.pMachine(i)->_numOutputs++;
						}
					}
					else
					{
						song.pMachine(i)->_outputMachines[c]=255;
					}

					if (song.pMachine(i)->_inputCon[c])
					{
						if (song.pMachine(i)->_inputMachines[c] < 0 || song.pMachine(i)->_inputMachines[c] >= MAX_MACHINES)
						{
							song.pMachine(i)->_inputCon[c] = FALSE;
							song.pMachine(i)->_inputMachines[c] = 255;
						}
						else if (!song.pMachine(song.pMachine(i)->_inputMachines[c]))
						{
							song.pMachine(i)->_inputCon[c]=FALSE;
							song.pMachine(i)->_inputMachines[c]=255;
						}
						else
						{
							song.pMachine(i)->_numInputs++;
						}
					}
					else
					{
						song.pMachine(i)->_inputMachines[c]=255;
					}
				}
			}
		}

		// translate any data that is required
		CMainFrame::GetInstance().UpdateComboGen();
		
			
		// allow stuff to work again
		song.IsMachineLock(false);

		Progress.OnCancel();

		if ((!riffFile.Close()))
		{
			std::stringstream _error;
			_error << boost::format(_T("\"%1%\" からの読み込みｴﾗｰ!!!")) % riffFile.szName;
			throw SF::IPsySongLoader::Exception(_error.str());
//			MessageBox(NULL,error,_T("ﾌｧｲﾙｴﾗｰ!!!"),0);
		}
	}
	
	void Psy3SongLoader::LoadFileInfo(RiffFile& riffFile,Song& song,const bool fullopen)
	{
		UINT version = 0;
		UINT size = 0;
		
		riffFile.Read(&version,sizeof(version));
		riffFile.Read(&size,sizeof(size));

		if (version > CURRENT_FILE_VERSION_INFO)
		{
			// there is an error, this file is newer than this build of psycle
//					MessageBox(NULL,_T("Info Seqment of File is from a newer version of psycle!"),NULL,NULL);
			riffFile.Skip(size);
		}
		else
		{
			TCHAR _buffer[256];
			song.Name(riffFile.ReadString(_buffer,Song::MAX_NAME_LEN));// Name 名前
			song.Author(riffFile.ReadString(_buffer,Song::MAX_AUTHOR_LEN));// Auhotr 作者
			song.Comment(riffFile.ReadString(_buffer,Song::MAX_COMMENT_LEN));// Comment コメント
		}

	}

	void Psy3SongLoader::LoadSongInfo(RiffFile& riffFile,Song& song,const bool fullopen)
	{
		UINT version = 0;
		UINT size = 0;
		UINT index = 0;

		int temp;

		riffFile.Read(&version,sizeof(version));
		riffFile.Read(&size,sizeof(size));
		if (version > CURRENT_FILE_VERSION_SNGI)
		{
			// there is an error, this file is newer than this build of psycle
//					MessageBox(NULL,"Song Segment of File is from a newer version of psycle!",NULL,NULL);
			riffFile.Skip(size);
		}
		else
		{
			// why all these temps?  to make sure if someone changes the defs of
			// any of these members, the rest of the file reads ok.  assume 
			// everything is an int, when we write we do the same thing.

			song.SongTracks(riffFile.ReadInt());// # of tracks for whole song
			song.BeatsPerMin(riffFile.ReadInt());// bpm
			song.TicksPerBeat(riffFile.ReadInt());//tpb
			song.CurrentOctave(riffFile.ReadInt());// current octave
			song.MachineSoloed(riffFile.ReadInt());
			song.TrackSoloed(riffFile.ReadInt());

			song.SeqBus(riffFile.ReadInt());

			song.MidiSelected(riffFile.ReadInt());
			song.AuxcolSelected(riffFile.ReadInt());
			song.InstSelected(riffFile.ReadInt());

			riffFile.Read(&temp,sizeof(temp));  // sequence width, for multipattern

			song.TrackArmedCount(0);
			for (int i = 0; i < song.SongTracks(); i++)
			{
				song.IsTrackMuted(i,riffFile.ReadBool());
				song.IsTrackArmed(i,riffFile.ReadBool());
				if (song.IsTrackArmed(i))
				{
					song.TrackArmedCount(song.TrackArmedCount() + 1);
				}
			}

			Global::pPlayer->bpm = song.BeatsPerMin();
			Global::pPlayer->tpb = song.TicksPerBeat();
			// calculate samples per tick

			song.SamplesPerTick( 
				(Global::pConfig->_pOutputDriver->_samplesPerSec * 15 * 4)
					/ (Global::pPlayer->bpm * Global::pPlayer->tpb));
		}


	}
	
	void Psy3SongLoader::LoadSequenceData(RiffFile& riffFile,Song& song,const bool fullopen)
	{
		UINT version = 0;
		UINT size = 0;
		UINT index = 0;


		riffFile.Read(&version,sizeof(version));
		riffFile.Read(&size,sizeof(size));
		if (version > CURRENT_FILE_VERSION_SEQD)
		{
			// there is an error, this file is newer than this build of psycle
//					MessageBox(NULL,"Sequence section of File is from a newer version of psycle!",NULL,NULL);
			riffFile.Skip(size);
		}
		else
		{
			riffFile.Read(&index,sizeof(index)); // index, for multipattern - for now always 0
			if (index < MAX_SEQUENCES)
			{
				//riffFile.Read(&temp,sizeof(temp)); // play length for this sequence
				song.PlayLength(riffFile.ReadInt());
				
				char pTemp[256];
				riffFile.ReadString(pTemp,sizeof(pTemp)); // name, for multipattern, for now unused

				for (int i = 0; i < song.PlayLength(); i++)
				{
					song.PlayOrder(i,riffFile.ReadInt());
				}
			}
			else
			{
//						MessageBox(NULL,"Sequence section of File is from a newer version of psycle!",NULL,NULL);
				riffFile.Skip(size - sizeof(index));
			}
		}


	}
	void Psy3SongLoader::LoadPatternData(RiffFile& riffFile,Song& song,const bool fullopen)
	{
		UINT version = 0;
		UINT size = 0;
		UINT index = 0;

		int temp;

		version = riffFile.ReadUINT();
		size = riffFile.ReadUINT();
		if (version > CURRENT_FILE_VERSION_PATD)
		{
			// there is an error, this file is newer than this build of psycle
//					MessageBox(NULL,"Pattern section of File is from a newer version of psycle!",NULL,NULL);
			riffFile.Skip(size);
		}
		else
		{
			index = riffFile.ReadUINT();
			if (index < MAX_PATTERNS)
			{
				song.RemovePattern(index); // clear it out if it already exists
				song.PatternLines(index,riffFile.ReadInt());
				riffFile.Read(&temp,sizeof(temp)); // num tracks per pattern // eventually this may be variable per pattern, like when we get multipattern
				
				riffFile.ReadString(song.PatternName(index),32);
				
				size = riffFile.ReadUINT();
				
				byte* pSource = new byte[size];
				riffFile.Read(pSource,size);
				byte* pDest;

				BEERZ77Decomp2(pSource, &pDest);
				delete pSource;
				pSource = pDest;

				for (int y = 0; y < song.PatternLines(index); y++)
				{
					unsigned char* pData = song._ppattern(index) + (y * MULTIPLY);
					PatternEntry* _pent;
					for(int idx = 0;idx < song.SongTracks();idx++){
						memcpy(pData,pSource,5);
						UCHAR n = *(pData);
						_pent = reinterpret_cast<PatternEntry*>(pData);
						_pent->_volcmd = 0;
                        // ボリュームコマンドの変換
						if(_pent->_cmd == PatternCmd::VELOCITY)
						{
							_pent->_cmd = 0;
							_pent->_volume = _pent->_parameter;
							_pent->_volcmd = PatternCmd::VELOCITY;
							_pent->_parameter = 0x0;
						} else {
							if(n < 120){
								_pent->_volume = DEFAULT_VOLUME;
								_pent->_volcmd = PatternCmd::VELOCITY;

							} else {
								_pent->_volume = 0x0;

								_pent->_volcmd = 0x0;

							}
						}
						pData += EVENT_SIZE;
						pSource += 5;
					}

				}
				delete pDest;
			}
			else
			{
//				MessageBox(NULL,"Pattern section of File is from a newer version of psycle!",NULL,NULL);
				riffFile.Skip(size - sizeof(index));
			}
		}

	}
	void Psy3SongLoader::LoadMachineData(RiffFile& riffFile,Song& song,const bool fullopen)
	{
		UINT version = 0;
		UINT size = 0;
		UINT index = 0;

	
		int curpos = 0;
		riffFile.Read(&version,sizeof(version));
		riffFile.Read(&size,sizeof(size));
		if ( !fullopen )
		{
			curpos = riffFile.GetPos();
		}
		if (version > CURRENT_FILE_VERSION_MACD)
		{
			// there is an error, this file is newer than this build of psycle
//					MessageBox(NULL,"Machine section of File is from a newer version of psycle!",NULL,NULL);
			riffFile.Skip(size);
		}
		else
		{
			riffFile.Read(&index,sizeof(index));
			if (index < MAX_MACHINES)
			{
				// we had better load it
				song.DestroyMachine(index);
				song.pMachine(index,Machine::LoadFileChunk(riffFile,index,version,fullopen));
				if ( !fullopen ) riffFile.Seek(curpos + size); // skips specific chunk.
			}
			else
			{
//						MessageBox(NULL,"Instrument section of File is from a newer version of psycle!",NULL,NULL);
				riffFile.Skip(size - sizeof(index));
			}
		}
	}
	
	void Psy3SongLoader::LoadInstrumentData(RiffFile& riffFile,Song& song,const bool fullopen)
	{
		UINT version = 0;
		UINT size = 0;
		UINT index = 0;


		int curpos = 0;
		riffFile.Read(&version,sizeof(version));
		riffFile.Read(&size,sizeof(size));
		if (version > CURRENT_FILE_VERSION_INSD)
		{
			// there is an error, this file is newer than this build of psycle
//					MessageBox(NULL,"Instrument section of File is from a newer version of psycle!",NULL,NULL);
			riffFile.Skip(size);
		}
		else
		{
			riffFile.Read(&index,sizeof(index));
			if (index < MAX_INSTRUMENTS)
			{
				song.pInstrument(index)->LoadFileChunk(riffFile,version,fullopen);
			}
			else
			{
//						MessageBox(NULL,"Instrument section of File is from a newer version of psycle!",NULL,NULL);
				riffFile.Skip(size - sizeof(index));
			}
		}
	}



}
