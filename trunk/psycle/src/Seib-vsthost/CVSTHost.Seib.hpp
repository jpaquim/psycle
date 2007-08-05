/*****************************************************************************/
/* CVSTHost.hpp: interface for CVSTHost/CEffect classes (for VST SDK 2.4r2). */
/*****************************************************************************/

/*****************************************************************************/
/* Work Derived from the LGPL host "vsthost (1.16m)".						 */
/* (http://www.hermannseib.com/english/vsthost.htm)"						 */
/* vsthost has the following lincense:										 *

Copyright (C) 2006  Hermann Seib

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
******************************************************************************/

#pragma once
/// Tell the SDK that we want to support all the VST specs, not only VST2.4
#define VST_FORCE_DEPRECATED 0
#include <vst2.x/AEffectx.h>               /* VST header files                  */
#include <vst2.x/vstfxstore.h>

//////////////////////////////////////////////////////////////////////////
// This is part of Psycle, to catch the exceptions that happen when interacting
// with the plugins. To use your own, replace 
// PSYCLE__HOST__CATCH_ALL(*this) by whatever you find appropiate, like
// catch(...) {} 
#include <psycle/host/machine.hpp> // for throw.
//////////////////////////////////////////////////////////////////////////

namespace seib {
	namespace vst {
		/*! hostCanDos strings Plug-in -> Host */
		namespace HostCanDos
		{
			extern const char* canDoSendVstEvents;
			extern const char* canDoSendVstMidiEvent;
			extern const char* canDoSendVstTimeInfo;
			extern const char* canDoReceiveVstEvents;
			extern const char* canDoReceiveVstMidiEvent;
			extern const char* canDoReportConnectionChanges;
			extern const char* canDoAcceptIOChanges;
			extern const char* canDoSizeWindow;
			extern const char* canDoOffline;
			extern const char* canDoOpenFileSelector;
			extern const char* canDoCloseFileSelector;
			extern const char* canDoStartStopProcess;
			extern const char* canDoShellCategory;
			extern const char* canDoSendVstMidiEventFlagIsRealtime;
		}

		//-------------------------------------------------------------------------------------------------------
		/*! plugCanDos strings Host -> Plug-in */
		namespace PlugCanDos
		{
			extern const char* canDoSendVstEvents;
			extern const char* canDoSendVstMidiEvent;
			extern const char* canDoReceiveVstEvents;
			extern const char* canDoReceiveVstMidiEvent;
			extern const char* canDoReceiveVstTimeInfo;
			extern const char* canDoOffline;
			extern const char* canDoMidiProgramNames;
			extern const char* canDoBypass;
		}

		/*****************************************************************************/
		/* CFxBase : base class for FX Bank / Program Files                          */
		/*****************************************************************************/
		class CFxBase
		{
		protected:
			CFxBase();
		public:
			CFxBase(VstInt32 _version,VstInt32 _fxID, VstInt32 _fxVersion);
			CFxBase(const char *pszFile);
			CFxBase(FILE* pFileHandle);
			CFxBase & operator=(CFxBase const &org) { return DoCopy(org); }
			CFxBase & DoCopy(const CFxBase &org);

			bool Save(const char *pszFile);
			long GetMagic() { return version; }
			long GetVersion() { return version; }
			long GetFxID() {  return fxID; }
			long GetFxVersion() { return fxVersion; }
			bool Initialized() { return initialized; }
			std::string GetPathName() { return pathName; }
			// This function would normally be protected, but it is needed in the saving of Programs from a Bank.
			virtual bool SaveData(FILE* pFileHandle) { pf = pFileHandle; return SaveData(); }
		protected:
			VstInt32 fxMagic;			///< "Magic" identifier of this chunk. Tells if it is a Bank/Program and if it is chunk based.
			VstInt32 version;			///< format version
			VstInt32 fxID;				///< fx unique ID
			VstInt32 fxVersion;			///< fx version
			std::string pathName;
			bool initialized;
			// pf would normally be private, but it is needed in the loading of Programs from a Bank.
			FILE* pf;
		protected:
			bool Load(const char *pszFile);
			virtual bool LoadData() { return ReadHeader(); }
			virtual bool SaveData() { return WriteHeader(); }
			template <class T>
			bool Read(T &f,bool allowswap=true);
			template <class T>
			bool Write(T f,bool allowswap=true);
			bool ReadArray(void *f,int size);	
			bool WriteArray(void *f, int size);
			void Rewind(int bytes) { fseek(pf,-bytes,SEEK_CUR); }
			void Forward(int bytes) { fseek(pf,bytes,SEEK_CUR); }
			bool ReadHeader();
			bool WriteHeader();
			virtual void CreateInitialized();
		private:
			static bool NeedsBSwap;
		private:
			void SwapBytes(VstInt32 &f);
			void SwapBytes(float &f);
		};

