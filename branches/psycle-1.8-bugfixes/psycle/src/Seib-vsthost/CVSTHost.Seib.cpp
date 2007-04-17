/*****************************************************************************/
/* CVSTHost.cpp: implementation for CVSTHost/CEffect classes (VST SDK 2.4r2).*/
/* Work Derived from the LGPL host "vsthost (1.16l)".						 */
/* vsthost is Copyright (c) H. Seib, 2002-2006								 */
/* (http://www.hermannseib.com/english/vsthost.htm)"						 */
/*****************************************************************************/
/*                                                                           */
/* $Revision: 2718 $ */
/* $Date: 2006-06-04 00:22:21 +0200 (dg., 04 juny 2006) $ */
/* $Author: jaz001 $ */
/*                                                                           */
/*****************************************************************************/

#include <project.private.hpp>
#include "CVSTHost.Seib.hpp"                   /* private prototypes                */
// Unneeded sources:
//#include "global.hpp" // for debug loggers.
#include "machine.hpp"// for throw.

#ifdef WIN32
	#pragma warning(push)
	#pragma warning(disable:4201) // nonstandard extension used : nameless struct/union
	#include <MMSystem.h>
	#pragma warning(pop)

#endif

namespace seib {
	namespace vst {
		/*****************************************************************************/
		/* Static Data                                                               */
		/*****************************************************************************/
		bool CFxBase::NeedsBSwap;
		int CVSTHost::quantization = 0x40000000;
		VstTimeInfo CVSTHost::vstTimeInfo;

		CVSTHost * CVSTHost::pHost = NULL;      /* pointer to the one and only host  */

		namespace exceptions
		{
			namespace dispatch_errors
			{
				std::string am_opcode_to_string(long opcode) throw()
				{
					switch(opcode)
					{
						#if defined $
							#error "macro clash"
						#endif
						#define $(code) case audioMaster##code: return "audioMaster"#code;

						// from AEffect.h
						$(Automate)		$(Version)	$(CurrentId)	$(Idle)	$(PinConnected)

						// from aeffectx.h

						$(WantMidi)	$(GetTime)	$(ProcessEvents)$(SetTime)	$(TempoAt)
						
						$(GetNumAutomatableParameters) $(GetParameterQuantization)

						$(IOChanged) $(NeedIdle)

						$(SizeWindow)	$(GetSampleRate)$(GetBlockSize)	$(GetInputLatency)
						$(GetOutputLatency)	$(GetPreviousPlug)	$(GetNextPlug)
						$(WillReplaceOrAccumulate)

						$(GetCurrentProcessLevel)$(GetAutomationState)	$(OfflineStart)
						$(OfflineRead)	$(OfflineWrite)	$(OfflineGetCurrentPass) $(OfflineGetCurrentMetaPass)

						$(SetOutputSampleRate)	$(GetOutputSpeakerArrangement)

						$(GetVendorString) $(GetProductString)	$(GetVendorVersion)		$(VendorSpecific)

						$(SetIcon)	$(CanDo)	$(GetLanguage)		$(OpenWindow)	$(CloseWindow)

						$(GetDirectory)	$(UpdateDisplay)	$(BeginEdit)	$(EndEdit)	$(OpenFileSelector)
						$(CloseFileSelector)	$(EditFile)

						$(GetChunkFile)	$(GetInputSpeakerArrangement)
						#undef $

						default:
						{
							std::ostringstream s;
							s << "unknown opcode " << opcode;
							return s.str();
						}
					}
				}
				const std::string operation_description(long code) throw()
				{
					std::ostringstream s; s << code << ": " << am_opcode_to_string(code);
					return s.str();
				}
			}
		}

		// Data Extracted from audioeffectx.cpp, SDK version 2.4
		//---------------------------------------------------------------------------------------------
		// 'canDo' strings. note other 'canDos' can be evaluated by calling the according
		// function, for instance if getSampleRate returns 0, you
		// will certainly want to assume that this selector is not supported.
		//---------------------------------------------------------------------------------------------

		/*! hostCanDos strings Plug-in -> Host */
		namespace HostCanDos
		{
			const char* canDoSendVstEvents = "sendVstEvents"; ///< Host supports send of Vst events to plug-in
			const char* canDoSendVstMidiEvent = "sendVstMidiEvent"; ///< Host supports send of MIDI events to plug-in
			const char* canDoSendVstTimeInfo = "sendVstTimeInfo"; ///< Host supports send of VstTimeInfo to plug-in
			const char* canDoReceiveVstEvents = "receiveVstEvents"; ///< Host can receive Vst events from plug-in
			const char* canDoReceiveVstMidiEvent = "receiveVstMidiEvent"; ///< Host can receive MIDI events from plug-in 
			const char* canDoReportConnectionChanges = "reportConnectionChanges"; ///< Host will indicates the plug-in when something change in plug-in�s routing/connections with #suspend/#resume/#setSpeakerArrangement 
			const char* canDoAcceptIOChanges = "acceptIOChanges"; ///< Host supports #ioChanged ()
			const char* canDoSizeWindow = "sizeWindow"; ///< used by VSTGUI
			const char* canDoOffline = "offline"; ///< Host supports offline feature
			const char* canDoOpenFileSelector = "openFileSelector"; ///< Host supports function #openFileSelector ()
			const char* canDoCloseFileSelector = "closeFileSelector"; ///< Host supports function #closeFileSelector ()
			const char* canDoStartStopProcess = "startStopProcess"; ///< Host supports functions #startProcess () and #stopProcess ()
			const char* canDoShellCategory = "shellCategory"; ///< 'shell' handling via uniqueID. If supported by the Host and the Plug-in has the category #kPlugCategShell
			const char* canDoSendVstMidiEventFlagIsRealtime = "sendVstMidiEventFlagIsRealtime"; ///< Host supports flags for #VstMidiEvent
		}

