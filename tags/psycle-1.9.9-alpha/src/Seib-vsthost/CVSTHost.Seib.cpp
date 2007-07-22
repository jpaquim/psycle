/*****************************************************************************/
/* CVSTHost.cpp: implementation of the CVSTHost class.                       */
/* Work Derived from vsthost. Copyright (c) H. Seib, 2002-2005               */
/* (http://www.hermannseib.com/english/vsthost.htm)"						 */
/* Please, read file src/Seib-vsthost/readme.txt before using these sources	 */
/*****************************************************************************/
/*                                                                           */
/* $Revision$ */
/* $Date$ */
/* $Author$ */
/*                                                                           */
/*****************************************************************************/

#include <packageneric/pre-compiled.private.hpp>
#include "CVSTHost.Seib.hpp"                   /* private prototypes                */

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
		int CFxBase::FxSetVersion = 1;                /* highest known VST FX version      */
		int CVSTHost::quantization = 0x40000000;
		VstTimeInfo CVSTHost::vstTimeInfo;

		// Data Extracted from AudioEffectx.cpp, SDK version 2.3
		//---------------------------------------------------------------------------------------------
		// 'canDo' strings. note other 'canDos' can be evaluated by calling the according
		// function, for instance if getSampleRate returns 0, you
		// will certainly want to assume that this selector is not supported.
		//---------------------------------------------------------------------------------------------

		const char* hostCanDos [] =
		{
			"sendVstEvents",
			"sendVstMidiEvent",
			"sendVstTimeInfo",
			"receiveVstEvents",
			"receiveVstMidiEvent",
			"receiveVstTimeInfo",

			"reportConnectionChanges",
			"acceptIOChanges",
			"sizeWindow",

			"asyncProcessing",
			"offline",
			"supplyIdle",
			"supportShell",		// 'shell' handling via uniqueID as suggested by Waves
			"openFileSelector"
		#if VST_2_2_EXTENSIONS
			,
			"editFile",
			"closeFileSelector"
		#endif // VST_2_2_EXTENSIONS
		#if VST_2_3_EXTENSIONS
			,
			"startStopProcess"
		#endif // VST_2_3_EXTENSIONS
		};

		const char* plugCanDos [] =
		{
			"sendVstEvents",
			"sendVstMidiEvent",
			"sendVstTimeInfo",
			"receiveVstEvents",
			"receiveVstMidiEvent",
			"receiveVstTimeInfo",
			"offline",
			"plugAsChannelInsert",
			"plugAsSend",
			"mixDryWet",
			"noRealTime",
			"multipass",
			"metapass",
			"1in1out",
			"1in2out",
			"2in1out",
			"2in2out",
			"2in4out",
			"4in2out",
			"4in4out",
			"4in8out",	// 4:2 matrix to surround bus
			"8in4out",	// surround bus to 4:2 matrix
			"8in8out"
		#if VST_2_1_EXTENSIONS
			,
			"midiProgramNames",
			"conformsToWindowRules"		// mac: doesn't mess with grafport. general: may want
			// to call sizeWindow (). if you want to use sizeWindow (),
			// you must return true (1) in canDo ("conformsToWindowRules")
		#endif // VST_2_1_EXTENSIONS
		#if VST_2_3_EXTENSIONS
			,
			"bypass"
		#endif // VST_2_3_EXTENSIONS
		};

		/*****************************************************************************/
		/* SwapBytes : swaps bytes for big/little-endian difference                  */
		/*****************************************************************************/

		CFxBase::CFxBase()
		{
			const char szChnk[] = "CcnK";
			const long lChnk = cMagic;
			NeedsBSwap = (memcmp(szChnk, &lChnk, 4) != 0);
		}
		void CFxBase::SwapBytes(long &l)
		{	//\todo : could this be improved?
			unsigned char *b = (unsigned char *)&l;
			long intermediate =  ((long)b[0] << 24) |
				((long)b[1] << 16) |
				((long)b[2] << 8) |
				(long)b[3];
			l = intermediate;
		}
		void CFxBase::SwapBytes(VstInt32 &l)
		{
			long *pl = (long*)&l;
			SwapBytes(*pl);
		}

		void CFxBase::SwapBytes(float &f)
		{
			long *pl = (long *)&f;
			SwapBytes(*pl);
		}

		/*===========================================================================*/
		/* CFxProgram class members                                                  */
		/*===========================================================================*/

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
			}
			if (pParams)
			{
				delete[] pParams;
				pParams = 0;
			}
		}

		void CFxProgram::Init()
		{
			pChunk =0; pParams=0; lChunkSize=0;
			memset(&program,0,sizeof(program));
			szFileName[0] = '\0';
			program.version = 1;
		}

		/*****************************************************************************/
		/* DoCopy : combined for copy constructor and assignment operator            */
		/*****************************************************************************/

		CFxProgram & CFxProgram::DoCopy(const CFxProgram &org)
		{
			FreeMemory();

			program = org.program;  
			if (org.pChunk)
			{
				SetChunkSize(org.lChunkSize);
				memcpy(pChunk, org.pChunk, lChunkSize);
			}
			else
			{
				if (!org.pParams || program.numParams <=0)
					throw (int)1;
				SetParamSize(program.numParams);
				memcpy(pParams,org.pParams,program.numParams*sizeof(float));
			}
			strcpy(szFileName, org.szFileName);
			return *this;
		}
		bool CFxProgram::SetParamSize(int nParams)
		{
			if (pParams)
				delete[] pParams;
			float *newpars = new float[nParams];
			if (!newpars)
				return false;
			pParams=newpars;
			program.numParams = nParams;
			return true;
		}
		bool CFxProgram::SetChunkSize(int nChunkSize)
		{
			if (pChunk)
				delete[] pChunk;
			unsigned char *newchunk = new unsigned char[nChunkSize];
			if (!newchunk)
				return false;
			pChunk=newchunk;
			lChunkSize=nChunkSize;
			return true;
		}

		bool CFxProgram::SetChunk(void *chunk)
		{
			if ( !pChunk )
				return false;

			memcpy(pChunk, chunk, lChunkSize); return true;
		}
		bool CFxProgram::Load(const char *pszFile) { return false; }
		bool CFxProgram::Save(const char *pszFile) { return false; }

		/*===========================================================================*/
		/* CFxBank class members                                                     */
		/*===========================================================================*/

		/*****************************************************************************/
		/* CFxBank : constructor                                                     */
		/*****************************************************************************/

		CFxBank::CFxBank(const char *pszFile)
		{
			Init();                                 /* initialize data areas             */
			if (pszFile)                            /* if a file name has been passed    */
				Load(pszFile);                    /* load the corresponding bank       */
		}

		CFxBank::CFxBank(int nPrograms, int nParams)
		{
			Init();                                 /* initialize data areas             */
			SetSize(nPrograms, nParams);            /* set new size                      */
		}

		CFxBank::CFxBank(int nChunkSize)
		{
			Init();                                 /* initialize data areas             */
			SetSize(nChunkSize);                    /* set new size                      */
		}

		/*****************************************************************************/
		/* ~CFxBank : destructor                                                     */
		/*****************************************************************************/

		CFxBank::~CFxBank()
		{
			Unload();                               /* unload all data                   */
		}

		/*****************************************************************************/
		/* Init : initializes all data areas                                         */
		/*****************************************************************************/

		void CFxBank::Init()
		{
			bBank = NULL;                           /* no bank data loaded               */
			Unload();                               /* reset all parameters              */
		}


		/*****************************************************************************/
		/* Unload : removes a loaded bank from memory                                */
		/*****************************************************************************/

		void CFxBank::Unload()
		{
			if (bBank)
				delete[] bBank;
			*szFileName = '\0';                     /* reset file name                   */
			bBank = NULL;                           /* reset bank pointer                */
			nBankLen = 0;                           /* reset bank length                 */
			bChunk = false;                         /* and of course it's no chunk.      */
		}

		/*****************************************************************************/
		/* DoCopy : combined for copy constructor and assignment operator            */
		/*****************************************************************************/

		CFxBank & CFxBank::DoCopy(const CFxBank &org)
		{
			unsigned char *nBank = NULL;
			if (org.nBankLen)
			{
				unsigned char *nBank = new unsigned char[org.nBankLen];
				if (!nBank)
					throw(int)1;
				memcpy(nBank, org.bBank, org.nBankLen);
			}
			Unload();                               /* remove previous data              */
			bBank = nBank;                          /* and copy in the other one's       */
			bChunk = org.bChunk;
			nBankLen = org.nBankLen;
			strcpy(szFileName, org.szFileName);
			return *this;
		}

		/*****************************************************************************/
		/* SetSize : sets new size                                                   */
		/*****************************************************************************/

		bool CFxBank::SetSize(int nPrograms, int nParams)
		{
			int nTotLen = sizeof(fxBank) - sizeof(fxProgram);
			int nProgLen = sizeof(fxProgram) + (nParams - 1) * sizeof(float);
			nTotLen += nPrograms * nProgLen;
			unsigned char *nBank = new unsigned char[nTotLen];
			if (!nBank)
				return false;

			Unload();
			bBank = nBank;
			nBankLen = nTotLen;
			bChunk = false;

			memset(nBank, 0, nTotLen);              /* initialize new bank               */
			fxBank *pSet = (fxBank *)bBank;
			pSet->chunkMagic = cMagic;
			pSet->byteSize = 0;
			pSet->fxMagic = bankMagic;
			pSet->version = FxSetVersion;
			pSet->numPrograms = nPrograms;

			unsigned char *bProg = (unsigned char *)pSet->content.programs;
			for (int i = 0; i < nPrograms; i++)
			{
				fxProgram * pProg = (fxProgram *)(bProg + i * nProgLen);
				pProg->chunkMagic = cMagic;
				pProg->byteSize = 0;
				pProg->fxMagic = fMagic;
				pProg->version = 1;
				pProg->numParams = nParams;
				for (int j = 0; j < nParams; j++)
					pProg->content.params[j] = 0.0;
			}
			return true;
		}

		bool CFxBank::SetSize(int nChunkSize)
		{
			int nTotLen = sizeof(fxBank) + nChunkSize - 8;
			unsigned char *nBank = new unsigned char[nTotLen];
			if (!nBank)
				return false;

			Unload();
			bBank = nBank;
			nBankLen = nTotLen;
			bChunk = true;

			memset(nBank, 0, nTotLen);              /* initialize new bank               */
			fxBank *pSet = (fxBank *)bBank;
			pSet->chunkMagic = cMagic;
			pSet->byteSize = 0;
			pSet->fxMagic = chunkBankMagic;
			pSet->version = FxSetVersion;
			pSet->numPrograms = 1;
			pSet->content.data.size = nChunkSize;

			return true;
		}

		/*****************************************************************************/
		/* LoadBank : loads a bank file                                              */
		/*****************************************************************************/

		bool CFxBank::Load(const char *pszFile)
		{
			FILE *fp = fopen(pszFile, "rb");        /* try to open the file              */
			if (!fp)                                /* upon error                        */
				return false;                         /* return an error                   */
			bool brc = true;                        /* default to OK                     */
			unsigned char *nBank = NULL;
			try
			{
				fseek(fp, 0, SEEK_END);               /* get file size                     */
				size_t tLen = (size_t)ftell(fp);
				rewind(fp);

				nBank = new unsigned char[tLen];      /* allocate storage                  */
				if (!nBank)
					throw (int)1;
				/* read chunk set to determine cnt.  */
				if (fread(nBank, 1, tLen, fp) != tLen)
					throw (int)1;
				fxBank *pSet = (fxBank *)nBank;         /* position on set                   */
				if (NeedsBSwap)                       /* eventually swap necessary bytes   */
				{
					SwapBytes(pSet->chunkMagic);
					SwapBytes(pSet->byteSize);
					SwapBytes(pSet->fxMagic);
					SwapBytes(pSet->version);
					SwapBytes(pSet->fxID);
					SwapBytes(pSet->fxVersion);
					SwapBytes(pSet->numPrograms);
				}
				if ((pSet->chunkMagic != cMagic) ||   /* if erroneous data in there        */
					(pSet->version > FxSetVersion) ||
					((pSet->fxMagic != bankMagic) &&
					(pSet->fxMagic != chunkBankMagic)))
					throw (int)1;                       /* get out                           */

				if (pSet->fxMagic == bankMagic)
				{
					fxProgram * pProg = pSet->content.programs; /* position on 1st program           */
					int nProg = 0;
					while (nProg < pSet->numPrograms)   /* walk program list                 */
					{
						if (NeedsBSwap)                   /* eventually swap necessary bytes   */
						{
							SwapBytes(pProg->chunkMagic);
							SwapBytes(pProg->byteSize);
							SwapBytes(pProg->fxMagic);
							SwapBytes(pProg->version);
							SwapBytes(pProg->fxID);
							SwapBytes(pProg->fxVersion);
							SwapBytes(pProg->numParams);
						}
						/* if erroneous data                 */
						if ((pProg->chunkMagic != cMagic)|| 
							(pProg->fxMagic != fMagic))
							throw (int)1;                   /* get out                           */
						if (NeedsBSwap)                   /* if necessary                      */
						{                               /* swap all parameter bytes          */
							int j;
							for (j = 0; j < pProg->numParams; j++)
								SwapBytes(pProg->content.params[j]);
						}
						unsigned char *pNext = (unsigned char *)(pProg + 1);
						pNext += (sizeof(float) * (pProg->numParams - 1));
						if (pNext > nBank + tLen)         /* VERY simple fuse                  */
							throw (int)1;

						pProg = (fxProgram *)pNext;
						nProg++;
					}
				}
				/* if it's a chunk file              */
				else if (pSet->fxMagic == chunkBankMagic)
				{
					fxBank * pCSet = (fxBank *)nBank;
					if (NeedsBSwap)                     /* eventually swap necessary bytes   */
					{
						SwapBytes(pCSet->content.data.size);
						/* size check - must not be too large*/
						if (pCSet->content.data.size + sizeof(*pCSet) - 8 > tLen)
							throw (int)1;
					}
				}

				Unload();                             /* otherwise remove eventual old data*/
				bBank = nBank;                        /* and put in new data               */
				nBankLen = (int)tLen;
				bChunk = (pSet->fxMagic == chunkBankMagic);
			}
			catch(...)
			{
				brc = false;                          /* if any error occured, say NOPE    */
				if (nBank)                            /* and remove loaded data            */
					delete[] nBank;
			}

			fclose(fp);                             /* close the file                    */
			return brc;                             /* and return                        */
		}

		/*****************************************************************************/
		/* SaveBank : save bank to file                                              */
		/*****************************************************************************/

		bool CFxBank::Save(const char *pszFile)
		{
			if (!IsLoaded())
				return false;
			/* create internal copy for mod      */
			unsigned char *nBank = new unsigned char[nBankLen];
			if (!nBank)                             /* if impossible                     */
				return false;
			memcpy(nBank, bBank, nBankLen);

			fxBank *pSet = (fxBank *)nBank;           /* position on set                   */
			int numPrograms = pSet->numPrograms;
			if (NeedsBSwap)                         /* if byte-swapping needed           */
			{
				SwapBytes(pSet->chunkMagic);
				SwapBytes(pSet->byteSize);
				SwapBytes(pSet->fxMagic);
				SwapBytes(pSet->version);
				SwapBytes(pSet->fxID);
				SwapBytes(pSet->fxVersion);
				SwapBytes(pSet->numPrograms);
			}
			if (bChunk)
			{
				fxBank *pCSet = (fxBank *)nBank;
				if (NeedsBSwap)                       /* if byte-swapping needed           */
					SwapBytes(pCSet->content.data.size);
			}
			else
			{
				fxProgram * pProg = pSet->content.programs;   /* position on 1st program           */
				int numParams = pProg->numParams;
				int nProg = 0;
				while (nProg < numPrograms)           /* walk program list                 */
				{
					if (NeedsBSwap)                     /* eventually swap all necessary     */
					{
						SwapBytes(pProg->chunkMagic);
						SwapBytes(pProg->byteSize);
						SwapBytes(pProg->fxMagic);
						SwapBytes(pProg->version);
						SwapBytes(pProg->fxID);
						SwapBytes(pProg->fxVersion);
						SwapBytes(pProg->numParams);
						for (int j = 0; j < numParams; j++)
							SwapBytes(pProg->content.params[j]);
					}
					unsigned char *pNext = (unsigned char *)(pProg + 1);
					pNext += (sizeof(float) * (numParams - 1));
					if (pNext > nBank + nBankLen)       /* VERY simple fuse                  */
						break;

					pProg = (fxProgram *)pNext;
					nProg++;
				}
			}

			bool brc = true;                        /* default to OK                     */
			FILE *fp = NULL;
			try
			{
				fp = fopen(pszFile, "wb");            /* try to open the file              */
				if (!fp)                              /* upon error                        */
					throw (int)1;                       /* return an error                   */
				if (fwrite(nBank, 1, nBankLen, fp) != (size_t)nBankLen)
					throw (int)1;
			}
			catch(...)
			{
				brc = false;
			}
			if (fp)
				fclose(fp);
			delete[] nBank;

			return brc;
		}

		/*****************************************************************************/
		/* GetProgram : returns pointer to one of the loaded programs                */
		/*****************************************************************************/

		fxProgram * CFxBank::GetProgram(int nProgNum)
		{
			if ((!IsLoaded()) || (bChunk))          /* if nothing loaded or chunk file   */
				return NULL;                          /* return OUCH                       */

			fxBank *pSet = (fxBank *)bBank;           /* position on set                   */
			fxProgram * pProg = pSet->content.programs;     /* position on 1st program           */
		#if 1
			int nProgLen = sizeof(fxProgram) + (pProg->numParams - 1) * sizeof(float);
			unsigned char *pThatProg = ((unsigned char *)pProg) + (nProgNum * nProgLen);
			pProg = (fxProgram *)pThatProg;
		#else
			/*---------------------------------------------------------------------------*/
			/* presumably, the following logic is overkill; if all programs have the     */
			/* same number of parameters, a simple multiplication would do.              */
			/* But that's not stated anywhere in the VST SDK...                          */
			/*---------------------------------------------------------------------------*/
			int i;
			for (i = 0; i < nProgNum; i++)
			{
				unsigned char *pNext = (unsigned char *)(pProg + 1);
				pNext += (sizeof(float) * (pProg->numParams - 1));
				if (pNext > bBank + nBankLen)         /* VERY simple fuse                  */
					return NULL;

				pProg = (fxProgram *)pNext;
			}
		#endif
			return pProg;
		}

		/*===========================================================================*/
		/* CEffect class members                                                     */
		/*===========================================================================*/

		/*****************************************************************************/
		/* CEffect : constructor                                                     */
		/*****************************************************************************/

		CEffect::CEffect(LoadedAEffect &loadstruct)
			: aEffect(0)
