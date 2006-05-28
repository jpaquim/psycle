/*****************************************************************************/
/* CVSTHost.hpp: interface for the CVSTHost class.							 */
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

#pragma once
#define VST_FORCE_DEPRECATED 0
#include <vst/AEffectx.h>               /* VST header files                  */
#include <vst/vstfxstore.h>
//#include <vst/AEffEditor.h>

namespace seib {
	namespace vst {
		/*****************************************************************************/
		/* CFxBase : base class for FX Bank / Program Files                          */
		/*****************************************************************************/

		class CFxBase
		{
		public:
			CFxBase();

		protected:
			static bool NeedsBSwap;
			static int FxSetVersion;
		protected:
			void SwapBytes(float &f);
			void SwapBytes(VstInt32 &l);
			void SwapBytes(long &l);

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
			long GetVersion() { if (!bBank) return 0; return ((fxBank*)bBank)->version; }
			long GetFxID() { if (!bBank) return 0; return ((fxBank*)bBank)->fxID; }
			void SetFxID(long id) { if (bBank) ((fxBank*)bBank)->fxID = id; if (!bChunk) for (int i = GetNumPrograms() -1; i >= 0; i--) GetProgram(i)->fxID = id; }
			long GetFxVersion() { if (!bBank) return 0; return ((fxBank*)bBank)->fxVersion; }
			void SetFxVersion(long v) { if (bBank) ((fxBank*)bBank)->fxVersion = v; if (!bChunk) for (int i = GetNumPrograms() -1; i >= 0; i--) GetProgram(i)->fxVersion = v; }
			long GetNumPrograms() { if (!bBank) return 0; return ((fxBank*)bBank)->numPrograms; }
			long GetNumParams() { if (bChunk) return 0; return GetProgram(0)->numParams; }
			long GetChunkSize() { if (!bChunk) return 0; return ((fxBank *)bBank)->content.data.size; }
			void *GetChunk() { if (!bChunk) return 0; return ((fxBank *)bBank)->content.data.chunk; }
			bool SetChunk(void *chunk) { if (!bChunk) return false; memcpy(((fxBank *)bBank)->content.data.chunk, chunk, ((fxBank *)bBank)->content.data.size); return true; }

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
				return p->content.params[nParm];
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
				p->content.params[nParm] = val;
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
		/* LoadedAEffect:															 */
		/*		Struct definition to ease  CEffect creation/destruction				 */
		/* Sometimes it might be preferable to create the CEffect *after* the        */
		/* AEffect has been loaded, in order to have different subclasses for        */
		/* different types. Yet, it might be necessary that the plugin frees the     */
		/* library once it is destroyed. This is why it requires this information	 */
		/*****************************************************************************/
		class CVSTHost;

		typedef struct LoadedAEffect LoadedAEffect;
		struct LoadedAEffect {
			AEffect *aEffect;
			CVSTHost *pHost;
#ifdef WIN32
			char *sFileName;
			HMODULE hModule;
#endif
		};

		/*****************************************************************************/
		/* CEffect : class definition for audio effect objects                       */
		/*****************************************************************************/
		class CEffect
		{
		public:
			CEffect(LoadedAEffect &loadstruct);
			virtual ~CEffect();
		protected:
			virtual void Load(LoadedAEffect &loadstruct);
			virtual void Unload();

		protected:
			AEffect *aEffect;
			bool bEditOpen;
			bool bNeedIdle;
			bool bWantMidi;
		#ifdef WIN32
			HMODULE hModule;
			char *sFileName;
			char *sDir;
		#elif MAC
			// yet to do
			// no idea how things look here...
		#endif