		//-------------------------------------------------------------------------------------------------------
		/*! plugCanDos strings Host -> Plug-in */
		namespace PlugCanDos
		{
			const char* canDoSendVstEvents = "sendVstEvents"; ///< plug-in will send Vst events to Host
			const char* canDoSendVstMidiEvent = "sendVstMidiEvent"; ///< plug-in will send MIDI events to Host
			const char* canDoReceiveVstEvents = "receiveVstEvents"; ///< plug-in can receive MIDI events from Host
			const char* canDoReceiveVstMidiEvent = "receiveVstMidiEvent"; ///< plug-in can receive MIDI events from Host 
			const char* canDoReceiveVstTimeInfo = "receiveVstTimeInfo"; ///< plug-in can receive Time info from Host 
			const char* canDoOffline = "offline"; ///< plug-in supports offline functions (#offlineNotify, #offlinePrepare, #offlineRun)
			const char* canDoMidiProgramNames = "midiProgramNames"; ///< plug-in supports function #getMidiProgramName ()
			const char* canDoBypass = "bypass"; ///< plug-in supports function #setBypass ()
		}


		/*===========================================================================*/
		/* CFxBase class members                                                     */
		/*===========================================================================*/

		CFxBase::CFxBase()
		{ CreateInitialized(); }
		void CFxBase::CreateInitialized()
		{
			fxMagic=0; version = 0; fxID = 0; fxVersion = 0; pf = 0;
			const char szChnk[] = "CcnK";
			const long lChnk = cMagic;
			NeedsBSwap = (memcmp(szChnk, &lChnk, 4) != 0);
		}
		CFxBase::CFxBase(VstInt32 _version,VstInt32 _fxID, VstInt32 _fxVersion)
			: fxMagic(0)
			, version(_version)
			, fxID(_fxID)
			, fxVersion(_fxVersion)
			, pf(0)
		{
			const char szChnk[] = "CcnK";
			const long lChnk = cMagic;
			NeedsBSwap = (memcmp(szChnk, &lChnk, 4) != 0);
		}
		CFxBase::CFxBase(const char *pszFile)
		{
			CreateInitialized();
			Load(pszFile);
		}
		CFxBase::CFxBase(FILE* pFileHandle)
		{
			CreateInitialized();
			pf = pFileHandle;
			if (!pf)
				return;
			LoadData();
		}
		CFxBase & CFxBase::DoCopy(const CFxBase &org)
		{
			fxMagic=org.fxMagic;
			version=org.version;
			fxID=org.fxID;
			fxVersion=org.fxVersion;
			return *this;
		}
		void CFxBase::SwapBytes(VstInt32 &l)
		{
			///\todo: could this be improved?
			unsigned char *b = (unsigned char *)&l;
			VstInt32 intermediate =  ((VstInt32)b[0] << 24) |
				((VstInt32)b[1] << 16) |
				((VstInt32)b[2] << 8) |
				(VstInt32)b[3];
			l = intermediate;
		}

		void CFxBase::SwapBytes(float &f)
		{
			VstInt32 *pl = (VstInt32 *)&f;
			SwapBytes(*pl);
		}

		template <class T>
		bool CFxBase::Read(T &f,bool allowswap)	{ int i=fread(&f,sizeof(T),1,pf); if (NeedsBSwap && allowswap) SwapBytes(f); return (bool)i; }
		template <class T>
		bool CFxBase::Write(T f, bool allowswap)	{ if (NeedsBSwap && allowswap) SwapBytes(f); int i=fwrite(&f,sizeof(T),1,pf);  return (bool)i; }
		bool CFxBase::ReadArray(void* f,int size)	{ int i=fread(f,size,1,pf); return (bool)i; }
		bool CFxBase::WriteArray(void* f, int size)	{ int i=fwrite(f,size,1,pf);  return (bool)i; }
		bool CFxBase::ReadHeader()
		{
			VstInt32 chunkMagic(0),byteSize(0);
			Read(chunkMagic);
			if (chunkMagic != cMagic)
				return false;
			Read(byteSize);
			Read(fxMagic);
			Read(version);
			Read(fxID);
			Read(fxVersion);
			return true;
		}
		bool CFxBase::WriteHeader()
		{
			Write(cMagic);
			Write(0);
			Write(fxMagic);
			Write(version);
			Write(fxID);
			Write(fxVersion);
			return true;
		}

		bool CFxBase::Load(const char *pszFile)
		{
			pf = fopen(pszFile, "rb");
			if (!pf)
				return false;

			bool retval = LoadData();
			fclose(pf);
			return retval;
		}
		bool CFxBase::Save(const char *pszFile)
		{
			pf = fopen(pszFile, "wb");
			if (!pf)
				return false;

			bool retval = SaveData();
			fclose(pf);
			return retval;
		}


		/*===========================================================================*/
		/* CFxProgram class members                                                  */
		/*===========================================================================*/

		CFxProgram::CFxProgram(VstInt32 _fxID, VstInt32 _fxVersion, VstInt32 size, bool isChunk, void *data)
			:CFxBase(1,_fxID, _fxVersion)
		{
			Init();
			if (!data)
			{
				//Create an amount of memory predefined
				if (isChunk) SetChunkSize(size);
				else SetNumParams(size);
			}
			else
			{
				if (isChunk) SetChunk(data,size);
				else SetParameters(static_cast<const float*>(data),size);
			}
		}
		CFxProgram::CFxProgram(FILE *pFileHandle)
			:CFxBase()
		{
			Init();
			pf = pFileHandle;
			if (!pf)
				return;
			LoadData();
		}
		
		CFxProgram::~CFxProgram()
		{
			FreeMemory();
		}

		void CFxProgram::FreeMemory()
		{
			if (pChunk)
			{
				delete[] pChunk;
				pChunk = 0;
				chunkSize = 0;
			}
			if (pParams)
			{
				delete[] pParams;
				pParams = 0;
				numParams = 0;
			}
		}

		void CFxProgram::Init()
		{
			numParams = chunkSize = 0;
			pParams = 0;
			pChunk = 0;
			memset(prgName,0,sizeof(prgName));
			ParamMode();
		}

		CFxProgram & CFxProgram::DoCopy(const CFxProgram &org)
		{
			FreeMemory();
			CFxBase::DoCopy(org);
			memcpy(prgName,org.prgName,sizeof(prgName));
			if (org.pChunk)
			{
				SetChunk(org.pChunk,org.chunkSize);
			}
			else
			{
				SetParameters(org.pParams,org.numParams);
			}
			return *this;
		}