//			, sFileName(0)
			, sDir(0)
			, bEditOpen(false)
			, bNeedIdle(false)
			, bWantMidi(false)
		{
			Load(loadstruct);
		}

		/*****************************************************************************/
		/* ~CEffect : destructor                                                     */
		/*****************************************************************************/

		CEffect::~CEffect()
		{
			Unload();
		}

		/*****************************************************************************/
		/* Load : loads (sets up) the effect module                                  */
		/*****************************************************************************/

		void CEffect::Load(LoadedAEffect &loadstruct)
		{
			aEffect=loadstruct.aEffect;
			ploader=loadstruct.pluginloader;

		#ifdef WIN32
			char const * name = loadstruct.sFileName;
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

//			sFileName = new char[strlen(name) + 1];
//			if (sFileName)
//				strcpy(sFileName, name);
		#elif MAC
			// yet to be done
		#endif

			// The trick, store the CEffect's class instance so that the host can talk to us.
			// I am unsure what other hosts use for resvd1 and resvd2
			aEffect->resvd1 = ToVstPtr(this);

			Open();                     /* open the effect                   */
			SetSampleRate(loadstruct.pHost->GetSampleRate()); /* adjust its sample rate            */
			SetBlockSize(loadstruct.pHost->GetBlockSize());
			//6 :        Host to Plug, canDo ( bypass )   returned : 0
			//7 :        Host to Plug, setPanLaw ( 0 , 0.707107 )   returned : false 

		}

		/*****************************************************************************/
		/* Unload : unloads (frees) effect module                                    */
		/*****************************************************************************/

		void CEffect::Unload()
		{
			Close();                             /* make sure it's closed             */
			aEffect = NULL;                         /* and reset the pointer             */
			delete ploader;

		#ifdef WIN32
			if (sDir)                               /* reset directory            */
			{
				delete[] sDir;	sDir = NULL;
			}
//			if (sFileName)                              /* reset name                 */
//			{
//				delete[] sFileName;	sFileName = NULL;
//			}
		#elif MAC
			// yet to be done!
		#endif
		}

		/*****************************************************************************/
		/* LoadBank : loads a .fxb file ... IF it's for this effect                  */
		/*****************************************************************************/

		bool CEffect::LoadBank(const char *name)
		{
			try
			{
				CFxBank fx(name);                     /* load the bank                     */
				if (!fx.IsLoaded())                   /* if error loading                  */
					throw (int)1;
			}
			catch(...)                              /* if any error occured              */
			{
				return false;                         /* return NOT!!!                     */
			}

			return true;                            /* pass back OK                      */
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

		CVSTHost * CVSTHost::pHost = NULL;      /* pointer to the one and only host  */

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

			pHost = this;                           /* install this instance as the one  */
		}

		/*****************************************************************************/
		/* ~CVSTHost : destructor                                                    */
		/*****************************************************************************/

		CVSTHost::~CVSTHost()
		{
			if (pHost == this)                      /* if we're the chosen one           */
				pHost = NULL;                         /* remove ourselves from pointer     */
		}

		/*****************************************************************************/
		/* LoadPlugin : loads and initializes a plugin                               */
		/*****************************************************************************/

		CEffect* CVSTHost::LoadPlugin(const char * sName)
		{
			PluginLoader* loader = new PluginLoader;
			if (!loader->loadLibrary(sName))
			{
				delete loader;
				return 0;
			}

			PluginEntryProc mainEntry = loader->getMainEntry ();
			if(!mainEntry)
			{
				delete loader;
				return 0;
			}

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
				loadstruct.pHost = this;
				loadstruct.pluginloader = loader;
				loadstruct.sFileName = sName;
				return CreateEffect(loadstruct);
			}
			
			delete loader;
			return 0;
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
		VstIntPtr VSTCALLBACK CVSTHost::AudioMasterCallback
		(
		AEffect* effect,
		VstInt32 opcode,
		VstInt32 index,
		VstIntPtr value,
		void* ptr,
		float opt
		)
