#pragma once
#include "Machine.h"
#include "Vst\AEffectx.h"
#include "constants.h"
#include "Helpers.h"
#include "Configuration.h"
#include <operating_system/exception.h>
#include <operating_system/exceptions/code_description.h>
#if !defined _WINAMP_PLUGIN_
	#include "NewMachine.h"
#endif
///\file
///\brief interface file for psycle::host::VSTPlugin
namespace psycle
{
	namespace host
	{
		/// VST Host.
		namespace vst
		{
			/// Classes derived from exception thrown by vst plugins.
			namespace exceptions
			{
				/// Exception caused by an error in a call to the vst dispatch function.
				class dispatch_error : public host::exceptions::function_error
				{
				public:
					inline dispatch_error(const std::string & what) : function_error(what) {}
				};
			}

			/// midi's 16 channels limit?
			const int max_io = 16;

			/// Dialog max ticks for parameters.
			const int quantization = 65535;

			/// VST plugin.
			class plugin : public Machine
			{
			public:
				typedef AEffect * (* PVSTMAIN) (audioMasterCallback audioMaster);
				plugin();
				/// <bohan>
				/// I added throw() to the signature of the desctructor to make it clear.
				/// It's just informative and is implicit because
				/// any object of any class is disallowed to throw any exception from its destructor.
				virtual ~plugin() throw();
				/// <bohan> since Free can throw exceptions, we must always enclose it in try-catch statements.
				void Free() throw(...);
				/// <bohan> since Instance can throw exceptions, we must always enclose it in try-catch statements.
				void Instance(const char dllname[], const bool overwriteName = true) throw(...);
				/// <bohan> since Dispatch can throw exceptions, we must always enclose it in try-catch statements.
				long int Dispatch(long int operation = 0, long int index = 0, long int value = 0, void * ptr = 0, float opt = 0) throw(...);
				/// Host callback dispatcher.
				static long AudioMaster(AEffect *effect, long opcode, long index, long value, void *ptr, float opt);
				virtual bool Load(RiffFile* pFile);
				///\todo To be removed when changing the fileformat.
				bool LoadChunk(RiffFile* pFile);
				virtual bool LoadSpecificFileChunk(RiffFile* pFile, int version);
				bool LoadDll(char* psFileName);
				#if !defined _WINAMP_PLUGIN_
					bool SaveChunk(RiffFile* pFile,bool &isfirst);
					virtual void SaveSpecificChunk(RiffFile* pFile) ;
					virtual void SaveDllName(RiffFile* pFile);
				#endif
				inline virtual int GetNumParams() throw() { return _pEffect->numParams; }
				inline virtual void GetParamName(int numparam, char * name)
				{
					if(numparam < _pEffect->numParams)
					try
					{
						Dispatch(effGetParamName, numparam, 0, name);
					}
					catch(...)
					{
						std::strcpy(name, "fucked up");
						//throw;
					}
					else std::strcpy(name,"Out of Range");
				}
				inline virtual void GetParamValue(int numparam, char * parval)
				{
					if(numparam < _pEffect->numParams) DescribeValue(numparam, parval);
					else std::strcpy(parval,"Out of Range");
				}
				inline virtual int GetParamValue(int numparam)
				{
					if(numparam < _pEffect->numParams) return f2i(_pEffect->getParameter(_pEffect, numparam) * 65535);
					else return -1; /// \todo <bohan> ???
				}
				inline virtual char * GetName() throw() { return _sProductName; }
				inline virtual void SetSampleRate(int sr)
				{
					try
					{
						Dispatch(effSetSampleRate, 0, 0, 0, (float) sr);
					}
					catch(...)
					{
						// <bohan>
						// exception blocked here for now,
						// but we really should do something...
						//throw;
					}
				}
				inline const long int & GetVersion() const throw() { return _version; }
				inline const char * const GetVendorName() const throw() { return _sVendorName; }
				inline virtual const char * const GetDllName() const throw() { return _sDllName; }
				inline const long int & NumParameters() const throw() { return _pEffect->numParams; }
				inline float GetParameter(long int parameter) const { return _pEffect->getParameter(_pEffect, parameter); }
				bool DescribeValue(int parameter, char * psTxt);
				bool SetParameter(int parameter, float value);
				bool SetParameter(int parameter, int value);
				void SetCurrentProgram(int prg);
				int GetCurrentProgram();
				inline const int NumPrograms() const throw() { return _pEffect->numPrograms; }
				inline const bool & IsSynth() const throw() { return _isSynth; }
				inline bool AddMIDI(unsigned char data0, unsigned char data1 = 0, unsigned char data2 = 0);
				inline void SendMidi();
				AEffect * _pEffect;
				unsigned char _program;
				bool instantiated;
				///\todo Remove when Changing the FileFormat.
				int _instance;
				/// It needs to use Process
				bool requiresProcess;
				/// It needs to use ProcessRepl
				bool requiresRepl;		
				#if !defined _WINAMP_PLUGIN_
					CFrameWnd * editorWnd;
				#endif
			protected:
				HMODULE h_dll;
				/// Contains dll name
				char * _sDllName;
				char _sProductName[64];
				char _sVendorName[64];
				long _version;
				bool _isSynth;
				bool wantidle;
				float * inputs[max_io];
				float * outputs[max_io];
				float junk[STREAM_SIZE];
				static VstTimeInfo _timeInfo;
				VstMidiEvent midievent[MAX_VST_EVENTS];
				VstEvents events;
				int	queue_size;
			};

			/// vst note for an instrument.
			class note
			{
			public:
				unsigned char key;
				unsigned char midichan;
			};

			/// vst "instrument" (input) plugin.
			class instrument : public plugin
			{
			public:
				instrument(int index);
				virtual void Tick(int channel, PatternEntry * pEntry);
				virtual void Work(int numSamples);
				virtual void Stop(void);
				bool AddNoteOn(unsigned char channel, unsigned char key, unsigned char velocity, unsigned char midichannel = 0);
				bool AddNoteOff(unsigned char channel, unsigned char midichannel = 0, bool addatStart = false);
			protected:
				note trackNote[MAX_TRACKS];
			};

			/// vst "fx" (filter) plugin.
			class fx : public plugin
			{
			public:
				fx(int index);
				virtual ~fx() throw();
				virtual void Tick(int channel, PatternEntry * pEntry);
				virtual void Work(int numSamples);
			protected:
				float * _pOutSamplesL;
				float * _pOutSamplesR;
			};
		}
	}
}