		public:
			virtual bool LoadBank(const char *name);
			virtual bool SaveBank(const char *name);
			virtual void WantsMidi(bool enable) { bWantMidi=enable; }
			virtual void NeedsIdle(bool enable) { bNeedIdle=enable; }
			// Not to be used, except if no other way.
			AEffect	*GetAEffect() { return aEffect; }
			//////////////////////////////////////////////////////////////////////////
			// AEffect Properties
			// magic is only used in the loader to verify it is a VST plugin
			//long int magic()
			long int numPrograms()	{	if (!aEffect)	throw (int)1;	return aEffect->numPrograms;	}
			long int numParams()	{	if (!aEffect)	throw (int)1;	return aEffect->numParams;		}
			long int numInputs()	{	if (!aEffect)	throw (int)1;	return aEffect->numInputs;		}
			long int numOutputs()	{	if (!aEffect)	throw (int)1;	return aEffect->numOutputs;		}
			//flags
			bool HasEditor()		{	if (!aEffect)	throw (int)1;	return aEffect->flags & effFlagsHasEditor;			}
			bool HasClip()			{	if (!aEffect)	throw (int)1;	return aEffect->flags & effFlagsHasClip;			}
			bool HasVu()			{	if (!aEffect)	throw (int)1;	return aEffect->flags & effFlagsHasVu;				}
			bool CanInputMono()		{	if (!aEffect)	throw (int)1;	return aEffect->flags & effFlagsCanMono;			}
			bool CanProcessReplace(){	if (!aEffect)	throw (int)1;	return aEffect->flags & effFlagsCanReplacing;		}
			bool ProgramIsChunk()	{	if (!aEffect)	throw (int)1;	return aEffect->flags & effFlagsProgramChunks;		}
			bool IsSynth()			{	if (!aEffect)	throw (int)1;	return aEffect->flags & effFlagsIsSynth;			}
			bool HasNoTail()		{	if (!aEffect)	throw (int)1;	return aEffect->flags & effFlagsNoSoundInStop;		}
			bool ExternalAsync()	{	if (!aEffect)	throw (int)1;	return aEffect->flags & effFlagsExtIsAsync;			}
			bool ExternalBuffer()	{	if (!aEffect)	throw (int)1;	return aEffect->flags & effFlagsExtHasBuffer;		}
			// the real plugin ID.
			long int uniqueId()		{	if (!aEffect)	throw (int)1;	return aEffect->uniqueID;		}
			// version() was the plugin version in VST1.0, but it is unused in VST2, in favour of GetVendorVersion()
			//long int version();
			long int initialDelay() {	if (!aEffect)	throw (int)1;	return aEffect->initialDelay;	}