		bool CFxProgram::SetParameters(const float* pnewparams,int params)
		{
			if (!SetNumParams(params,false))
				return false;
			memcpy(pParams,pnewparams,params*sizeof(float));
			return true;
		}
		bool CFxProgram::SetNumParams(VstInt32 nPars, bool initializeData)
		{
			if (nPars <=0 )
				return false;

			pParams = new float[nPars];
			numParams = nPars;
			if (pParams && initializeData)
				memset(pParams,0,nPars*sizeof(float));
			return !!pParams;
		}
		bool CFxProgram::SetParameter(int nParm, float val)
		{
			if (nParm >= numParams)
				return false;
			if (val < 0.0)
				val = 0.0;
			else if (val > 1.0)
				val = 1.0;
			pParams[nParm] = val;
			return true;
		}
		bool CFxProgram::SetChunk(const void *chunk, VstInt32 size)
		{
			if (!SetChunkSize(size,false))
				return false;

			memcpy(pChunk,chunk,size);
			return true;
		}

		bool CFxProgram::SetChunkSize(VstInt32 size,bool initializeData)
		{
			if (size <=0 )
				return false;

			pChunk = new unsigned char[size];
			chunkSize = size;
			if ( pChunk && initializeData )
				memset(pChunk,0,size);
			return !!pChunk;
		}

		bool CFxProgram::LoadData()
		{
			CFxBase::LoadData();
			if (fxMagic == fMagic) ParamMode();
			else if ( fxMagic == chunkPresetMagic) ChunkMode();
			else return false;

			Read(numParams);
			ReadArray(prgName,sizeof(prgName));
			if(IsChunk())
			{
				VstInt32 size;
				Read(size);
				if (!SetChunkSize(size,false))
					return false;
				ReadArray(pChunk,size);
			}
			else
			{
				if (!SetNumParams(numParams,false))
					return false;
				for(int i = 0 ; i < numParams ; i++)
					Read(pParams[i]);
			}
			return true;
		}
		bool CFxProgram::SaveData()
		{
			///\todo: update the "bytesize" value!
			CFxBase::SaveData();
			Write(numParams);
			WriteArray(prgName,sizeof(prgName));
			if(IsChunk())
			{
				const int size = GetChunkSize();
				Write(size);
				WriteArray(pChunk,size);
			}
			else
			{
				for (int i = 0; i < numParams ; i++)
					Write(pParams[i]);
			}
			return true;
		}

		/*===========================================================================*/
		/* CFxBank class members                                                     */
		/*===========================================================================*/

		CFxBank::CFxBank(VstInt32 _fxID, VstInt32 _fxVersion, VstInt32 _numPrograms, int _chunkSize, void*_data)
			: CFxBase(2,_fxID, _fxVersion)
			, numPrograms(_numPrograms)
		{
			numPrograms = _numPrograms;
			currentProgram=0;
			if (_chunkSize)
			{
				ChunkMode();
				SetChunk(_data,_chunkSize);
			}
			else
			{
				ProgramMode();
				for ( int i = 0; i < _numPrograms ; i++)
				{
					///\todo : think on a way to use void* _data to pass an array of programs.
				}
			}
		}

		CFxBank::CFxBank(FILE *pFileHandle)
			:CFxBase()
		{
			Init();
			pf = pFileHandle;
			if (!pf)
				return;
			LoadData();
		}

		/*****************************************************************************/
		/* ~CFxBank : destructor                                                     */
		/*****************************************************************************/

		CFxBank::~CFxBank()
		{
			FreeMemory();                               /* unload all data                   */
		}

		/*****************************************************************************/
		/* Init : initializes all data areas                                         */
		/*****************************************************************************/

		void CFxBank::Init()
		{
			pChunk = 0;
			numPrograms = currentProgram = chunkSize = 0;
			programs.clear();
		}


		/*****************************************************************************/
		/* FreMemory : removes a loaded bank from memory                                */
		/*****************************************************************************/

		void CFxBank::FreeMemory()
		{
			if (pChunk)
				delete[] pChunk;
//			*szFileName = '\0';                     /* reset file name                   */
			pChunk = 0;                           /* reset bank pointer                */
			chunkSize = 0;
//			nBankLen = 0;                           /* reset bank length                 */
//			bChunk = false;                         /* and of course it's no chunk.      */
			programs.clear();
			numPrograms = 0;
			currentProgram = 0;
		}

		/*****************************************************************************/
		/* DoCopy : combined for copy constructor and assignment operator            */
		/*****************************************************************************/

		CFxBank & CFxBank::DoCopy(const CFxBank &org)
		{
			FreeMemory();
			CFxBase::DoCopy(org);
			if (org.pChunk)
			{
				SetChunk(org.pChunk,org.chunkSize);
			}
			else
			{
				for (int i=0; i < org.numPrograms; i++)
				{
					CFxProgram newprog(org.programs[i]);
					programs.push_back(newprog);
				}
			}
			numPrograms=org.numPrograms;
			currentProgram=org.currentProgram;
			return *this;
		}

		/*****************************************************************************/
		/* SetChunk / SetChunkSize : sets a new chunk								 */
		/*****************************************************************************/
		bool CFxBank::SetChunk(const void *chunk, VstInt32 size)
		{
			if (!SetChunkSize(size))
				return false;

			memcpy(pChunk,chunk,size);
			return true;
		}

		bool CFxBank::SetChunkSize(VstInt32 size, bool initializeData)
		{
			if (size <=0 )
				return false;

			pChunk = new unsigned char[size];
			chunkSize = size;
			if ( pChunk && initializeData )
				memset(pChunk,0,size);
			return !!pChunk;
		}

		/*****************************************************************************/
		/* LoadBank : loads a bank file                                              */
		/*****************************************************************************/

		bool CFxBank::LoadData()
		{
			CFxBase::LoadData();
			if (fxMagic == bankMagic) ProgramMode();
			else if ( fxMagic == chunkBankMagic) ChunkMode();
			else return false;

			Read(numPrograms);
			if (version == 2) { Read(currentProgram); Forward(124); }
			else { currentProgram = 0; Forward(128); }

			if (fxMagic == chunkBankMagic)
			{
				VstInt32 size;
				Read(size);
				SetChunkSize(size,false);
				ReadArray(pChunk,size);
			}
			else
			{
				for (int i=0; i< numPrograms; i++)
				{
					CFxProgram loadprog(pf);
					programs.push_back(loadprog);
				}
			}
			return true;
		}

