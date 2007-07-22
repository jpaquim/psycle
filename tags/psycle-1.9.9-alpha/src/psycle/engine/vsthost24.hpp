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
//#include <cstdint>
namespace psycle
{
	namespace engine
	{
		namespace vst
		{
			using namespace seib::vst;
			class plugin;

			class host : public CVSTHost
			{
			public:
				host():quantization(65535){};
				virtual ~host();

				virtual plugin* host::GetPreviousPlugIn(CEffect & pEffect, int pinIndex);
				virtual plugin* host::GetNextPlugIn(CEffect & pEffect, int pinIndex);

				///< Helper class for Machine Creation.
				static Machine* CreateFromType(Machine::id_type _id, std::string _dllname);
				virtual plugin * CreateEffect(LoadedAEffect &loadstruct) { return new plugin(loadstruct); }
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
				virtual bool OnBeginEdit(CEffect &pEffect,long index) { return false; }
				virtual void OnSetParameterAutomated(CEffect &pEffect, long index, float value) { return; }
				virtual bool OnEndEdit(CEffect &pEffect,long index) { return false; }
				virtual bool OnOpenFileSelector (CEffect &pEffect, VstFileSelect *ptr) { return false; }
				virtual bool OnCloseFileSelector (CEffect &pEffect, VstFileSelect *ptr) { return false; }

			};

			class plugin : public Machine, public CEffect
			{
			protected:
				CCriticalSection door;
			public:
				plugin(id_t id,LoadedAEffect &loadstruct);
				virtual ~plugin();
				// Actions
				//////////////////////////////////////////////////////////////////////////
				virtual void Init();
				virtual void Work(int numSamples);
				virtual void Tick() {;}
				virtual void Tick(int track, PatternEntry * pData) {;}
				virtual void Stop() {;}
	//			virtual bool LoadOldFileFormat(RiffFile * pFile);
	//			bool LoadChunkOldFileFormat(RiffFile* pFile);
				virtual bool LoadSpecificChunk(RiffFile* pFile, int version);
				virtual void SaveSpecificChunk(RiffFile * pFile);
				virtual void SaveDllName(RiffFile * pFile);
				virtual bool ConnectTo(Machine& dstMac,int dstport=0,int outport=0,float volume=1.0f);
				virtual bool Disconnect(Machine& dstMac);

				virtual void EnterCritical();
				virtual void LeaveCritical();

				// Properties
				//////////////////////////////////////////////////////////////////////////
				virtual void SetSampleRate(int sr) { SetSampleRate((float)sr); }
				//\todo:
				virtual const std::string GetDllName() { return sFileName; }
				virtual const std::string GetBrand() { return; }
				virtual const std::string GetVendorName() { return; }
				virtual const std::uint32_t GetVersion() { return 0; }
				virtual const std::uint32_t GetCategory() { return 0; }
				//
				virtual void GetParamRange(int numparam,int &minval, int &maxval) {	minval = 0; maxval = quantization; }
				virtual int GetNumParams() { return numParams(); }
				virtual void GetParamValue(int numparam, char * parval);
				virtual int GetParamValue(int numparam)
				{
					if(numparam < numParams())
						return f2i(GetParameter(numparam) * quantization);
				}
				virtual bool SetParameter(int numparam, int value) { SetParameter(numparam,float(value)/float(quantization)); }
				virtual bool DescribeValue(int parameter, char * psTxt);
			};
		}
	}
}