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

		struct fxmainheader
		{
			VstInt32 chunkMagic;		///< 'CcnK'
			VstInt32 byteSize;			///< size of this chunk, excl. magic + byteSize
			VstInt32 fxMagic;			///< 'FxCk' (regular) or 'FPCh' (opaque chunk)
		};
		struct fxprogramheader
		{
			VstInt32 version;			///< format version (currently 1)
			VstInt32 fxID;				///< fx unique ID
			VstInt32 fxVersion;			///< fx version

			VstInt32 numParams;			///< number of parameters
			char prgName[28];			///< program name (null-terminated ASCII string)
		};
		struct fxbankheader
		{
			VstInt32 version;			///< format version (1 or 2)
			VstInt32 fxID;				///< fx unique ID
			VstInt32 fxVersion;			///< fx version

			VstInt32 numPrograms;		///< number of programs

			#if VST_2_4_EXTENSIONS
				VstInt32 currentProgram;	///< version 2: current program number
				char future[124];			///< reserved, should be zero
			#else
				char future[128];			///< reserved, should be zero
			#endif
		};

		/*****************************************************************************/
		/* CFxBase : base class for FX Bank / Program Files                          */
		/*****************************************************************************/

		class CFxBase
		{
		public:
			CFxBase();
		public:
			virtual bool Load(const char *pszFile) { return false; }
			virtual bool Save(const char *pszFile) { return false; }

		protected:
			static bool NeedsBSwap;
			static int FxSetVersion;
		protected:
			void SwapBytes(float &f);
			void SwapBytes(VstInt32 &l);
			void SwapBytes(long &l);

		};
		/*****************************************************************************/
		/* CFxProgram : class for an .fxp (Program) file                             */
		/*****************************************************************************/

		class CFxProgram : public CFxBase
		{
		public:
			// Create program from a .fxp file.
			CFxProgram(const char *pszFile = 0) { Init(); 	if (pszFile)  Load(pszFile); }
			// Create empty instance with size (bChunk=false) parameters or chunk of specified size (bChunk=true)
			CFxProgram(int size, bool bChunk=false) { Init(); if (bChunk) SetChunkSize(size);	else SetParamSize(size); }
			//Copy constructor.
			CFxProgram(CFxProgram const &org){ Init(); DoCopy(org); }
			virtual ~CFxProgram();
			CFxProgram & operator=(CFxProgram const &org) { return DoCopy(org); }
		public:
			virtual bool Load(const char *pszFile);
			virtual bool Save(const char *pszFile);

			bool SetParamSize(int nParams);
			bool SetChunkSize(int nChunkSize);
			bool IsChunk() { return !!pChunk; }
			// access functions
		public:
			long GetVersion() { return program.version; }
			long GetFxID() {  return program.fxID; }
			void SetFxID(long id) {  program.fxID = id;  }
			long GetFxVersion() { return program.fxVersion; }
			void SetFxVersion(long v) { program.fxVersion = v; }
			long GetNumParams() { if (pChunk) return 0; return program.numParams; }
			long GetChunkSize() { if (!pChunk) return 0; return lChunkSize; }
			void *GetChunk() { if (!pChunk) return 0; return pChunk; }
			bool SetChunk(void *chunk);

			char * GetProgramName()	{ return program.prgName;	}
			void SetProgramName(const char *name = "")
			{
				std::strncpy(program.prgName, name, sizeof(program.prgName));
				program.prgName[sizeof(program.prgName)-1] = '\0';
			}
			float GetParameter(int nParm)
			{
				if (!pParams) return 0;
				return pParams[nParm];
			}
			bool SetParameter(int nParm, float val = 0.0)
			{
				if (!pParams) return false;
				if (nParm > program.numParams) return false;
				if (val < 0.0)
					val = 0.0;
				if (val > 1.0)
					val = 1.0;
				pParams[nParm] = val;
				return true;
			}

		protected:
			char szFileName[256];
			fxprogramheader program;
			float* pParams;
			unsigned char* pChunk;
			int lChunkSize;

		protected:
			void Init();
			CFxProgram & DoCopy(CFxProgram const &org);
			void FreeMemory();

		};


		/*****************************************************************************/
		/* CFxBank : class for an .fxb (Bank) file                                   */
		/*****************************************************************************/

		class CFxBank : public CFxBase
		{
		public:
			CFxBank(const char *pszFile = 0);
			CFxBank(int nPrograms, int nParams);
			CFxBank(int nChunkSize);
			CFxBank(CFxBank const &org) { DoCopy(org); }
			virtual ~CFxBank();
			CFxBank & operator=(CFxBank const &org) { return DoCopy(org); }
		public:
			virtual bool Load(const char *pszFile);
			virtual bool Save(const char *pszFile);
			virtual void Unload();
			virtual bool IsLoaded() { return !!bBank; }

			bool IsChunk() { return bChunk; }
			bool SetSize(int nPrograms, int nParams);
			bool SetSize(int nChunkSize);

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

		//-------------------------------------------------------------------------------------------------------
		// PluginLoader (From VST SDK 2.4 "minihost.cpp")
		//-------------------------------------------------------------------------------------------------------
		typedef AEffect* (*PluginEntryProc) (audioMasterCallback audioMaster);

		struct PluginLoader
		{
			void* module;

			PluginLoader ()
				: module (0)
			{}

			~PluginLoader ()
			{
				if(module)
				{
				#if _WIN32
					FreeLibrary ((HMODULE)module);
				#elif TARGET_API_MAC_CARBON
					CFBundleUnloadExecutable ((CFBundleRef)module);
					CFRelease ((CFBundleRef)module);
				#endif
				}
			}

			bool loadLibrary (const char* fileName)
			{
			#if _WIN32
				module = LoadLibrary (fileName);
			#elif TARGET_API_MAC_CARBON
				CFStringRef fileNameString = CFStringCreateWithCString (NULL, fileName, kCFStringEncodingUTF8);
				if (fileNameString == 0)
					return false;
				CFURLRef url = CFURLCreateWithFileSystemPath (NULL, fileNameString, kCFURLPOSIXPathStyle, false);
				CFRelease (fileNameString);
				if (url == 0)
					return false;
				module = CFBundleCreate (NULL, url);
				CFRelease (url);
				if (module && CFBundleLoadExecutable ((CFBundleRef)module) == false)
					return false;
			#endif
				return module != 0;
			}

			PluginEntryProc getMainEntry ()
			{
				PluginEntryProc mainProc = 0;
			#if _WIN32
				mainProc = (PluginEntryProc)GetProcAddress ((HMODULE)module, "VSTPluginMain");
				if(!mainProc)
					mainProc = (PluginEntryProc)GetProcAddress ((HMODULE)module, "main");
			#elif TARGET_API_MAC_CARBON
				mainProc = (PluginEntryProc)CFBundleGetFunctionPointerForName((CFBundleRef)module, CFSTR("VSTPluginMain"));
				if (!mainProc)
					mainProc = (PluginEntryProc)CFBundleGetFunctionPointerForName((CFBundleRef)module, CFSTR("main_macho"));
			#endif
				return mainProc;
			}
			//-------------------------------------------------------------------------------------------------------
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
			PluginLoader *pluginloader;
			const char *sFileName;
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
			PluginLoader* ploader;
			void *sDir;
//		#ifdef WIN32
//			char *sFileName;
//		#elif MAC
//			// yet to do
//			// no idea how things look here...
//		#endif

			bool bEditOpen;
			bool bNeedIdle;
			bool bWantMidi;

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
			VstInt32 numPrograms()	{	if (!aEffect)	throw (int)1;	return aEffect->numPrograms;	}
			VstInt32 numParams()	{	if (!aEffect)	throw (int)1;	return aEffect->numParams;		}
			VstInt32 numInputs()	{	if (!aEffect)	throw (int)1;	return aEffect->numInputs;		}
			VstInt32 numOutputs()	{	if (!aEffect)	throw (int)1;	return aEffect->numOutputs;		}
			//flags
			bool HasEditor()		{	if (!aEffect)	throw (int)1;	return aEffect->flags & effFlagsHasEditor;			}
			bool DECLARE_VST_DEPRECATED(HasClip)()		{	if (!aEffect)	throw (int)1;	return aEffect->flags & effFlagsHasClip;			}
			bool DECLARE_VST_DEPRECATED(HasVu)()		{	if (!aEffect)	throw (int)1;	return aEffect->flags & effFlagsHasVu;				}
			bool DECLARE_VST_DEPRECATED(CanInputMono)()	{	if (!aEffect)	throw (int)1;	return aEffect->flags & effFlagsCanMono;			}
			bool CanProcessReplace(){	if (!aEffect)	throw (int)1;	return aEffect->flags & effFlagsCanReplacing;		}
			bool ProgramIsChunk()	{	if (!aEffect)	throw (int)1;	return aEffect->flags & effFlagsProgramChunks;		}
			bool IsSynth()			{	if (!aEffect)	throw (int)1;	return aEffect->flags & effFlagsIsSynth;			}
			bool HasNoTail()		{	if (!aEffect)	throw (int)1;	return aEffect->flags & effFlagsNoSoundInStop;		}
			bool DECLARE_VST_DEPRECATED(ExternalAsync)(){	if (!aEffect)	throw (int)1;	return aEffect->flags & effFlagsExtIsAsync;			}
			bool DECLARE_VST_DEPRECATED(ExternalBuffer)(){	if (!aEffect)	throw (int)1;	return aEffect->flags & effFlagsExtHasBuffer;		}

			VstInt32 DECLARE_VST_DEPRECATED(RealQualities)(){	if (!aEffect)	throw (int)1;	return aEffect->realQualities;		}
			VstInt32 DECLARE_VST_DEPRECATED(OffQualities)()	{	if (!aEffect)	throw (int)1;	return aEffect->offQualities;		}
			float DECLARE_VST_DEPRECATED(IORatio)()			{	if (!aEffect)	throw (int)1;	return aEffect->ioRatio;			}

			// the real plugin ID.
			VstInt32 uniqueId()		{	if (!aEffect)	throw (int)1;	return aEffect->uniqueID;		}
			// version() is rarely used (from my experience) with VST2, in favour of GetVendorVersion(). Yet, it hasn't been deprecated in 2.4.
			VstInt32 version()		{	if (!aEffect)	throw (int)1;	return aEffect->version;		}
			VstInt32 initialDelay() {	if (!aEffect)	throw (int)1;	return aEffect->initialDelay;	}

		protected:
			virtual VstIntPtr Dispatch(VstInt32 opCode, VstInt32 index=0, VstIntPtr value=0, void* ptr=0, float opt=0.);
		public:
			//////////////////////////////////////////////////////////////////////////
			// plugin dispatch functions
			inline void Open() { Dispatch(effOpen); }
		protected:
			// Warning! After a "Close()", the "AEffect" is deleted and the plugin cannot be used again. (see audioeffect.cpp)
			// This is why i set it as protected, and calling it from the destructor.
			inline void Close() { Dispatch(effClose); }
		public:
			inline void SetProgram(VstIntPtr lValue) { Dispatch(effSetProgram, 0, lValue); }
			// returns the index of the program. Zero based.
			inline VstInt32 GetProgram() { return Dispatch(effGetProgram); }
			// size of ptr string limited to kVstMaxProgNameLen chars + \0 delimiter.
			inline void SetProgramName(char *ptr) { Dispatch(effSetProgramName, 0, 0, ptr); }
			inline void GetProgramName(char *ptr) { Dispatch(effGetProgramName, 0, 0, ptr); }
			// Unit of the paramter. size of ptr string limited to kVstMaxParamStrLen char + \0 delimiter
			inline void GetParamLabel(VstInt32 index, char *ptr) { Dispatch(effGetParamLabel, index, 0, ptr); }
			// Value of the parameter. size of ptr string limited to kVstMaxParamStrLen + \0 delimiter for safety.
			inline void GetParamDisplay(VstInt32 index, char *ptr) { Dispatch(effGetParamDisplay, index, 0, ptr); }
			// Name of the parameter. size of ptr string limited to kVstMaxParamStrLen char + \0 delimiter (might be not followed by plugin devs)
			inline void GetParamName(VstInt32 index, char *ptr) { Dispatch(effGetParamName, index, 0, ptr); }
			inline void SetSampleRate(float fSampleRate) { Dispatch(effSetSampleRate, 0, 0, 0, fSampleRate); }
			inline void SetBlockSize(VstIntPtr value) { Dispatch(effSetBlockSize, 0, value); }
			inline void MainsChanged(bool bOn) { Dispatch(effMainsChanged, 0, bOn); }
			inline float DECLARE_VST_DEPRECATED(GetVu)() { return Dispatch(effGetVu) / 32767.0f; }
			inline bool EditGetRect(ERect **ptr) { return Dispatch(effEditGetRect, 0, 0, ptr)==1?true:false; }
			// return value is true (succeeded) or false.
			inline bool EditOpen(void *ptr) { VstInt32 l = Dispatch(effEditOpen, 0, 0, ptr); if (l > 0) bEditOpen = true; return bEditOpen; }
			inline void EditClose() { Dispatch(effEditClose); bEditOpen = false; }
			// This has to be called repeatedly from the idle process ( usually the UI thread, with idle priority )
			// The plugins usually have checks so that it skips the call if no update is required.
			inline void EditIdle() { if (bEditOpen) Dispatch(effEditIdle); }
		#if MAC
			inline void DECLARE_VST_DEPRECATED(EditDraw)(void *rectarea) { Dispatch(effEditDraw, 0, 0, rectarea); }
			inline long DECLARE_VST_DEPRECATED(EditMouse)(VstInt32 x, VstIntPtr y) { return Dispatch(effEditMouse, x, y); }
			inline long DECLARE_VST_DEPRECATED(EditKey)(long value) { return Dispatch(effEditKey, 0, value); }
			inline void DECLARE_VST_DEPRECATED(EditTop)() { Dispatch(effEditTop); }
			inline void DECLARE_VST_DEPRECATED(EditSleep)() { Dispatch(effEditSleep); }
		#endif
			// 2nd check that it is a valid VST. (appart from kEffectMagic )
			inline bool DECLARE_VST_DEPRECATED(Identify)() { return (Dispatch(effIdentify) == CCONST ('N', 'v', 'E', 'f')); }
			// returns "byteSize".
			inline long GetChunk(void **ptr, bool isPreset = false) { return Dispatch(effGetChunk, isPreset, 0, ptr); }
			//\todo: If return value is zero, it is not supported. It might return byteSize when correct, but I am unsure.
			inline long SetChunk(void *data, long byteSize, bool isPreset = false) { return Dispatch(effSetChunk, isPreset, byteSize, data); }
		// VST 2.0
			inline long ProcessEvents(VstEvents* ptr) { return Dispatch(effProcessEvents, 0, 0, ptr); }
			inline bool CanBeAutomated(long index) { return (bool)Dispatch(effCanBeAutomated, index); }
			// A textual description of the parameter's value. A null pointer is used to check the capability (return true).
			inline bool String2Parameter(long index, char *text) { return (bool)Dispatch(effString2Parameter, index, 0, text); }
			inline long DECLARE_VST_DEPRECATED(GetNumProgramCategories)() { return Dispatch(effGetNumProgramCategories); }
			// text is a string up to kVstMaxProgNameLen chars + \0 delimiter
			inline bool GetProgramNameIndexed(long category, long index, char* text) { return (bool)Dispatch(effGetProgramNameIndexed, index, category, text); }
			// copy current program to the one in index.
			inline bool DECLARE_VST_DEPRECATED(CopyProgram)(long index) { return (bool)Dispatch(effCopyProgram, index); }
			//Input index has been (dis-)connected. The application may issue this call when implemented.
			inline void DECLARE_VST_DEPRECATED(ConnectInput)(long index, bool state) { Dispatch(effConnectInput, index, state); }
			//Output index has been (dis-)connected. The application may issue this call when implemented.
			inline void DECLARE_VST_DEPRECATED(ConnectOutput)(long index, bool state) { Dispatch(effConnectOutput, index, state); }
			inline bool GetInputProperties(long index, VstPinProperties *ptr) { return (bool)Dispatch(effGetInputProperties, index, 0, ptr); }
			inline bool GetOutputProperties(long index, VstPinProperties *ptr) { return (bool)Dispatch(effGetOutputProperties, index, 0, ptr); }
			inline long GetPlugCategory() { return Dispatch(effGetPlugCategory); }
			// get position of dsp buffer. (to verify that it is "on time")
			inline long DECLARE_VST_DEPRECATED(GetCurrentPosition)() { return Dispatch(effGetCurrentPosition); }
			// get the address of the dsp buffer.
			inline float* DECLARE_VST_DEPRECATED(GetDestinationBuffer)() { return (float*)Dispatch(effGetDestinationBuffer); }
			inline bool OfflineNotify(VstAudioFile* ptr, long numAudioFiles, bool start) { return (bool)Dispatch(effOfflineNotify, start, numAudioFiles, ptr); }
			inline bool OfflinePrepare(VstOfflineTask *ptr, long count) { return (bool)Dispatch(effOfflinePrepare, 0, count, ptr); }
			inline bool OfflineRun(VstOfflineTask *ptr, long count) { return (bool)Dispatch(effOfflineRun, 0, count, ptr); }
			//\todo: get more information about this function, and its relation with process and processReplacing.
			inline bool ProcessVarIo(VstVariableIo* varIo) { return (bool)Dispatch(effProcessVarIo, 0, 0, varIo); }
			inline bool SetSpeakerArrangement(VstSpeakerArrangement* pluginInput, VstSpeakerArrangement* pluginOutput) { return (bool)Dispatch(effSetSpeakerArrangement, 0, (long)pluginInput, pluginOutput); }
			inline void DECLARE_VST_DEPRECATED(SetBlockSizeAndSampleRate)(long blockSize, float sampleRate) { Dispatch(effSetBlockSizeAndSampleRate, 0, blockSize, 0, sampleRate); }
			inline bool SetBypass(bool onOff) { return (bool)Dispatch(effSetBypass, 0, onOff); }
			// ptr is a string up to kVstMaxEffectNameLen chars + \0 delimiter
			inline bool GetEffectName(char *ptr) { return (bool)Dispatch(effGetEffectName, 0, 0, ptr); }
			// ptr is a string up to 256 chars + \0 delimiter
			inline bool DECLARE_VST_DEPRECATED(GetErrorText)(char *ptr) { return (bool)Dispatch(effGetErrorText, 0, 0, ptr); }
			// ptr is a string up to kVstMaxVendorStrLen chars + \0 delimiter
			inline bool GetVendorString(char *ptr) { return (bool)Dispatch(effGetVendorString, 0, 0, ptr); }
			// ptr is a string up to kVstMaxProductStrLen chars + \0 delimiter
			inline bool GetProductString(char *ptr) { return (bool)Dispatch(effGetProductString, 0, 0, ptr); }
			inline long GetVendorVersion() { return Dispatch(effGetVendorVersion); }
			inline long VendorSpecific(long index, long value, void *ptr, float opt) { return Dispatch(effVendorSpecific, index, value, ptr, opt); }
			//returns 0 -> don't know, 1 -> yes, -1 -> no.
			inline long CanDo(const char *ptr) { return Dispatch(effCanDo, 0, 0, (void *)ptr); }
			inline long GetTailSize() { return Dispatch(effGetTailSize); }
			// "returns 0 by default"  ???
			inline long DECLARE_VST_DEPRECATED(Idle)() { if (bNeedIdle) return Dispatch(effIdle); else return 0; }
			inline long DECLARE_VST_DEPRECATED(GetIcon)() { return Dispatch(effGetIcon); }
			inline long DECLARE_VST_DEPRECATED(SetViewPosition)(long x, long y) { return Dispatch(effSetViewPosition, x, y); }
			inline long GetParameterProperties(long index, VstParameterProperties* ptr) { return Dispatch(effGetParameterProperties, index, 0, ptr); }
			// Seems something related to MAC ( to be used with editkey )
			inline bool DECLARE_VST_DEPRECATED(KeysRequired)() { return (bool)Dispatch(effKeysRequired); }
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

			virtual void DECLARE_VST_DEPRECATED(Process)(float **inputs, float **outputs, VstInt32 sampleframes);
			virtual void ProcessReplacing(float **inputs, float **outputs, VstInt32 sampleframes);
			virtual void ProcessDouble (double** inputs, double** outputs, VstInt32 sampleFrames);
			virtual void SetParameter(VstInt32 index, float parameter);
			virtual float GetParameter(VstInt32 index);

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
		public:
			CVSTHost();
			virtual ~CVSTHost();

		protected:
			long lBlockSize;

			static CVSTHost * pHost;
			static int quantization;

			CEffect *DECLARE_VST_DEPRECATED(GetPreviousPlugIn)(CEffect &pEffect,int pinIndex){ return 0;};
			CEffect *DECLARE_VST_DEPRECATED(GetNextPlugIn)(CEffect &pEffect, int pinIndex){ return 0;};
		public:
			CEffect* CVSTHost::LoadPlugin(const char * sName);

			static VstTimeInfo vstTimeInfo;
			static VstIntPtr VSTCALLBACK AudioMasterCallback (AEffect* effect, VstInt32 opcode, VstInt32 index, VstIntPtr value, void* ptr, float opt);
//			static long VSTCALLBACK AudioMasterCallback(AEffect *effect, long opcode, long index, long value, void *ptr, float opt);

			// overridable functions
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

			// text is a string up to kVstMaxVendorStrLen chars + \0 delimiter
			virtual bool OnGetVendorString(char *text) { strcpy(text, "Seib-Psycledelics"); return true; } // forgive this little vanity :-)
			// text is a string up to kVstMaxProductStrLen chars + \0 delimiter
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
			virtual void DECLARE_VST_DEPRECATED(OnWantEvents)(CEffect &pEffect, long filter);
			//\todo : investigate if the "flags" of vstTimeInfo should be per-plugin, instead of per-host.
			virtual VstTimeInfo *OnGetTime(CEffect &pEffect, long lMask) { CalcTimeInfo(lMask); return &vstTimeInfo; }
			virtual bool OnProcessEvents(CEffect &pEffect, VstEvents* events) { return false; }
			// aeffectx.hpp: "VstTimenfo* in <ptr>, filter in <value>, not supported". Not Implemented in the VST SDK.
			virtual bool DECLARE_VST_DEPRECATED(OnSetTime)(CEffect &pEffect, long filter, VstTimeInfo *timeInfo) { return false; }
			//  pos in Sample frames, return bpm* 10000
			virtual long DECLARE_VST_DEPRECATED(OnTempoAt)(CEffect &pEffect, long pos) { return 0; }
			virtual long DECLARE_VST_DEPRECATED(OnGetNumAutomatableParameters)(CEffect &pEffect) { return 0; }
			//	0 :  	Not implemented.
			//	1 : 	Full single float precision is maintained in automation.
			//other : 	The integer value that represents +1.0.
			virtual long DECLARE_VST_DEPRECATED(OnGetParameterQuantization)(CEffect &pEffect) { return quantization; }
			//Tell host numInputs and/or numOutputs and/or initialDelay has changed.
			// The host could call a suspend (if the plugin was enabled (in resume state)) and then ask for getSpeakerArrangement
			// and/or check the numInputs and numOutputs and initialDelay and then call a resume.
			virtual bool OnIoChanged(CEffect &pEffect) { return pEffect.OnIOChanged(); }
			//\todo: This seems to indicate that the plugin requires idle calls ( pEffect->Idle() ) continuously from the idle function.
			virtual bool DECLARE_VST_DEPRECATED(OnNeedIdle)(CEffect &pEffect);
			virtual bool OnSizeWindow(CEffect &pEffect, long width, long height);
			// Will cause application to call AudioEffect's  setSampleRate/setBlockSize method (when implemented).
			virtual long OnUpdateSampleRate(CEffect &pEffect){ pEffect.SetSampleRate(vstTimeInfo.sampleRate); return vstTimeInfo.sampleRate; }
			virtual long OnUpdateBlockSize(CEffect &pEffect) { pEffect.SetBlockSize(lBlockSize); return lBlockSize; }
			//	Returns the ASIO input latency values.
			virtual long OnGetInputLatency(CEffect &pEffect) { return 0; }
			// Returns the ASIO output latency values. To be used mostly for GUI sync with audio.
			virtual long OnGetOutputLatency(CEffect &pEffect) { return 0; }
			// asks the host if it will use this plugin with "processReplacing"
			virtual bool DECLARE_VST_DEPRECATED(OnWillProcessReplacing)(CEffect &pEffect) { return false; }
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
			virtual void DECLARE_VST_DEPRECATED(OnSetOutputSampleRate)(CEffect &pEffect, float sampleRate) { }
			virtual VstSpeakerArrangement* DECLARE_VST_DEPRECATED(OnGetOutputSpeakerArrangement)(CEffect &pEffect) { return pEffect.OnHostOutputSpeakerArrangement(); }
			// Specification says 0 -> don't know, 1 ->yes, -1 : no, but audioeffectx.cpp says "!= 0 -> true", and since plugins use audioeffectx...
			virtual bool OnCanDo(CEffect &pEffect,const char *ptr);
			virtual long OnGetHostLanguage() { return 0; }
			virtual void * DECLARE_VST_DEPRECATED(OnOpenWindow)(CEffect &pEffect, VstWindow* window);
			virtual bool DECLARE_VST_DEPRECATED(OnCloseWindow)(CEffect &pEffect, VstWindow* window);
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
			virtual bool DECLARE_VST_DEPRECATED(OnEditFile)(CEffect &pEffect, char *ptr) { return false; }
			virtual bool DECLARE_VST_DEPRECATED(OnGetChunkFile)(CEffect &pEffect, void * nativePath) { return false; }
			// VST 2.3 Extensions
			virtual VstSpeakerArrangement *DECLARE_VST_DEPRECATED(OnGetInputSpeakerArrangement)(CEffect &pEffect) { return pEffect.OnHostInputSpeakerArrangement(); }
		};
	}
}