		/*****************************************************************************/
		/* SaveBank : save bank to file                                              */
		/*****************************************************************************/

		bool CFxBank::SaveData()
		{
			///\todo: update the "bytesize" value!
			CFxBase::SaveData();
			Write(numPrograms);
			if (version == 2) { Write(currentProgram); Forward(124); }
			else Forward(128);

			if (fxMagic == chunkBankMagic)
			{
				VstInt32 size = chunkSize;
				Write(size);
				WriteArray(pChunk,size);
			}
			else
			{
				for (int i=0; i< numPrograms; i++)
				{
					programs[i].SaveData(pf);
				}
			}
			return true;
		}

		/*===========================================================================*/
		/* CEffect class members                                                     */
		/*===========================================================================*/

		/*****************************************************************************/
		/* CEffect : constructor                                                     */
		/*****************************************************************************/

		CEffect::CEffect(LoadedAEffect &loadstruct)
			: aEffect(0)
			, ploader(0)
//			, sFileName(0)
			, sDir(0)
			, bEditOpen(false)
			, bNeedIdle(false)
			, bWantMidi(false)
		{
			Load(loadstruct);
		}

		/*****************************************************************************/
		/* CEffect : constructor from an AEffect object.                             */
		/*   This constructor is only used in the CVSTHost::AudioMaster callback     */
		/*****************************************************************************/
		CEffect::CEffect(AEffect *effect)
			: ploader(0)
			, sDir(0)
		{
			aEffect=effect;
		}
		/*****************************************************************************/
		/* ~CEffect : destructor                                                     */
		/*****************************************************************************/

		CEffect::~CEffect()
		{
			if  (ploader)	Unload();
		}

		/*****************************************************************************/
		/* Load : loads (sets up) the effect module                                  */
		/*****************************************************************************/

		void CEffect::Load(LoadedAEffect &loadstruct)
		{
			aEffect=loadstruct.aEffect;
			ploader=loadstruct.pluginloader;

		#ifdef WIN32
			char const * name = (char*)(loadstruct.pluginloader->sFileName);
			char const * const p = strrchr(name, '\\');
			if (p)
			{
				sDir = new char[p - name + 1];
				if (sDir)
				{
					memcpy(sDir, name, p - name);
					((char*)sDir)[p - name] = '\0';
				}
			}
			else { sDir = new char[1]; ((char*)sDir)[0]='\0'; }

		#elif MAC
			// yet to be done
		#endif

			// The trick, store the CEffect's class instance so that the host can talk to us.
			// I am unsure what other hosts use for resvd1 and resvd2
			aEffect->resvd1 = ToVstPtr(this);

			SetSampleRate(CVSTHost::pHost->GetSampleRate()); /* adjust its sample rate            */
			SetBlockSize(CVSTHost::pHost->GetBlockSize());
			Open();                     /* open the effect                   */
			//6 :        Host to Plug, canDo ( bypass )   returned : 0
			//7 :        Host to Plug, setPanLaw ( 0 , 0.707107 )   returned : false 

		}

		/*****************************************************************************/
		/* Unload : unloads (frees) effect module                                    */
		/*****************************************************************************/

		void CEffect::Unload()
		{
			MainsChanged(false);
			Close();                             /* make sure it's closed             */
			aEffect = NULL;                         /* and reset the pointer             */
			delete ploader;

		#ifdef WIN32
			if (sDir)                               /* reset directory            */
			{
				delete[] sDir;	sDir = NULL;
			}
		#elif MAC
			// yet to be done!
		#endif
		}

		/*****************************************************************************/
		/* LoadBank : loads a .fxb file ... IF it's for this effect                  */
		/*****************************************************************************/

		bool CEffect::LoadBank(const char *name)
		{
			//BeginLoadBank()//EndLoadBank
			return false;                         /* return NOT!!!                     */

		}

		/*****************************************************************************/
		/* SaveBank : saves current sound bank to a .fxb file                        */
		/*****************************************************************************/

		bool CEffect::SaveBank(const char *name)
		{
			return false;                           /* return error for now              */
		}

		/*****************************************************************************/
		/* EffDispatch : calls an effect's dispatcher                                */
		/*****************************************************************************/

		VstIntPtr CEffect::Dispatch(VstInt32 opCode, VstInt32 index, VstIntPtr value, void* ptr, float opt)
		{
			if (!aEffect)
				throw (int)1;

			return aEffect->dispatcher(aEffect, opCode, index, value, ptr, opt);
		}

		/*****************************************************************************/
		/* EffProcess : calls an effect's process() function                        */
		/*****************************************************************************/

		void CEffect::Process(float **inputs, float **outputs, VstInt32 sampleframes)
		{
			if (!aEffect)
				throw (int)1;

			aEffect->process(aEffect, inputs, outputs, sampleframes);
		}

		/*****************************************************************************/
		/* EffProcessReplacing : calls an effect's processReplacing() function       */
		/*****************************************************************************/

		void CEffect::ProcessReplacing(float **inputs, float **outputs, VstInt32 sampleframes)
		{
			if ((!aEffect) ||
				(!(aEffect->flags & effFlagsCanReplacing)))
				throw (int)1;

			aEffect->processReplacing(aEffect, inputs, outputs, sampleframes);
		}

		void CEffect::ProcessDouble(double **inputs, double **outputs, VstInt32 sampleframes)
		{
			if (!aEffect)
				throw (int)1;

			aEffect->processDoubleReplacing(aEffect, inputs, outputs, sampleframes);
		}

		/*****************************************************************************/
		/* EffSetParameter : calls an effect's setParameter() function               */
		/*****************************************************************************/

		void CEffect::SetParameter(VstInt32 index, float parameter)
		{
			if (!aEffect)
				throw (int)1;

			aEffect->setParameter(aEffect, index, parameter);
		}

		/*****************************************************************************/
		/* EffGetParameter : calls an effect's getParameter() function               */
		/*****************************************************************************/

		float CEffect::GetParameter(VstInt32 index)
		{
			if (!aEffect)
				throw (int)1;

			return aEffect->getParameter(aEffect, index);
		}

		/*****************************************************************************/
		/* OnGetDirectory : returns the plug's directory (char* on pc, FSSpec on mac)*/
		/*****************************************************************************/

