/*****************************************************************************/
/* CVSTHost.hpp: interface for the CVSTHost class.							 */
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
#include <vst/aeffectx.h>               /* VST header files                  */
#define VST_2_1_EXTENSIONS 1
#define VST_2_2_EXTENSIONS 1
#define VST_2_3_EXTENSIONS 1
#include <vst/vstfxstore.h>
#include <vst/AEffEditor.h>

namespace seib {
	namespace vst {
		/*****************************************************************************/
		/* CFxBase : base class for FX Bank / Program Files                          */
		/*****************************************************************************/

		class CFxBase
		{
		public:

		protected:
			static bool NeedsBSwap;

		protected:
			static void SwapBytes(float &f);
			static void SwapBytes(long &l);

		};

		/*****************************************************************************/
		/* CFxBank : class for an Fx Bank file                                       */
		/*****************************************************************************/

		class CFxBank : public CFxBase
		{
		public:
			CFxBank(char *pszFile = 0);
			CFxBank(int nPrograms, int nParams);
			CFxBank(int nChunkSize);
			CFxBank(CFxBank const &org) { DoCopy(org); }
			virtual ~CFxBank();
			CFxBank & operator=(CFxBank const &org) { return DoCopy(org); }
		public:
			bool SetSize(int nPrograms, int nParams);
			bool SetSize(int nChunkSize);
			bool LoadBank(char *pszFile);
			bool SaveBank(char *pszFile);
			void Unload();
			bool IsLoaded() { return !!bBank; }
			bool IsChunk() { return bChunk; }

			// access functions
		public:
			long GetVersion() { if (!bBank) return 0; return ((fxSet*)bBank)->version; }
			long GetFxID() { if (!bBank) return 0; return ((fxSet*)bBank)->fxID; }
			void SetFxID(long id) { if (bBank) ((fxSet*)bBank)->fxID = id; if (!bChunk) for (int i = GetNumPrograms() -1; i >= 0; i--) GetProgram(i)->fxID = id; }
			long GetFxVersion() { if (!bBank) return 0; return ((fxSet*)bBank)->fxVersion; }
			void SetFxVersion(long v) { if (bBank) ((fxSet*)bBank)->fxVersion = v; if (!bChunk) for (int i = GetNumPrograms() -1; i >= 0; i--) GetProgram(i)->fxVersion = v; }
			long GetNumPrograms() { if (!bBank) return 0; return ((fxSet*)bBank)->numPrograms; }
			long GetNumParams() { if (bChunk) return 0; return GetProgram(0)->numParams; }
			long GetChunkSize() { if (!bChunk) return 0; return ((fxChunkSet *)bBank)->chunkSize; }
			void *GetChunk() { if (!bChunk) return 0; return ((fxChunkSet *)bBank)->chunk; }
			bool SetChunk(void *chunk) { if (!bChunk) return false; memcpy(((fxChunkSet *)bBank)->chunk, chunk, ((fxChunkSet *)bBank)->chunkSize); return true; }

			fxProgram * GetProgram(int nProgNum);
			char * GetProgramName(int nProgram)
			{
				fxProgram *p = GetProgram(nProgram);
				if (!p)
					return NULL;
				return p->prgName;
			}
			void SetProgramName(int nProgram, char *name = "")
			{
				fxProgram *p = GetProgram(nProgram);
				if (!p)
					return;
				strncpy(p->prgName, name, sizeof(p->prgName));
				p->prgName[sizeof(p->prgName)-1] = '\0';
			}
			float GetProgParm(int nProgram, int nParm)
			{
				fxProgram *p = GetProgram(nProgram);
				if (!p || nParm > p->numParams)
					return 0;
				return p->params[nParm];
			}
			bool SetProgParm(int nProgram, int nParm, float val = 0.0)
			{
				fxProgram *p = GetProgram(nProgram);
				if (!p || nParm > p->numParams)
					return false;
				if (val < 0.0)
					val = 0.0;
				if (val > 1.0)
					val = 1.0;
				p->params[nParm] = val;
				return true;
			}

		protected:
			char szFileName[256];
			unsigned char * bBank;
			int nBankLen;
			bool bChunk;

		protected:
			void Init();
			CFxBank & DoCopy(CFxBank const &org);
		};

