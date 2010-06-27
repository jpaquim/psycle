/***************************************************************************
	*   Copyright (C) 2006 by  Stefan Nattkemper   *
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
#include "psy2filter.h"
#include "fileio.h"

#include "machine.h"
#include "internal_machines.h"
#include "sampler.h"
//#include"XMSampler.h"
#include "plugin.h"
//#include "VSTHost.h"
//todo:
//#include "cacheddllfinder.h"
#include <algorithm>
#include <cctype>

#ifdef __unix__
#include "convert_internal_machines.h"
#endif

namespace psycle
{
	namespace host
	{
		struct ToLower
	    {
	      char operator() (char c) const  { return std::tolower(c); }
	    };
	

		std::string const Psy2Filter::FILE_FOURCC = "PSY2";
		const int Psy2Filter::PSY2_MAX_TRACKS = 32;
		const int Psy2Filter::PSY2_MAX_WAVES  = 16;
		const int Psy2Filter::PSY2_MAX_INSTRUMENTS = 255;
		const int Psy2Filter::PSY2_MAX_PLUGINS = 256;


		Psy2Filter::Psy2Filter() : PsyFilter()
		{
			singleCat=0;
			singleLine=0;
		}


		Psy2Filter::~Psy2Filter()
		{
			// TODO: put destructor code here
		}

		bool Psy2Filter::testFormat(const std::string & fileName)
		{
			RiffFile file;
			file.Open(fileName);
			char Header[9];
			file.ReadChunk(&Header, 8);
			Header[8]=0;
			file.Close();
			if (strcmp(Header,"PSY2SONG")==0) return true;
			return false;
		}
		void Psy2Filter::preparePatternSequence( Song & song )
		{
			seqList.clear();
			song.patternSequence()->removeAll();
			// creatse a single Pattern Category
			singleCat = song.patternSequence()-> patternData()->createNewCategory("SinglePattern");
			// here we add in one single Line the patterns
			singleLine = song.patternSequence()->createNewLine();
		}

		bool Psy2Filter::load(const std::string & fileName, Song & song)
		{
			std::int32_t num;
			RiffFile file;
			file.Open(fileName);
//			progress.emit(1,0,"");
//			progress.emit(2,0,"Loading... psycle song fileformat version 2...");

			song.clear();
			preparePatternSequence(song);

			// skip header
			file.Skip(8);
			LoadINFO(&file,song);
			LoadSNGI(&file,song);
			LoadSEQD(&file,song);

			file.Read(num);
			Machine::id_type i;
			for(i =0 ; i < num; ++i)
			{
				LoadPATD(&file,song,i);
			}
			LoadINSD(&file,song);
			LoadWAVD(&file,song);
			PreLoadVSTs(&file,song);
			#ifdef __unix__
			convert_internal_machines::Converter converter;
			#endif
			#ifdef __unix__
			LoadMACD(&file,song,&converter);
			TidyUp(&file,song,&converter);
			#endif
			return true;
		}

		bool Psy2Filter::LoadINFO(RiffFile* file,Song& song)
		{
			char Name[32];
			char Author[32];
			char Comment[128];

			file->ReadChunk(Name, 32); Name[31]=0;
			song.setName(Name);
			file->ReadChunk(Author, 32); Author[31]=0;
			song.setAuthor(Author);
			bool err = file->ReadChunk(Comment, 128); Comment[127]=0;
			song.setComment(Comment);
			return err;
		}

		bool Psy2Filter::LoadSNGI(RiffFile* file,Song& song)
		{
			std::int32_t tmp;
			unsigned char oct;
			file->Read(tmp);
			song.setBpm(tmp);

			file->Read(tmp);
			if( tmp <= 0)
			{
				// Shouldn't happen but has happened.
				song.m_LinesPerBeat = 4;
			}
			else song.m_LinesPerBeat = static_cast<int>( 44100 * 15 * 4 / (tmp * song.bpm()) );

			file->Read(oct);
			song.currentOctave = oct;

			file->Read(busMachine);
			return true;
		}
		bool Psy2Filter::LoadSEQD(RiffFile* file,Song& song)
		{
			std::int32_t length,tmp;
			unsigned char playOrder[128];
			file->Read(playOrder);
			file->Read(length);
			for (int i(0) ; i < length; ++i)
			{
				seqList.push_back(playOrder[i]);
			}

			file->Read(tmp); song.setTracks(tmp);
			return true;
		}

		PatternEvent Psy2Filter::convertEntry( unsigned char * data ) const
		{
			PatternEvent event;

			event.setNote(*data++);
			event.setInstrument(*data++);
			event.setMachine(*data++);
			event.setCommand(*data++);
			event.setParameter(*data++);

			return event;
		}

		bool Psy2Filter::LoadPATD(RiffFile* file,Song& song,int index)
		{
				std::int32_t numLines;
				char patternName[32];
				file->Read(numLines);
				file->ReadChunk(patternName, sizeof(patternName)); patternName[31]=0;

				if(numLines > 0)
				{
				
				// create a SinglePattern
				std::string indexStr;
				std::ostringstream o;
				if (!(o << index))
					indexStr = "error";
				else
					indexStr = o.str();
				SinglePattern* pat =
						singleCat->createNewPattern(std::string(patternName)+indexStr);
				pat->setBeatZoom(song.m_LinesPerBeat);
				TimeSignature & sig =  pat->timeSignatures().back();
				float beats = numLines / (float) song.m_LinesPerBeat;
				pat->setID(index);
				sig.setCount((int) (beats / 4) );
				float uebertrag = beats - ((int) beats);
				if ( uebertrag != 0) {
					TimeSignature uebertragSig(uebertrag);
					pat->addBar(uebertragSig);
				}
				for(int y(0) ; y < numLines ; ++y) // lines
				{
					for (int x = 0; x < song.tracks(); x++) {
						unsigned char entry[5];
						file->Read(entry);
						PatternEvent event = convertEntry(entry);
						if (!event.empty()) {
							float position = y / (float) song.m_LinesPerBeat;
							(*pat)[position].notes()[x] = event;
						}
					}
					file->Skip((PSY2_MAX_TRACKS-song.tracks())*EVENT_SIZE);
				}
			}
			///\todo: ?
/*			else
			{
				patternLines[i] = 64;
				RemovePattern(i);
			}
*/
			return true;
		}
		bool Psy2Filter::LoadINSD(RiffFile* file,Song& song)
		{
			std::int32_t i;
			file->Read(song.instSelected);

			for(i=0 ; i < PSY2_MAX_INSTRUMENTS ; ++i)
			{
				file->ReadChunk(song._pInstrument[i]->_sName,32); song._pInstrument[i]->_sName[31]=0;
			}
			for(i=0 ; i < PSY2_MAX_INSTRUMENTS ; ++i)
			{
				file->Read(song._pInstrument[i]->_NNA);
			}
			for(i=0 ; i < PSY2_MAX_INSTRUMENTS ; ++i)
			{
				file->Read(song._pInstrument[i]->ENV_AT);
			}
			for(i=0 ; i < PSY2_MAX_INSTRUMENTS ; ++i)
			{
				file->Read(song._pInstrument[i]->ENV_DT);
			}
			for(i=0 ; i < PSY2_MAX_INSTRUMENTS ; ++i)
			{
				file->Read(song._pInstrument[i]->ENV_SL);
			}
			for(i=0 ; i < PSY2_MAX_INSTRUMENTS ; ++i)
			{
				file->Read(song._pInstrument[i]->ENV_RT);
			}
			for(i=0 ; i < PSY2_MAX_INSTRUMENTS ; ++i)
			{
				file->Read(song._pInstrument[i]->ENV_F_AT);
			}
			for(i=0 ; i < PSY2_MAX_INSTRUMENTS ; ++i)
			{
				file->Read(song._pInstrument[i]->ENV_F_DT);
			}
			for(i=0 ; i < PSY2_MAX_INSTRUMENTS ; ++i)
			{
				file->Read(song._pInstrument[i]->ENV_F_SL);
			}
			for(i=0 ; i < PSY2_MAX_INSTRUMENTS ; ++i)
			{
				file->Read(song._pInstrument[i]->ENV_F_RT);
			}
			for(i=0 ; i < PSY2_MAX_INSTRUMENTS ; ++i)
			{
				file->Read(song._pInstrument[i]->ENV_F_CO);
			}
			for(i=0 ; i < PSY2_MAX_INSTRUMENTS ; ++i)
			{
				file->Read(song._pInstrument[i]->ENV_F_RQ);
			}
			for(i=0 ; i < PSY2_MAX_INSTRUMENTS ; ++i)
			{
				file->Read(song._pInstrument[i]->ENV_F_EA);
			}
			for(i=0 ; i < PSY2_MAX_INSTRUMENTS ; ++i)
			{
				file->Read(song._pInstrument[i]->ENV_F_TP);
			}
			for(i=0 ; i < PSY2_MAX_INSTRUMENTS ; ++i)
			{
				file->Read(song._pInstrument[i]->_pan);
			}
			for(i=0 ; i < PSY2_MAX_INSTRUMENTS ; ++i)
			{
				file->Read(song._pInstrument[i]->_RPAN);
			}
			for(i=0 ; i < PSY2_MAX_INSTRUMENTS ; ++i)
			{
				file->Read(song._pInstrument[i]->_RCUT);
			}
			for(i=0 ; i < PSY2_MAX_INSTRUMENTS ; ++i)
			{
				file->Read(song._pInstrument[i]->_RRES);
			}
			return true;
		}
				
		bool Psy2Filter::LoadWAVD(RiffFile* file,Song& song)
		{
			std::int32_t i;
			// Skip wave selected
			file->Skip(4);
			for (i=0; i<PSY2_MAX_INSTRUMENTS; i++)
			{
				for (int w=0; w<PSY2_MAX_WAVES; w++)
				{
					std::uint32_t wltemp;
					file->Read(wltemp);
				
					if (wltemp > 0)
					{
						if ( w == 0 )
						{
							Instrument& pIns = *(song._pInstrument[i]);
							std::int16_t tmpFineTune;
							pIns.waveLength=wltemp;
							file->ReadChunk(pIns.waveName, 32); pIns.waveName[31]=0;
							file->Read(pIns.waveVolume);
							file->Read(tmpFineTune);
							pIns.waveFinetune=tmpFineTune;
							file->Read(pIns.waveLoopStart);
							file->Read(pIns.waveLoopEnd);
							file->Read(pIns.waveLoopType);
							file->Read(pIns.waveStereo);
							pIns.waveDataL = new std::int16_t[pIns.waveLength];
							file->ReadChunk(pIns.waveDataL, pIns.waveLength * sizeof(std::int16_t));
							if (pIns.waveStereo)
							{
								pIns.waveDataR = new std::int16_t[pIns.waveLength];
								file->ReadChunk(pIns.waveDataR, pIns.waveLength * sizeof(std::int16_t));
							}
						}
						else 
						{
							//skip info
							file->Skip(42+sizeof(bool));
							bool stereo;
							file->Read(stereo);
							//skip data.
							file->Skip(wltemp*2);
							if ( stereo )
							{
								file->Skip(wltemp*2);
							}
						}
					}
				}
			}
			return true;
		}

		bool Psy2Filter::PreLoadVSTs(RiffFile* file,Song& song)
		{
			std::int32_t i;
			for (i=0; i<PSY2_MAX_PLUGINS; i++)
			{
				file->Read(vstL[i].valid);
				if( vstL[i].valid )
				{
					file->ReadChunk(vstL[i].dllName,128); vstL[i].dllName[127]=0;
					std::string strname = vstL[i].dllName;
					std::transform(strname.begin(),strname.end(),strname.begin(),ToLower());
					std::strcpy(vstL[i].dllName,strname.c_str());
				
					file->Read(vstL[i].numpars);
					vstL[i].pars = new float[vstL[i].numpars];

					for (int c=0; c<vstL[i].numpars; c++)
					{
						file->Read(vstL[i].pars[c]);
					}
				}
			}
			return true;
		}
		
		#ifdef __unix__
		bool Psy2Filter::LoadMACD(RiffFile* file,Song& song,convert_internal_machines::Converter* converter)
		{
			std::int32_t i;
			file->Read(_machineActive);
			std::memset(pMac,0,sizeof pMac);

			for (i=0; i<128; i++)
			{
				Sampler* pSampler;
//				XMSampler* pXMSampler;
				Plugin* pPlugin;
//				vst::plugin * pVstPlugin(0);
				std::int32_t x,y,type;
				if (_machineActive[i])
				{
					progress.emit(4,8192+i*(4096/128),"");

					file->Read(x);
					file->Read(y);

					file->Read(type);

					if(converter->plugin_names().exists(type))
						pMac[i] = &converter->redirect(i, type, *file,song);
					else switch (type)
					{
					case MACH_PLUGIN:
					{
						pMac[i] = pPlugin = new Plugin(i,&song);
						// Should the "Init()" function go here? -> No. Needs to load the dll first.
						if (!pMac[i]->LoadPsy2FileFormat(file))
						{
							Machine* pOldMachine = pMac[i];
							pMac[i] = new Dummy(*((Dummy*)pOldMachine));
							// dummy name goes here
							std::stringstream s;
							s << "X!" << pOldMachine->GetEditName();
							pOldMachine->SetEditName(s.str());
							pMac[i]->_type = MACH_DUMMY;
							pOldMachine->_pSamplesL = NULL;
							pOldMachine->_pSamplesR = NULL;
							zapObject(pOldMachine);
						}
						break;
					}
					case MACH_MASTER:
						pMac[i] = song._pMachine[MASTER_INDEX];
						goto init_and_load;
					case MACH_SAMPLER:
						pMac[i] = pSampler = new Sampler(i,&song);
						goto init_and_load;
//					case MACH_XMSAMPLER:
//						pMac[i] = pXMSampler = new XMSampler(i);
//						goto init_and_load;
					case MACH_VST:
					case MACH_VSTFX:
//						if (type == MACH_VST) pMac[i] = pVstPlugin = new vst::instrument(i);
//						else if (type == MACH_VSTFX)	pMac[i] = pVstPlugin = new vst::fx(i);
						pMac[i] = new Dummy(i,&song);
						goto init_and_load_VST;
					case MACH_SCOPE:
					case MACH_DUMMY:
						pMac[i] = new Dummy(i,&song);
						goto init_and_load;
					default:
					{
						std::ostringstream s;
						s << "unkown machine type: " << type;
//						MessageBox(0, s.str().c_str(), "Loading old song", MB_ICONERROR);
					}
						pMac[i] = new Dummy(i,&song);
		
					init_and_load:
						pMac[i]->Init();
						pMac[i]->LoadPsy2FileFormat(file);
						break;
					init_and_load_VST:
						pMac[i]->LoadPsy2FileFormat(file);
						std::stringstream s;
						s << "X!" << pMac[i]->GetEditName();
						pMac[i]->SetEditName(s.str());
			
						file->Skip(sizeof(bool)+5);
/*
						if ((pMac[i]->LoadOldFileFormat(file)) && (vstL[pVstPlugin->_instance].valid)) // Machine::Init() is done Inside "Load()"
						{
							std::string path = vstL[pVstPlugin->_instance].dllName;
							if(!Global::dllfinder().LookupDllPath(path,MACH_VST)) 
							{
								try
								{
									pVstPlugin->LoadDll(path,false);
								}
								catch(...)
								{
									std::ostringstream ss;
									ss << "Plugin instancation threw an exception " << path << " - replacing with Dummy.";
									MessageBox(NULL,ss.str().c_str(), "Loading Error", MB_OK);
			
									Machine* pOldMachine = pMac[i];
									pMac[i] = new Dummy(*((Dummy*)pOldMachine));
									pOldMachine->_pSamplesL = NULL;
									pOldMachine->_pSamplesR = NULL;
									// dummy name goes here
									std::stringstream ss2;
									ss2 << "X!" << pOldMachine->GetEditName();
									pOldMachine->SetEditName(ss2.str());
									zapObject(pOldMachine);
									pMac[i]->_subclass = MACH_DUMMY;
									((Dummy*)pMac[i])->wasVST = true;
								}
							}
							else
							{
								std::ostringstream ss;
								ss << "Missing  or disabled VST plug-in: " << vstL[pVstPlugin->_instance].dllName;
								MessageBox(NULL,ss.str().c_str(), "Loading Error", MB_OK);
			
								Machine* pOldMachine = pMac[i];
								pMac[i] = new Dummy(*((Dummy*)pOldMachine));
								pOldMachine->_pSamplesL = NULL;
								pOldMachine->_pSamplesR = NULL;
								// dummy name goes here
								std::stringstream ss2;
								ss2 << "X!" << pOldMachine->GetEditName();
								pOldMachine->SetEditName(ss2.str());
								zapObject(pOldMachine);
								pMac[i]->_subclass = MACH_DUMMY;
								((Dummy*)pMac[i])->wasVST = true;
							}
						}
						else
						{
							Machine* pOldMachine = pMac[i];
							pMac[i] = new Dummy(*((Dummy*)pOldMachine));
							pOldMachine->_pSamplesL = NULL;
							pOldMachine->_pSamplesR = NULL;
							// dummy name goes here
							std::stringstream ss;
							ss << "X!" << pOldMachine->GetEditName();
							pOldMachine->SetEditName(ss.str());
							zapObject(pOldMachine);
							pMac[i]->_subclass = MACH_DUMMY;
							((Dummy*)pMac[i])->wasVST = true;
						}
*/
					}
					pMac[i]->SetPosX(x);
					pMac[i]->SetPosY(y);
				}
			}
			
			// Extra data, Information about instruments, and machines.
		
			for (i=0; i<PSY2_MAX_INSTRUMENTS; i++)
			{
				file->Read(song._pInstrument[i]->_loop);
			}
			for (i=0; i<PSY2_MAX_INSTRUMENTS; i++)
			{
				file->Read(song._pInstrument[i]->_lines);
			}

			if ( file->Read(busEffect) == false ) // Patch 1: BusEffects (twf)
			{
				int j=0;
				for ( i=0;i<128;i++ ) 
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

/*			bool chunkpresent=false;
			file->Read(chunkpresent); // Patch 2: VST's Chunk.

			if ( fullopen ) for ( i=0;i<128;i++ ) 
			{
				if (_machineActive[i])
				{
					if ( pMac[i]->subclass() == MACH_DUMMY ) 
					{
						if (((Dummy*)pMac[i])->wasVST && chunkpresent )
						{
							// Since we don't know if the plugin saved it or not, 
							// we're stuck on letting the loading crash/behave incorrectly.
							// There should be a flag, like in the VST loading Section to be correct.
							MessageBox(NULL,"Missing or Corrupted VST plug-in has chunk, trying not to crash.", "Loading Error", MB_OK);
						}
					}
					else if (( pMac[i]->subclass() == MACH_VST ) || 
							( pMac[i]->subclass() == MACH_VSTFX))
					{
						bool chunkread = false;
						try
						{
							vst::plugin & plugin(*reinterpret_cast<vst::plugin*>(pMac[i]));
							if(chunkpresent) chunkread = plugin.LoadChunkOldFileFormat(file);
							plugin.proxy().dispatcher(effSetProgram, 0, plugin._program);
						}
						catch(const std::exception &)
						{
							// o_O`
						}
						if(!chunkpresent || !chunkread)
						{
							vst::plugin & plugin(*reinterpret_cast<vst::plugin*>(pMac[i]));
							const int vi = plugin._instance;
							const int numpars = vstL[vi].numpars;
							for (int c(0) ; c < numpars; ++c)
							{
								try
								{
									plugin.proxy().setParameter(c, vstL[vi].pars[c]);
								}
								catch(const std::exception &)
								{
									// o_O`
								}
							}
						}
					}
				}
			}
*/
			
			return true;
		}
		bool Psy2Filter::TidyUp(RiffFile*file,Song&song,convert_internal_machines::Converter* converter)
		{
			std::int32_t i;
			// Clean "pars" array.
			for (i=0; i<PSY2_MAX_PLUGINS; i++) 
			{
				if( vstL[i].valid )
				{
					zapObject(vstL[i].pars);
				}
			}
			
			// now that we have loaded all the patterns, time to prepare them in the multisequence.
			double pos = 0;
			std::vector<int>::iterator it = seqList.begin();
			for ( ; it < seqList.end(); ++it)
			{
				SinglePattern* pat = song.patternSequence()->patternData()->findById(*it);
				singleLine->createEntry(pat,pos);
				pos+=pat->beats();
			}
		
		
			// Since the old file format stored volumes on each output
			// rather than on each input, we must convert
			//
			float volMatrix[128][MAX_CONNECTIONS];
			for (i=0; i<128; i++) // First, we add the output volumes to a Matrix for latter reference
			{
				if (!_machineActive[i])
				{
					zapObject(pMac[i]);
				}
				else if (!pMac[i])
				{
					_machineActive[i] = false;
				}
				else 
				{
					for (int c=0; c<MAX_CONNECTIONS; c++)
					{
						volMatrix[i][c] = pMac[i]->_inputConVol[c];
					}
				}
			}
			
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

			// move machines around to where they really should go
			// now we have to remap all the inputs and outputs again... ouch
			

			for (i = 0; i < 64; i++)
			{
				if ((busMachine[i] < MAX_MACHINES-1) && (busMachine[i] > 0))
				{
					if (_machineActive[busMachine[i]])
					{
						if (pMac[busMachine[i]]->_mode == MACHMODE_GENERATOR)
						{
							song._pMachine[i] = pMac[busMachine[i]];
							_machineActive[busMachine[i]] = false; // don't update this twice;

							for (int c=0; c<MAX_CONNECTIONS; c++)
							{
								if (song._pMachine[i]->_inputCon[c])
								{
									for (int x=0; x<64; x++)
									{
										if (song._pMachine[i]->_inputMachines[c] == busMachine[x])
										{
											song._pMachine[i]->_inputMachines[c] = x;
											break;
										}
										else if (song._pMachine[i]->_inputMachines[c] == busEffect[x])
										{
											song._pMachine[i]->_inputMachines[c] = x+MAX_BUSES;
											break;
										}
									}
								}

								if (song._pMachine[i]->_connection[c])
								{
									if (song._pMachine[i]->_outputMachines[c] == 0)
									{
										song._pMachine[i]->_outputMachines[c] = MASTER_INDEX;
									}
									else
									{
										for (int x=0; x<64; x++)
										{
											if (song._pMachine[i]->_outputMachines[c] == busMachine[x])
											{
												song._pMachine[i]->_outputMachines[c] = x;
												break;
											}
											else if (song._pMachine[i]->_outputMachines[c] == busEffect[x])
											{
												song._pMachine[i]->_outputMachines[c] = x+MAX_BUSES;
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
							song._pMachine[i+MAX_BUSES] = pMac[busEffect[i]];
							_machineActive[busEffect[i]] = false; // don't do this again

							for (int c=0; c<MAX_CONNECTIONS; c++)
							{
								if (song._pMachine[i+MAX_BUSES]->_inputCon[c])
								{
									for (int x=0; x<64; x++)
									{
										if (song._pMachine[i+MAX_BUSES]->_inputMachines[c] == busMachine[x])
										{
											song._pMachine[i+MAX_BUSES]->_inputMachines[c] = x;
											break;
										}
										else if (song._pMachine[i+MAX_BUSES]->_inputMachines[c] == busEffect[x])
										{
											song._pMachine[i+MAX_BUSES]->_inputMachines[c] = x+MAX_BUSES;
											break;
										}
									}
								}
								if (song._pMachine[i+MAX_BUSES]->_connection[c])
								{
									if (song._pMachine[i+MAX_BUSES]->_outputMachines[c] == 0)
									{
										song._pMachine[i+MAX_BUSES]->_outputMachines[c] = MASTER_INDEX;
									}
									else
									{
										for (int x=0; x<64; x++)
										{
											if (song._pMachine[i+MAX_BUSES]->_outputMachines[c] == busMachine[x])
											{
												song._pMachine[i+MAX_BUSES]->_outputMachines[c] = x;
												break;
											}
											else if (song._pMachine[i+MAX_BUSES]->_outputMachines[c] == busEffect[x])
											{
												song._pMachine[i+MAX_BUSES]->_outputMachines[c] = x+MAX_BUSES;
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
			for (int c=0; c<MAX_CONNECTIONS; c++)
			{
				if (song._pMachine[MASTER_INDEX]->_inputCon[c])
				{
					for (int x=0; x<64; x++)
					{
						if (song._pMachine[MASTER_INDEX]->_inputMachines[c] == busMachine[x])
						{
							song._pMachine[MASTER_INDEX]->_inputMachines[c] = x;
							break;
						}
						else if (song._pMachine[MASTER_INDEX]->_inputMachines[c] == busEffect[x])
						{
							song._pMachine[MASTER_INDEX]->_inputMachines[c] = x+MAX_BUSES;
							break;
						}
					}
				}
			}
		

		
			// fix machine #s

			for (i = 0; i < MAX_MACHINES-1; i++)
			{
				if (song._pMachine[i])
				{
					song._pMachine[i]->_macIndex = i;
					for (int j = i+1; j < MAX_MACHINES-1; j++)
					{
						if (song._pMachine[i] == song._pMachine[j])
						{
							assert(false);
							// we have duplicate machines...
							// this should NEVER happen
							// delete the second one :(
							song._pMachine[j] = NULL;
							// and we should remap anything that had wires to it to the first one
						}
					}
				}
			}
			
		
			// test all connections for invalid machines. disconnect invalid machines.
			for (i = 0; i < MAX_MACHINES; i++)
			{
				if (song._pMachine[i])
				{
					song._pMachine[i]->_connectedInputs = 0;
					song._pMachine[i]->_connectedOutputs = 0;

					for (int c = 0; c < MAX_CONNECTIONS; c++)
					{
						if (song._pMachine[i]->_connection[c])
						{
							if (song._pMachine[i]->_outputMachines[c] < 0 || song._pMachine[i]->_outputMachines[c] >= MAX_MACHINES)
							{
								song._pMachine[i]->_connection[c]=false;
								song._pMachine[i]->_outputMachines[c]=-1;
							}
							else if (!song._pMachine[song._pMachine[i]->_outputMachines[c]])
							{
								song._pMachine[i]->_connection[c]=false;
								song._pMachine[i]->_outputMachines[c]=-1;
							}
							else 
							{
								song._pMachine[i]->_connectedOutputs++;
							}
						}
						else
						{
							song._pMachine[i]->_outputMachines[c]=255;
						}

						if (song._pMachine[i]->_inputCon[c])
						{
							if (song._pMachine[i]->_inputMachines[c] < 0 || song._pMachine[i]->_inputMachines[c] >= MAX_MACHINES-1)
							{
								song._pMachine[i]->_inputCon[c]=false;
								song._pMachine[i]->_inputMachines[c]=-1;
							}
							else if (!song._pMachine[song._pMachine[i]->_inputMachines[c]])
							{
								song._pMachine[i]->_inputCon[c]=false;
								song._pMachine[i]->_inputMachines[c]=-1;
							}
							else
							{
								song._pMachine[i]->_connectedInputs++;
							}
						}
						else
						{
							song._pMachine[i]->_inputMachines[c]=255;
						}
					}
				}
			}
		
		// Reparse any pattern for converted machines.
			converter->retweak(song);
			song.seqBus=0;
			return true;
		}

				#endif

		bool Machine::LoadPsy2FileFormat(RiffFile* pFile)
		{
			char edName[32];
			pFile->ReadChunk(edName, 16); edName[15] = 0;
			_editName=edName;

			pFile->Read(_inputMachines);
			pFile->Read(_outputMachines);
			pFile->Read(_inputConVol);
			pFile->Read(_connection);
			pFile->Read(_inputCon);
			pFile->Read(_connectionPoint);
			pFile->Read(_connectedInputs);
			pFile->Read(_connectedOutputs);

			pFile->Read(_panning);
			Machine::SetPan(_panning);

			pFile->Skip(4*8); // SubTrack[]
			pFile->Skip(4); // numSubtracks
			pFile->Skip(4); // interpol

			pFile->Skip(4); // outwet
			pFile->Skip(4); // outdry

			pFile->Skip(4); // distPosThreshold
			pFile->Skip(4); // distPosClamp
			pFile->Skip(4); // distNegThreshold
			pFile->Skip(4); // distNegClamp

			pFile->Skip(1); // sinespeed
			pFile->Skip(1); // sineglide
			pFile->Skip(1); // sinevolume
			pFile->Skip(1); // sinelfospeed
			pFile->Skip(1); // sinelfoamp

			pFile->Skip(4); // delayTimeL
			pFile->Skip(4); // delayTimeR
			pFile->Skip(4); // delayFeedbackL
			pFile->Skip(4); // delayFeedbackR

			pFile->Skip(4); // filterCutoff
			pFile->Skip(4); // filterResonance
			pFile->Skip(4); // filterLfospeed
			pFile->Skip(4); // filterLfoamp
			pFile->Skip(4); // filterLfophase
			pFile->Skip(4); // filterMode

			return true;
		}

		bool Master::LoadPsy2FileFormat(RiffFile* pFile)
		{
			char edName[32];
			pFile->ReadChunk(edName, 16); edName[15] = 0;
			_editName=edName;
			
			pFile->Read(_inputMachines);
			pFile->Read(_outputMachines);
			pFile->Read(_inputConVol);
			pFile->Read(_connection);
			pFile->Read(_inputCon);
			pFile->Read(_connectionPoint);
			pFile->Read(_connectedInputs);
			pFile->Read(_connectedOutputs);
			
			pFile->Read(_panning);
			Machine::SetPan(_panning);

			pFile->Skip(4*8); // SubTrack[]
			pFile->Skip(4); // numSubtracks
			pFile->Skip(4); // interpol

			/////////////
			_outDry = 0; pFile->Read(_outDry); // outdry
			/////////////

			pFile->Skip(4); // outwet
			
			pFile->Skip(4); // distPosThreshold
			pFile->Skip(4); // distPosClamp
			pFile->Skip(4); // distNegThreshold
			pFile->Skip(4); // distNegClamp

			pFile->Skip(1); // sinespeed
			pFile->Skip(1); // sineglide
			pFile->Skip(1); // sinevolume
			pFile->Skip(1); // sinelfospeed
			pFile->Skip(1); // sinelfoamp

			pFile->Skip(4); // delayTimeL
			pFile->Skip(4); // delayTimeR
			pFile->Skip(4); // delayFeedbackL
			pFile->Skip(4); // delayFeedbackR

			pFile->Skip(4); // filterCutoff
			pFile->Skip(4); // filterResonance
			pFile->Skip(4); // filterLfospeed
			pFile->Skip(4); // filterLfoamp
			pFile->Skip(4); // filterLfophase
			pFile->Skip(4); // filterMode

			return true;
		}

		bool Sampler::LoadPsy2FileFormat(RiffFile* pFile)
		{
			int i;

			char edName[32];
			pFile->ReadChunk(edName, 16); edName[15] = 0;
			_editName=edName;

			pFile->Read(_inputMachines);
			pFile->Read(_outputMachines);
			pFile->Read(_inputConVol);
			pFile->Read(_connection);
			pFile->Read(_inputCon);
			pFile->Read(_connectionPoint);
			pFile->Read(_connectedInputs);
			pFile->Read(_connectedOutputs);

			pFile->Read(_panning);
			Machine::SetPan(_panning);
			pFile->Skip(4*8); // SubTrack[]
			pFile->Read(_numVoices); // numSubtracks

			if (_numVoices < 4)
			{
				// Psycle versions < 1.1b2 had polyphony per channel,not per machine.
				_numVoices = 8;
			}

			pFile->Read(i); // interpol
			switch (i)
			{
			case 2:
				_resampler.SetQuality(dsp::R_SPLINE);
				break;
			case 0:
				_resampler.SetQuality(dsp::R_NONE);
				break;
			default:
			case 1:
				_resampler.SetQuality(dsp::R_LINEAR);
				break;
			}

			pFile->Skip(4); // outdry
			pFile->Skip(4); // outwet

			pFile->Skip(4); // distPosThreshold
			pFile->Skip(4); // distPosClamp
			pFile->Skip(4); // distNegThreshold
			pFile->Skip(4); // distNegClamp

			pFile->Skip(1); // sinespeed
			pFile->Skip(1); // sineglide
			pFile->Skip(1); // sinevolume
			pFile->Skip(1); // sinelfospeed
			pFile->Skip(1); // sinelfoamp

			pFile->Skip(4); // delayTimeL
			pFile->Skip(4); // delayTimeR
			pFile->Skip(4); // delayFeedbackL
			pFile->Skip(4); // delayFeedbackR

			pFile->Skip(4); // filterCutoff
			pFile->Skip(4); // filterResonance
			pFile->Skip(4); // filterLfospeed
			pFile->Skip(4); // filterLfoamp
			pFile->Skip(4); // filterLfophase
			pFile->Skip(4); // filterMode

			return true;
		}

		bool Plugin::LoadPsy2FileFormat(RiffFile* pFile)
		{
			bool result = true;
			char junk[256];
			std::memset(junk, 0, sizeof junk);

			char sDllName[256];
			int numParameters;

			pFile->Read(sDllName); // Plugin dll name
			sDllName[255]='\0';
			std::string strname = sDllName;
			std::transform(strname.begin(),strname.end(),strname.begin(),ToLower());

			//Patch: Automatically replace old AS's by AS2F.
			bool wasAB=false;
			bool wasAS1=false;
			bool wasAS2=false;
			if (strname == "arguru bass.dll" )
			{
				strname = "arguru synth 2f.dll";
				wasAB=true;
			}
			else if (strname == "arguru synth.dll" )
			{
				strname = "arguru synth 2f.dll";
				wasAS1=true;
			}
			else if (strname == "arguru synth 2.dll" )
			{
				strname = "arguru synth 2f.dll";
				wasAS2=true;
			}
			else if (strname == "synth21.dll" )
			{
				strname = "arguru synth 2f.dll";
				wasAS2=true;
			}

//			Global::dllfinder().LookupDllPath(strname,MACH_PLUGIN);
			try
			{
				result = LoadDll(strname);
			}
			catch(...)
			{
//				char sError[_MAX_PATH];
//				sprintf(sError,"Missing or corrupted native Plug-in \"%s\" - replacing with Dummy.",sDllName);
//				MessageBox(NULL,sError, "Error", MB_OK);
				result = false;
			}

			Init();

			char edName[32];
			pFile->ReadChunk(edName, 16); edName[15] = 0;
			_editName=edName;

			pFile->Read(numParameters);
			if(result)
			{
				std::int32_t * Vals = new std::int32_t[numParameters];
				pFile->ReadChunk(Vals, numParameters * sizeof(std::int32_t));
				try
				{
					if ( wasAB ) // Patch to replace Arguru Bass by Arguru Synth 2f
					{
						proxy().ParameterTweak(0,Vals[0]);
						for (int i=1;i<15;i++)
						{
							proxy().ParameterTweak(i+4,Vals[i]);
						}
						proxy().ParameterTweak(19,0);
						proxy().ParameterTweak(20,Vals[15]);
						if (numParameters>16)
						{
							proxy().ParameterTweak(24,Vals[16]);
							proxy().ParameterTweak(25,Vals[17]);
						}
						else
						{
							proxy().ParameterTweak(24,0);
							proxy().ParameterTweak(25,0);
						}
					}
					else for (int i=0; i<numParameters; i++)
					{
						proxy().ParameterTweak(i,Vals[i]);
					}
				}
				catch(const std::exception &)
				{
//					loggers::warning(UNIVERSALIS__COMPILER__LOCATION);
				}
				try
				{
					int size = proxy().GetDataSize();
					//pFile->Read(size);	// This would have been the right thing to do
					if(size)
					{
						char * pData = new char[size];
						pFile->ReadChunk(pData, size); // Number of parameters
						try
						{
							proxy().PutData(pData); // Internal load
						}
						catch(const std::exception &)
						{
						}
						delete[] pData;
					}
				}
				catch(std::exception const &)
				{
//					loggers::warning(UNIVERSALIS__COMPILER__LOCATION);
				}
				if(wasAS1) // Patch to replace Synth1 by Arguru Synth 2f
				{
					try
					{
						proxy().ParameterTweak(17,Vals[17]+10);
						proxy().ParameterTweak(24,0);
						proxy().ParameterTweak(25,0);
					}
					catch(const std::exception &)
					{
					}
				}
				if(wasAS2)
				{
					try
					{
						proxy().ParameterTweak(24,0);
						proxy().ParameterTweak(25,0);
					}
					catch(const std::exception&)
					{
					}
				}
				delete[] Vals;
			}
			else
			{
				for (int i=0; i<numParameters; i++)
				{
					pFile->ReadChunk(junk, 4);
				}
			}

			pFile->Read(_inputMachines);
			pFile->Read(_outputMachines);
			pFile->Read(_inputConVol);
			pFile->Read(_connection);
			pFile->Read(_inputCon);
			pFile->Read(_connectionPoint);
			pFile->Read(_connectedInputs);
			pFile->Read(_connectedOutputs);

			pFile->Read(_panning);
			Machine::SetPan(_panning);

			pFile->Skip(4*8); // SubTrack[]
			pFile->Skip(4); // numSubtracks
			pFile->Skip(4); // interpol

			pFile->Skip(4); // outwet
			pFile->Skip(4); // outdry

			pFile->Skip(4); // distPosThreshold
			pFile->Skip(4); // distPosClamp
			pFile->Skip(4); // distNegThreshold
			pFile->Skip(4); // distNegClamp

			pFile->Skip(1); // sinespeed
			pFile->Skip(1); // sineglide
			pFile->Skip(1); // sinevolume
			pFile->Skip(1); // sinelfospeed
			pFile->Skip(1); // sinelfoamp

			pFile->Skip(4); // delayTimeL
			pFile->Skip(4); // delayTimeR
			pFile->Skip(4); // delayFeedbackL
			pFile->Skip(4); // delayFeedbackR

			pFile->Skip(4); // filterCutoff
			pFile->Skip(4); // filterResonance
			pFile->Skip(4); // filterLfospeed
			pFile->Skip(4); // filterLfoamp
			pFile->Skip(4); // filterLfophase
			pFile->Skip(4); // filterMode

			return result;
		}
/*
		namespace vst
		{
			bool plugin::LoadChunkPsy2FileFormat(RiffFile * pFile)
			{
				bool b;
				try
				{
					b = proxy().flags() & effFlagsProgramChunks;
				}
				catch(const std::exception &)
				{
					b = false;
				}
				if(!b) return false;

				// read chunk size
				std::uint32_t chunk_size;
				pFile->Read(chunk_size);

				// read chunk data
				char * chunk(new char[chunk_size]);
				pFile->ReadChunk(chunk, chunk_size);

				try
				{
					proxy().dispatcher(effSetChunk, 0, chunk_size, chunk);
				}
				catch(const std::exception &)
				{
					// [bohan] hmm, so, data just gets lost?
					delete[] chunk;
					return false;
				}

				delete[] chunk;
				return true;
			}

			bool plugin::LoadPsy2FileFormat(RiffFile * pFile)
			{
				Machine::Init();

				char edName[32];
				pFile->ReadChunk(edName, 16); edName[15] = 0;
				_editName=edName;

				pFile->Read(_inputMachines);
				pFile->Read(_outputMachines);
				pFile->Read(_inputConVol);
				pFile->Read(_connection);
				pFile->Read(_inputCon);
				pFile->Read(_connectionPoint);
				pFile->Read(_connectedInputs);
				pFile->Read(_connectedOutputs);

				pFile->Read(_panning);
				Machine::SetPan(_panning);

				pFile->Skip(4*8); // SubTrack[]
				pFile->Skip(4); // numSubtracks
				pFile->Skip(4); // interpol

				pFile->Skip(4); // outwet
				pFile->Skip(4); // outdry

				pFile->Skip(4); // distPosThreshold
				pFile->Skip(4); // distPosClamp
				pFile->Skip(4); // distNegThreshold
				pFile->Skip(4); // distNegClamp

				pFile->Skip(1); // sinespeed
				pFile->Skip(1); // sineglide
				pFile->Skip(1); // sinevolume
				pFile->Skip(1); // sinelfospeed
				pFile->Skip(1); // sinelfoamp

				pFile->Skip(4); // delayTimeL
				pFile->Skip(4); // delayTimeR
				pFile->Skip(4); // delayFeedbackL
				pFile->Skip(4); // delayFeedbackR

				pFile->Skip(4); // filterCutoff
				pFile->Skip(4); // filterResonance
				pFile->Skip(4); // filterLfospeed
				pFile->Skip(4); // filterLfoamp
				pFile->Skip(4); // filterLfophase
				pFile->Skip(4); // filterMode

				bool old;
				pFile->Read(old); // old format
				pFile->Read(_instance); // ovst.instance
				if(old)
				{
					char mch;
					pFile->Read(mch);
					_program = 0;
				}
				else
				{
					pFile->Read(_program);
				}
				return true;
			}
		}
		*/
	}
}