		void * CEffect::OnGetDirectory()
		{
			return sDir;
		}



		/*===========================================================================*/
		/* CVSTHost class members                                                    */
		/*===========================================================================*/

		/*****************************************************************************/
		/* CVSTHost : constructor                                                    */
		/*****************************************************************************/

		CVSTHost::CVSTHost()
		{
			if (pHost)                              /* disallow more than one host!      */
				throw((int)1);

			lBlockSize = 1024;
			vstTimeInfo.samplePos = 0.0;
			vstTimeInfo.sampleRate = 44100.;
			vstTimeInfo.nanoSeconds = 0.0;
			vstTimeInfo.ppqPos = 0.0;
			vstTimeInfo.tempo = 120.0;
			vstTimeInfo.barStartPos = 0.0;
			vstTimeInfo.cycleStartPos = 0.0;
			vstTimeInfo.cycleEndPos = 0.0;
			vstTimeInfo.timeSigNumerator = 4;
			vstTimeInfo.timeSigDenominator = 4;
			vstTimeInfo.smpteOffset = 0;
			vstTimeInfo.smpteFrameRate = 1;
			vstTimeInfo.samplesToNextClock = 0;
			vstTimeInfo.flags = 0;

			loadingShellId = 0;
			loadingEffect = false;
			pHost = this;                           /* install this instance as the one  */
		}

		/*****************************************************************************/
		/* ~CVSTHost : destructor                                                    */
		/*****************************************************************************/

		CVSTHost::~CVSTHost()
		{
			if (pHost == this)                      /* if we are the chosen one           */
				pHost = NULL;                         /* remove ourselves from pointer     */
		}

		/*****************************************************************************/
		/* LoadPlugin : loads and initializes a plugin                               */
		/*****************************************************************************/

		CEffect* CVSTHost::LoadPlugin(const char * sName,VstInt32 shellIdx)
		{
			PluginLoader* loader = new PluginLoader;
			if (!loader->loadLibrary(sName))
			{
				delete loader;
				std::ostringstream s; s
					<< "Couldn't open the library: " << sName << std::endl;
				throw psycle::host::exceptions::library_errors::loading_error(s.str());
			}

			PluginEntryProc mainEntry = loader->getMainEntry ();
			if(!mainEntry)
			{
				delete loader;
				std::ostringstream s; s
					<< "couldn't locate the main entry to VST: " << sName << std::endl;
					throw psycle::host::exceptions::library_errors::loading_error(s.str());
			}

			loadingEffect = true;
			loadingShellId = shellIdx;
			AEffect* effect = mainEntry (AudioMasterCallback);
			if (effect && (effect->magic != kEffectMagic))
			{
				delete effect;
				effect = NULL;
			}
			if (effect)
			{
				LoadedAEffect loadstruct;
				loadstruct.aEffect = effect;
				loadstruct.pluginloader = loader;
				CEffect*neweffect = CreateEffect(loadstruct);
				loadingEffect=false;
				return neweffect;
			}
			delete loader;
			std::ostringstream s; s
				<< "VST main call returned a null/wrong AEffect: " << sName << std::endl;
			throw psycle::host::exceptions::library_errors::loading_error(s.str());
		}


		/*****************************************************************************/
		/* CalcTimeInfo : calculates time info from nanoSeconds                      */
		/*****************************************************************************/

		void CVSTHost::CalcTimeInfo(long lMask)
		{
			// Either your player/sequencer or your overloaded member should update the following ones.
			// They shouldn't need any calculations appart from your usual work procedures.
			//sampleRate			(Via the SetSampeRate() function )
			//samplePos
			//tempo
			//cyclestart			// locator positions in quarter notes.
			//cycleend				// locator positions in quarter notes.
			//timeSigNumerator		} Via SetTimeSignature() function
			//timeSigDenominator	} ""	""
			//smpteFrameRate		(See VstSmpteFrameRate in aeffectx.h)

			//ppqPos	(sample pos in 1ppq units)
			const double seconds = vstTimeInfo.samplePos / vstTimeInfo.sampleRate;
			if((lMask & kVstPpqPosValid) || (lMask & kVstBarsValid) || (lMask && kVstClockValid))
			{
				vstTimeInfo.flags |= kVstPpqPosValid;
				vstTimeInfo.ppqPos = seconds * vstTimeInfo.tempo / 60.L;

				//barstartpos,  ( 10.25ppq , 1ppq = 1 beat). ppq pos of the previous bar. (ppqpos/sigdenominator ?)
				if(lMask & kVstBarsValid)
				{
					vstTimeInfo.barStartPos= vstTimeInfo.timeSigDenominator* (int)vstTimeInfo.ppqPos / (int)vstTimeInfo.timeSigDenominator;
					vstTimeInfo.flags |= kVstBarsValid;
				}
				//samplestoNextClock, how many samples from the current position to the next 24ppq.  ( i.e. 1/24 beat ) (actually, to the nearest. previous-> negative value)
				if(lMask & kVstClockValid)
				{
//					option 1:
					const double onesampleclock = (60.L * vstTimeInfo.sampleRate) / (vstTimeInfo.tempo*24.L);		// get size of one 24ppq in samples.
					vstTimeInfo.samplesToNextClock = onesampleclock * (((int)vstTimeInfo.samplePos / (int)onesampleclock)+1); // quantize.

//					option 2:
//					const double ppqclockpos = 24 * (((int)vstTimeInfo.ppqPos / 24)+1);								// Quantize ppqpos
//					const double sampleclockpos = ppqclockpos * 60.L * vstTimeInfo.sampleRate / vstTimeInfo.tempo;	// convert to samples
//					vstTimeInfo.samplestoNextClock = sampleclockpos - ppqclockpos;									// get the difference.
					vstTimeInfo.flags |= kVstBarsValid;
				}
			}
			//smpteOffset
			if(lMask & kVstSmpteValid)
			{
				//	24 fps ,  25 fps,	29.97 fps,	30 fps,	29.97 drop, 30 drop , Film 16mm ,  Film 35mm , none, none,
				//	HDTV: 23.976 fps,	HDTV: 24.976 fps,	HDTV: 59.94 fps,	HDTV: 60 fps
				static double fSmpteDiv[] =
				{	24.f,		25.f,		29.97f,		30.f,	29.97f,		30.f ,		0.f,		0.f,	0.f,	0.f,
					23.976f,	24.976f,	59.94f,		60.f
				};
				double dOffsetInSecond = seconds - floor(seconds);
				vstTimeInfo.smpteOffset = (long)(dOffsetInSecond *
					fSmpteDiv[vstTimeInfo.smpteFrameRate] *
					80.L);
				vstTimeInfo.flags |= kVstSmpteValid;
			}
			//nanoseconds (system time)
			if(lMask & kVstNanosValid)
			{
			#ifdef WIN32
				vstTimeInfo.nanoSeconds = timeGetTime();
				vstTimeInfo.flags |= kVstNanosValid;
			#else
				//add the appropiate code.
			#endif
			}
		}