		/*****************************************************************************/
		/* CFxProgram : class for an .fxp (Program) file                             */
		/*****************************************************************************/
		class CFxProgram : public CFxBase
		{
		public:
			CFxProgram(const char *pszFile = 0):CFxBase(){ Init(); initialized=Load(pszFile); }
			CFxProgram(FILE *pFileHandle);
			// Create a CFxProgram from parameters.
			// _fxID and _fxVersion are mandatory. 
			// if isChunk == false, size is the number of parameters, and data, if not empty, contains 
			// an array of floats that correspond to the parameter values.
			// if isChunk == true, size is the size of the chunk, and data, if not empty, contains the chunk data.
			CFxProgram(VstInt32 _fxID, VstInt32 _fxVersion, VstInt32 size, bool isChunk=false, void *data=0);
			CFxProgram(CFxProgram const &org):CFxBase(){ Init(); DoCopy(org); }
			virtual ~CFxProgram();
			CFxProgram & operator=(CFxProgram const &org) { FreeMemory(); return DoCopy(org); }

			// access functions
			const char * GetProgramName() const	{ return prgName;	}
			void SetProgramName(const char *name = "")
			{
				//leave last char for null.
				std::strncpy(prgName, name, 27);
			}
			long GetNumParams() const{ return numParams; }
			float GetParameter(VstInt32 nParm) const{  return (nParm < numParams) ? pParams[nParm] : 0; }
			bool SetParameter(VstInt32 nParm, float val = 0.0);
			long GetChunkSize() const{ return chunkSize; }
			const void *GetChunk() const { return pChunk; }
			bool CopyChunk(const void *chunk,const int size) {	ChunkMode(); return SetChunk(chunk,size);	}
			bool IsChunk() const{ return fxMagic == chunkPresetMagic; }

			virtual bool SaveData(FILE* pFileHandle) { return CFxBase::SaveData(pFileHandle); }

		protected:
			char prgName[28];			///< program name (null-terminated ASCII string)

			VstInt32 numParams;			///< number of parameters
			float* pParams;				///< variable sized array with parameter values
			int chunkSize;				///< Size of the opaque chunk.
			unsigned char* pChunk;		///< variable sized array with opaque program data

		protected:
			void Init();
			CFxProgram & DoCopy(CFxProgram const &org);
			void FreeMemory();
			virtual bool LoadData();
			virtual bool SaveData();

		private:
			void ChunkMode() { FreeMemory(); fxMagic = chunkPresetMagic; }
			void ParamMode() { FreeMemory(); fxMagic = fMagic; }
			bool SetParameters(const float* pnewparams,int params);
			bool SetChunk(const void *chunk, VstInt32 size);
			bool SetNumParams(VstInt32 nPars,bool initializeData=true);
			bool SetChunkSize(VstInt32 size,bool initializeData=true);
		};


		/*****************************************************************************/
		/* CFxBank : class for an .fxb (Bank) file                                   */
		/*****************************************************************************/

		class CFxBank : public CFxBase
		{
		public:
			CFxBank(const char *pszFile = 0):CFxBase(){ Init(); initialized=Load(pszFile); }
			CFxBank(FILE* pFileHandle);
			// Create a CFxBank from parameters.
			// _fxID, _fxVersion and _numPrograms are mandatory.
			// If isChunk == false, the Bank is created as a regular bank of numPrograms, _size is then the
			// number of parameters of the program, and data can be zero, or an array of CFxPrograms to copy to the CFxBank.
			// if isChunk == true ,create a chunk of size "_size", and copy the contents of "data".
			CFxBank(VstInt32 _fxID, VstInt32 _fxVersion, VstInt32 _numPrograms, bool isChunk=false, int _size=0, void *_data=0);
			CFxBank(CFxBank const &org) { Init(); DoCopy(org); }
			virtual ~CFxBank();
			CFxBank & operator=(CFxBank const &org) { FreeMemory(); return DoCopy(org); }

			// access functions
			long GetNumPrograms() const { return numPrograms; }
			
			long GetChunkSize() const { return chunkSize; }
			void * const GetChunk() const { return pChunk; }
			bool CopyChunk(const void *chunk,const int size) {	ChunkMode(); return SetChunk(chunk,size);	}
			bool IsChunk() const{ return fxMagic == chunkBankMagic; }

			// if nProgNum is not specified (i.e, it is -1) , currentProgram is used as index.
			CFxProgram& GetProgram(VstInt32 nProgNum=-1)
			{
				if ( nProgNum < 0 || nProgNum >= numPrograms) return programs[currentProgram];
				else return programs[nProgNum];
			}
			void SetProgramIndex(VstInt32 nProgNum) { if (nProgNum < numPrograms ) currentProgram = nProgNum; }
			VstInt32 GetProgramIndex() const { return currentProgram;	}
			virtual bool SaveData(FILE* pFileHandle) { return CFxBase::SaveData(pf); }

		protected:
			VstInt32 numPrograms;
			VstInt32 currentProgram;
			int chunkSize;
			unsigned char * pChunk;
			std::vector<CFxProgram> programs;

		protected:
			void Init();
			CFxBank & DoCopy(CFxBank const &org);
			bool SetChunk(const void *chunk, VstInt32 size);
			bool SetChunkSize(VstInt32 size, bool initializeData=true);
			void FreeMemory();
			virtual bool LoadData();
			virtual bool SaveData();

