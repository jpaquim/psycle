/*****************************************************************************/
/* ImplVSTHost.h: CImplVSTHost / CImplEffect implementation                  */
/* Work Derived from vsthost. Copyright (c) H. Seib, 2002-2005               */
/* (http://www.hermannseib.com/english/vsthost.htm)"						 */
/*****************************************************************************/
/*                                                                           */
/* $Archive::                                                              $ */
/* $Revision$ */
/* $Date$ */
/* $Author$ */
/* $Modtime::                                                              $ */
/* $Workfile::                                                             $ */
/*                                                                           */
/*****************************************************************************/

#pragma once

#include <Seib-vsthost/CVSTHost.Seib.hpp>

/*****************************************************************************/
/* CImplEffect class declaration                                              */
/*****************************************************************************/
namespace Seib{
	namespace vsthost{
		class CEffectWnd;
		class CImplEffect : public CEffect  
		{
		public:
			CImplEffect(CVSTHost *pHost);
			virtual ~CImplEffect();

		public:
			bool LoadBank(const char *name);
			void LeaveCritical();
			void EnterCritical();
			virtual bool Load(const char *name);
			virtual bool Unload();
			virtual long EffDispatch(long opCode, long index=0, long value=0, void *ptr=0, float opt=0.);

			long int magic() { return pEffect->magic; }
			long int numPrograms() { return pEffect->numPrograms; }
			long int numParams() { return pEffect->numParams; }
			long int numInputs() { return pEffect->numInputs; }
			long int numOutputs() { return pEffect->numOutputs;	}
			long int flags(int flag=-1) { return pEffect->flags & flag; }
			long int uniqueId() { return pEffect->uniqueID; }
			long int version() { return pEffect->version; }
			long int initialDelay() { return pEffect->initialDelay; }

			virtual void * OnOpenWindow(VstWindow* window);
			virtual bool OnCloseWindow(VstWindow* window);
			virtual void OnSizeEditorWindow(long width, long height);
/*
			void SetFrameWnd(CChildFrame *pFWnd = 0) { pFrameWnd = pFWnd; }
			CChildFrame *GetFrameWnd() { return pFrameWnd; }
			void SetEditWnd(CEffectWnd *pEWnd = 0) { pEditWnd = pEWnd; }
			CEffectWnd *GetEditWnd() { return pEditWnd; }
			void SetParmWnd(CEffectWnd *pPWnd = 0) { pParmWnd = pPWnd; }
			CEffectWnd *GetParmWnd() { return pParmWnd; }
*/			
			unsigned short GetChnMask() { return wChnMask; }
			void SetChnMask(unsigned short mask) { wChnMask = mask; }
			bool OnSetParameterAutomated(long index, float value);
			virtual bool OnUpdateDisplay();
			void SetChunkFile(CString sFile) { sChunkFile = sFile; }
			CString GetChunkFile() { return sChunkFile; }
			CString GetDisplayName()
			{
				char szBuf[256] = "";
				if (EffGetProductString(szBuf))
					return szBuf;
				CString sCutName(sName);
				int nIdx = sCutName.ReverseFind('\\');
				if (nIdx >= 0)
					sCutName = sCutName.Mid(nIdx + 1);
				return sCutName;
			}
/*
			void InsertIntoChain(CSmpEffect *prev)
			{
				RemoveFromChain();
				EnterCritical();
				if (prev)
				{
					pNext = prev->pNext;
					pPrev = prev;
					prev->pNext = this;
					if (pNext)
						pNext->pPrev = this;
				}
				LeaveCritical();
			}
			void RemoveFromChain()
			{
				EnterCritical();
				if (pPrev)
					pPrev->pNext = pNext;
				if (pNext)
					pNext->pPrev = pPrev;
				pPrev = pNext = NULL;
				LeaveCritical();
			}
			CSmpEffect *GetPrev() { return pPrev; }
			CSmpEffect *GetNext() { return pNext; }
*/