		/*****************************************************************************/
		/* SetSampleRate : sets sample rate                                          */
		/*****************************************************************************/

		void CVSTHost::SetSampleRate(float fSampleRate)
		{
			//\todo : inform of the change? ( kVstTranportChanged )
			if (fSampleRate == vstTimeInfo.sampleRate)   /* if no change                      */
				return;                               /* do nothing.                       */
			vstTimeInfo.sampleRate = fSampleRate;
		}

		/*****************************************************************************/
		/* SetBlockSize : sets the block size                                        */
		/*****************************************************************************/

		void CVSTHost::SetBlockSize(long lSize)
		{
			//\todo : inform of the change? ( kVstTranportChanged )
			if (lSize == lBlockSize)                /* if no change                      */
				return;                               /* do nothing.                       */
			lBlockSize = lSize;                     /* remember new block size           */
		}

		/*****************************************************************************/
		/* AudioMasterCallback : callback to be called by plugins                    */
		/*****************************************************************************/
		VstIntPtr CVSTHost::AudioMasterCallback
		(
		AEffect* effect,
		VstInt32 opcode,
		VstInt32 index,
		VstIntPtr value,
		void* ptr,
		float opt
		)
		{
			if (!pHost)
				throw (int)1;	// It would have no sense that there is no host.

			CEffect *pEffect=0;
			bool fakeeffect=false;

			if (pHost->loadingEffect)
			{
				fakeeffect = true;
				pEffect = pHost->CreateEffect(effect);
			}
			else
			{
				if ( !effect )
				{
					std::stringstream s; s
						<< "AudioMaster call with unknown AEffect (this is a bad behaviour from a plugin)" << std::endl
						<< "opcode is " << exceptions::dispatch_errors::operation_description(opcode)
						<< " with index: " << index << ", value: " << value << ", and opt:" << opt << std::endl;
					std::stringstream title; title
						<< "Machine Error: ";
					psycle::host::loggers::info(title.str() + '\n' + s.str());
					
					// As bad behaviour as this is, we try to simulate a pEffect plugin for this call, so that at least calls to
					// GetProductString and similar can be answered.
					// CEffect will throw an exception if the function requires a callback to the plugin.
					fakeeffect=true;
					pEffect = pHost->CreateEffect(0);
				}
				else pEffect = (CEffect*)(effect->resvd1);

				if ( !pEffect ) 
				{
					char name[5]={0};
					//bool read=false;
					memcpy(name,&(effect->uniqueID),4);
					name[4]='\0';

/*					if ( effect->uniqueID != CCONST("NoEf")) // How good is a plugin that calls an audiomaster function before identifying itself?
					{
						read =effect->dispatcher(effect,effGetEffectName,0,0,name,0);
						if (!read) read = effect->dispatcher(effect,effGetProductString,0,0,name,0);
						if (!read) memset(name,0,sizeof(name));
					}
					else strcpy(name,"Unknown Effect");
*/
					std::stringstream s; s
						<< "AudioMaster call, with unknown pEffect (most probably, a call from the plugin constructor)" << std::endl
						<< "Aeffect: " << name << "opcode is " << exceptions::dispatch_errors::operation_description(opcode)
						<< " with index: " << index << ", value: " << value << ", and opt:" << opt << std::endl;
					std::stringstream title; title
						<< "Machine Error: ";
					psycle::host::loggers::info(title.str() + '\n' + s.str());
					// The VST SDK 2.0 said this:
					// [QUOTE]
					//	Whenever the Host instanciates a plug-in, after the main() call, it also immediately informs the
					//	plug-in about important system parameters like sample rate, and sample block size. Because the
					//	audio effect object is constructed in our plug-in�s main(), before the host gets any information about
					//	the created object, you need to be careful what functions are called within the constructor of the
					//	plug-in. You may be talking but no-one is listening.
					// [/QUOTE]
					// The truth is that most plugins call different audioMaster operations, and some even disallowed operations (WantEvents!),
					// so this tries to alleviate the- problem, as much as it can.
					
					fakeeffect=true;
					pEffect= pHost->CreateEffect(effect);
				}
			}

			VstIntPtr result;
			switch (opcode)
			{
			case audioMasterAutomate :
				pHost->OnSetParameterAutomated(*pEffect, index, opt);
				break;
			case audioMasterVersion :
				return pHost->OnGetVSTVersion();
			case audioMasterCurrentId :
				if (pHost->loadingEffect) result = pHost->loadingShellId;
				else result = pHost->OnCurrentId(*pEffect);
				if (fakeeffect )delete pEffect;
				return result;
			case audioMasterIdle :
				pHost->OnIdle(*pEffect);
				break;
			case audioMasterPinConnected :
				result = !((value) ? 
					pHost->OnOutputConnected(*pEffect, index) :
					pHost->OnInputConnected(*pEffect, index));
				if (fakeeffect )delete pEffect;
				return result;
		// VST 2.0
			case audioMasterWantMidi :
				pHost->OnWantEvents(*pEffect, value);
				break;
			case audioMasterGetTime :
				result = reinterpret_cast<VstIntPtr>(pHost->OnGetTime(*pEffect, value));
				if (fakeeffect )delete pEffect;
				return result;
			case audioMasterProcessEvents :
				result = pHost->OnProcessEvents(*pEffect, (VstEvents *)ptr);
				if (fakeeffect )delete pEffect;
				return result;
			case audioMasterSetTime :
				result = pHost->OnSetTime(*pEffect, value, (VstTimeInfo *)ptr);
				if (fakeeffect )delete pEffect;
				return result;
			case audioMasterTempoAt :
				result = pHost->OnTempoAt(*pEffect, value);
				if (fakeeffect )delete pEffect;
				return result;
			case audioMasterGetNumAutomatableParameters :
				result = pHost->OnGetNumAutomatableParameters(*pEffect);
				if (fakeeffect )delete pEffect;
				return result;
			case audioMasterGetParameterQuantization :
				result = pHost->OnGetParameterQuantization(*pEffect);
				if (fakeeffect )delete pEffect;
				return result;
			case audioMasterIOChanged :
				result = pHost->OnIoChanged(*pEffect);
				if (fakeeffect )delete pEffect;
				return result;
			case audioMasterNeedIdle :
				result = pHost->OnNeedIdle(*pEffect);
				if (fakeeffect )delete pEffect;
				return result;
			case audioMasterSizeWindow :
				result = pHost->OnSizeWindow(*pEffect, index, value);
				if (fakeeffect )delete pEffect;
				return result;
			case audioMasterGetSampleRate :
				result = pHost->OnUpdateSampleRate(*pEffect);
				if (fakeeffect )delete pEffect;
				return result;
			case audioMasterGetBlockSize :
				result = pHost->OnUpdateBlockSize(*pEffect);
				if (fakeeffect )delete pEffect;
				return result;
			case audioMasterGetInputLatency :
				result = pHost->OnGetInputLatency(*pEffect);
				if (fakeeffect )delete pEffect;
				return result;
			case audioMasterGetOutputLatency :
				result = pHost->OnGetOutputLatency(*pEffect);
				if (fakeeffect )delete pEffect;
				return result;
			case audioMasterGetPreviousPlug :
				result = reinterpret_cast<VstIntPtr>(pHost->GetPreviousPlugIn(*pEffect,index)->GetAEffect());
				if (fakeeffect )delete pEffect;
				return result;
			case audioMasterGetNextPlug :
				result = reinterpret_cast<VstIntPtr>(pHost->GetNextPlugIn(*pEffect,index)->GetAEffect());
				if (fakeeffect )delete pEffect;
				return result;
			case audioMasterWillReplaceOrAccumulate :
				result = pHost->OnWillProcessReplacing(*pEffect);
				if (fakeeffect )delete pEffect;
				return result;
			case audioMasterGetCurrentProcessLevel :
				result = pHost->OnGetCurrentProcessLevel(*pEffect);
				if (fakeeffect )delete pEffect;
				return result;
			case audioMasterGetAutomationState :
				result = pHost->OnGetAutomationState(*pEffect);
				if (fakeeffect )delete pEffect;
				return result;
			case audioMasterOfflineStart :
				result = pHost->OnOfflineStart(*pEffect,	(VstAudioFile *)ptr,value,index);
				if (fakeeffect )delete pEffect;
				return result;
			case audioMasterOfflineRead :
				result = pHost->OnOfflineRead(*pEffect,(VstOfflineTask *)ptr,(VstOfflineOption)value,!!index);
				if (fakeeffect )delete pEffect;
				return result;
			case audioMasterOfflineWrite :
				result = pHost->OnOfflineWrite(*pEffect,(VstOfflineTask *)ptr,(VstOfflineOption)value);
				if (fakeeffect )delete pEffect;
				return result;
			case audioMasterOfflineGetCurrentPass :
				result = pHost->OnOfflineGetCurrentPass(*pEffect);
				if (fakeeffect )delete pEffect;
				return result;
			case audioMasterOfflineGetCurrentMetaPass :
				result = pHost->OnOfflineGetCurrentMetaPass(*pEffect);
				if (fakeeffect )delete pEffect;
				return result;
			case audioMasterSetOutputSampleRate :
				pHost->OnSetOutputSampleRate(*pEffect, opt);
				break;
			case audioMasterGetOutputSpeakerArrangement :
				result = reinterpret_cast<VstIntPtr>(pHost->OnGetOutputSpeakerArrangement(*pEffect));
				if (fakeeffect )delete pEffect;
				return result;
			case audioMasterGetVendorString :
				result = pHost->OnGetVendorString((char *)ptr);
				if (fakeeffect )delete pEffect;
				return result;
			case audioMasterGetProductString :
				result = pHost->OnGetProductString((char *)ptr);
				if (fakeeffect )delete pEffect;
				return result;
			case audioMasterGetVendorVersion :
				result = pHost->OnGetHostVendorVersion();
				if (fakeeffect )delete pEffect;
				return result;
			case audioMasterVendorSpecific :
				result = pHost->OnHostVendorSpecific(*pEffect, index, value, ptr, opt);
				if (fakeeffect )delete pEffect;
				return result;
			case audioMasterSetIcon :
				// undefined in VST 2.0 specification. Deprecated in v2.4
				break;
			case audioMasterCanDo :
				result = pHost->OnCanDo(*pEffect,(const char *)ptr);
				if (fakeeffect )delete pEffect;
				return result;
			case audioMasterGetLanguage :
				result = pHost->OnGetHostLanguage();
				if (fakeeffect )delete pEffect;
				return result;
			case audioMasterOpenWindow :
				result = (long)pHost->OnOpenWindow(*pEffect, (VstWindow *)ptr);
				if (fakeeffect )delete pEffect;
				return result;
			case audioMasterCloseWindow :
				result = pHost->OnCloseWindow(*pEffect, (VstWindow *)ptr);
				if (fakeeffect )delete pEffect;
				return result;
			case audioMasterGetDirectory :
				result = reinterpret_cast<VstIntPtr>(pHost->OnGetDirectory(*pEffect));
				if (fakeeffect )delete pEffect;
				return result;
			case audioMasterUpdateDisplay :
				result = pHost->OnUpdateDisplay(*pEffect);
				if (fakeeffect )delete pEffect;
				return result;
		// VST 2.1
		#ifdef VST_2_1_EXTENSIONS
			case audioMasterBeginEdit :
				result = pHost->OnBeginEdit(*pEffect,index);
				if (fakeeffect )delete pEffect;
				return result;
			case audioMasterEndEdit :
				result = pHost->OnEndEdit(*pEffect,index);
				if (fakeeffect )delete pEffect;
				return result;
			case audioMasterOpenFileSelector :
				result = pHost->OnOpenFileSelector(*pEffect, (VstFileSelect *)ptr);
				if (fakeeffect )delete pEffect;
				return result;
		#endif
		// VST 2.2
		#ifdef VST_2_2_EXTENSIONS
			case audioMasterCloseFileSelector :
				result = pHost->OnCloseFileSelector(*pEffect, (VstFileSelect *)ptr);
				if (fakeeffect )delete pEffect;
				return result;
			case audioMasterEditFile :
				result = pHost->OnEditFile(*pEffect, (char *)ptr);
				if (fakeeffect )delete pEffect;
				return result;
			case audioMasterGetChunkFile :
				result = pHost->OnGetChunkFile(*pEffect, ptr);
				if (fakeeffect )delete pEffect;
				return result;
		#endif
		// VST 2.3
		#ifdef VST_2_3_EXTENSIONS
			case audioMasterGetInputSpeakerArrangement :
				result = reinterpret_cast<VstIntPtr>(pHost->OnGetInputSpeakerArrangement(*pEffect));
				if (fakeeffect )delete pEffect;
				return result;
		#endif

			}
			if (fakeeffect )delete pEffect;
			return 0L;
		}