		protected:
			virtual long Dispatch(long opCode, long index=0, long value=0, void *ptr=0, float opt=0.);
		public:
			//////////////////////////////////////////////////////////////////////////
			// plugin dispatch functions
			inline void Open() { Dispatch(effOpen); }
		protected:
			// Warning! After a "Close()", the "AEffect" is deleted and the plugin cannot be used again. (see audioeffect.cpp)
			inline void Close() { Dispatch(effClose); }
		public:
			inline void SetProgram(long lValue) { Dispatch(effSetProgram, 0, lValue); }
			// returns the index of the program. Zero based.
			inline long GetProgram() { return Dispatch(effGetProgram); }
			// size of ptr string limited to 24char + \0 delimiter.
			inline void SetProgramName(char *ptr) { Dispatch(effSetProgramName, 0, 0, ptr); }
			inline void GetProgramName(char *ptr) { Dispatch(effGetProgramName, 0, 0, ptr); }
			// Unit of the paramter. size of ptr string limited to 8char + \0 delimiter
			inline void GetParamLabel(long index, char *ptr) { Dispatch(effGetParamLabel, index, 0, ptr); }
			// Value of the parameter. no provision of size. Use 24char + \0 delimiter for safety.
			inline void GetParamDisplay(long index, char *ptr) { Dispatch(effGetParamDisplay, index, 0, ptr); }
			// Name of the parameter. size of ptr string limited to 8char + \0 delimiter (might be not followed by plugin devs)
			inline void GetParamName(long index, char *ptr) { Dispatch(effGetParamName, index, 0, ptr); }
			inline void SetSampleRate(float fSampleRate) { Dispatch(effSetSampleRate, 0, 0, 0, fSampleRate); }
			inline void SetBlockSize(long value) { Dispatch(effSetBlockSize, 0, value); }
			inline void MainsChanged(bool bOn) { Dispatch(effMainsChanged, 0, bOn); }
			inline float GetVu() { return (float)Dispatch(effGetVu) / (float)32767.; }
			inline long EditGetRect(ERect **ptr) { return Dispatch(effEditGetRect, 0, 0, ptr); }
			// return value is true (succeeded) or false.
			inline long EditOpen(void *ptr) { long l = Dispatch(effEditOpen, 0, 0, ptr); if (l > 0) bEditOpen = true; return l; }
			inline void EditClose() { Dispatch(effEditClose); bEditOpen = false; }
			// This has to be called repeatedly from the idle process ( usually the UI thread, with idle priority )
			// The plugins usually have checks so that it skips the call if no update is required.
			inline void EditIdle() { if (bEditOpen) Dispatch(effEditIdle); }
		#if MAC
			inline void EditDraw(void *rectarea) { Dispatch(effEditDraw, 0, 0, rectarea); }
			inline long EditMouse(long x, long y) { return Dispatch(effEditMouse, x, y); }
			inline long EditKey(long value) { return Dispatch(effEditKey, 0, value); }
			inline void EditTop() { Dispatch(effEditTop); }
			inline void EditSleep() { Dispatch(effEditSleep); }
		#endif
			// 2nd check that it is a valid VST. (appart from kEffectMagic )
			inline bool Identify() { return (Dispatch(effIdentify) == CCONST ('N', 'v', 'E', 'f')); }
			// returns "byteSize".
			inline long GetChunk(void **ptr, bool isPreset = false) { return Dispatch(effGetChunk, isPreset, 0, ptr); }
			//\todo: If return value is zero, it is not supported. It might return byteSize when correct, but I am unsure.
			inline long SetChunk(void *data, long byteSize, bool isPreset = false) { return Dispatch(effSetChunk, isPreset, byteSize, data); }
		// VST 2.0
			inline long ProcessEvents(VstEvents* ptr) { return Dispatch(effProcessEvents, 0, 0, ptr); }
			inline bool CanBeAutomated(long index) { return (bool)Dispatch(effCanBeAutomated, index); }
			// A textual description of the parameter's value. A null pointer is used to check the capability (return true).
			inline bool String2Parameter(long index, char *text) { return (bool)Dispatch(effString2Parameter, index, 0, text); }
			inline long GetNumProgramCategories() { return Dispatch(effGetNumProgramCategories); }
			// text is a string up to 24 chars + \0 delimiter
			inline bool GetProgramNameIndexed(long category, long index, char* text) { return (bool)Dispatch(effGetProgramNameIndexed, index, category, text); }
			// copy current program to the one in index.
			inline bool CopyProgram(long index) { return (bool)Dispatch(effCopyProgram, index); }
			//Input index has been (dis-)connected. The application may issue this call when implemented.
			inline void ConnectInput(long index, bool state) { Dispatch(effConnectInput, index, state); }
			//Output index has been (dis-)connected. The application may issue this call when implemented.
			inline void ConnectOutput(long index, bool state) { Dispatch(effConnectOutput, index, state); }
			inline bool GetInputProperties(long index, VstPinProperties *ptr) { return (bool)Dispatch(effGetInputProperties, index, 0, ptr); }
			inline bool GetOutputProperties(long index, VstPinProperties *ptr) { return (bool)Dispatch(effGetOutputProperties, index, 0, ptr); }
			inline long GetPlugCategory() { return Dispatch(effGetPlugCategory); }
			// get position of dsp buffer. (to verify that it is "on time")
			inline long GetCurrentPosition() { return Dispatch(effGetCurrentPosition); }
			// get the address of the dsp buffer.
			inline float* GetDestinationBuffer() { return (float*)Dispatch(effGetDestinationBuffer); }
			inline bool OfflineNotify(VstAudioFile* ptr, long numAudioFiles, bool start) { return (bool)Dispatch(effOfflineNotify, start, numAudioFiles, ptr); }
			inline bool OfflinePrepare(VstOfflineTask *ptr, long count) { return (bool)Dispatch(effOfflinePrepare, 0, count, ptr); }
			inline bool OfflineRun(VstOfflineTask *ptr, long count) { return (bool)Dispatch(effOfflineRun, 0, count, ptr); }
			//\todo: get more information about this function, and its relation with process and processReplacing.
			inline bool ProcessVarIo(VstVariableIo* varIo) { return (bool)Dispatch(effProcessVarIo, 0, 0, varIo); }
			inline bool SetSpeakerArrangement(VstSpeakerArrangement* pluginInput, VstSpeakerArrangement* pluginOutput) { return (bool)Dispatch(effSetSpeakerArrangement, 0, (long)pluginInput, pluginOutput); }
			inline void SetBlockSizeAndSampleRate(long blockSize, float sampleRate) { Dispatch(effSetBlockSizeAndSampleRate, 0, blockSize, 0, sampleRate); }
			inline bool SetBypass(bool onOff) { return (bool)Dispatch(effSetBypass, 0, onOff); }
			// ptr is a string up to 32chars + \0 delimiter
			inline bool GetEffectName(char *ptr) { return (bool)Dispatch(effGetEffectName, 0, 0, ptr); }
			// ptr is a string up to 256chars + \0 delimiter
			inline bool GetErrorText(char *ptr) { return (bool)Dispatch(effGetErrorText, 0, 0, ptr); }
			// ptr is a string up to 64chars + \0 delimiter
			inline bool GetVendorString(char *ptr) { return (bool)Dispatch(effGetVendorString, 0, 0, ptr); }
			// ptr is a string up to 64chars + \0 delimiter
			inline bool GetProductString(char *ptr) { return (bool)Dispatch(effGetProductString, 0, 0, ptr); }
			inline long GetVendorVersion() { return Dispatch(effGetVendorVersion); }
			inline long VendorSpecific(long index, long value, void *ptr, float opt) { return Dispatch(effVendorSpecific, index, value, ptr, opt); }
			//returns 0 -> don't know, 1 -> yes, -1 -> no.
			inline long CanDo(const char *ptr) { return Dispatch(effCanDo, 0, 0, (void *)ptr); }
			inline long GetTailSize() { return Dispatch(effGetTailSize); }
			// "returns 0 by default"  ???
			inline long Idle() { if (bNeedIdle) return Dispatch(effIdle); else return 0; }
			inline long GetIcon() { return Dispatch(effGetIcon); }
			inline long SetViewPosition(long x, long y) { return Dispatch(effSetViewPosition, x, y); }
			inline long GetParameterProperties(long index, VstParameterProperties* ptr) { return Dispatch(effGetParameterProperties, index, 0, ptr); }
			// Seems something related to MAC ( to be used with editkey )
			inline bool KeysRequired() { return (bool)Dispatch(effKeysRequired); }
			inline long GetVstVersion() { return Dispatch(effGetVstVersion); }
		// VST 2.1 extensions
			inline long KeyDown(VstKeyCode &keyCode) { return Dispatch(effEditKeyDown, keyCode.character, keyCode.virt, 0, keyCode.modifier); }
			inline long KeyUp(VstKeyCode &keyCode) { return Dispatch(effEditKeyUp, keyCode.character, keyCode.virt, 0, keyCode.modifier); }
			inline void SetKnobMode(long value) { Dispatch(effSetEditKnobMode, 0, value); }
			inline long GetMidiProgramName(long channel, MidiProgramName* midiProgramName) { return Dispatch(effGetMidiProgramName, channel, 0, midiProgramName); }
			inline long GetCurrentMidiProgram (long channel, MidiProgramName* currentProgram) { return Dispatch(effGetCurrentMidiProgram, channel, 0, currentProgram); }
			inline long GetMidiProgramCategory (long channel, MidiProgramCategory* category) { return Dispatch(effGetMidiProgramCategory, channel, 0, category); }
			inline long HasMidiProgramsChanged (long channel) { return Dispatch(effHasMidiProgramsChanged, channel); }
			inline long GetMidiKeyName(long channel, MidiKeyName* keyName) { return Dispatch(effGetMidiKeyName, channel, 0, keyName); }
			inline bool BeginSetProgram() { return (bool)Dispatch(effBeginSetProgram); }
			inline bool EndSetProgram() { return (bool)Dispatch(effEndSetProgram); }
		// VST 2.3 Extensions
			inline bool GetSpeakerArrangement(VstSpeakerArrangement** pluginInput, VstSpeakerArrangement** pluginOutput) { return (bool)Dispatch(effGetSpeakerArrangement, 0, (long)pluginInput, pluginOutput); }
			//Called in offline (non RealTime) processing before process is called, indicates how many samples will be processed.	Actually returns value.
			inline long SetTotalSampleToProcess (long value) { return Dispatch(effSetTotalSampleToProcess, 0, value); }
			//Points to a char buffer of size 64, which is to be filled with the name of the plugin including the terminating 0.
			inline long GetNextShellPlugin(char *name) { return Dispatch(effShellGetNextPlugin, 0, 0, name); }
			//Called one time before the start of process call.
			inline long StartProcess() { return Dispatch(effStartProcess); }
			inline long StopProcess() { return Dispatch(effStopProcess); }
			inline bool SetPanLaw(long type, float val) { return (bool)Dispatch(effSetPanLaw, 0, type, 0, val); }
			//	0 : 	Not implemented.
			//	1 : 	The bank/program can be loaded.
			//-1  : 	The bank/program can't be loaded.
			inline long BeginLoadBank(VstPatchChunkInfo* ptr) { return Dispatch(effBeginLoadBank, 0, 0, ptr); }
			inline long BeginLoadProgram(VstPatchChunkInfo* ptr) { return Dispatch(effBeginLoadProgram, 0, 0, ptr); }

