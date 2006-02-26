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

#include "Constants.hpp"
#include "Machine.hpp"
#include <Seib-vsthost/CVSTHost.Seib.hpp>

namespace psycle {
	namespace host {
		namespace vst2 {
			/*****************************************************************************/
			/* plugin class declaration                                                  */
			/*****************************************************************************/
			using namespace ::seib::vst;

			class CEffectWnd;
			class plugin : public Machine,CEffect
			{
				class note
				{
				public:
					unsigned char key;
					unsigned char midichan;
				};
			public:
				plugin(CVSTHost *pHost);
				virtual ~plugin();

				// Actions //////////////////////////////////////////////////////////////////////////
				bool LoadBank(const char *name);
				void LeaveCritical();
				void EnterCritical();
				virtual bool Load(const char *name);
				virtual bool Unload();
				virtual void Tick(int channel, PatternEntry * pEntry);
				virtual void Stop(void);

				virtual long Dispatch(long opCode, long index=0, long value=0, void *ptr=0, float opt=0.);
				virtual void Process(long sampleFrames)
				{
					// operate using internally allocated buffers
					CEffect::Process(inBufs, outBufs, sampleFrames);
					TRACE1("effProcess (max=%1.5f)\n", CalcMax(sampleFrames));
				}
				virtual void ProcessReplacing(long sampleFrames)
				{
					// operate using internally allocated buffers
					CEffect::ProcessReplacing(inBufs, outBufs, sampleFrames);
					TRACE1("effProcessReplacing (max=%1.5f)\n", CalcMax(sampleFrames));
				}
			private:
				/// reserves space for a new midi event in the queue.
				/// \return midi event to be filled in, or null if queue is full.
				VstMidiEvent* reserveVstMidiEvent();
				VstMidiEvent* reserveVstMidiEventAtFront(); // ugly hack

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
				BYTE * CreateMIDISubset(void *pEvData, unsigned short wChnMask = 0xffff);
				virtual void OnMidiIn(CMidiMsg &msg);
				virtual void PassThruEffect(CImplEffect *pEff, float **pBuffer, int nLength, int nChannels, BYTE *pEvData, bool bReplacing);
			protected:
				BYTE * CreateMIDIEvents(int nLength, DWORD dwStamp, int nm, int ns);
			public:
				// Events //////////////////////////////////////////////////////////////////////////
				virtual void * OnOpenWindow(VstWindow* window);
				virtual bool OnCloseWindow(VstWindow* window);
				virtual void OnSizeEditorWindow(long width, long height);
				virtual bool OnUpdateDisplay();

				// Properties //////////////////////////////////////////////////////////////////////////
				long int Magic() { assert(pEffect!=NULL); return pEffect->magic; }
				long int NumPrograms() { assert(pEffect!=NULL); return pEffect->numPrograms; }
				long int NumParams() { assert(pEffect!=NULL); return pEffect->numParams; }
				long int NumInputs() { assert(pEffect!=NULL); return pEffect->numInputs; }
				long int NumOutputs() { assert(pEffect!=NULL); return pEffect->numOutputs;	}
				long int Flags(int flag=-1) { assert(pEffect!=NULL); return pEffect->flags & flag; }
				long int UniqueId() { assert(pEffect!=NULL); return pEffect->uniqueID; }
				long int Version() { assert(pEffect!=NULL); return pEffect->version; }
				long int InitialDelay() { assert(pEffect!=NULL); return pEffect->initialDelay; }


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
				void SetChunkFile(CString sFile) { sChunkFile = sFile; }
				CString GetChunkFile() { return sChunkFile; }
				CString GetDisplayName()
				{
					char szBuf[256] = "";
					if (GetProductString(szBuf))
						return szBuf;
					CString sCutName(sName);
					int nIdx = sCutName.ReverseFind('\\');
					if (nIdx >= 0)
						sCutName = sCutName.Mid(nIdx + 1);
					return sCutName;
				}
				virtual CString GetDllName() const throw() { return sFileName; }
				void SetParameter(int numparam,int value,int quantization=65535);
				void GetParameter(int numparam,int value);
				void GetParameter(int numparam,char * text);
				void GetParameterLabel(int numparam,char *text);
				void GetParameterUnit(int numparam,char *text);
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
				note trackNote[MAX_TRACKS];

				int nMidis;
				DWORD * pMidis;
				int nSysEx;
				CMidiMsg *pSysEx;
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
				void OnSamples(float **pBuffer, int nLength, int nChannels = 2, DWORD dwStamp = 0);
				virtual bool OnProcessEvents(int nEffect, VstEvents* events);

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
				float * pOutputs[2];

			};
		}
	}
}