		/*****************************************************************************/
		/* OnCanDo : returns whether the host can do a specific action               */
		/*****************************************************************************/

		bool CVSTHost::OnCanDo(CEffect &pEffect, const char *ptr)
		{
			using namespace HostCanDos;
			// For the host, according to audioeffectx.cpp , "!= 0 -> true", so there isn't "-1 : can't do".
			if (	(!strcmp(ptr, canDoSendVstEvents )) // "sendVstEvents"
				||	(!strcmp(ptr, canDoSendVstMidiEvent )) // "sendVstMidiEvent"
				||	(!strcmp(ptr, canDoSendVstTimeInfo )) // "sendVstTimeInfo",
				//||	(!strcmp(ptr, hostCanDos[3] )) // "receiveVstEvents",
				//||	(!strcmp(ptr, hostCanDos[4] )) // "receiveVstMidiEvent",
				//||	(!strcmp(ptr, hostCanDos[5] )) // "receiveVstTimeInfo",

				//||	(!strcmp(ptr, hostCanDos[6] )) // "reportConnectionChanges",
				//||	(!strcmp(ptr, hostCanDos[7] )) // "acceptIOChanges",
				//||	(!strcmp(ptr, hostCanDos[8] )) // "sizeWindow",

				//||	(!strcmp(ptr, hostCanDos[9] )) // "asyncProcessing",
				//||	(!strcmp(ptr, hostCanDos[10] )) // "offline",
				//||	(!strcmp(ptr, hostCanDos[11] )) // "supplyIdle",
				//||	(!strcmp(ptr, hostCanDos[12] )) // "supportShell",
				//||	(!strcmp(ptr, hostCanDos[13] )) // "openFileSelector"
				//||	(!strcmp(ptr, hostCanDos[14] )) // "editFile",
				//||	(!strcmp(ptr, hostCanDos[15] )) // "closeFileSelector"
				//||	(!strcmp(ptr, hostCanDos[16] )) // "startStopProcess"
				)
				return true;
			return false;                           /* per default, no.                  */
		}