		/*****************************************************************************/
		/* CFxProgram : class for an Fx Program file                                 */
		/*****************************************************************************/

		// not really designed yet...
		class CFxProgram : public CFxBase
		{
		};

		/*****************************************************************************/
		/* CEffect : class definition for audio effect objects                       */
		/*****************************************************************************/

		class CVSTHost;
		class CEffect
		{
			typedef AEffect * (* PVSTMAIN) (audioMasterCallback audioMaster);

		public:
			CEffect(CVSTHost *pHost);
			virtual ~CEffect();

		public:
			CVSTHost *pHost;
			AEffect *pEffect;
			char *sFileName;
			bool bEditOpen;
			bool bNeedIdle;
			bool bWantMidi;

		#ifdef WIN32

			HMODULE hModule;
			char *sDir;

		#elif MAC

			// yet to do
			// no idea how things look here...

		#endif

			virtual bool Load(const char *name);
			virtual bool Unload();

			virtual bool LoadBank(char *name);
			virtual bool SaveBank(char *name);

			virtual long Dispatch(long opCode, long index=0, long value=0, void *ptr=0, float opt=0.);
			virtual void Process(float **inputs, float **outputs, long sampleframes);
			virtual void ProcessReplacing(float **inputs, float **outputs, long sampleframes);
			virtual void SetParameter(long index, float parameter);
			virtual float GetParameter(long index);

