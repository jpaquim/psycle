#pragma once
/** @file
 *  @brief header file
 *  $Date: 2005/01/15 22:44:19 $
 *  $Revision: 1.1 $
 */

#include "iConfigurable.h"

namespace model {
	namespace audiodriver {
		typedef float* (*AUDIODRIVERWORKFN)(void* context, int& numSamples);
		namespace configuration 
		{
			const UINT DEFAULT_SAMPLES_PER_SEC = 44100;
			const UINT DEFAULT_BIT_DEPTH = 16;
			//typedef boost::array<int,10> SampleRateType;
			const UINT SAMPLE_RATES[] = {8000,11025,16000,22050,32000,44100,48000,96000,192000,384000,768000};
			const UINT SAMPLE_RATES_SIZE = sizeof(model::audiodriver::configuration::SAMPLE_RATES) / sizeof(UINT);
		};

		///Exception
		class Exception : public std::exception
		{
		public:
			Exception(const string& reason)
			{
				m_Reason = reason;
			};
			const TCHAR * what() {return m_Reason.c_str();};
			const string& what_str() { return m_Reason;};
		private:
			string m_Reason;
		};

		struct ATL_NO_VTABLE iAudioDriver : public iConfigurable
		{
			virtual ~iAudioDriver() = 0;
			virtual void Initialize(
				const HWND hwnd,
				const AUDIODRIVERWORKFN pCallback,
				const void* context) = 0;
//			virtual void Release();
			virtual void Reset() = 0;

			virtual const bool Enable(const bool value) = 0;

			__declspec(property(get=GetWritePos)) const int WritePos;
			__declspec(property(get=GetPlayPos)) const int PlayPos;
			__declspec(property(get=GetNumBuffers)) const int NumBuffers;
			__declspec(property(get=GetBufferSize)) const int BufferSize;
			__declspec(property(get=GetSamplesPerSec)) const int SamplesPerSec;
			__declspec(property(get=GetIsInitialized)) const bool IsInitialized;
			__declspec(property(get=GetIsConfigured)) const bool IsConfigured;
			__declspec(property(get=GetDriverName)) const string& DriverName;
			__declspec(property(get=GetDriverDescription)) const string& DriverDescription;

			virtual const int GetWritePos() const = 0;
			virtual const int GetPlayPos() const = 0;
			virtual const int GetNumBuffers() const = 0;
			virtual const int GetBufferSize() const = 0;
			virtual const int GetSamplesPerSec() const = 0;
			virtual const bool GetIsInitialized() const = 0;
			virtual const bool GetIsConfigured() const = 0;
			virtual const string& GetDriverName() const = 0;
			virtual const string& GetDriverDescription() const = 0;
		};
	}
}