		/*****************************************************************************/
		/* OnWantEvents : called when the effect calls wantEvents()                  */
		/*****************************************************************************/
		// Generally, this is called on resume to indicate that the plugin is going to accept events.
		void CVSTHost::OnWantEvents(CEffect & pEffect, long filter)
		{
			if ( filter == kVstMidiType )
			{
				pEffect.WantsMidi(true);
//				return true;
			}
//			return false;
		}

		/*****************************************************************************/
		/* OnIdle : idle processing                                                  */
		/*****************************************************************************/
		// Call application idle routine (this will call effEditIdle for all open editors too) 
		void CVSTHost::OnIdle(CEffect & pEffect)
		{
//			int j = GetSize();
//			for (int i = 0; i < j; i++)
//				pEffect.EditIdle();
//			return 0;
		}

		/*****************************************************************************/
		/* OnNeedIdle : called when the effect calls needIdle()                      */
		/*****************************************************************************/
		// Ideally, this would only send a message to the "Idle" process to execute it.
		// host::OnIdle and plug::EditIdle are 1.0 functions and host::needIdle and plug::idle are 2.0
		// Seems that host::OnIdle is called for when it is required, and host::needidle for a permanent loop.
		bool CVSTHost::OnNeedIdle(CEffect & pEffect)
		{
			pEffect.NeedsIdle(true);
			pEffect.Idle();
			return true;
		}

		/*****************************************************************************/
		/* OnGetDirectory : called when the effect calls getDirectory()              */
		/*****************************************************************************/

		void * CVSTHost::OnGetDirectory(CEffect & pEffect)
		{
			return pEffect.OnGetDirectory();
		}

		/*****************************************************************************/
		/* OnOpenWindow : called to open a new window                                */
		/*****************************************************************************/

		void * CVSTHost::OnOpenWindow(CEffect & pEffect, VstWindow* window)
		{
			return pEffect.OnOpenWindow(window);
		}

		/*****************************************************************************/
		/* OnCloseWindow : called to close a window                                  */
		/*****************************************************************************/

		bool CVSTHost::OnCloseWindow(CEffect & pEffect, VstWindow* window)
		{
			return pEffect.OnCloseWindow(window);
		}

		/*****************************************************************************/
		/* OnSizeWindow : called when the effect calls sizeWindow()                  */
		/*****************************************************************************/

		bool CVSTHost::OnSizeWindow(CEffect & pEffect, long width, long height)
		{
			return pEffect.OnSizeEditorWindow(width, height);
		}


		/*****************************************************************************/
		/* OnUpdateDisplay : called when effect calls updateDisplay()                */
		/*****************************************************************************/

		bool CVSTHost::OnUpdateDisplay(CEffect & pEffect)
		{
			return pEffect.OnUpdateDisplay();
		}

		/*****************************************************************************/
		/* OnGetVersion : returns the VST Host VST Version                           */
		/*****************************************************************************/

		long CVSTHost::OnGetVSTVersion()
		{
			return kVstVersion;
		}
	}
}
