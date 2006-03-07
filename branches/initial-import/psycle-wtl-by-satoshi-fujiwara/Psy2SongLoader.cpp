/** @file 
 *  @brief implementation file
 *  $Date: 2005/01/15 22:44:20 $
 *  $Revision: 1.6 $
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
#include ".\psy2songloader.h"

namespace SF {
	Psy2SongLoader::Psy2SongLoader(void)
	{
	}

	Psy2SongLoader::~Psy2SongLoader(void)
	{

	}

	void Psy2SongLoader::Load(RiffFile& riffFile,Song& song,const bool fullopen)
	{
		CProgressDialog Progress;
		Progress.Create(NULL);
		Progress.SetWindowText(SF::CResourceString(IDS_MSG0072));
		Progress.ShowWindow(SW_SHOW);
		
		int num;
		int _temp;

		bool _machineActive[128];
		unsigned char busEffect[64];
		unsigned char busMachine[64];

		song.New();
		
		// Information 
		
		char _buffer[128];

		song.Name(string(CA2T(static_cast<char*>(riffFile.Read(_buffer,Psy2SongLoader::NAME_LENGTH)))));
		song.Author(string(CA2T(static_cast<char*>(riffFile.Read(_buffer, Psy2SongLoader::AUTHOR_LENGTH)))));
		song.Comment(string(CA2T(static_cast<char*>(riffFile.Read(_buffer, Psy2SongLoader::COMMENT_LENGTH)))));

		song.BeatsPerMin(riffFile.ReadInt());
		song.SamplesPerTick(riffFile.ReadInt());

		if ( song.SamplesPerTick() <= 0 ) { // Shouldn't happen but has happened.
			song.TicksPerBeat(4);
			song.SamplesPerTick(4315);
		} else {
			song.TicksPerBeat(44100 * 15 * 4 / (song.SamplesPerTick() * song.BeatsPerMin()));
		}

		Global::pPlayer->bpm = song.BeatsPerMin();
		Global::pPlayer->tpb = song.TicksPerBeat();
		
		// The old format assumes we output at 44100 samples/sec, so...
		//
		song.SamplesPerTick(song.SamplesPerTick() 
			* Global::pConfig->_pOutputDriver->_samplesPerSec / 44100);

		song.CurrentOctave(riffFile.ReadChar());
		
		riffFile.Read(&busMachine[0], sizeof(busMachine));
		
		for(int i = 0;i < MAX_SONG_POSITIONS;i++){
			song.PlayOrder(i,riffFile.ReadChar());
		}
		
//		riffFile.Read(&m_PlayOrder, sizeof(m_PlayOrder));
		
		song.PlayLength(riffFile.ReadInt());
		song.SongTracks(riffFile.ReadInt());

		// Patterns
		//
		num = riffFile.ReadInt();

		for (int i = 0; i < num; i++)
		{
			
			song.PatternLines(i,riffFile.ReadInt());
			
			char _pattern_name[Song::PATTERN_NAME_LEN];
			riffFile.Read(_pattern_name, Song::PATTERN_NAME_LEN);

			memcpy(song.PatternName(i),CA2T(_pattern_name),Song::PATTERN_NAME_LEN * sizeof(TCHAR));

			if (song.PatternLines(i) > 0)
			{
				unsigned char * pData = song.CreateNewPattern(i);
				for (int c = 0; c < song.PatternLines(i); c++)
				{
					for(int d = 0; d < OLD_MAX_TRACKS; d++)
					{
						riffFile.Read((char*)pData,OLD_EVENT_SIZE);
						PatternEntry* _entry = reinterpret_cast<PatternEntry*>(pData);
						_entry->_volcmd = 0;
						if(_entry->_cmd == PatternCmd::VELOCITY){
							_entry->_volume = _entry->_parameter;
							_entry->_volcmd = PatternCmd::VELOCITY;
							_entry->_cmd = _entry->_parameter = 0;
						} else {
							if(_entry->_note < 120){
								_entry->_volume = DEFAULT_VOLUME;
								_entry->_volcmd = PatternCmd::VELOCITY;
							} else {
								_entry->_volume = 0x0;
								_entry->_volcmd = 0x0;

							}
						}
						pData += EVENT_SIZE;
					}
					pData += EVENT_SIZE * (MAX_TRACKS - OLD_MAX_TRACKS);
				}
			}
			else
			{
				song.PatternLines(i,64);
				song.RemovePattern(i);
			}
		}
		
		Progress.m_Progress.SetPos(2048);
		::Sleep(1);

		// Instruments
		//
		song.InstSelected(riffFile.ReadInt());
		char _instname[32];

		for (i = 0; i < OLD_MAX_INSTRUMENTS; i++)
		{
			riffFile.Read(_instname,sizeof(_instname));
			_tcscpy(song.pInstrument(i)->_sName,CA2T(_instname));
		}

		for (i = 0; i < OLD_MAX_INSTRUMENTS; i++)
		{
			riffFile.Read(&song.pInstrument(i)->_NNA, sizeof(song.pInstrument(0)->_NNA));
		}
		for (i=0; i<OLD_MAX_INSTRUMENTS; i++)
		{
			riffFile.Read(&song.pInstrument(i)->ENV_AT, sizeof(song.pInstrument(0)->ENV_AT));
		}
		for (i=0; i<OLD_MAX_INSTRUMENTS; i++)
		{
			riffFile.Read(&song.pInstrument(i)->ENV_DT, sizeof(song.pInstrument(0)->ENV_DT));
		}
		for (i=0; i<OLD_MAX_INSTRUMENTS; i++)
		{
			riffFile.Read(&song.pInstrument(i)->ENV_SL, sizeof(song.pInstrument(0)->ENV_SL));
		}
		for (i=0; i<OLD_MAX_INSTRUMENTS; i++)
		{
			riffFile.Read(&song.pInstrument(i)->ENV_RT, sizeof(song.pInstrument(0)->ENV_RT));
		}
		for (i=0; i<OLD_MAX_INSTRUMENTS; i++)
		{
			riffFile.Read(&song.pInstrument(i)->ENV_F_AT, sizeof(song.pInstrument(0)->ENV_F_AT));
		}
		for (i=0; i<OLD_MAX_INSTRUMENTS; i++)
		{
			riffFile.Read(&song.pInstrument(i)->ENV_F_DT, sizeof(song.pInstrument(0)->ENV_F_DT));
		}
		for (i=0; i<OLD_MAX_INSTRUMENTS; i++)
		{
			riffFile.Read(&song.pInstrument(i)->ENV_F_SL, sizeof(song.pInstrument(0)->ENV_F_SL));
		}
		for (i=0; i<OLD_MAX_INSTRUMENTS; i++)
		{
			riffFile.Read(&song.pInstrument(i)->ENV_F_RT, sizeof(song.pInstrument(0)->ENV_F_RT));
		}
		for (i=0; i<OLD_MAX_INSTRUMENTS; i++)
		{
			riffFile.Read(&song.pInstrument(i)->ENV_F_CO, sizeof(song.pInstrument(0)->ENV_F_CO));
		}
		for (i=0; i<OLD_MAX_INSTRUMENTS; i++)
		{
			riffFile.Read(&song.pInstrument(i)->ENV_F_RQ, sizeof(song.pInstrument(0)->ENV_F_RQ));
		}
		for (i=0; i<OLD_MAX_INSTRUMENTS; i++)
		{
			riffFile.Read(&song.pInstrument(i)->ENV_F_EA, sizeof(song.pInstrument(0)->ENV_F_EA));
		}
		for (i=0; i<OLD_MAX_INSTRUMENTS; i++)
		{
			riffFile.Read(&song.pInstrument(i)->ENV_F_TP, sizeof(song.pInstrument(0)->ENV_F_TP));
		}
		for (i=0; i<OLD_MAX_INSTRUMENTS; i++)
		{
			riffFile.Read(&song.pInstrument(i)->_pan, sizeof(song.pInstrument(0)->_pan));
		}
		for (i=0; i<OLD_MAX_INSTRUMENTS; i++)
		{
			riffFile.Read(&song.pInstrument(i)->_RPAN, sizeof(song.pInstrument(0)->_RPAN));
		}
		for (i=0; i<OLD_MAX_INSTRUMENTS; i++)
		{
			riffFile.Read(&song.pInstrument(i)->_RCUT, sizeof(song.pInstrument(0)->_RCUT));
		}
		for (i=0; i<OLD_MAX_INSTRUMENTS; i++)
		{
			riffFile.Read(&song.pInstrument(i)->_RRES, sizeof(song.pInstrument(0)->_RRES));
		}
		
		Progress.m_Progress.SetPos(4096);
		::Sleep(1);

		// Waves
		//

		
		riffFile.Read(&_temp, sizeof(int));
		song.WaveSelected(_temp);

		for (i = 0; i < OLD_MAX_INSTRUMENTS; i++)
		{
			for (int w=0; w<OLD_MAX_WAVES; w++)
			{
				riffFile.Read(&song.pInstrument(i)->waveLength[w], sizeof(song.pInstrument(0)->waveLength[0]));
				if (song.pInstrument(i)->waveLength[w] > 0)
				{
					short tmpFineTune;

					char _wave_name[32];
					riffFile.Read(_wave_name,32);
					_tcscpy(song.pInstrument(i)->waveName[w],CA2T(_wave_name));
					
					//riffFile.Read(&song.pInstrument(i)->waveName[w], 32);
					riffFile.Read(&song.pInstrument(i)->waveVolume[w], sizeof(song.pInstrument(0)->waveVolume[0]));
					riffFile.Read(&tmpFineTune, sizeof(short));
					song.pInstrument(i)->waveFinetune[w]=(int)tmpFineTune;
					riffFile.Read(&song.pInstrument(i)->waveLoopStart[w], sizeof(song.pInstrument(0)->waveLoopStart[0]));
					riffFile.Read(&song.pInstrument(i)->waveLoopEnd[w], sizeof(song.pInstrument(0)->waveLoopEnd[0]));
					riffFile.Read(&song.pInstrument(i)->waveLoopType[w], sizeof(song.pInstrument(0)->waveLoopType[0]));
					riffFile.Read(&song.pInstrument(i)->waveStereo[w], sizeof(song.pInstrument(0)->waveStereo[0]));
					song.pInstrument(i)->waveDataL[w] = new signed short[song.pInstrument(i)->waveLength[w]];
					riffFile.Read(song.pInstrument(i)->waveDataL[w], song.pInstrument(i)->waveLength[w]*sizeof(short));
					if (song.pInstrument(i)->waveStereo[w])
					{
						song.pInstrument(i)->waveDataR[w] = new signed short[song.pInstrument(i)->waveLength[w]];
						riffFile.Read(song.pInstrument(i)->waveDataR[w], song.pInstrument(i)->waveLength[w]*sizeof(short));
					}
				}
			}
		}
		
		Progress.m_Progress.SetPos(4096 + 2048);
		::Sleep(1);

		// VST DLLs
		//

		VSTLoader vstL[MAX_PLUGINS]; 
		for (i = 0; i < MAX_PLUGINS; i++)
		{
			vstL[i].valid = riffFile.ReadBool();
			if( vstL[i].valid )
			{
				riffFile.ReadStringA2T(vstL[i].dllName,sizeof(vstL[i].dllName) / sizeof(TCHAR));
				_tcslwr(vstL[i].dllName);
				
				vstL[i].numpars = riffFile.ReadInt();
				vstL[i].pars = new float[vstL[i].numpars];

				for (int c = 0; c < vstL[i].numpars; c++)
				{
					vstL[i].pars[c] = riffFile.ReadFloat();
				}
			}
		}
		
		Progress.m_Progress.SetPos(8192);
		::Sleep(1);
		// Machines
		//
		
		song.IsMachineLock(true);

		for(int k = 0;k < 128;k++){
			_machineActive[k] = riffFile.ReadBool();
		}
		
		Machine* pMac[128];
		
		memset(pMac,0,sizeof(pMac));

#if defined(CONVERT_INTERNAL_MACHINES)
		psycle::convert_internal_machines::Converter converter; // conversion
#endif

		for (i = 0; i < 128; i++)
		{
#if !defined(CONVERT_INTERNAL_MACHINES)
			Sine* pSine;
			Distortion* pDistortion;
			Delay* pDelay;
			Filter2p* pFilter;
			Gainer* pGainer;
			Flanger* pFlanger;
#endif
			Sampler* pSampler;
			Plugin* pPlugin;
			VSTPlugin* pVstPlugin;

			int x,y,type;
			if (_machineActive[i])
			{
#if !defined(_WINAMP_PLUGIN_)
				Progress.m_Progress.SetPos(8192+i*(4096/128));
				::Sleep(1);
#endif

				x = riffFile.ReadInt();
				y = riffFile.ReadInt();

				type = riffFile.ReadInt();

#if defined(CONVERT_INTERNAL_MACHINES)
				if(converter.plugin_names().exists(type))
					pMac[i] = &converter.redirect(i, type, riffFile); // conversion
				else
#endif
				{
					bool _berror = false;
					switch (type)
					{
					case MACH_MASTER:
						pMac[i] = song.pMachine(MASTER_INDEX);
						pMac[i]->Init();
						pMac[i]->Load(riffFile);
						break;
					case MACH_SAMPLER:
						pMac[i] = pSampler = new Sampler(i);
						pMac[i]->Init();
						pMac[i]->Load(riffFile);
						break;
	#if !defined(CONVERT_INTERNAL_MACHINES)
					case MACH_SINE:
						pMac[i] = pSine = new Sine(i);
						pMac[i]->Init();
						pMac[i]->Load(riffFile);
						break;
					case MACH_DIST:
						pMac[i] = pDistortion = new Distortion(i);
						pMac[i]->Init();
						pMac[i]->Load(riffFile);
						break;
					case MACH_DELAY:
						pMac[i] = pDelay = new Delay(i);
						pMac[i]->Init();
						pMac[i]->Load(riffFile);
						break;
					case MACH_2PFILTER:
						pMac[i] = pFilter = new Filter2p(i);
						pMac[i]->Init();
						pMac[i]->Load(riffFile);
						break;
					case MACH_GAIN:
						pMac[i] = pGainer = new Gainer(i);
						pMac[i]->Init();
						pMac[i]->Load(riffFile);
						break;
					case MACH_FLANGER:
						pMac[i] = pFlanger = new Flanger(i);
						pMac[i]->Init();
						pMac[i]->Load(riffFile);
						break;
	#endif
					case MACH_PLUGIN:
						{
						pMac[i] = pPlugin = new Plugin(i);
						// Should the "Init()" function go here? -> No. Needs to load the dll first.
						if (!pMac[i]->Load(riffFile))
						{
							_berror = true;
						}
						break;
						}
					case MACH_VST:
					case MACH_VSTFX:
						{
							try {
								if ( type == MACH_VST ) 
								{
									pMac[i] = pVstPlugin = new VSTInstrument(i);
								}
								else if ( type == MACH_VSTFX ) 
								{
									pMac[i] = pVstPlugin = new VSTFX(i);
								}

								if ((pMac[i]->Load(riffFile)) && (vstL[pVstPlugin->_instance].valid)) // Machine::Init() is done Inside "Load()"
								{
									TCHAR sPath2[_MAX_PATH];
									CString sPath;

									if ( CNewMachine::dllNames.Lookup(vstL[pVstPlugin->_instance].dllName,sPath) )
									{
										_tcscpy(sPath2,sPath);
										if (!CNewMachine::TestFilename(sPath2))
										{
											throw Song::MachineNotFoundException(string(sPath2));
										}
										else if (pVstPlugin->Instance(sPath2,false) != VSTINSTANCE_NO_ERROR)
										{
											throw Song::MachineNotFoundException(string(sPath2));
										}
									}
									else
									{
										throw Song::MachineNotFoundException(string(vstL[pVstPlugin->_instance].dllName));
									}
								}
								else
								{
									_berror = true;
								}
							} catch (Song::MachineNotFoundException exc) {
								::MessageBox(NULL,
									(SF::tformat(SF::CResourceString(IDS_ERR_MSG0015)) % exc.what()).str().data(),
									SF::CResourceString(IDS_ERR_MSG0016), MB_OK);
								_berror = true;
							}

						}
						break;
					case MACH_SCOPE:
					case MACH_DUMMY:
						pMac[i] = new Dummy(i);
						pMac[i]->Init();
						pMac[i]->Load(riffFile);
						break;
					default:
						{
							::MessageBox(0, 
								(SF::tformat(SF::CResourceString(IDS_ERR_MSG0047)) % type).str().c_str(),
								SF::CResourceString(IDS_ERR_MSG0048), MB_ICONERROR);
						}
						pMac[i] = new Dummy(i);
						pMac[i]->Init();
						pMac[i]->Load(riffFile);
					}

					//エラーであればダミープラグをセットする
					if(_berror)
					{
						Machine* pOldMachine = pMac[i];
						pMac[i] = new Dummy(*((Dummy*)pOldMachine));
						pOldMachine->_pSamplesL = NULL;
						pOldMachine->_pSamplesR = NULL;
						// dummy name goes here
						_stprintf(pMac[i]->_editName,_T("X %s"),pOldMachine->_editName);
						delete pOldMachine;
						pMac[i]->_type = MACH_DUMMY;
						pMac[i]->wasVST = true;
					}
				}
				
				switch (pMac[i]->_mode)
				{
				case MACHMODE_GENERATOR:
					if ( x > song.ViewSizeX() - CMainFrame::GetInstance().m_view.MachineCoords.sGenerator.width ) 
						x = song.ViewSizeX() - CMainFrame::GetInstance().m_view.MachineCoords.sGenerator.width;
					if ( y > song.ViewSizeY() - CMainFrame::GetInstance().m_view.MachineCoords.sGenerator.height )
						y = song.ViewSizeY()-CMainFrame::GetInstance().m_view.MachineCoords.sGenerator.height;
					break;
				case MACHMODE_FX:
					if ( x > song.ViewSizeX() - CMainFrame::GetInstance().m_view.MachineCoords.sEffect.width ) 
						x = song.ViewSizeX()-CMainFrame::GetInstance().m_view.MachineCoords.sEffect.width;
					if ( y > song.ViewSizeY() - CMainFrame::GetInstance().m_view.MachineCoords.sEffect.height ) 
						y = song.ViewSizeY()-CMainFrame::GetInstance().m_view.MachineCoords.sEffect.height;
					break;

				case MACHMODE_MASTER:
					if ( x > song.ViewSizeX() - CMainFrame::GetInstance().m_view.MachineCoords.sMaster.width ) 
						x = song.ViewSizeX()-CMainFrame::GetInstance().m_view.MachineCoords.sMaster.width;
					if ( y > song.ViewSizeY() - CMainFrame::GetInstance().m_view.MachineCoords.sMaster.height ) 
						y = song.ViewSizeY()-CMainFrame::GetInstance().m_view.MachineCoords.sMaster.height;
					break;
				}

				pMac[i]->_x = x;
				pMac[i]->_y = y;
			}
		}

		Progress.m_Progress.SetPos(8192 + 4096);
		::Sleep(1);

		// Since the old file format stored volumes on each output
		// rather than on each input, we must convert
		//
		float volMatrix[128][MAX_CONNECTIONS];
		for (i=0; i<128; i++) // First, we add the output volumes to a Matrix for latter reference
		{
			if (!_machineActive[i])
			{
				if (pMac[i])
				{
					delete pMac[i];
					pMac[i] = NULL;
				}
			}
			else if (!pMac[i])
			{
				_machineActive[i] = FALSE;
			}
			else 
			{
				for (int c=0; c<MAX_CONNECTIONS; c++)
				{
					volMatrix[i][c] = pMac[i]->_inputConVol[c];
				}
			}
		}
#if !defined(_WINAMP_PLUGIN_)
		
		Progress.m_Progress.SetPos(8192+4096+1024);
		::Sleep(1);
#endif
		for (i=0; i<128; i++) // Next, we go to fix this for each
		{
			if (_machineActive[i])		// valid machine (important, since we have to navigate!)
			{
				for (int c=0; c<MAX_CONNECTIONS; c++) // all of its input connections.
				{
					if (pMac[i]->_inputCon[c])	// If there's a valid machine in this inputconnection,
					{
						Machine* pOrigMachine = pMac[pMac[i]->_inputMachines[c]]; // We get that machine
						int d = pOrigMachine->FindOutputWire(i);

						float val = volMatrix[pMac[i]->_inputMachines[c]][d];
						if( val >= 4.000001f ) 
						{
							val*=0.000030517578125f; // BugFix
						}
						else if ( val < 0.00004f) 
						{
							val*=32768.0f; // BugFix
						}

						pMac[i]->InitWireVolume(pOrigMachine->_type,c,val);
					}
				}
			}
		}
#if !defined(_WINAMP_PLUGIN_)
		
		Progress.m_Progress.SetPos(8192+4096+2048);
		::Sleep(1);
#endif
		for (i=0; i<OLD_MAX_INSTRUMENTS; i++)
		{
			riffFile.Read(song.pInstrument(i)->_loop);
		}
		for (i=0; i<OLD_MAX_INSTRUMENTS; i++)
		{
			riffFile.Read(song.pInstrument(i)->_lines);
		}

		if ( riffFile.Read(&busEffect[0],sizeof(busEffect)) == false ) // Patch 1: BusEffects (twf)
		{
			int j = 0;
			for ( i = 0;i<128;i++ ) 
			{
				if (_machineActive[i] && pMac[i]->_mode != MACHMODE_GENERATOR )
				{
					busEffect[j]=i;	
					j++;
				}
			}
			for (j; j < 64; j++)
			{
				busEffect[j] = 255;
			}
		}
		// Patch 1.2: Fixes crash/inconsistence when deleting a machine which couldn't be loaded
		// (.dll not found, or Load failed), which is, then, replaced by a DUMMY machine.
		int j=0;
		for ( i=0;i<64;i++ ) 
		{
			if (busMachine[i] != 255 && _machineActive[busMachine[i]]) 
			{ // If there's a machine in the generators' bus that it is not a generator:
				if (pMac[busMachine[i]]->_mode != MACHMODE_GENERATOR ) 
				{
					pMac[busMachine[i]]->_mode = MACHMODE_FX;
					while (busEffect[j] != 255 && j<MAX_BUSES) 
					{
						j++;
					}
					busEffect[j]=busMachine[i];
					busMachine[i]=255;
				}
			}
		}
		for ( i=0;i<64;i++ ) 
		{
			if ((busMachine[i] != 255) && (_machineActive[busEffect[i]]) && (pMac[busMachine[i]]->_mode != MACHMODE_GENERATOR)) 
			{
				busMachine[i] = 255;
			}
			if ((busEffect[i] != 255) && (_machineActive[busEffect[i]]) && (pMac[busEffect[i]]->_mode != MACHMODE_FX)) 
			{
				busEffect[i] = 255;
			}
		}

		bool chunkpresent=false;
		try {
			riffFile.Read(&chunkpresent,sizeof(chunkpresent)); // Patch 2: VST's Chunk.
		} catch(RiffFile::Exception& e) {
			;// エラーが発生したら、チャンクはない
		}

		if ( fullopen ) for ( i=0;i<128;i++ ) 
		{
			if (_machineActive[i])
			{
				if ( pMac[i]->wasVST && chunkpresent )
				{
					// Since we don't know if the plugin saved it or not, 
					// ﾌﾟﾗｸﾞｲﾝがそれを保存したか否かどうか私たちは知らないので、
					// we're stuck on letting the loading crash/behave incorrectly.
					// There should be a flag, like in the VST loading Section to be correct.
					// VSTﾛｰﾄﾞｾｸｼｮﾝが正確であるように、ﾌﾗｸﾞがあるべきです。
					::MessageBox(NULL,_T("Missing or Corrupted VST plug-in has chunk, trying not to crash."), _T("Loading Error"), MB_OK);
				}
				else if (( pMac[i]->_type == MACH_VST ) || 
						( pMac[i]->_type == MACH_VSTFX))
				{
					bool chunkread = false;
					if( chunkpresent )
						chunkread=((VSTPlugin*)pMac[i])->LoadChunk(riffFile);
					((VSTPlugin*)pMac[i])->SetCurrentProgram(((VSTPlugin*)pMac[i])->_program);
					if ( !chunkpresent || !chunkread )
					{
						const int vi = ((VSTPlugin*)pMac[i])->_instance;
						const int numpars=vstL[vi].numpars;
						for (int c=0; c<numpars; c++)
						{
							((VSTPlugin*)pMac[i])->SetParameter(c, vstL[vi].pars[c]);
						}
					}
				}
			}
		}
		for (i=0; i<MAX_PLUGINS; i++) // Clean "pars" array.
		{
			if( vstL[i].valid )
			{
				delete vstL[i].pars;
			}
		}

		// move machines around to where they really should go
		// now we have to remap all the inputs and outputs again... ouch
#if !defined(_WINAMP_PLUGIN_)
		
		Progress.m_Progress.SetPos(8192+4096+2048+1024);
		::Sleep(1);
#endif

		for (i = 0; i < 64; i++)
		{
			if ((busMachine[i] < MAX_MACHINES-1) && (busMachine[i] > 0))
			{
				if (_machineActive[busMachine[i]])
				{
					if (pMac[busMachine[i]]->_mode == MACHMODE_GENERATOR)
					{
						song.pMachine(i,pMac[busMachine[i]]);
						_machineActive[busMachine[i]] = FALSE; // don't update this twice;

						for (int c=0; c<MAX_CONNECTIONS; c++)
						{
							if (song.pMachine(i)->_inputCon[c])
							{
								for (int x=0; x<64; x++)
								{
									if (song.pMachine(i)->_inputMachines[c] == busMachine[x])
									{
										song.pMachine(i)->_inputMachines[c] = x;
										break;
									}
									else if (song.pMachine(i)->_inputMachines[c] == busEffect[x])
									{
										song.pMachine(i)->_inputMachines[c] = x+MAX_BUSES;
										break;
									}
								}
							}

							if (song.pMachine(i)->_connection[c])
							{
								if (song.pMachine(i)->_outputMachines[c] == 0)
								{
									song.pMachine(i)->_outputMachines[c] = MASTER_INDEX;
								}
								else
								{
									for (int x=0; x<64; x++)
									{
										if (song.pMachine(i)->_outputMachines[c] == busMachine[x])
										{
											song.pMachine(i)->_outputMachines[c] = x;
											break;
										}
										else if (song.pMachine(i)->_outputMachines[c] == busEffect[x])
										{
											song.pMachine(i)->_outputMachines[c] = x+MAX_BUSES;
											break;
										}
									}
								}
							}
						}
					}
				}
			}
			if ((busEffect[i] < MAX_MACHINES-1) && (busEffect[i] > 0))
			{
				if (_machineActive[busEffect[i]])
				{
					if (pMac[busEffect[i]]->_mode == MACHMODE_FX)
					{
						song.pMachine(i + MAX_BUSES,pMac[busEffect[i]]);
						_machineActive[busEffect[i]] = FALSE; // don't do this again

						for (int c = 0; c < MAX_CONNECTIONS; c++)
						{
							if (song.pMachine(i + MAX_BUSES)->_inputCon[c])
							{
								for (int x=0; x<64; x++)
								{
									if (song.pMachine(i + MAX_BUSES)->_inputMachines[c] == busMachine[x])
									{
										song.pMachine(i + MAX_BUSES)->_inputMachines[c] = x;
										break;
									}
									else if (song.pMachine(i+MAX_BUSES)->_inputMachines[c] == busEffect[x])
									{
										song.pMachine(i + MAX_BUSES)->_inputMachines[c] = x+MAX_BUSES;
										break;
									}
								}
							}
							if (song.pMachine(i + MAX_BUSES)->_connection[c])
							{
								if (song.pMachine(i + MAX_BUSES)->_outputMachines[c] == 0)
								{
									song.pMachine(i + MAX_BUSES)->_outputMachines[c] = MASTER_INDEX;
								}
								else
								{
									for (int x=0; x<64; x++)
									{
										if (song.pMachine(i+MAX_BUSES)->_outputMachines[c] == busMachine[x])
										{
											song.pMachine(i+MAX_BUSES)->_outputMachines[c] = x;
											break;
										}
										else if (song.pMachine(i+MAX_BUSES)->_outputMachines[c] == busEffect[x])
										{
											song.pMachine(i+MAX_BUSES)->_outputMachines[c] = x+MAX_BUSES;
											break;
										}
									}
								}
							}
						}
					}
				}
			}
		}

		// fix machine #s

		for (i = 0; i < MAX_MACHINES-1; i++)
		{
			if (song.pMachine(i))
			{
				song.pMachine(i)->_macIndex = i;
				for (j = i+1; j < MAX_MACHINES-1; j++)
				{
					if (song.pMachine(i) == song.pMachine(j))
					{
						// we have duplicate machines...
						// this should NEVER happen
						// delete the second one :(
						song.pMachine(j,NULL);
						// and we should remap anything that had wires to it to the first one
					}
				}
			}
		}
#if !defined(_WINAMP_PLUGIN_)
		
		Progress.m_Progress.SetPos(8192+4096+2048+1024+512);
		::Sleep(1);
#endif
		// test all connections

		for (int c=0; c<MAX_CONNECTIONS; c++)
		{
			if (song.pMachine(MASTER_INDEX)->_inputCon[c])
			{
				for (int x=0; x<64; x++)
				{
					if (song.pMachine(MASTER_INDEX)->_inputMachines[c] == busMachine[x])
					{
						song.pMachine(MASTER_INDEX)->_inputMachines[c] = x;
						break;
					}
					else if (song.pMachine(MASTER_INDEX)->_inputMachines[c] == busEffect[x])
					{
						song.pMachine(MASTER_INDEX)->_inputMachines[c] = x+MAX_BUSES;
						break;
					}
				}
			}
		}
		
		Progress.m_Progress.SetPos(16384);
		::Sleep(1);

		// test all connections for invalid machines. disconnect invalid machines.
		for (i = 0; i < MAX_MACHINES; i++)
		{
			if (song.pMachine(i))
			{
				song.pMachine(i)->_numInputs = 0;
				song.pMachine(i)->_numOutputs = 0;

				for (int c = 0; c < MAX_CONNECTIONS; c++)
				{
					if (song.pMachine(i)->_connection[c])
					{
						if (song.pMachine(i)->_outputMachines[c] < 0 || song.pMachine(i)->_outputMachines[c] >= MAX_MACHINES)
						{
							song.pMachine(i)->_connection[c]=FALSE;
							song.pMachine(i)->_outputMachines[c]=255;
						}
						else if (!song.pMachine(song.pMachine(i)->_outputMachines[c]))
						{
							song.pMachine(i)->_connection[c]=FALSE;
							song.pMachine(i)->_outputMachines[c]=255;
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
						if (song.pMachine(i)->_inputMachines[c] < 0 || song.pMachine(i)->_inputMachines[c] >= MAX_MACHINES-1)
						{
							song.pMachine(i)->_inputCon[c]=FALSE;
							song.pMachine(i)->_inputMachines[c]=255;
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

		if ( fullopen ) converter.retweak(song); // conversion

		song.IsMachineLock(false);
		song.SeqBus(0);
		
		Progress.OnCancel();

		if (!riffFile.Close())
		{
		//	char error[MAX_PATH];
		//	_stprintf(error,_T("\"%s\" からの読み込みｴﾗｰ!!!"),riffFile.szName);
		//	std::string a = (boost::format("%1%") % riffFile.szName;
			throw IPsySongLoader::Exception((SF::tformat(SF::CResourceString(IDS_ERR_MSG0050)) % riffFile.szName).str());
			//MessageBox(NULL,error,_T("ﾌｧｲﾙｴﾗｰ!!!"),0);
		}

	}
	

}