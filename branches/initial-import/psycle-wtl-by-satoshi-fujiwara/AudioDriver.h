#ifndef _AUDIODRIVER_H
#define _AUDIODRIVER_H
#pragma once
/** @file
 *  @brief header file
 *  $Date: 2005/01/15 22:44:19 $
 *  $Revision: 1.4 $
 */


#include "iAudioDriver.h"
#include "iController.h"
namespace model {
	namespace audiodriver {
		namespace configuration
		{
			class AudioDriver : public iCompornentConfiguration {
			public:
				AudioDriver(){Init();};
				~AudioDriver(){};
				void Save(const TCHAR * fileName){;};
				void Load(const TCHAR * fileName){;};
				void Reset(){Initialize();};
				void Initialize(){Init();};

				__declspec(property(get=GetBitDepth,put=PutBitDepth)) int BitDepth;
				__declspec(property(get=GetSamplesPerSec,put=PutSamplesPerSec)) int SamplesPerSec;
				__declspec(property(get=GetChannelMode,put=PutChannelMode)) int ChannelMode;
				__declspec(property(get=GetIsConfigured,put=PutIsConfigured)) bool IsConfigured;

				PROP_IMP(int,BitDepth);
				PROP_IMP(int,SamplesPerSec);
				PROP_IMP(int,ChannelMode);
				PROP_IMP(bool,IsConfigured);

			protected:

				void Init()
				{ m_SamplesPerSec = 44100; m_BitDepth = 16; m_ChannelMode = 3; m_IsConfigured = false;};

				int m_SamplesPerSec;
				int m_ChannelMode;
				int m_BitDepth;
				bool m_IsInitialized;
				bool m_IsConfigured;

			};
		};

		class AudioDriverEvent : public ATL::CEvent
		{
		public:
			AudioDriverEvent() : CEvent(FALSE, TRUE) {}
		};

		class AudioDriver : public iAudioDriver 
		{

		protected:

			static void QuantizeWithDither(
				float *pin,
				int *piout,
				int c);

			static void Quantize(
				float *pin,
				int *piout,
				int c);

			iController * m_pController;
		};

		class DummyAudioDriver : public AudioDriver
		{
		public:
			explicit DummyAudioDriver(iController *pController)
			{	m_pController = pController;
			};

			virtual ~DummyAudioDriver() {};
//			void Release(){delete this;};

			iCompornentConfiguration* const Configuration(){return NULL;};

			void Initialize(
				const HWND hwnd,
				const AUDIODRIVERWORKFN pCallback,
				const void* context) {};

			void Reset(){};

			const bool Enable(const bool e) { return false; };	
			const int GetWritePos() const { return 0; };
			const int GetPlayPos() const { return 0; };
			const int GetNumBuffers() const { return 0; };
			const int GetBufferSize() const { return 0; };
			const bool GetIsInitialized() const { return true; };
			const bool GetIsConfigured() const { return true; };
			const int GetSamplesPerSec() const {return 0;};
			const string& GetDriverName() const {return m_DriverName;};
			const string& GetDriverDescription() const{return m_DriverDescription;};
		private :
			static string m_DriverName;
			static string m_DriverDescription;

		};
	};
};

#endif