			void SetOutputBuffer(int nBuf, float *pData = 0)
			{
				if (nBuf < pEffect->numOutputs)
					// outBufs allocation isn't checked here; this is intentional.
					// We rely on a sensible implementation that doesn't set
					// output buffers before initialization is finished (which
					// would throw out an effect if buffer allocation fails).
					outBufs[nBuf] = pData;
			}

			float ** GetInputBuffers() { return outBufs; }
			float * GetInputBuffer(int nBuf)
			{
				if (nBuf < pEffect->numInputs)
					return inBufs[nBuf];
				else
					return 0;
			}

			virtual void EffProcess(long sampleFrames)
			{
				// operate using internally allocated buffers
				CEffect::EffProcess(inBufs, outBufs, sampleFrames);
				TRACE1("effProcess (max=%1.5f)\n", CalcMax(sampleFrames));
			}
			virtual void EffProcessReplacing(long sampleFrames)
			{
				// operate using internally allocated buffers
				CEffect::EffProcessReplacing(inBufs, outBufs, sampleFrames);
				TRACE1("effProcessReplacing (max=%1.5f)\n", CalcMax(sampleFrames));
			}
			float CalcMax(long sampleFrames)
			{
				//\todo : CEffect::EffGetVu() ?
				int nOuts = pEffect->numOutputs;
				int b, s;
				float fCur, *p;
				fMax = 0.0f;
				for (b = 0; b < nOuts; b++)
				{
					p = outBufs[b];
					for (s = 0; s < sampleFrames; s++)
					{
						fCur = fabsf(*p++);
						if (fCur > fMax)
							fMax = fCur;
					}
				}
				return fMax;
			}

		protected:
			unsigned short wChnMask; // Used for store to which MIDI channels this plugin is listening to.
			int nAllocatedOutbufs;	// Stores input sample channels
			int nAllocatedInbufs;	// Stores output sample channels
			static int EvalException(int n_except);
			CRITICAL_SECTION cs;

			float ** outBufs;
			float ** inBufs;
			float fMax;
/*			CChildFrame *pFrameWnd;
			CEffectWnd * pEditWnd;
			CEffectWnd * pParmWnd;
			CPtrArray paWnds;                   // array of secondary VstWindows     
*/
			CString sChunkFile;

/*			// Effect chaining
			CSmpEffect *pPrev;
			CSmpEffect *pNext;
*/
		};

		/*****************************************************************************/
		/* CImplVSTHost class declaration                                             */
		/*****************************************************************************/

		class CMidiMsg;
		class CImplVSTHost : public CVSTHost  
		{
		public:
			CImplVSTHost();
			virtual ~CImplVSTHost();

		public:
			BYTE * CreateMIDISubset(void *pEvData, unsigned short wChnMask = 0xffff);
			void OnSamples(float **pBuffer, int nLength, int nChannels = 2, DWORD dwStamp = 0);
			virtual bool OnProcessEvents(int nEffect, VstEvents* events);
			virtual void OnMidiIn(CMidiMsg &msg);
			virtual void PassThruEffect(CImplEffect *pEff, float **pBuffer, int nLength, int nChannels, BYTE *pEvData, bool bReplacing);

			/* create customized effects         */
			virtual CEffect * CreateEffect() { return new CImplEffect(this); }
			// overridden callback functions
			virtual long OnAudioMasterCallback(int nEffect, long opcode, long index, long value, void *ptr, float opt);
			virtual bool OnSetParameterAutomated(int nEffect, long index, float value);
			virtual bool OnCanDo(const char *ptr);
			virtual bool OnOpenFileSelector (int nEffect, VstFileSelect *ptr);
			virtual bool OnCloseFileSelector (int nEffect, VstFileSelect *ptr);
			virtual bool OnGetChunkFile(int nEffect, void * nativePath);

		protected:
			CRITICAL_SECTION cs;
			int nMidis;
			DWORD * pMidis;
			float * pOutputs[2];
			int nSysEx;
			CMidiMsg *pSysEx;

		protected:
			BYTE * CreateMIDIEvents(int nLength, DWORD dwStamp, int nm, int ns);

		};
	}
}