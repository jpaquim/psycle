///\file
///\brief interface file for psycle::host::vsthost
#pragma once
//#include <psycle/engine/detail/project.hpp>
//#include <universalis/processor/exceptions/fpu.hpp>
//#include <universalis/compiler/location.hpp>
#include "machine.hpp"
#include <seib-vsthost/CVSTHost.Seib.hpp>
//#include "SongStructs.hpp"
//#include "constants.hpp"
//#include <psycle/engine/global.hpp>
//#include "FileIO.hpp"
//#include <psycle/helpers/dsp.hpp>
//#include <psycle/helpers/helpers.hpp>
//#include <stdexcept>
#include <cstring>
namespace psycle
{
	namespace host
	{
		namespace vst
		{
			// Maximum number of Audio Input/outputs
			// \todo : this shouldn't be a static value. Host should ask the plugin and the array get created dynamically.
			const int max_io = 16;
			// Dialog max ticks for parameters.
			const int quantization = 65535;
			const int MAX_VST_EVENTS = 128;

			// The real VstEvents in the SDK is defined as events[2], so  it cannot be used from a Host point of view.
			typedef struct VstEventsDynamicstruct
			{
				VstInt32 numEvents;		///< number of Events in array
				VstIntPtr reserved;		///< zero (Reserved for future use)
				VstEvent* events[MAX_VST_EVENTS];	///< event pointer array, variable size
			} VstEventsDynamic;

			using namespace seib::vst;


			class host;

			class plugin : public Machine, public CEffect
			{
			protected:
				CCriticalSection door;
				class note
				{
				public:
					unsigned char key;
					unsigned char midichan;
				};
				/// midi events queue, is sent to processEvents.
				note trackNote[MAX_TRACKS];
				VstMidiEvent midievent[MAX_VST_EVENTS];
				VstEventsDynamic mevents;
				int	queue_size;
				/// reserves space for a new midi event in the queue.
				/// \return midi event to be filled in, or null if queue is full.
				VstMidiEvent* reserveVstMidiEvent();
				VstMidiEvent* reserveVstMidiEventAtFront(); // ugly hack

				float * inputs[max_io];
				float * outputs[max_io];
				float * _pOutSamplesL;
				float * _pOutSamplesR;
				float junk[STREAM_SIZE];
				std::string _sDllName;
				std::string _sProductName;
				std::string _sVendorName;

				/// It needs to use Process
				bool requiresProcess;
				/// It needs to use ProcessRepl
				bool requiresRepl;		

			public:
				plugin(LoadedAEffect &loadstruct);
				//this constructor is to be used with the old Song loading routine, in order to create an "emtpy" plugin.
				plugin(AEffect *effect):CEffect(effect),editorWnd(0) {};
				virtual ~plugin();
				// Actions
				//////////////////////////////////////////////////////////////////////////
				virtual void Init(){ Machine::Init();}
				virtual void Work(int numSamples);
				virtual void Tick() {;}
				virtual void Tick(int track, PatternEntry * pData);
				virtual void Stop();
				// old fileformat {
				virtual bool PreLoad(RiffFile * pFile, unsigned char &_program, int &_instance);
				virtual bool LoadFromMac(vst::plugin *pMac);
				virtual bool LoadChunk(RiffFile* pFile);
				// }
				virtual bool LoadSpecificChunk(RiffFile* pFile, int version);
				virtual void SaveSpecificChunk(RiffFile * pFile);
				virtual void SaveDllName(RiffFile * pFile);
//				virtual bool ConnectTo(Machine& dstMac,int dstport=0,int outport=0,float volume=1.0f);
//				virtual bool Disconnect(Machine& dstMac);

				virtual void EnterCritical() {;}
				virtual void LeaveCritical() {;}