			void Open() { Dispatch(effOpen); }
			void Close() { Dispatch(effClose); }
			void SetProgram(long lValue) { Dispatch(effSetProgram, 0, lValue); }
			long GetProgram() { return Dispatch(effGetProgram); }
			void SetProgramName(char *ptr) { Dispatch(effSetProgramName, 0, 0, ptr); }
			void GetProgramName(char *ptr) { Dispatch(effGetProgramName, 0, 0, ptr); }
			void GetParamLabel(long index, char *ptr) { Dispatch(effGetParamLabel, index, 0, ptr); }
			void GetParamDisplay(long index, char *ptr) { Dispatch(effGetParamDisplay, index, 0, ptr); }
			void GetParamName(long index, char *ptr) { Dispatch(effGetParamName, index, 0, ptr); }
			void SetSampleRate(float fSampleRate) { Dispatch(effSetSampleRate, 0, 0, 0, fSampleRate); }
			void SetBlockSize(long value) { Dispatch(effSetBlockSize, 0, value); }
			void MainsChanged(bool bOn) { Dispatch(effMainsChanged, 0, bOn); }
			float GetVu() { return (float)Dispatch(effGetVu) / (float)32767.; }
			long EditGetRect(ERect **ptr) { return Dispatch(effEditGetRect, 0, 0, ptr); }
			long EditOpen(void *ptr) { long l = Dispatch(effEditOpen, 0, 0, ptr); if (l > 0) bEditOpen = true; return l; }
			void EditClose() { Dispatch(effEditClose); bEditOpen = false; }
			void EditIdle() { if (bEditOpen) Dispatch(effEditIdle); }
		#if MAC
			void EditDraw(void *ptr) { Dispatch(nect, effEditDraw, 0, 0, ptr); }
			long EditMouse(long index, long value) { return Dispatch(nect, effEditMouse, index, value); }
			long EditKey(long value) { return Dispatch(effEditKey, 0, value); }
			void EditTop() { Dispatch(effEditTop); }
			void EditSleep() { Dispatch(effEditSleep); }
		#endif
			long Identify() { return Dispatch(effIdentify); }
			long GetChunk(void **ptr, bool isPreset = false) { return Dispatch(effGetChunk, isPreset, 0, ptr); }
			long SetChunk(void *data, long byteSize, bool isPreset = false) { return Dispatch(effSetChunk, isPreset, byteSize, data); }
			/* VST 2.0                           */
			long ProcessEvents(VstEvents* ptr) { return Dispatch(effProcessEvents, 0, 0, ptr); }
			long CanBeAutomated(long index) { return Dispatch(effCanBeAutomated, index); }
			long String2Parameter(long index, char *ptr) { return Dispatch(effString2Parameter, index, 0, ptr); }
			long GetNumProgramCategories() { return Dispatch(effGetNumProgramCategories); }
			long GetProgramNameIndexed(long category, long index, char* text) { return Dispatch(effGetProgramNameIndexed, index, category, text); }
			long CopyProgram(long index) { return Dispatch(effCopyProgram, index); }
			long ConnectInput(long index, bool state) { return Dispatch(effConnectInput, index, state); }
			long ConnectOutput(long index, bool state) { return Dispatch(effConnectOutput, index, state); }
			long GetInputProperties(long index, VstPinProperties *ptr) { return Dispatch(effGetInputProperties, index, 0, ptr); }
			long GetOutputProperties(long index, VstPinProperties *ptr) { return Dispatch(effGetOutputProperties, index, 0, ptr); }
			long GetPlugCategory() { return Dispatch(effGetPlugCategory); }
			long GetCurrentPosition() { return Dispatch(effGetCurrentPosition); }
			long GetDestinationBuffer() { return Dispatch(effGetDestinationBuffer); }
			long OfflineNotify(VstAudioFile* ptr, long numAudioFiles, bool start) { return Dispatch(effOfflineNotify, start, numAudioFiles, ptr); }
			long OfflinePrepare(VstOfflineTask *ptr, long count) { return Dispatch(effOfflinePrepare, 0, count, ptr); }
			long OfflineRun(VstOfflineTask *ptr, long count) { return Dispatch(effOfflineRun, 0, count, ptr); }
			long ProcessVarIo(VstVariableIo* varIo) { return Dispatch(effProcessVarIo, 0, 0, varIo); }
			long SetSpeakerArrangement(VstSpeakerArrangement* pluginInput, VstSpeakerArrangement* pluginOutput) { return Dispatch(effSetSpeakerArrangement, 0, (long)pluginInput, pluginOutput); }
			long SetBlockSizeAndSampleRate(long blockSize, float sampleRate) { return Dispatch(effSetBlockSizeAndSampleRate, 0, blockSize, 0, sampleRate); }
			long SetBypass(bool onOff) { return Dispatch(effSetBypass, 0, onOff); }
			long GetEffectName(char *ptr) { return Dispatch(effGetEffectName, 0, 0, ptr); }
			long GetErrorText(char *ptr) { return Dispatch(effGetErrorText, 0, 0, ptr); }
			long GetVendorString(char *ptr) { return Dispatch(effGetVendorString, 0, 0, ptr); }
			long GetProductString(char *ptr) { return Dispatch(effGetProductString, 0, 0, ptr); }
			long GetVendorVersion() { return Dispatch(effGetVendorVersion); }
			long VendorSpecific(long index, long value, void *ptr, float opt) { return Dispatch(effVendorSpecific, index, value, ptr, opt); }
			long CanDo(const char *ptr) { return Dispatch(effCanDo, 0, 0, (void *)ptr); }
			long GetTailSize() { return Dispatch(effGetTailSize); }
			long Idle() { if (bNeedIdle) return Dispatch(effIdle); else return 0; }
			long GetIcon() { return Dispatch(effGetIcon); }
			long SetViewPosition(long x, long y) { return Dispatch(effSetViewPosition, x, y); }
			long GetParameterProperties(long index, VstParameterProperties* ptr) { return Dispatch(effGetParameterProperties, index, 0, ptr); }
			long KeysRequired() { return Dispatch(effKeysRequired); }
			long GetVstVersion() { return Dispatch(effGetVstVersion); }
			/* VST 2.1 extensions                */
			long KeyDown(VstKeyCode &keyCode) { return Dispatch(effEditKeyDown, keyCode.character, keyCode.virt, 0, keyCode.modifier); }
			long KeyUp(VstKeyCode &keyCode) { return Dispatch(effEditKeyUp, keyCode.character, keyCode.virt, 0, keyCode.modifier); }
			void SetKnobMode(long value) { Dispatch(effSetEditKnobMode, 0, value); }
			long GetMidiProgramName(long channel, MidiProgramName* midiProgramName) { return Dispatch(effGetMidiProgramName, channel, 0, midiProgramName); }
			long GetCurrentMidiProgram (long channel, MidiProgramName* currentProgram) { return Dispatch(effGetCurrentMidiProgram, channel, 0, currentProgram); }
			long GetMidiProgramCategory (long channel, MidiProgramCategory* category) { return Dispatch(effGetMidiProgramCategory, channel, 0, category); }
			long HasMidiProgramsChanged (long channel) { return Dispatch(effHasMidiProgramsChanged, channel); }
			long GetMidiKeyName(long channel, MidiKeyName* keyName) { return Dispatch(effGetMidiKeyName, channel, 0, keyName); }
			long BeginSetProgram() { return Dispatch(effBeginSetProgram); }
			long EndSetProgram() { return Dispatch(effEndSetProgram); }
			/* VST 2.3 Extensions                */
			long GetSpeakerArrangement(VstSpeakerArrangement** pluginInput, VstSpeakerArrangement** pluginOutput) {Dispatch(effGetSpeakerArrangement, 0, (long)pluginInput, pluginOutput); }
			long SetTotalSampleToProcess (long value) { return Dispatch(effSetTotalSampleToProcess, 0, value); }
			long GetNextShellPlugin(char *name) { return Dispatch(effShellGetNextPlugin, 0, 0, name); }
			long StartProcess() { return Dispatch(effStartProcess); }
			long StopProcess() { return Dispatch(effStopProcess); }
			long SetPanLaw(long type, float val) { return Dispatch(effSetPanLaw, 0, type, 0, val); }
			long BeginLoadBank(VstPatchChunkInfo* ptr) { return Dispatch(effBeginLoadBank, 0, 0, ptr); }
			long BeginLoadProgram(VstPatchChunkInfo* ptr) { return Dispatch(effBeginLoadProgram, 0, 0, ptr); }