			// overridables
		public:
			virtual void EnterCritical(){;}
			virtual void LeaveCritical(){;}

			virtual void Process(float **inputs, float **outputs, long sampleframes);
			virtual void ProcessReplacing(float **inputs, float **outputs, long sampleframes);
			virtual void SetParameter(long index, float parameter);
			virtual float GetParameter(long index);

			virtual void * OnGetDirectory();
			virtual bool OnSizeEditorWindow(long width, long height) { return false; }
			virtual bool OnUpdateDisplay() { return false; }
			virtual void * OnOpenWindow(VstWindow* window) { return 0; }
			virtual bool OnCloseWindow(VstWindow* window) { return false; }
			virtual bool IsInputConnected(int input) { return true; }
			virtual bool IsOutputConnected(int input) { return true; }
			// AEffect asks host about its input/outputspeakers.
			virtual VstSpeakerArrangement* OnHostInputSpeakerArrangement() { return 0; }
			virtual VstSpeakerArrangement* OnHostOutputSpeakerArrangement() { return 0; }
			// AEffect informs of changed IO. verify numins/outs, speakerarrangement and the likes.
			virtual bool OnIOChanged() { return false; }

		};

		/*****************************************************************************/
		/* CVSTHost class declaration                                                */
		/*****************************************************************************/