				bool AddMIDI(unsigned char data0, unsigned char data1 = 0, unsigned char data2 = 0);
				bool AddNoteOn(unsigned char channel, unsigned char key, unsigned char velocity, unsigned char midichannel = 0);
				bool AddNoteOff(unsigned char channel, unsigned char midichannel = 0, bool addatStart = false);
				inline void SendMidi();
				// Properties
				//////////////////////////////////////////////////////////////////////////
				virtual void SetSampleRate(int sr) { SetSampleRate((float)sr); }
				//\todo:
				virtual const char * const GetDllName() const throw() { return _sDllName.c_str(); }
				virtual char * GetName() throw() { return (char*)_sProductName.c_str(); }
				inline const char * const GetVendorName() const throw() { return _sVendorName.c_str(); }
				virtual const std::uint32_t GetVersion() { return GetVstVersion(); }
				//
				virtual void GetParamRange(int numparam,int &minval, int &maxval) {	minval = 0; maxval = quantization; }
				virtual int GetNumParams() { return numParams(); }
				virtual void GetParamName(int numparam, char * parval) { CEffect::GetParamName(numparam,parval); }
				virtual void GetParamValue(int numparam, char * parval);
				virtual int GetParamValue(int numparam)
				{
					if(numparam < numParams())
						return f2i(GetParameter(numparam) * quantization);
					return 0;
				}
				virtual bool SetParameter(int numparam, int value) { CEffect::SetParameter(numparam,float(value)/float(quantization)); return true; }
				virtual void SetParameter(int numparam, float value) { CEffect::SetParameter(numparam,value); }
				virtual bool DescribeValue(int parameter, char * psTxt);

				CFrameWnd * editorWnd;
			};

			class host : public CVSTHost
			{
			public:
				host(){	quantization = 65535; };
				virtual ~host(){;}

				virtual plugin* host::GetPreviousPlugIn(CEffect & pEffect, int pinIndex);
				virtual plugin* host::GetNextPlugIn(CEffect & pEffect, int pinIndex);

				///< Helper class for Machine Creation.
				static Machine* CreateFromType(int _id, std::string _dllname);
				virtual CEffect * CreateEffect(LoadedAEffect &loadstruct) { return new plugin(loadstruct); }
				virtual CEffect * CreateEffect(AEffect *effect) { return new plugin(effect); }
				virtual void CalcTimeInfo(long lMask = -1);

				///> Plugin gets Info from the host
				virtual bool OnGetProductString(char *text) { strcpy(text, "Psycle"); return true; }
				virtual long OnGetHostVendorVersion() { return 1850; }
				virtual bool OnCanDo(CEffect &pEffect,const char *ptr);
				virtual long OnGetHostLanguage() { return kVstLangEnglish; }

				//\todo : Optimize it more to avoid recalculate when already done?
				//virtual VstTimeInfo *OnGetTime(CEffect &pEffect, long lMask);
				virtual long OnTempoAt(CEffect &pEffect, long pos);
				virtual long OnGetNumAutomatableParameters(CEffect &pEffect);
				virtual long OnGetAutomationState(CEffect &pEffect);
				//virtual long OnGetInputLatency(CEffect &pEffect) { return 0; }
				virtual long OnGetOutputLatency(CEffect &pEffect);
				//\todo : how can this function be implemented? :o
				virtual long OnGetCurrentProcessLevel(CEffect &pEffect) { return 0; }
				//\todo : returning the effect ID for now.
				virtual long OnCurrentId(CEffect &pEffect) { return pEffect.uniqueId(); }
				virtual long OnGetParameterQuantization(CEffect &pEffect) { return quantization; }
				//\todo : determine how to reply to this function.
				virtual bool OnWillProcessReplacing(CEffect &pEffect) { return false; }
				//\todo : investigate which file is this function really asking for.
				virtual bool OnGetChunkFile(CEffect &pEffect, void * nativePath) { return false; }

				///> Plugin sends actions to the host
				virtual void OnIdle(CEffect &pEffect);
				virtual bool OnNeedIdle(CEffect &pEffect);

				virtual bool OnProcessEvents(CEffect &pEffect, VstEvents* events) { return false; }
				virtual bool OnBeginEdit(CEffect &pEffect,long index);
				virtual void OnSetParameterAutomated(CEffect &pEffect, long index, float value);
				virtual bool OnEndEdit(CEffect &pEffect,long index);
				virtual bool OnOpenFileSelector (CEffect &pEffect, VstFileSelect *ptr);
				virtual bool OnCloseFileSelector (CEffect &pEffect, VstFileSelect *ptr);

			};
		}
	}
}