			// overridables
		public:
			virtual void * OnGetDirectory();
			virtual void OnSizeEditorWindow(long width, long height) { }
			virtual bool OnUpdateDisplay() { return false; }
			virtual void * OnOpenWindow(VstWindow* window) { return 0; }
			virtual bool OnCloseWindow(VstWindow* window) { return false; }

		};

		/*****************************************************************************/
		/* CVSTHost class declaration                                                */
		/*****************************************************************************/

		class CVSTHost
		{
			friend class CEffect;
		public:
			CVSTHost();
			virtual ~CVSTHost();

		protected:
			void CalcTimeInfo(long lMask = -1);
			VstTimeInfo vstTimeInfo;
			float fSampleRate;
			long lBlockSize;

			int naEffects;
			int nmaEffects;
			void **aEffects;
			static CVSTHost * pHost;

			static long VSTCALLBACK AudioMasterCallback(AEffect *effect, long opcode, long index, long value, void *ptr, float opt);
			int Search(AEffect *pEffect);
			int GetPreviousPlugIn(int nEffect,int pinIndex);
			int GetNextPlugIn(int nEffect, int pinIndex);
			long Dispatch(int nEffect, long opCode, long index=0, long value=0, void *ptr=0, float opt=0.);

		public:
			int LoadPlugin(const char * lpszName);
			int GetSize() { return naEffects; }
			CEffect *GetAt(int nIndex) { if ((nIndex >= 0) && (nIndex < naEffects)) return (CEffect *)aEffects[nIndex]; else return 0; }
			void RemoveAt(int nIndex);
			void RemoveAll();

			void Process(int nEffect, float **inputs, float **outputs, long sampleframes);
			void ProcessReplacing(int nEffect, float **inputs, float **outputs, long sampleframes);
			void SetParameter(int nEffect, long index, float parameter);
			float GetParameter(int nEffect, long index);