/*		long VSTCALLBACK CVSTHost::AudioMasterCallback
		(
		AEffect *effect,
		long opcode,
		long index,
		long value,
		void *ptr,
		float opt
		)
*/
		{
			if (!pHost)
				throw (int)1;	// It would have no sense that there is no host.

			if ( !effect )
				throw (int)2;	// An effect has to identify itself if it wants to receive an answer

			CEffect *pEffect = (CEffect*)(effect->resvd1);
			if ( !pEffect ) 
				throw (int)3;	// An effect shoud not call audiomaster in its constructor, nor mess with resvd1.

			switch (opcode)
			{
			case audioMasterAutomate :
				pHost->OnSetParameterAutomated(*pEffect, index, opt);
				break;
			case audioMasterVersion :
				return pHost->OnGetVSTVersion(*pEffect);
			case audioMasterCurrentId :
				return pHost->OnCurrentId(*pEffect);
			case audioMasterIdle :
				pHost->OnIdle(*pEffect);
				break;
			case audioMasterPinConnected :
				return !((value) ? 
					pHost->OnOutputConnected(*pEffect, index) :
					pHost->OnInputConnected(*pEffect, index));
		// VST 2.0
			case audioMasterWantMidi :
				pHost->OnWantEvents(*pEffect, value);
				break;
			case audioMasterGetTime :
				return (long)pHost->OnGetTime(*pEffect, value);
			case audioMasterProcessEvents :
				return pHost->OnProcessEvents(*pEffect, (VstEvents *)ptr);
			case audioMasterSetTime :
				return pHost->OnSetTime(*pEffect, value, (VstTimeInfo *)ptr);
			case audioMasterTempoAt :
				return pHost->OnTempoAt(*pEffect, value);
			case audioMasterGetNumAutomatableParameters :
				return pHost->OnGetNumAutomatableParameters(*pEffect);
			case audioMasterGetParameterQuantization :
				return pHost->OnGetParameterQuantization(*pEffect);
			case audioMasterIOChanged :
				return pHost->OnIoChanged(*pEffect);
			case audioMasterNeedIdle :
				return pHost->OnNeedIdle(*pEffect);
			case audioMasterSizeWindow :
				return pHost->OnSizeWindow(*pEffect, index, value);
			case audioMasterGetSampleRate :
				return pHost->OnUpdateSampleRate(*pEffect);
			case audioMasterGetBlockSize :
				return pHost->OnUpdateBlockSize(*pEffect);
			case audioMasterGetInputLatency :
				return pHost->OnGetInputLatency(*pEffect);
			case audioMasterGetOutputLatency :
				return pHost->OnGetOutputLatency(*pEffect);
			case audioMasterGetPreviousPlug :
				return (long)(pHost->GetPreviousPlugIn(*pEffect,index)->GetAEffect());
			case audioMasterGetNextPlug :
				return (long)(pHost->GetNextPlugIn(*pEffect,index)->GetAEffect());
			case audioMasterWillReplaceOrAccumulate :
				return pHost->OnWillProcessReplacing(*pEffect);
			case audioMasterGetCurrentProcessLevel :
				return pHost->OnGetCurrentProcessLevel(*pEffect);
			case audioMasterGetAutomationState :
				return pHost->OnGetAutomationState(*pEffect);
			case audioMasterOfflineStart :
				return pHost->OnOfflineStart(*pEffect,
					(VstAudioFile *)ptr,
					value,
					index);
			case audioMasterOfflineRead :
				return pHost->OnOfflineRead(*pEffect,
					(VstOfflineTask *)ptr,
					(VstOfflineOption)value,
					!!index);
			case audioMasterOfflineWrite :
				return pHost->OnOfflineWrite(*pEffect,
					(VstOfflineTask *)ptr,
					(VstOfflineOption)value);
			case audioMasterOfflineGetCurrentPass :
				return pHost->OnOfflineGetCurrentPass(*pEffect);
			case audioMasterOfflineGetCurrentMetaPass :
				return pHost->OnOfflineGetCurrentMetaPass(*pEffect);
			case audioMasterSetOutputSampleRate :
				pHost->OnSetOutputSampleRate(*pEffect, opt);
				break;
			case audioMasterGetOutputSpeakerArrangement :
				return (long)pHost->OnGetOutputSpeakerArrangement(*pEffect);
			case audioMasterGetVendorString :
				return pHost->OnGetVendorString((char *)ptr);
			case audioMasterGetProductString :
				return pHost->OnGetProductString((char *)ptr);
			case audioMasterGetVendorVersion :
				return pHost->OnGetHostVendorVersion();
			case audioMasterVendorSpecific :
				return pHost->OnHostVendorSpecific(*pEffect, index, value, ptr, opt);
			case audioMasterSetIcon :
				// undefined in VST 2.0 specification. Deprecated in v2.4
				break;
			case audioMasterCanDo :
				return pHost->OnCanDo(*pEffect,(const char *)ptr);
			case audioMasterGetLanguage :
				return pHost->OnGetHostLanguage();
			case audioMasterOpenWindow :
				return (long)pHost->OnOpenWindow(*pEffect, (VstWindow *)ptr);
			case audioMasterCloseWindow :
				return pHost->OnCloseWindow(*pEffect, (VstWindow *)ptr);
			case audioMasterGetDirectory :
				return (long)pHost->OnGetDirectory(*pEffect);
			case audioMasterUpdateDisplay :
				return pHost->OnUpdateDisplay(*pEffect);
		// VST 2.1
		#ifdef VST_2_1_EXTENSIONS
			case audioMasterBeginEdit :
				return pHost->OnBeginEdit(*pEffect,index);
			case audioMasterEndEdit :
				return pHost->OnEndEdit(*pEffect,index);
			case audioMasterOpenFileSelector :
				return pHost->OnOpenFileSelector(*pEffect, (VstFileSelect *)ptr);
		#endif
		// VST 2.2
		#ifdef VST_2_2_EXTENSIONS
			case audioMasterCloseFileSelector :
				return pHost->OnCloseFileSelector(*pEffect, (VstFileSelect *)ptr);
			case audioMasterEditFile :
				return pHost->OnEditFile(*pEffect, (char *)ptr);
			case audioMasterGetChunkFile :
				return pHost->OnGetChunkFile(*pEffect, ptr);
		#endif
		// VST 2.3
		#ifdef VST_2_3_EXTENSIONS
			case audioMasterGetInputSpeakerArrangement :
				return (long)pHost->OnGetInputSpeakerArrangement(*pEffect);
		#endif

			}
			return 0L;
		}




		/*****************************************************************************/
		/* OnCanDo : returns whether the host can do a specific action               */
		/*****************************************************************************/

		bool CVSTHost::OnCanDo(CEffect &pEffect, const char *ptr)
		{
			// For the host, according to audioeffectx.cpp , "!= 0 -> true", so there isn't "-1 : can't do".
			if (	(!strcmp(ptr, hostCanDos[0] )) // "sendVstEvents"
				||	(!strcmp(ptr, hostCanDos[1] )) // "sendVstMidiEvent"
				||	(!strcmp(ptr, hostCanDos[2] )) // "sendVstTimeInfo",
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

		long CVSTHost::OnGetVSTVersion(CEffect & pEffect)
		{
		#if defined(VST_2_3_EXTENSIONS)
			return 2300L;
		#elif defined(VST_2_2_EXTENSIONS)
			return 2200L;
		#elif defined(VST_2_1_EXTENSIONS)
			return 2100L;
		#else 
			return 2L;
		#endif
		}
	}
}
