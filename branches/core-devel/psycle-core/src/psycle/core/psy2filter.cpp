/***************************************************************************
*   Copyright (C) 2007 Psycledelics     *
*   psycle.sf.net   *
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

#include "psy2filter.h"

#include "commands.h"
#include "fileio.h"
#include "internal_machines.h"
#include "sampler.h"
#include "plugin.h"
#include "song.h"

#include "convert_internal_machines.private.hpp"

namespace psy
{
	namespace core
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

		Psy2Filter::Psy2Filter()
		:
			singleCat(),
			singleLine()
		{}

		bool Psy2Filter::testFormat(const std::string & fileName)
		{
			RiffFile file;
			file.Open(fileName);
			char Header[9];
			file.ReadArray(Header, 8);
			Header[8]=0;
			file.Close();
			return !std::strcmp(Header,"PSY2SONG");
		}
		
		void Psy2Filter::preparePatternSequence( CoreSong & song )
		{
			seqList.clear();
			song.patternSequence()->removeAll();
			// creatse a single Pattern Category
			singleCat = song.patternSequence()-> patternData()->createNewCategory("SinglePattern");
			// here we add in one single Line the patterns
			singleLine = song.patternSequence()->createNewLine();
		}

		bool Psy2Filter::load(std::string const & plugin_path, const std::string & fileName, CoreSong & song, MachineCallbacks* callbacks)
		{
			std::int32_t num;
			RiffFile file;
			file.Open(fileName);
			//progress.emit(1,0,"");
			//progress.emit(2,0,"Loading... psycle song fileformat version 2...");

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
			convert_internal_machines::Converter converter(plugin_path);
			LoadMACD(plugin_path, &file,song,&converter,callbacks);
			TidyUp(&file,song,&converter);
			return true;
		}

		bool Psy2Filter::LoadINFO(RiffFile* file,CoreSong& song)
		{
			char Name[32];
			char Author[32];
			char Comment[128];

			file->ReadArray(Name, 32); Name[31]=0;
			song.setName(Name);
			file->ReadArray(Author, 32); Author[31]=0;
			song.setAuthor(Author);
			bool err = file->ReadArray(Comment, 128); Comment[127]=0;
			song.setComment(Comment);
			return err;
		}

		bool Psy2Filter::LoadSNGI(RiffFile* file,CoreSong& song)
		{
			std::int32_t tmp;
			unsigned char oct;
			file->Read(tmp);
			song.setBpm(tmp);

			file->Read(tmp);
			if( tmp <= 0)
			{
				// Shouldn't happen but has happened.
				song.setLinesPerBeat(4);
			}
			else song.setLinesPerBeat(static_cast<int>( 44100 * 15 * 4 / (tmp * song.bpm()) ));

			file->Read(oct);
			// note: we don't change the current octave of the gui anymore when loading a song

			file->ReadArray(busMachine,64);
			return true;
		}

		bool Psy2Filter::LoadSEQD(RiffFile* file,CoreSong& song)
		{
			std::int32_t length,tmp;
			unsigned char playOrder[128];
			file->ReadArray(playOrder,128);
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
			//Convert old tweak effect command to common tweak.
			if (data[0] == psy::core::commands::tweak_effect)
			{
				event.setNote(psy::core::commands::tweak);
				data++;
				event.setInstrument((*data)++);
				event.setMachine((*data)+0x40);
				data++;
			}
			else
			{
				event.setNote(*data++);
				event.setInstrument(*data++);
				event.setMachine(*data++);
			}
			event.setCommand(*data++);
			event.setParameter(*data++);
			return event;
		}

		bool Psy2Filter::LoadPATD(RiffFile* file,CoreSong& song,int index)
		{
			std::int32_t numLines;
			char patternName[32];
			file->Read(numLines);
			file->ReadArray(patternName, sizeof(patternName)); patternName[31]=0;

			if(numLines > 0)
			{
				// create a SinglePattern
				std::string indexStr;
				std::ostringstream o;
				if (!(o << index)) indexStr = "error";
				else indexStr = o.str();
				SinglePattern* pat = singleCat->createNewPattern(std::string(patternName)+indexStr);
				pat->setBeatZoom(song.linesPerBeat());
				TimeSignature & sig =  pat->timeSignatures().back();
				float beats = numLines / (float) song.linesPerBeat();
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
						file->ReadArray(entry,5);
						PatternEvent event = convertEntry(entry);
						if (!event.empty()) {
							float position = y / (float) song.linesPerBeat();
							if (event.note() == commands::tweak) {
								(*pat)[position].tweaks()[pat->tweakTrack(TweakTrackInfo(event.machine(),event.parameter(),TweakTrackInfo::twk))] = event;
							} else (*pat)[position].notes()[x] = event;
						}
					}
					file->Skip((PSY2_MAX_TRACKS-song.tracks())*EVENT_SIZE);
				}
			}
			///\todo: verify that the sequence doesn't use this pattern, and if it does, make it use an empty pattern
			/*
			else
			{
				patternLines[i] = 64;
				RemovePattern(i);
			}
			*/
			return true;
		}

		bool Psy2Filter::LoadINSD(RiffFile* file,CoreSong& song)
		{
			std::int32_t i;
			file->Read(song.instSelected);

			for(i=0 ; i < PSY2_MAX_INSTRUMENTS ; ++i)
			{
				file->ReadArray(song._pInstrument[i]->_sName,32); song._pInstrument[i]->_sName[31]=0;
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
				
		bool Psy2Filter::LoadWAVD(RiffFile* file,CoreSong& song)
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
							file->ReadArray(pIns.waveName, 32); pIns.waveName[31]=0;
							file->Read(pIns.waveVolume);
							file->Read(tmpFineTune);
							pIns.waveFinetune=tmpFineTune;
							file->Read(pIns.waveLoopStart);
							file->Read(pIns.waveLoopEnd);
							file->Read(pIns.waveLoopType);
							file->Read(pIns.waveStereo);
							pIns.waveDataL = new std::int16_t[pIns.waveLength];
							file->ReadArray(pIns.waveDataL, pIns.waveLength);
							if (pIns.waveStereo)
							{
								pIns.waveDataR = new std::int16_t[pIns.waveLength];
								file->ReadArray(pIns.waveDataR, pIns.waveLength);
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

		bool Psy2Filter::PreLoadVSTs(RiffFile* file,CoreSong& song)
		{
			std::int32_t i;
			for (i=0; i<PSY2_MAX_PLUGINS; i++)
			{
				file->Read(vstL[i].valid);
				if( vstL[i].valid )
				{
					file->ReadArray(vstL[i].dllName,128); vstL[i].dllName[127]=0;
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
		
		bool Psy2Filter::LoadMACD(std::string const & plugin_path, RiffFile* file,CoreSong& song,convert_internal_machines::Converter* converter, MachineCallbacks* callbacks)
		{
			std::int32_t i;
			file->ReadArray(_machineActive,128);
			std::memset(pMac,0,sizeof pMac);

			for (i=0; i<128; i++)
			{
				Sampler* pSampler=NULL;
				//XMSampler* pXMSampler=NULL;
				Plugin* pPlugin=NULL;
				//vst::plugin * pVstPlugin(0);
				std::int32_t x,y,type;
				if (_machineActive[i])
				{
					//progress.emit(4,8192+i*(4096/128),"");

					file->Read(x);
					file->Read(y);

					file->Read(type);

					if(converter->plugin_names().exists(type))
						pMac[i] = &converter->redirect(i, type, *file,song);
					else switch (type)
					{
					case MACH_PLUGIN:
					{
						pMac[i] = pPlugin = new Plugin(callbacks,i,&song);
						// Should the "Init()" function go here? -> No. Needs to load the dll first.
						if (!pMac[i]->LoadPsy2FileFormat(plugin_path, file))
						{
							Machine* pOldMachine = pMac[i];
							pMac[i] = new Dummy(pOldMachine);
							// dummy name goes here
							std::stringstream s;
							s << "X!" << pOldMachine->GetEditName();
							pMac[i]->SetEditName(s.str());
							delete pOldMachine; pOldMachine = 0;
						}
						break;
					}
					case MACH_MASTER:
						pMac[i] = song.machine(MASTER_INDEX);
						goto init_and_load;
					case MACH_SAMPLER:
						pMac[i] = pSampler = new Sampler(callbacks,i,&song);
						goto init_and_load;
					#if 0
					case MACH_XMSAMPLER:
						pMac[i] = pXMSampler = new XMSampler(i);
						goto init_and_load;
					#endif
					case MACH_VST:
					case MACH_VSTFX:
						//if (type == MACH_VST) pMac[i] = pVstPlugin = new vst::instrument(i);
						//else if (type == MACH_VSTFX) pMac[i] = pVstPlugin = new vst::fx(i);
						pMac[i] = new Dummy(callbacks,i,&song);
						if (type == MACH_VST ) pMac[i]->mode(MACHMODE_FX);
						else pMac[i]->mode(MACHMODE_GENERATOR);
						static_cast<Dummy*>(pMac[i])->wasVST=true;
						goto init_and_load_VST;
					case MACH_SCOPE:
					case MACH_DUMMY:
						pMac[i] = new Dummy(callbacks,i,&song);
						goto init_and_load;
					default:
						{
							std::ostringstream s;
							s << "unkown machine type: " << type;
							//MessageBox(0, s.str().c_str(), "Loading old song", MB_ICONERROR);
						}
						pMac[i] = new Dummy(callbacks,i,&song);
		
					init_and_load:
						pMac[i]->Init();
						pMac[i]->LoadPsy2FileFormat(plugin_path, file);
						break;
					init_and_load_VST:
						pMac[i]->LoadPsy2FileFormat(plugin_path, file);
						std::stringstream s;
						s << "X!" << pMac[i]->GetEditName();
						pMac[i]->SetEditName(s.str());
			
						file->Skip(sizeof(bool)+5);
						#if 0
						if ((pMac[i]->LoadOldFileFormat(file)) && (vstL[pVstPlugin->_instance].valid)) // Machine::Init() is done Inside "Load()"
						{
							std::string path = vstL[pVstPlugin->_instance].dllName;
							if(!Gloxxxxxxxxxxxxxxxxxxxxbal::dllfinder().LookupDllPath(path,MACH_VST)) 
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
						#endif
						break;
					}
					pMac[i]->SetPosX(x);
					pMac[i]->SetPosY(y);
				}
			}
			
			// Patch 0: Some extra data added around the 1.0 release.
			for (i=0; i<PSY2_MAX_INSTRUMENTS; i++)
			{
				file->Read(song._pInstrument[i]->_loop);
			}
			for (i=0; i<PSY2_MAX_INSTRUMENTS; i++)
			{
				file->Read(song._pInstrument[i]->_lines);
			}

			// Validate the machine arrays. At the same time we fill volMatrix that
			// we will use later on to correctly initialize the wire volumes.
			for (i=0; i<128; i++) // First, we add the output volumes to a Matrix for latter reference
			{
				if (!_machineActive[i])
				{
					delete pMac[i]; pMac[i]=0;
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

			// Patch 1: BusEffects (twf). Try to read it, and if it doesn't exist, generate it.
			if ( file->ReadArray(busEffect,64) == false )
			{
				int j=0;
				unsigned char invmach[128];
				memset(invmach,255,sizeof(invmach));
				// The guessing procedure does not rely on the machmode because if a plugin
				// is missing, then it is always tagged as a generator.
				for (int i = 0; i < 64; i++)
				{
					if (busMachine[i] != 255) invmach[busMachine[i]]=i;
				}
				for ( i=1;i<128;i++ ) // Machine 0 is the master machine.
				{
					if (_machineActive[i])
					{
						if (invmach[i] == 255)
						{
							busEffect[j]=i;
							j++;
						}
					}
				}
				for (; j < 64; j++)
				{
					busEffect[j] = 255;
				}
			}
			// Validate that there isn't any duplicated machine.
			for ( int i=0;i<64;i++ ) 
			{
				for ( int j=i+1;j<64;j++ ) 
				{
					if  (busMachine[i] == busMachine[j]) busMachine[j]=255;
					if  (busEffect[i] == busEffect[j]) busEffect[j]=255;
				}
				for (int j=0;j<64;j++)
				{
					if  (busMachine[i] == busEffect[j]) busEffect[j]=255;
				}
			}

			// Patch 1.2: Fixes erroneous machine mode when a dummy replaces a bad plugin
			// (missing dll, or when the load process failed).
			// At the same time, we validate the indexes of the busMachine and busEffects arrays.
			for ( i=0;i<64;i++ ) 
			{
				if (busEffect[i] != 255)
				{
					if ( busEffect[i] > 128 || !_machineActive[busEffect[i]] )
						busEffect[i] = 255;
					// If there's a dummy, force it to be an effect
					else if (pMac[busEffect[i]]->type() == MACH_DUMMY ) 
					{
						pMac[busEffect[i]]->mode(MACHMODE_FX);
					}
					// Else if the machine is a generator, move it to gens bus.
					// This can't happen, but it is here for completeness
					else if (pMac[busEffect[i]]->mode() == MACHMODE_GENERATOR)
					{
						int k=0;
						while (busEffect[k] != 255 && k<MAX_BUSES) 
						{
							k++;
						}
						busMachine[k]=busEffect[i];
						busEffect[i]=255;
					}
				}
				if (busMachine[i] != 255)
				{
					if (busMachine[i] > 128 || !_machineActive[busMachine[i]])
						busMachine[i] = 255;
					// If there's a dummy, force it to be a Generator
					else if (pMac[busMachine[i]]->type() == MACH_DUMMY ) 
					{
						pMac[busMachine[i]]->mode(MACHMODE_GENERATOR);
					}
					// Else if the machine is an fx, move it to FXs bus.
					// This can't happen, but it is here for completeness
					else if ( pMac[busMachine[i]]->mode() != MACHMODE_GENERATOR)
					{
						int j=0;
						while (busEffect[j] != 255 && j<MAX_BUSES) 
						{
							j++;
						}
						busEffect[j]=busMachine[i];
						busMachine[i]=255;
					}
				}
			}
			#if 0
			// Patch 2: VST's Chunk.
			bool chunkpresent=false;
			file->Read(chunkpresent);

			if ( fullopen  && chunkpresent) for ( i=0;i<128;i++ )
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
			#endif
			
			return true;
		}

		//Finished all the file loading. Now Process the data to the current structures
		bool Psy2Filter::TidyUp(RiffFile*file,CoreSong&song,convert_internal_machines::Converter* converter)
		{
			// The old fileformat stored the volumes on each output, 
			// so what we have in inputConVol is really the output
			// and we have to convert it.
			std::int32_t i;
			// Clean "pars" array.
			for (i=0; i<PSY2_MAX_PLUGINS; i++) 
			{
				if( vstL[i].valid )
				{
					delete vstL[i].pars; vstL[i].pars = 0;
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
		
		
			// The old fileformat stored the volumes on each output, 
			// so what we have in inputConVol is really the output
			// and we have to convert it.
			for (i=0; i<128; i++) //  we go to fix this for each
			{
				if (_machineActive[i]) // valid machine (important, since we have to navigate!)
				{
					for (int c=0; c<MAX_CONNECTIONS; c++) // all of its input connections.
					{
						if (pMac[i]->_inputCon[c] && pMac[i]->_inputMachines[c] > -1 && pMac[pMac[i]->_inputMachines[c]]) // If there's a valid machine in this inputconnection,
						{
							Machine* pOrigMachine = pMac[pMac[i]->_inputMachines[c]]; // We get that machine
							int d = pOrigMachine->FindOutputWire(i); // and wire
							if ( d == -1 )
							{
								pMac[i]->_inputCon[c] = false; pMac[i]->_inputMachines[c] = -1;
							}
							else
							{

								float val = volMatrix[pMac[i]->_inputMachines[c]][d];
								if( val > 4.1f ) 
								{
									val*=0.000030517578125f; // BugFix
								}
								else if ( val < 0.00004f) 
								{
									val*=32768.0f; // BugFix
								}

								pMac[i]->InitWireVolume(pOrigMachine->type(),c,val);
							}
						}
						else { pMac[i]->_inputCon[c] = false; pMac[i]->_inputMachines[c] = -1; }
					}
				}
			}

			// Psycle no longer uses busMachine and busEffect, since the song.machine_ Array directly maps
			// to the real machine.
			// Due to this, we have to move machines to where they really are, 
			// and remap the inputs and outputs indexes again... ouch
			// At the same time, we validate each wire, and the number count.
			unsigned char invmach[128];
			memset(invmach,255,sizeof(invmach));
			for (i = 0; i < 64; i++)
			{
				if (busMachine[i] != 255) invmach[busMachine[i]]=i;
				if (busEffect[i] != 255) invmach[busEffect[i]]=i+64;
			}
			invmach[0]=MASTER_INDEX;

			for (i = 0; i < 128; i++)
			{
				if (invmach[i] != 255)
				{
					song.machine(invmach[i], pMac[i]);
					Machine *cMac = song.machine(invmach[i]);
					_machineActive[i] = false; // mark as "converted"
					cMac->_connectedInputs = 0;
					cMac->_connectedOutputs = 0;
					for (int c=0; c<MAX_CONNECTIONS; c++)
					{
						if (cMac->_inputCon[c])
						{
							if (cMac->_inputMachines[c] < 0 || cMac->_inputMachines[c] >= MAX_MACHINES-1)
							{
								cMac->_inputCon[c]=false;
								cMac->_inputMachines[c]=-1;
							}
							else if (!pMac[cMac->_inputMachines[c]])
							{
								cMac->_inputCon[c]=false;
								cMac->_inputMachines[c]=-1;
							}
							else
							{
								cMac->_inputMachines[c] = invmach[cMac->_inputMachines[c]];
								cMac->_connectedInputs++;
							}
						}
						if (cMac->_connection[c])
						{
							if (cMac->_outputMachines[c] < 0 || cMac->_outputMachines[c] >= MAX_MACHINES)
							{
								cMac->_connection[c]=false;
								cMac->_outputMachines[c]=-1;
							}
							else if (!pMac[cMac->_outputMachines[c]])
							{
								cMac->_connection[c]=false;
								cMac->_outputMachines[c]=-1;
							}
							else 
							{
								cMac->_outputMachines[c] = invmach[cMac->_outputMachines[c]];
								cMac->_connectedOutputs++;
							}
						}
					}
				}
			}
			// verify that there isn't any machine that hasn't been copied into song.machine_
			// Shouldn't happen. It would mean a damaged file.
			int j=0;
			int k=64;
			for (i=0;i < 128; i++)
			{
				if (_machineActive[i])
				{
					if ( pMac[i]->mode() == MACHMODE_GENERATOR)
					{
						while (song.machine(j) && j<64) j++;
						song.machine(j,pMac[i]);
					}
					else
					{
						while (song.machine(k) && k<128) k++;
						song.machine(k,pMac[i]);
					}
				}
			}
		
			// Reparse any pattern for converted machines.
			converter->retweak(song);
			//for (i =0; i < MAX_MACHINES;++i) if ( _pMachine[i]) _pMachine[i]->PostLoad();
			song.seqBus=0;
			return true;
		}

		bool Machine::LoadPsy2FileFormat(std::string const & plugin_path, RiffFile* pFile)
		{
			char edName[32];
			pFile->ReadArray(edName, 16); edName[15] = 0;
			SetEditName(edName);

			pFile->ReadArray(_inputMachines,MAX_CONNECTIONS);
			pFile->ReadArray(_outputMachines,MAX_CONNECTIONS);
			pFile->ReadArray(_inputConVol,MAX_CONNECTIONS);
			pFile->ReadArray(_connection,MAX_CONNECTIONS);
			pFile->ReadArray(_inputCon,MAX_CONNECTIONS);
			pFile->Skip(96); // ConnectionPoints, 12*8bytes
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
			pFile->ReadArray(edName, 16); edName[15] = 0;
			SetEditName(edName);
			
			pFile->ReadArray(_inputMachines,MAX_CONNECTIONS);
			pFile->ReadArray(_outputMachines,MAX_CONNECTIONS);
			pFile->ReadArray(_inputConVol,MAX_CONNECTIONS);
			pFile->ReadArray(_connection,MAX_CONNECTIONS);
			pFile->ReadArray(_inputCon,MAX_CONNECTIONS);
			pFile->Skip(96); // ConnectionPoints, 12*8bytes
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

		bool Sampler::LoadPsy2FileFormat(std::string const & plugin_path, RiffFile* pFile)
		{
			int i;

			char edName[32];
			pFile->ReadArray(edName, 16); edName[15] = 0;
			SetEditName(edName);

			pFile->ReadArray(_inputMachines,MAX_CONNECTIONS);
			pFile->ReadArray(_outputMachines,MAX_CONNECTIONS);
			pFile->ReadArray(_inputConVol,MAX_CONNECTIONS);
			pFile->ReadArray(_connection,MAX_CONNECTIONS);
			pFile->ReadArray(_inputCon,MAX_CONNECTIONS);
			pFile->Skip(96); // ConnectionPoints, 12*8bytes
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

		bool Plugin::LoadPsy2FileFormat(std::string const & plugin_path, RiffFile* pFile)
		{
			bool result = true;
			char junk[256];
			std::memset(junk, 0, sizeof junk);

			char sDllName[256];
			int numParameters;

			pFile->ReadArray(sDllName,256); // Plugin dll name
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

			//Gloxxxxxxxxxxxxxxxxbal::dllfinder().LookupDllPath(strname,MACH_PLUGIN);
			try
			{
				result = LoadDll(plugin_path, strname);
			}
			catch(...)
			{
				//char sError[_MAX_PATH];
				//sprintf(sError,"Missing or corrupted native Plug-in \"%s\" - replacing with Dummy.",sDllName);
				//MessageBox(NULL,sError, "Error", MB_OK);
				result = false;
			}

			Init();

			char edName[32];
			pFile->ReadArray(edName, 16); edName[15] = 0;
			SetEditName(edName);

			pFile->Read(numParameters);
			if(result)
			{
				std::int32_t * Vals = new std::int32_t[numParameters];
				pFile->ReadArray(Vals, numParameters);
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
					//loggers::warning(UNIVERSALIS__COMPILER__LOCATION);
				}
				try
				{
					int size = proxy().GetDataSize();
					//pFile->Read(size); // This would have been the right thing to do
					if(size)
					{
						char * pData = new char[size];
						pFile->ReadArray(pData, size); // Number of parameters
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
					// loggers::warning(UNIVERSALIS__COMPILER__LOCATION);
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
					pFile->ReadArray(junk, 4);
				}
			}

			pFile->ReadArray(_inputMachines,MAX_CONNECTIONS);
			pFile->ReadArray(_outputMachines,MAX_CONNECTIONS);
			pFile->ReadArray(_inputConVol,MAX_CONNECTIONS);
			pFile->ReadArray(_connection,MAX_CONNECTIONS);
			pFile->ReadArray(_inputCon,MAX_CONNECTIONS);
			pFile->Skip(96); // ConnectionPoints, 12*8bytes
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
		#if 0
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
				SetEditName(edName);

				pFile->Read(_inputMachines);
				pFile->Read(_outputMachines);
				pFile->Read(_inputConVol);
				pFile->Read(_connection);
				pFile->Read(_inputCon);
				pFile->Skip(96); // ConnectionPoints, 12*8bytes
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
		#endif
	}
}