			void Open(int nEffect)
			{ if (GetAt(nEffect)) GetAt(nEffect)->Open(); }
			void Close(int nEffect)
			{ if (GetAt(nEffect)) GetAt(nEffect)->Close(); }
			void SetProgram(int nEffect, long lValue)
			{ if (GetAt(nEffect)) GetAt(nEffect)->SetProgram(lValue); }
			long GetProgram(int nEffect)
			{ if (GetAt(nEffect)) return (GetAt(nEffect))->GetProgram(); else return 0; }
			void SetProgramName(int nEffect, char *ptr)
			{ if (GetAt(nEffect)) GetAt(nEffect)->SetProgramName(ptr); }
			void GetProgramName(int nEffect, char *ptr)
			{ if (GetAt(nEffect)) GetAt(nEffect)->GetProgramName(ptr); }
			void GetParamLabel(int nEffect, long index, char *ptr)
			{ if (GetAt(nEffect)) GetAt(nEffect)->GetParamLabel(index, ptr); }
			void GetParamDisplay(int nEffect, long index, char *ptr)
			{ if (GetAt(nEffect)) GetAt(nEffect)->GetParamDisplay(index, ptr); }
			void GetParamName(int nEffect, long index, char *ptr)
			{ if (GetAt(nEffect)) GetAt(nEffect)->GetParamName(index, ptr); }
			void SetSampleRate(int nEffect, float fSampleRate)
			{ if (GetAt(nEffect)) GetAt(nEffect)->SetSampleRate(fSampleRate); }
			void SetBlockSize(int nEffect, long value)
			{ if (GetAt(nEffect)) GetAt(nEffect)->SetBlockSize(value); }
			void MainsChanged(int nEffect, bool bOn)
			{ if (GetAt(nEffect)) GetAt(nEffect)->MainsChanged(bOn); }
			float GetVu(int nEffect)
			{ if (GetAt(nEffect)) return GetAt(nEffect)->GetVu(); else return 0.f; }
			long EditGetRect(int nEffect, ERect **ptr)
			{ if (GetAt(nEffect)) return GetAt(nEffect)->EditGetRect(ptr); else return 0; }
			long EditOpen(int nEffect, void *ptr)
			{ if (GetAt(nEffect)) return GetAt(nEffect)->EditOpen(ptr); else return 0; }
			void EditClose(int nEffect)
			{ if (GetAt(nEffect)) GetAt(nEffect)->EditClose(); }
			void EditIdle(int nEffect)
			{ if (GetAt(nEffect)) GetAt(nEffect)->EditIdle(); }
		#if MAC
			void EditDraw(int nEffect, void *ptr)
			{ if (GetAt(nEffect)) GetAt(nEffect)->EditDraw(ptr); }
			long EditMouse(int nEffect, long index, long value)
			{ if (GetAt(nEffect)) return GetAt(nEffect)->EditMouse(index, value); else return 0; }
			long EditKey(int nEffect, long value)
			{ if (GetAt(nEffect)) return GetAt(nEffect)->EditKey(value); else return 0; }
			void EditTop(int nEffect)
			{ if (GetAt(nEffect)) GetAt(nEffect)->EditTop(); }
			void EditSleep(int nEffect)
			{ if (GetAt(nEffect)) GetAt(nEffect)->EditSleep(); }
		#endif
			long Identify(int nEffect)
			{ if (GetAt(nEffect)) return GetAt(nEffect)->Identify(); else return 0; }
			long GetChunk(int nEffect, void **ptr, bool isPreset = false)
			{ if (GetAt(nEffect)) return GetAt(nEffect)->GetChunk(ptr, isPreset); else return 0; }
			long SetChunk(int nEffect, void *data, long byteSize, bool isPreset = false)
			{ if (GetAt(nEffect)) return GetAt(nEffect)->SetChunk(data, byteSize, isPreset); else return 0; }
			/* VST 2.0                           */
			long ProcessEvents(int nEffect, VstEvents* ptr)
			{ if (GetAt(nEffect)) return GetAt(nEffect)->ProcessEvents(ptr); else return 0; }
			long CanBeAutomated(int nEffect, long index)
			{ if (GetAt(nEffect)) return GetAt(nEffect)->CanBeAutomated(index); else return 0; }
			long String2Parameter(int nEffect, long index, char *ptr)
			{ if (GetAt(nEffect)) return GetAt(nEffect)->String2Parameter(index, ptr); else return 0; }
			long GetNumProgramCategories(int nEffect)
			{ if (GetAt(nEffect)) return GetAt(nEffect)->GetNumProgramCategories(); else return 0; }
			long GetProgramNameIndexed(int nEffect, long category, long index, char* text)
			{ if (GetAt(nEffect)) return GetAt(nEffect)->GetProgramNameIndexed(category, index, text); else return 0; }
			long CopyProgram(int nEffect, long index)
			{ if (GetAt(nEffect)) return GetAt(nEffect)->CopyProgram(index); else return 0; }
			long ConnectInput(int nEffect, long index, bool state)
			{ if (GetAt(nEffect)) return GetAt(nEffect)->ConnectInput(index, state); else return 0; }
			long ConnectOutput(int nEffect, long index, bool state)
			{ if (GetAt(nEffect)) return GetAt(nEffect)->ConnectOutput(index, state); else return 0; }
			long GetInputProperties(int nEffect, long index, VstPinProperties *ptr)
			{ if (GetAt(nEffect)) return GetAt(nEffect)->GetInputProperties(index, ptr); else return 0; }
			long GetOutputProperties(int nEffect, long index, VstPinProperties *ptr)
			{ if (GetAt(nEffect)) return GetAt(nEffect)->GetOutputProperties(index, ptr); else return 0; }
			long GetPlugCategory(int nEffect)
			{ if (GetAt(nEffect)) return GetAt(nEffect)->GetPlugCategory(); else return 0; }
			long GetCurrentPosition(int nEffect)
			{ if (GetAt(nEffect)) return GetAt(nEffect)->GetCurrentPosition(); else return 0; }
			long GetDestinationBuffer(int nEffect)
			{ if (GetAt(nEffect)) return GetAt(nEffect)->GetDestinationBuffer(); else return 0; }
			long OfflineNotify(int nEffect, VstAudioFile* ptr, long numAudioFiles, bool start)
			{ if (GetAt(nEffect)) return GetAt(nEffect)->OfflineNotify(ptr, numAudioFiles, start); else return 0; }
			long OfflinePrepare(int nEffect, VstOfflineTask *ptr, long count)
			{ if (GetAt(nEffect)) return GetAt(nEffect)->OfflinePrepare(ptr, count); else return 0; }
			long OfflineRun(int nEffect, VstOfflineTask *ptr, long count)
			{ if (GetAt(nEffect)) return GetAt(nEffect)->OfflineRun(ptr, count); else return 0; }
			long ProcessVarIo(int nEffect, VstVariableIo* varIo)
			{ if (GetAt(nEffect)) return GetAt(nEffect)->ProcessVarIo(varIo); else return 0; }
			long SetSpeakerArrangement(int nEffect, VstSpeakerArrangement* pluginInput, VstSpeakerArrangement* pluginOutput)
			{ if (GetAt(nEffect)) return GetAt(nEffect)->SetSpeakerArrangement(pluginInput, pluginOutput); else return 0; }
			long SetBlockSizeAndSampleRate(int nEffect, long blockSize, float sampleRate)
			{ if (GetAt(nEffect)) return GetAt(nEffect)->SetBlockSizeAndSampleRate(blockSize, sampleRate); else return 0; }
			long SetBypass(int nEffect, bool onOff)
			{ if (GetAt(nEffect)) return GetAt(nEffect)->SetBypass(onOff); else return 0; }
			long GetEffectName(int nEffect, char *ptr)
			{ if (GetAt(nEffect)) return GetAt(nEffect)->GetEffectName(ptr); else return 0; }
			long GetErrorText(int nEffect, char *ptr)
			{ if (GetAt(nEffect)) return GetAt(nEffect)->GetErrorText(ptr); else return 0; }
			long GetVendorString(int nEffect, char *ptr)
			{ if (GetAt(nEffect)) return GetAt(nEffect)->GetVendorString(ptr); else return 0; }
			long GetProductString(int nEffect, char *ptr)
			{ if (GetAt(nEffect)) return GetAt(nEffect)->GetProductString(ptr); else return 0; }
			long GetVendorVersion(int nEffect)
			{ if (GetAt(nEffect)) return GetAt(nEffect)->GetVendorVersion(); else return 0; }
			long VendorSpecific(int nEffect, long index, long value, void *ptr, float opt)
			{ if (GetAt(nEffect)) return GetAt(nEffect)->VendorSpecific(index, value, ptr, opt); else return 0; }
			long CanDo(int nEffect, const char *ptr)
			{ if (GetAt(nEffect)) return GetAt(nEffect)->CanDo(ptr); else return 0; }
			long GetTailSize(int nEffect)
			{ if (GetAt(nEffect)) return GetAt(nEffect)->GetTailSize(); else return 0; }
			long Idle(int nEffect)
			{ if (GetAt(nEffect)) return GetAt(nEffect)->Idle(); else return 0; }
			long GetIcon(int nEffect)
			{ if (GetAt(nEffect)) return GetAt(nEffect)->GetIcon(); else return 0; }
			long SetViewPosition(int nEffect, long x, long y)
			{ if (GetAt(nEffect)) return GetAt(nEffect)->SetViewPosition(x, y); else return 0; }
			long GetParameterProperties(int nEffect, long index, VstParameterProperties* ptr)
			{ if (GetAt(nEffect)) return GetAt(nEffect)->GetParameterProperties(index, ptr); else return 0; }
			long KeysRequired(int nEffect)
			{ if (GetAt(nEffect)) return GetAt(nEffect)->KeysRequired(); else return 0; }
			long GetVstVersion(int nEffect)
			{ if (GetAt(nEffect)) return GetAt(nEffect)->GetVstVersion(); else return 0; }
			/* VST 2.1 extensions                */
			long KeyDown(int nEffect, VstKeyCode &keyCode)
			{ if (GetAt(nEffect)) return GetAt(nEffect)->KeyDown(keyCode); else return 0; }
			long KeyUp(int nEffect, VstKeyCode &keyCode)
			{ if (GetAt(nEffect)) return GetAt(nEffect)->KeyUp(keyCode); else return 0; }
			void SetKnobMode(int nEffect, long value)
			{ if (GetAt(nEffect)) GetAt(nEffect)->SetKnobMode(value); }
			long GetMidiProgramName(int nEffect, long channel, MidiProgramName* midiProgramName)
			{ if (GetAt(nEffect)) return GetAt(nEffect)->GetMidiProgramName(channel, midiProgramName); else return 0; }
			long GetCurrentMidiProgram(int nEffect, long channel, MidiProgramName* currentProgram)
			{ if (GetAt(nEffect)) return GetAt(nEffect)->GetCurrentMidiProgram(channel, currentProgram); else return 0; }
			long GetMidiProgramCategory(int nEffect, long channel, MidiProgramCategory* category)
			{ if (GetAt(nEffect)) return GetAt(nEffect)->GetMidiProgramCategory(channel, category); else return 0; }
			long HasMidiProgramsChanged(int nEffect, long channel)
			{ if (GetAt(nEffect)) return GetAt(nEffect)->HasMidiProgramsChanged(channel); else return 0; }
			long GetMidiKeyName(int nEffect, long channel, MidiKeyName* keyName)
			{ if (GetAt(nEffect)) return GetAt(nEffect)->GetMidiKeyName(channel, keyName); else return 0; }
			long BeginSetProgram(int nEffect)
			{ if (GetAt(nEffect)) return GetAt(nEffect)->BeginSetProgram(); else return 0; }
			long EndSetProgram(int nEffect)
			{ if (GetAt(nEffect)) return GetAt(nEffect)->BeginSetProgram(); else return 0; }
			/* VST 2.3 Extensions                */
			long GetSpeakerArrangement(int nEffect, VstSpeakerArrangement** pluginInput, VstSpeakerArrangement** pluginOutput)
			{ if (GetAt(nEffect)) return GetAt(nEffect)->GetSpeakerArrangement(pluginInput, pluginOutput); else return 0; }
			long SetTotalSampleToProcess(int nEffect, long value)
			{ if (GetAt(nEffect)) return GetAt(nEffect)->SetTotalSampleToProcess(value); else return 0; }
			long GetNextShellPlugin(int nEffect, char *name)
			{ if (GetAt(nEffect)) return GetAt(nEffect)->GetNextShellPlugin(name); else return 0; }
			long StartProcess(int nEffect)
			{ if (GetAt(nEffect)) return GetAt(nEffect)->StartProcess(); else return 0; }
			long StopProcess(int nEffect)
			{ if (GetAt(nEffect)) return GetAt(nEffect)->StopProcess(); else return 0; }
			long SetPanLaw(int nEffect, long type, float val)
			{ if (GetAt(nEffect)) return GetAt(nEffect)->SetPanLaw(type, val); else return 0; }
			long BeginLoadBank(int nEffect, VstPatchChunkInfo* ptr)
			{ if (GetAt(nEffect)) return GetAt(nEffect)->BeginLoadBank(ptr); else return 0; }
			long BeginLoadProgram(int nEffect, VstPatchChunkInfo* ptr)
			{ if (GetAt(nEffect)) return GetAt(nEffect)->BeginLoadProgram(ptr); else return 0; }