		class CVSTHost
		{
			typedef AEffect * (* PVSTMAIN) (audioMasterCallback audioMaster);
		public:
			CVSTHost();
			virtual ~CVSTHost();

		protected:
			VstTimeInfo vstTimeInfo;
			long lBlockSize;

			static CVSTHost * pHost;
			static int quantization;

			CEffect *GetPreviousPlugIn(CEffect &pEffect,int pinIndex){};
			CEffect *GetNextPlugIn(CEffect &pEffect, int pinIndex){};
		public:
			CEffect* CVSTHost::LoadPlugin(const char * sName);

		protected:
			static VstIntPtr* VSTCALLBACK AudioMasterCallback (AEffect* effect, VstInt32 opcode, VstInt32 index, VstIntPtr value, void* ptr, float opt);
//			static long VSTCALLBACK AudioMasterCallback(AEffect *effect, long opcode, long index, long value, void *ptr, float opt);

			// overridable functions
		public:
			virtual CEffect * CreateEffect(LoadedAEffect &loadstruct) { return new CEffect(loadstruct); }
			virtual void CalcTimeInfo(long lMask = -1);
			virtual void SetSampleRate(float fSampleRate=44100.);
			virtual void SetBlockSize(long lSize=1024);
			virtual float GetSampleRate() { return vstTimeInfo.sampleRate; }
			virtual long GetBlockSize() { return lBlockSize; }
			virtual void SetTimeSignature(long numerator, long denominator)
				{
					//\todo : inform of the change? ( kVstTranportChanged )
					vstTimeInfo.timeSigNumerator=numerator;
					vstTimeInfo.timeSigDenominator=denominator; 
					vstTimeInfo.flags |= kVstTimeSigValid;
				}