		private:
			void ChunkMode() { FreeMemory(); fxMagic = chunkBankMagic; }
			void ProgramMode() { FreeMemory(); fxMagic = bankMagic; }

		};

		class CPatchChunkInfo : public VstPatchChunkInfo
		{
		public:
			CPatchChunkInfo(CFxProgram fxstore);
			CPatchChunkInfo(CFxBank fxstore);
		};

		//-------------------------------------------------------------------------------------------------------
		// PluginLoader (From VST SDK 2.4 "minihost.cpp")
		//-------------------------------------------------------------------------------------------------------
		typedef AEffect* (*PluginEntryProc) (audioMasterCallback audioMaster);

		struct PluginLoader
		{
			void* module;
			void* sFileName;

			PluginLoader ()
				: module (0)
				, sFileName (0)
			{}

			virtual ~PluginLoader ()
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
				if (sFileName)
				{
				#if _WIN32
					delete sFileName;
				#elif TARGET_API_MAC_CARBON
					///\todo:
				#endif
				}
			}

			bool loadLibrary (const char* fileName)
			{
			#if _WIN32
				module = LoadLibrary (fileName);
				sFileName = new char[strlen(fileName) + 1];
				if (sFileName)
					strcpy((char*)sFileName, fileName);
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
				mainProc = reinterpret_cast<PluginEntryProc>(GetProcAddress ((HMODULE)module, "VSTPluginMain"));
				if(!mainProc)
					mainProc = reinterpret_cast<PluginEntryProc>(GetProcAddress ((HMODULE)module, "main"));
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
		/*		Struct definition to ease  CEffect creation/destruction.			 */
		/* Sometimes it might be prefferable to create the CEffect *after* the       */
		/* AEffect has been loaded, in order to have different subclasses for        */
		/* different types. Yet, it might be necessary that the plugin frees the     */
		/* library once it is destroyed. This is why it requires this information	 */
		/*****************************************************************************/
		class CVSTHost;

		typedef struct LoadedAEffect LoadedAEffect;
		struct LoadedAEffect {
			AEffect *aEffect;
			PluginLoader *pluginloader;
		};

		/*****************************************************************************/
		/* CEffect : class definition for audio effect objects                       */
		/*****************************************************************************/
		class CEffectWnd;
		class CEffect;
		class Crashingclass
		{
		public:
			Crashingclass(){};
			void SetEff(CEffect* ef){ this->ef = ef;}
			void crashed(std::exception const & e) const;
			CEffect* ef;
		};
		class CEffect
		{
		public:
			// Try to avoid to use the AEffect constructor. It acts as a wrapper then, not as an object.
			CEffect(AEffect *effect);
			CEffect(LoadedAEffect &loadstruct);
			virtual ~CEffect();
		protected:
			virtual void Load(LoadedAEffect &loadstruct);
			virtual void Unload();
		protected:
			AEffect *aEffect;
			PluginLoader* ploader;
			void *sDir;
			std::string loadingChunkName;

			CEffectWnd * editorWnd;
			bool bEditOpen;
			bool bNeedIdle;
			bool bNeedEditIdle;
			bool bWantMidi;
			bool bShellPlugin;
			bool bCanBypass;
			bool bMainsState;

			// overridables
		public:
			virtual bool LoadBank(CFxBank& fxstore);
			virtual bool LoadProgram(CFxProgram& fxstore);
			virtual CFxBank SaveBank(bool preferchunk=false);
			virtual CFxProgram SaveProgram(bool preferchunk=true);
			virtual void EnterCritical(){;}
			virtual void LeaveCritical(){;}
			Crashingclass crashclass;
			virtual void crashed2(std::exception const & e) {};
			virtual void WantsMidi(bool enable) { bWantMidi=enable; }
			virtual bool WantsMidi() { return bWantMidi; }
			virtual void KnowsToBypass(bool enable) { bCanBypass=enable; }
			virtual bool KnowsToBypass() { return bCanBypass; }
			virtual void NeedsIdle(bool enable) { bNeedIdle=enable; }
			virtual bool NeedsIdle(){ return bNeedIdle; }
			virtual void NeedsEditIdle(bool enable) { bNeedEditIdle=enable; }
			virtual bool NeedsEditIdle() { return bNeedEditIdle; }
			virtual bool IsShellPlugin() { return bShellPlugin; }
			virtual void IsShellPlugin(bool enable) { bShellPlugin = enable; }
			virtual void SetChunkFile(const char * nativePath) { loadingChunkName = nativePath; }
			virtual void SetEditWnd(CEffectWnd* wnd) { editorWnd = wnd; }

			// Overridable AEffect-to-host calls. (you can override them at the host level
			// if your implementation needs that)
			virtual void * OnGetDirectory();
			virtual bool OnGetChunkFile(char * nativePath);
			virtual bool OnSizeEditorWindow(long width, long height);
			///\todo: You might need to overload the OnUpdateDisplay in order to check other changes.
			virtual bool OnUpdateDisplay();
			virtual void * OnOpenWindow(VstWindow* window);
			virtual bool OnCloseWindow(VstWindow* window);
			virtual bool DECLARE_VST_DEPRECATED(IsInputConnected)(int input) { return true; } 
			virtual bool DECLARE_VST_DEPRECATED(IsOutputConnected)(int input) { return true; }
			// AEffect asks host about its input/outputspeakers.
			virtual VstSpeakerArrangement* OnHostInputSpeakerArrangement() { return 0; }
			virtual VstSpeakerArrangement* OnHostOutputSpeakerArrangement() { return 0; }
			// AEffect informs of changed IO. verify numins/outs, speakerarrangement and the likes.
			virtual bool OnIOChanged() { return false; }
			virtual bool OnBeginAutomating(long index);
			virtual bool OnEndAutomating(long index);
			virtual void OnSetParameterAutomated(long index, float value);
			virtual bool OnOpenFileSelector (VstFileSelect *ptr);
			virtual bool OnCloseFileSelector (VstFileSelect *ptr);

			//////////////////////////////////////////////////////////////////////////
			// Following comes the Wrapping of the VST Interface functions.
			virtual void DECLARE_VST_DEPRECATED(Process)(float **inputs, float **outputs, VstInt32 sampleframes);
			virtual void ProcessReplacing(float **inputs, float **outputs, VstInt32 sampleframes);
			virtual void ProcessDouble (double** inputs, double** outputs, VstInt32 sampleFrames);
			virtual void SetParameter(VstInt32 index, float parameter);
			virtual float GetParameter(VstInt32 index);
		public:
			// Not to be used, except if no other way.
			inline AEffect	*GetAEffect() { return aEffect; }
			//////////////////////////////////////////////////////////////////////////
			// AEffect Properties
			// magic is only used in the loader to verify that it is a VST plugin
			//long int magic()
			inline VstInt32 numPrograms() const	 throw(psycle::host::exceptions::function_error){	try { return aEffect->numPrograms;	} PSYCLE__HOST__CATCH_ALL(crashclass) return 0; }
			inline VstInt32 numParams() const	 throw(psycle::host::exceptions::function_error){	try { return aEffect->numParams;	} PSYCLE__HOST__CATCH_ALL(crashclass) return 0; }
			inline VstInt32 numInputs() const	 throw(psycle::host::exceptions::function_error){	try { return aEffect->numInputs;	} PSYCLE__HOST__CATCH_ALL(crashclass) return 0; }
			inline VstInt32 numOutputs() const	 throw(psycle::host::exceptions::function_error){	try { return aEffect->numOutputs;	} PSYCLE__HOST__CATCH_ALL(crashclass) return 0; }
			//flags
			inline bool HasEditor()const	 throw(psycle::host::exceptions::function_error)					{	try { return aEffect->flags & effFlagsHasEditor;	} PSYCLE__HOST__CATCH_ALL(crashclass) return 0; }
			inline bool DECLARE_VST_DEPRECATED(HasClip)() const	 throw(psycle::host::exceptions::function_error){	try { return aEffect->flags & effFlagsHasClip;		} PSYCLE__HOST__CATCH_ALL(crashclass) return 0; }
			inline bool DECLARE_VST_DEPRECATED(HasVu)() const	 throw(psycle::host::exceptions::function_error){	try { return aEffect->flags & effFlagsHasVu;		} PSYCLE__HOST__CATCH_ALL(crashclass) return 0; }
			inline bool DECLARE_VST_DEPRECATED(CanInputMono)()const	 throw(psycle::host::exceptions::function_error){	try { return aEffect->flags & effFlagsCanMono;			} PSYCLE__HOST__CATCH_ALL(crashclass) return 0; }
			inline bool CanProcessReplace() const	throw(psycle::host::exceptions::function_error)				{	try { return aEffect->flags & effFlagsCanReplacing;		} PSYCLE__HOST__CATCH_ALL(crashclass) return 0; }
			inline bool ProgramIsChunk() const	throw(psycle::host::exceptions::function_error)					{	try { return aEffect->flags & effFlagsProgramChunks;	} PSYCLE__HOST__CATCH_ALL(crashclass) return 0; }
			inline bool IsSynth() const	throw(psycle::host::exceptions::function_error)							{	try { return aEffect->flags & effFlagsIsSynth;			} PSYCLE__HOST__CATCH_ALL(crashclass) return 0; }
			inline bool HasNoTail() const	throw(psycle::host::exceptions::function_error)						{	try { return aEffect->flags & effFlagsNoSoundInStop;	} PSYCLE__HOST__CATCH_ALL(crashclass) return 0; }
			inline bool DECLARE_VST_DEPRECATED(ExternalAsync)() const	throw(psycle::host::exceptions::function_error){	try { return aEffect->flags & effFlagsExtIsAsync;	} PSYCLE__HOST__CATCH_ALL(crashclass) return 0; }
			inline bool DECLARE_VST_DEPRECATED(ExternalBuffer)() const	throw(psycle::host::exceptions::function_error){	try { return aEffect->flags & effFlagsExtHasBuffer;	} PSYCLE__HOST__CATCH_ALL(crashclass) return 0; }

			inline VstInt32 DECLARE_VST_DEPRECATED(RealQualities)() const	throw(psycle::host::exceptions::function_error){	try { return aEffect->realQualities;	} PSYCLE__HOST__CATCH_ALL(crashclass) return 0; }
			inline VstInt32 DECLARE_VST_DEPRECATED(OffQualities)() const	throw(psycle::host::exceptions::function_error){	try { return aEffect->offQualities;		} PSYCLE__HOST__CATCH_ALL(crashclass) return 0; }
			inline float DECLARE_VST_DEPRECATED(IORatio)() const	throw(psycle::host::exceptions::function_error)		{	try { return aEffect->ioRatio;			} PSYCLE__HOST__CATCH_ALL(crashclass) return 0; }

			// the real plugin ID.
			inline VstInt32 uniqueId() const	throw(psycle::host::exceptions::function_error)	{	try { return aEffect->uniqueID;		} PSYCLE__HOST__CATCH_ALL(crashclass) return 0; }
			// version() is never used (from my experience), in favour of GetVendorVersion(). Yet, it hasn't been deprecated in 2.4.
			inline VstInt32 version() const	throw(psycle::host::exceptions::function_error)		{	try { return aEffect->version;		} PSYCLE__HOST__CATCH_ALL(crashclass) return 0; }
			inline VstInt32 initialDelay() const	throw(psycle::host::exceptions::function_error){	try { return aEffect->initialDelay;	} PSYCLE__HOST__CATCH_ALL(crashclass) return 0; }

		protected:
			virtual VstIntPtr Dispatch(VstInt32 opCode, VstInt32 index=0, VstIntPtr value=0, void* ptr=0, float opt=0.) throw(psycle::host::exceptions::function_error);
		public:
			//////////////////////////////////////////////////////////////////////////
			// plugin dispatch functions
			inline void Open() { Dispatch(effOpen); }
		protected:
			// Warning! After a "Close()", the "AEffect" is deleted and the plugin cannot be used again. (see audioeffect.cpp)
			// This is why i set it as protected, and calling it from the destructor.
			inline void Close() { Dispatch(effClose); }
		public:
			// sets the index of the program. Zero based.
			inline void SetProgram(VstIntPtr lValue) { if (lValue >= 0 && lValue < numPrograms()) Dispatch(effSetProgram, 0, lValue); }
			// returns the index of the program. Zero based.
			inline VstInt32 GetProgram() { return Dispatch(effGetProgram); }
			// size of ptr string limited to kVstMaxProgNameLen chars + \0 delimiter.
			inline void SetProgramName(const char *ptr) { Dispatch(effSetProgramName, 0, 0, const_cast<char*>(ptr) ); }
			inline void GetProgramName(char *ptr) { Dispatch(effGetProgramName, 0, 0, ptr); }
			// Unit of the paramter. size of ptr string limited to kVstMaxParamStrLen char + \0 delimiter
			// NOTE-NOTE-NOTE-NOTE: Forget about the limit. Use the kVstMaxProgNameLen instead.
			inline void GetParamLabel(VstInt32 index, char *ptr) { Dispatch(effGetParamLabel, index, 0, ptr); }
			// Value of the parameter. size of ptr string limited to kVstMaxParamStrLen + \0 delimiter for safety.
			// NOTE-NOTE-NOTE-NOTE: Forget about the limit. It is exceeded sometimes. Use the kVstMaxProgNameLen instead.
			inline void GetParamDisplay(VstInt32 index, char *ptr) { Dispatch(effGetParamDisplay, index, 0, ptr); }
			// Name of the parameter. size of ptr string limited to kVstMaxParamStrLen char + \0 delimiter.
			// NOTE-NOTE-NOTE-NOTE: Forget about the limit. It's *way* exceeded usually. Use the kVstMaxProgNameLen instead.
			inline void GetParamName(VstInt32 index, char *ptr) { Dispatch(effGetParamName, index, 0, ptr); }
			// Returns the vu value. Range [0-1] >1 -> clipped
			inline float DECLARE_VST_DEPRECATED(GetVu)() { return Dispatch(effGetVu); }
			inline void SetSampleRate(float fSampleRate,bool ignorestate=false)
			{
				bool reinit=false;
				if (bMainsState && !ignorestate) { reinit=true; MainsChanged(false); }
				Dispatch(effSetSampleRate, 0, 0, 0, fSampleRate);
				if (reinit) MainsChanged(true);
			}
			inline void SetBlockSize(VstIntPtr value,bool ignorestate=false)
			{
				bool reinit=false;
				if (bMainsState && !ignorestate) { reinit=true; MainsChanged(false); }
				Dispatch(effSetBlockSize, 0, value);
				if (reinit) MainsChanged(true);
			}
			inline void MainsChanged(bool bOn)
			{
				if (bOn != bMainsState)
				{
					bMainsState=bOn;
					if (bOn) { Dispatch(effMainsChanged, 0, bOn); StartProcess(); }
					else {  StopProcess(); Dispatch(effMainsChanged, 0, bOn); }
				}
			}
			inline bool EditGetRect(ERect **ptr) { return Dispatch(effEditGetRect, 0, 0, ptr)==1?true:false; }
			inline void EditOpen(void *ptr) { Dispatch(effEditOpen, 0, 0, ptr); bEditOpen = true; }
			inline void EditClose() { Dispatch(effEditClose); bEditOpen = false; }
			// This has to be called repeatedly from the idle process ( usually the UI thread, with idle priority )
			// The plugins usually have checks so that it skips the call if no update is required.
			inline void EditIdle() { if(bEditOpen) Dispatch(effEditIdle); bNeedEditIdle=false; }
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
			inline long GetChunk(void **ptr, bool onlyCurrentProgram = false) { return Dispatch(effGetChunk, onlyCurrentProgram, 0, ptr); }
			// return value is not specified in the VST SDK. Don't assume anything.
			inline long SetChunk(const void *data, long byteSize, bool onlyCurrentProgram = false) { return Dispatch(effSetChunk, onlyCurrentProgram, byteSize, const_cast<void*>(data)); }
		// VST 2.0
			inline long ProcessEvents(VstEvents* ptr) { return Dispatch(effProcessEvents, 0, 0, ptr); }
			inline bool CanBeAutomated(long index) { return (bool)Dispatch(effCanBeAutomated, index); }
			// A textual description of the parameter's value. A null pointer is used to check the capability (return true).
			inline bool String2Parameter(long index, char *text) { return (bool)Dispatch(effString2Parameter, index, 0, text); }
			inline long DECLARE_VST_DEPRECATED(GetNumProgramCategories)() { return Dispatch(effGetNumProgramCategories); }
			// text is a string up to kVstMaxProgNameLen chars + \0 delimiter
			inline bool GetProgramNameIndexed(long category, long index, char* text)
			{
				if (!Dispatch(effGetProgramNameIndexed, index, category, text))
				{
					VstInt32 cprog= GetProgram();
					SetProgram(index);
					GetProgramName(text);
					SetProgram(cprog);
					if (!*text)
						sprintf(text, "Program %d", index);
				}
				return true;
			}
			// copy current program to the one in index.
			inline bool DECLARE_VST_DEPRECATED(CopyProgram)(long index) { return (bool)Dispatch(effCopyProgram, index); }
			//Input index has been (dis-)connected. The application may issue this call when implemented.
			inline void DECLARE_VST_DEPRECATED(ConnectInput)(long index, bool state) { Dispatch(effConnectInput, index, state); }
			//Output index has been (dis-)connected. The application may issue this call when implemented.
			inline void DECLARE_VST_DEPRECATED(ConnectOutput)(long index, bool state) { Dispatch(effConnectOutput, index, state); }
			inline bool GetInputProperties(long index, VstPinProperties *ptr) { return (bool)Dispatch(effGetInputProperties, index, 0, ptr); }
			inline bool GetOutputProperties(long index, VstPinProperties *ptr) { return (bool)Dispatch(effGetOutputProperties, index, 0, ptr); }
			inline VstPlugCategory GetPlugCategory() { return (VstPlugCategory)Dispatch(effGetPlugCategory); }
			// get position of dsp buffer. (to verify that it is "on time")
			inline long DECLARE_VST_DEPRECATED(GetCurrentPosition)() { return Dispatch(effGetCurrentPosition); }
			// get the address of the dsp buffer.
			inline float* DECLARE_VST_DEPRECATED(GetDestinationBuffer)() { return (float*)Dispatch(effGetDestinationBuffer); }
			inline bool OfflineNotify(VstAudioFile* ptr, long numAudioFiles, bool start) { return (bool)Dispatch(effOfflineNotify, start, numAudioFiles, ptr); }
			inline bool OfflinePrepare(VstOfflineTask *ptr, long count) { return (bool)Dispatch(effOfflinePrepare, 0, count, ptr); }
			inline bool OfflineRun(VstOfflineTask *ptr, long count) { return (bool)Dispatch(effOfflineRun, 0, count, ptr); }
			/// This function is for Offline processing.
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
			//returns 0 -> don't know, 1 -> yes, -1 -> no. Use the PlugCanDo's strings.
			inline long CanDoInt(const char *ptr) { return Dispatch(effCanDo, 0, 0, (void *)ptr); }
			inline bool CanDo(const char *ptr) { return (Dispatch(effCanDo, 0, 0, (void *)ptr)>0); }
			inline long GetTailSize() { return Dispatch(effGetTailSize); }
			// Recursive Idle() call for plugin. bNeedIdle is set to true when the plugin calls "pHost->OnNeedIdle()"
			inline void DECLARE_VST_DEPRECATED(Idle)() { VstInt32 l=0; if (bNeedIdle) l = Dispatch(effIdle); if (!l) bNeedIdle=false; }
			inline long DECLARE_VST_DEPRECATED(GetIcon)() { return Dispatch(effGetIcon); }
			inline long DECLARE_VST_DEPRECATED(SetViewPosition)(long x, long y) { return Dispatch(effSetViewPosition, x, y); }
			inline long GetParameterProperties(long index, VstParameterProperties* ptr) { return Dispatch(effGetParameterProperties, index, 0, ptr); }
			// Seems something related to MAC ( to be used with editkey )
			inline bool DECLARE_VST_DEPRECATED(KeysRequired)() { return (bool)Dispatch(effKeysRequired); }
			inline long GetVstVersion() { VstInt32 v=Dispatch(effGetVstVersion); if (v > 1000) return v; else return (v)?2000:1000; }
		// VST 2.1 extensions
			// Seems something related to MAC ( to be used with editkey )
			inline long KeyDown(VstKeyCode &keyCode) { return Dispatch(effEditKeyDown, keyCode.character, keyCode.virt, 0, keyCode.modifier); }
			// Seems something related to MAC ( to be used with editkey )
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
		// VST 2.4 Extensions
			inline bool SetProcessPrecision(VstProcessPrecision precision)	{ return Dispatch(effSetProcessPrecision,0,precision,0,0); }
			inline long	GetNumMidiInputChannels() { return Dispatch(effGetNumMidiInputChannels,0,0,0,0); }
			inline long	GetNumMidiOutputChannels() { return Dispatch(effGetNumMidiOutputChannels,0,0,0,0); }
		};

		/*****************************************************************************/
		/* CVSTHost class declaration                                                */
		/*****************************************************************************/

		class CVSTHost
		{
		public:
			CVSTHost();
			virtual ~CVSTHost();

		public:
			static CVSTHost * pHost;
			static VstTimeInfo vstTimeInfo;

		protected:
			long lBlockSize;
			static int quantization;
			bool loadingEffect;
			VstInt32 loadingShellId;
			bool isShell;

		public:
			CEffect* LoadPlugin(const char * sName,VstInt32 shellIdx=0);
			static VstIntPtr VSTCALLBACK AudioMasterCallback (AEffect* effect, VstInt32 opcode, VstInt32 index, VstIntPtr value, void* ptr, float opt);

			// overridable functions
		protected:
			virtual CEffect * CreateEffect(LoadedAEffect &loadstruct) { return new CEffect(loadstruct); }
			virtual CEffect * CreateWrapper(AEffect *effect) { return new CEffect(effect); }
			// The base class function gives kVstPpqPosValid, kVstBarsValid, kVstClockValid, kVstSmpteValid, and kVstNanosValid
			// Ensure that samplePos, sampleRate, tempo, and timesigNumerator/Denominator are correct before calling it.
			virtual void CalcTimeInfo(long lMask = -1);
		public:
			virtual void Log(std::string message) {};
			virtual void SetSampleRate(float fSampleRate=44100.);
			virtual void SetBlockSize(long lSize=1024);
			virtual void SetTimeSignature(long numerator, long denominator);
			virtual float GetSampleRate() { return vstTimeInfo.sampleRate; }
			virtual long GetBlockSize() { return lBlockSize; }

			// text is a string up to kVstMaxVendorStrLen chars + \0 delimiter
			virtual bool OnGetVendorString(char *text) { strcpy(text, "Seib-Psycledelics"); return true; }
			// text is a string up to kVstMaxProductStrLen chars + \0 delimiter
			virtual bool OnGetProductString(char *text) { strcpy(text, "Default CVSTHost."); return true; }
			virtual long OnGetHostVendorVersion() { return 1612; } // 1.16l
			virtual long OnHostVendorSpecific(CEffect &pEffect, long lArg1, long lArg2, void* ptrArg, float floatArg) { return 0; }
			virtual long OnGetVSTVersion() { return kVstVersion; }

			// Plugin calls this function when it has changed one parameter (from the GUI)in order for the host to record it.
			virtual void OnSetParameterAutomated(CEffect &pEffect, long index, float value) { pEffect.OnSetParameterAutomated(index,value); }
			// onCurrentId is used normally when loading Shell type plugins. This is taken care in the AudioMaster callback.
			// the function here is called in any other cases.
			virtual long OnCurrentId(CEffect &pEffect) { return pEffect.uniqueId(); }
			// This callback forces an inmediate "EditIdle()" call for the plugins.
			virtual void OnIdle(CEffect &pEffect);
			virtual bool DECLARE_VST_DEPRECATED(OnInputConnected)(CEffect &pEffect, long input) { return pEffect.IsInputConnected(input); }
			virtual bool DECLARE_VST_DEPRECATED(OnOutputConnected)(CEffect &pEffect, long output) { return pEffect.IsOutputConnected(output); }
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
			// Tell host numInputs and/or numOutputs and/or initialDelay has changed.
			// The host could call a suspend (if the plugin was enabled (in resume state)) and then ask for getSpeakerArrangement
			// and/or check the numInputs and numOutputs and initialDelay and then call a resume.
			virtual bool OnIoChanged(CEffect &pEffect) { return pEffect.OnIOChanged(); }
			// This function is called from a plugin when it needs one (or more)call(s) to CEffect::Idle().
			// You should have a continuous call to CEffect::Idle() on your Idle Call.
			// CVSTHost::OnNeedIdle() and CEffect::Idle() take care of calling only when needed.
			virtual bool DECLARE_VST_DEPRECATED(OnNeedIdle)(CEffect &pEffect);
			virtual bool OnSizeWindow(CEffect &pEffect, long width, long height) { return pEffect.OnSizeEditorWindow(width, height); }
			// Will cause application to call AudioEffect's  setSampleRate/setBlockSize method (when implemented).
			virtual long OnUpdateSampleRate(CEffect &pEffect){
				if (!loadingEffect) pEffect.SetSampleRate(vstTimeInfo.sampleRate,true);
				return vstTimeInfo.sampleRate; }
			virtual long OnUpdateBlockSize(CEffect &pEffect) {
				pEffect.SetSampleRate(vstTimeInfo.sampleRate,true);
				return lBlockSize; }
			//	Returns the ASIO input latency values.
			virtual long OnGetInputLatency(CEffect &pEffect) { return 0; }
			// Returns the ASIO output latency values. To be used mostly for GUI sync with audio.
			virtual long OnGetOutputLatency(CEffect &pEffect) { return 0; }
			virtual CEffect *DECLARE_VST_DEPRECATED(GetPreviousPlugIn)(CEffect &pEffect,int pinIndex);
			virtual CEffect *DECLARE_VST_DEPRECATED(GetNextPlugIn)(CEffect &pEffect, int pinIndex);
			// asks the host if it will use this plugin with "processReplacing"
			virtual bool DECLARE_VST_DEPRECATED(OnWillProcessReplacing)(CEffect &pEffect) { return false; }
			//	0 :  	Not supported.
			//	1 : 	Currently in user thread (gui).
			//	2 : 	Currently in audio thread or irq (where process is called).
			//	3 : 	Currently in 'sequencer' thread or irq (midi, timer etc).
			//	4 : 	Currently offline processing and thus in user thread.
			//other : 	Not defined, but probably pre-empting user thread.
			virtual long OnGetCurrentProcessLevel(CEffect &pEffect) { return 0; }
			//	kVstAutomationUnsupported 	not supported by Host
			//	kVstAutomationOff 	off
			//	kVstAutomationRead 	read
			//	kVstAutomationWrite 	write
			//	kVstAutomationReadWrite 	read and write
			virtual long OnGetAutomationState(CEffect &pEffect) { return kVstAutomationUnsupported; }
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
			// Used for variable I/O processing. ( offline processing )
			virtual void DECLARE_VST_DEPRECATED(OnSetOutputSampleRate)(CEffect &pEffect, float sampleRate) { return; }
			virtual VstSpeakerArrangement* DECLARE_VST_DEPRECATED(OnGetOutputSpeakerArrangement)(CEffect &pEffect) { return pEffect.OnHostOutputSpeakerArrangement(); }
			// Specification says 0 -> don't know, 1 ->yes, -1 : no, but audioeffectx.cpp says "!= 0 -> true", and since plugins use audioeffectx...
			virtual bool OnCanDo(CEffect &pEffect,const char *ptr);
			virtual long OnGetHostLanguage() { return kVstLangEnglish; }
			virtual void * DECLARE_VST_DEPRECATED(OnOpenWindow)(CEffect &pEffect, VstWindow* window) { return pEffect.OnOpenWindow(window); }
			virtual bool DECLARE_VST_DEPRECATED(OnCloseWindow)(CEffect &pEffect, VstWindow* window) { return pEffect.OnCloseWindow(window); }
			virtual void * OnGetDirectory(CEffect &pEffect) { return pEffect.OnGetDirectory(); }
			//\todo: "Something has changed, update 'multi-fx' display." ???
			virtual bool OnUpdateDisplay(CEffect &pEffect) { return pEffect.OnUpdateDisplay(); }
			// VST 2.1 Extensions
			// Notifies that "OnSetParameterAutomated" is going to be called. (once per mouse clic)
			virtual bool OnBeginEdit(CEffect &pEffect,long index) { return pEffect.OnBeginAutomating(index); }
			virtual bool OnEndEdit(CEffect &pEffect,long index) { return pEffect.OnEndAutomating(index); }
			virtual bool OnOpenFileSelector (CEffect &pEffect, VstFileSelect *ptr) { return pEffect.OnOpenFileSelector(ptr); }
			// VST 2.2 Extensions
			virtual bool OnCloseFileSelector (CEffect &pEffect, VstFileSelect *ptr) { return pEffect.OnCloseFileSelector(ptr); }
			// open an editor for audio (defined by XML text in ptr)
			virtual bool DECLARE_VST_DEPRECATED(OnEditFile)(CEffect &pEffect, char *ptr) { return false; }
			virtual bool DECLARE_VST_DEPRECATED(OnGetChunkFile)(CEffect &pEffect, void * nativePath) { return pEffect.OnGetChunkFile(static_cast<char*>(nativePath)); }
			// VST 2.3 Extensions
			virtual VstSpeakerArrangement *DECLARE_VST_DEPRECATED(OnGetInputSpeakerArrangement)(CEffect &pEffect) { return pEffect.OnHostInputSpeakerArrangement(); }
		};
	}
}