			// overridable functions
		public:
			virtual CEffect * CreateEffect() { return new CEffect(this); }
			virtual void SetSampleRate(float fSampleRate=44100.);
			virtual void SetBlockSize(long lSize=1024);
			virtual void Process(float **inputs, float **outputs, long sampleframes);
			virtual void ProcessReplacing(float **inputs, float **outputs, long sampleframes);

			virtual bool OnGetVendorString(char *text) { strcpy(text, "Seib"); return true; } // forgive this little vanity :-)
			virtual long OnGetHostVendorVersion() { return 1; }
			virtual bool OnGetProductString(char *text) { strcpy(text, "Default CVSTHost"); return true; }
			virtual bool OnGetSpeakerArrangement(int nEffect, VstSpeakerArrangement* pluginInput, VstSpeakerArrangement* pluginOutput) { return false; }
			virtual void OnSetOutputSampleRate(int nEffect, float sampleRate) { }
			virtual bool OnOfflineStart(int nEffect, VstAudioFile* audioFiles, long numAudioFiles, long numNewAudioFiles) { return false; }
			virtual bool OnOfflineRead(int nEffect, VstOfflineTask* offline, VstOfflineOption option, bool readSource) { return false; }
			virtual bool OnOfflineWrite(int nEffect, VstOfflineTask* offline, VstOfflineOption option) { return false; }
			virtual long OnOfflineGetCurrentPass(int nEffect) { return 0; }
			virtual long OnOfflineGetCurrentMetaPass(int nEffect) { return 0; }
			virtual long OnGetAutomationState(int nEffect) { return 0; }
			virtual long OnGetCurrentProcessLevel(int nEffect) { return 0; }
			virtual bool OnWillProcessReplacing(int nEffect) { return false; }
			virtual long OnGetOutputLatency(int nEffect) { return 0; }
			virtual long OnGetInputLatency(int nEffect) { return 0; }
			virtual long OnUpdateBlockSize(int nEffect);
			virtual long OnTempoAt(int nEffect, long pos) { return 0; }
			virtual long OnUpdateSampleRate(int nEffect);
			virtual bool OnSizeWindow(int nEffect, long width, long height);
			virtual bool OnNeedIdle(int nEffect);
			virtual long OnAudioMasterCallback(int nEffect, long opcode, long index, long value, void *ptr, float opt);
			virtual long OnGetVersion(int nEffect);
			virtual bool OnCanDo(const char *ptr);
			virtual bool OnWantEvents(int nEffect, long filter);
			virtual long OnIdle(int nEffect=-1);
			virtual bool OnInputConnected(int nEffect, long input) { return true; }
			virtual bool OnOutputConnected(int nEffect, long output) { return true; }
			virtual bool OnSetParameterAutomated(int nEffect, long index, float value) { return false; }
			virtual bool OnProcessEvents(int nEffect, VstEvents* events) { return false; }
			virtual VstTimeInfo *OnGetTime(int nEffect, long lMask) { return &vstTimeInfo; }
			virtual bool OnSetTime(int nEffect, long filter, VstTimeInfo *timeInfo) { return false; }
			virtual long OnGetNumAutomatableParameters(int nEffect) { return 0; }
			virtual long OnGetParameterQuantization(int nEffect) { return 0x40000000; }
			virtual bool OnIoChanged(int nEffect) { return false; }
			virtual long OnHostVendorSpecific(int nEffect, long lArg1, long lArg2, void* ptrArg, float floatArg) { return 0; }
			virtual long OnGetHostLanguage() { return 0; }
			virtual void * OnOpenWindow(int nEffect, VstWindow* window);
			virtual bool OnCloseWindow(int nEffect, VstWindow* window);
			virtual void * OnGetDirectory(int nEffect);
			virtual bool OnUpdateDisplay(int nEffect);
			// VST 2.1 Extensions
			virtual bool OnBeginEdit(int nEffect) { return false; }
			virtual bool OnEndEdit(int nEffect) { return false; }
			virtual bool OnOpenFileSelector (int nEffect, VstFileSelect *ptr) { return false; }
			// VST 2.2 Extensions
			virtual bool OnCloseFileSelector (int nEffect, VstFileSelect *ptr) { return false; }
			virtual bool OnEditFile(int nEffect, char *ptr) { return false; }
			virtual bool OnGetChunkFile(int nEffect, void * nativePath) { return false; }
			// VST 2.3 Extensions
			virtual VstSpeakerArrangement *OnGetInputSpeakerArrangement(int nEffect) { return 0; }
		};
	}
}