			// text is a string up to 64 chars + \0 delimiter
			virtual bool OnGetVendorString(char *text) { strcpy(text, "Seib-Psycledelics"); return true; } // forgive this little vanity :-)
			// text is a string up to 64 chars + \0 delimiter
			virtual bool OnGetProductString(char *text) { strcpy(text, "Default CVSTHost."); return true; }
			virtual long OnGetHostVendorVersion() { return 1000; }
			virtual long OnHostVendorSpecific(CEffect &pEffect, long lArg1, long lArg2, void* ptrArg, float floatArg) { return 0; }
			virtual long OnGetVSTVersion(CEffect &pEffect);

			// Plugin calls this function when it has changed one parameter (usually, from the GUI)in order for the host to record it.
			virtual void OnSetParameterAutomated(CEffect &pEffect, long index, float value) { return; }
			//\todo: investigate. SDK says " returns the Unique ID of the plugin currently being loaded". uniqueId as in "aEffect->uniqueId" ?
			// It might end being an old (unused now) function, since it is from VST1.0.
			virtual long OnCurrentId(CEffect &pEffect) { return 0; }
			// Call application idle routine (this will call effEditIdle for all open editors too)
			// Feedback to the host application and to call the idle's function of this editor (thru host application (MAC/WINDOWS/MOTIF)). The idle frequency is between 10Hz and 20Hz
			//\todo: From the above sentences, this call would set a flag to update all the editors in the Idle (GUI, OnPaint) thread.
			virtual void OnIdle(CEffect &pEffect);
			virtual bool OnInputConnected(CEffect &pEffect, long input) { return pEffect.IsInputConnected(input); }
			virtual bool OnOutputConnected(CEffect &pEffect, long output) { return pEffect.IsOutputConnected(output); }
			virtual void OnWantEvents(CEffect &pEffect, long filter);
			//\todo : investigate if the "flags" of vstTimeInfo should be per-plugin, instead of per-host.
			virtual VstTimeInfo *OnGetTime(CEffect &pEffect, long lMask) { CalcTimeInfo(lMask); return &vstTimeInfo; }
			virtual bool OnProcessEvents(CEffect &pEffect, VstEvents* events) { return false; }
			// aeffectx.hpp: "VstTimenfo* in <ptr>, filter in <value>, not supported". Not Implemented in the VST SDK.
			virtual bool OnSetTime(CEffect &pEffect, long filter, VstTimeInfo *timeInfo) { return false; }
			//  pos in Sample frames, return bpm* 10000
			virtual long OnTempoAt(CEffect &pEffect, long pos) { return 0; }
			virtual long OnGetNumAutomatableParameters(CEffect &pEffect) { return 0; }
			//	0 :  	Not implemented.
			//	1 : 	Full single float precision is maintained in automation.
			//other : 	The integer value that represents +1.0.
			virtual long OnGetParameterQuantization(CEffect &pEffect) { return quantization; }
			//Tell host numInputs and/or numOutputs and/or initialDelay has changed.
			// The host could call a suspend (if the plugin was enabled (in resume state)) and then ask for getSpeakerArrangement
			// and/or check the numInputs and numOutputs and initialDelay and then call a resume.
			virtual bool OnIoChanged(CEffect &pEffect) { return pEffect.OnIOChanged(); }
			//\todo: This seems to indicate that the plugin requires idle calls ( pEffect->Idle() ) continuously from the idle function.
			virtual bool OnNeedIdle(CEffect &pEffect);
			virtual bool OnSizeWindow(CEffect &pEffect, long width, long height);
			// Will cause application to call AudioEffect's  setSampleRate/setBlockSize method (when implemented).
			virtual long OnUpdateSampleRate(CEffect &pEffect){ pEffect.SetSampleRate(vstTimeInfo.sampleRate); return vstTimeInfo.sampleRate; }
			virtual long OnUpdateBlockSize(CEffect &pEffect) { pEffect.SetBlockSize(lBlockSize); return lBlockSize; }
			//	Returns the ASIO input latency values.
			virtual long OnGetInputLatency(CEffect &pEffect) { return 0; }
			// Returns the ASIO output latency values. To be used mostly for GUI sync with audio.
			virtual long OnGetOutputLatency(CEffect &pEffect) { return 0; }
			// asks the host if it will use this plugin with "processReplacing"
			virtual bool OnWillProcessReplacing(CEffect &pEffect) { return false; }
			//	0 :  	Not supported.
			//	1 : 	Currently in user thread (gui).
			//	2 : 	Currently in audio thread or irq (where process is called).
			//	3 : 	Currently in 'sequencer' thread or irq (midi, timer etc).
			//	4 : 	Currently offline processing and thus in user thread.
			//other : 	Not defined, but probably pre-empting user thread.
			virtual long OnGetCurrentProcessLevel(CEffect &pEffect) { return 0; }
			//	0 :  	Not supported.
			//	1 : 	Off.
			//	2 : 	Read.
			//	3 : 	Write.
			//	4 : 	Read/write.
			virtual long OnGetAutomationState(CEffect &pEffect) { return 0; }
			// As already seen, a single VstOfflineTask structure can be used both to read an existing file, and to overwrite it.
			// Moreover, the offline specification states that it is possible, at any time, to read both the original samples
			// and the new ones (the "overwritten" samples). This is the reason for the readSource parameter:
			// set it to true to read the original samples and to false to read the recently written samples.
			virtual bool OnOfflineRead(CEffect &pEffect, VstOfflineTask* offline, VstOfflineOption option, bool readSource = true) { return false; }
			virtual bool OnOfflineWrite(CEffect &pEffect, VstOfflineTask* offline, VstOfflineOption option) { return false; }
			// The parameter numNewAudioFiles is the number of files that the Plug-In want to create. 
			virtual bool OnOfflineStart(CEffect &pEffect, VstAudioFile* audioFiles, long numAudioFiles, long numNewAudioFiles) { return false; }
			virtual long OnOfflineGetCurrentPass(CEffect &pEffect) { return 0; }
			virtual long OnOfflineGetCurrentMetaPass(CEffect &pEffect) { return 0; }
			// Used for variable I/O processing.
			virtual void OnSetOutputSampleRate(CEffect &pEffect, float sampleRate) { }
			virtual VstSpeakerArrangement* OnGetOutputSpeakerArrangement(CEffect &pEffect) { return pEffect.OnHostOutputSpeakerArrangement(); }
			// Specification says 0 -> don't know, 1 ->yes, -1 : no, but audioeffectx.cpp says "!= 0 -> true", and since plugins use audioeffectx...
			virtual bool OnCanDo(CEffect &pEffect,const char *ptr);
			virtual long OnGetHostLanguage() { return 0; }
			virtual void * OnOpenWindow(CEffect &pEffect, VstWindow* window);
			virtual bool OnCloseWindow(CEffect &pEffect, VstWindow* window);
			virtual void * OnGetDirectory(CEffect &pEffect);
			//\todo: "Something has changed, update 'multi-fx' display." ???
			virtual bool OnUpdateDisplay(CEffect &pEffect);
//			virtual long OnAudioMasterCallback(CEffect &pEffect, long opcode, long index, long value, void *ptr, float opt);
			// VST 2.1 Extensions
			// Notifies that "setParameterAutoMated" is gonna be called. (once per mouse clic)
			virtual bool OnBeginEdit(CEffect &pEffect,long index) { return false; }
			virtual bool OnEndEdit(CEffect &pEffect,long index) { return false; }
			virtual bool OnOpenFileSelector (CEffect &pEffect, VstFileSelect *ptr) { return false; }
			// VST 2.2 Extensions
			virtual bool OnCloseFileSelector (CEffect &pEffect, VstFileSelect *ptr) { return false; }
			// open an editor for audio (defined by XML text in ptr)
			virtual bool OnEditFile(CEffect &pEffect, char *ptr) { return false; }
			virtual bool OnGetChunkFile(CEffect &pEffect, void * nativePath) { return false; }
			// VST 2.3 Extensions
			virtual VstSpeakerArrangement *OnGetInputSpeakerArrangement(CEffect &pEffect) { return pEffect.OnHostInputSpeakerArrangement(); }
		};
	}
}