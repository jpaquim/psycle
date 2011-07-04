#ifndef _DIRECTSOUND_H
#define _DIRECTSOUND_H

/** @file
 *  @brief header file
 *  $Date: 2005/01/15 22:44:19 $
 *  $Revision: 1.4 $
 */

#include <mmsystem.h>
#include <dsound.h>
#include "AudioDriver.h"

namespace model {
	namespace audiodriver {
		class DirectSound;
		namespace configuration {
			class DirectSound : public AudioDriver
			{
			public:
				// DirectSoundDevice情報保持クラス
				struct DeviceInfo 
				{
					typedef std::vector<int> SupportedSampleRatesType;

					__declspec(property(get=GetName,put=PutName)) string& Name;
					__declspec(property(get=GetGuid,put=PutGuid)) GUID& Guid;
					__declspec(property(get=GetCapabilities,put=PutCapabilities)) DSCAPS& Capabilities;

					PROP_IMP(string&,Name);
					PROP_IMP(GUID&,Guid);
					PROP_IMP(DSCAPS&,Capabilities);

					DeviceInfo()
					{	Name = _T("");
						Guid = GUID_NULL;
						ZeroMemory(&m_Capabilities,sizeof(DSCAPS));
						m_Capabilities.dwSize = sizeof(DSCAPS);
					};

					DeviceInfo(const DeviceInfo& src)
					{
						Name = src.Name;
						Guid = src.Guid;
						Capabilities = src.Capabilities;
					};

					DeviceInfo& operator=(const DeviceInfo& src)
					{
						if(this == &src) return *this;
						Name = src.Name;
						Guid = src.Guid;
						Capabilities = src.Capabilities;
					};

				private:
					string m_Name;
					GUID m_Guid;
					DSCAPS m_Capabilities;
				};

				typedef std::map<string,DeviceInfo> DeviceList;


				explicit DirectSound(::model::audiodriver::DirectSound& ds) : m_DirectSound(ds) {Init();};
				~DirectSound(){};
				void Save(const string& fileName);
				void Load(const string& fileName);
				void Reset(){Initialize();};
				void Initialize(){Init();};

				DirectSound& operator=(const DirectSound& src)
				{
					if(this == &src) return *this;

					NumBuffers = src.NumBuffers;
					BufferSize = src.BufferSize;
					IsExclusive = src.IsExclusive;
					IsDither = src.IsDither;
					DeviceDescription  = src.DeviceDescription;
					SamplesPerSec = src.SamplesPerSec;
					IsConfigured = src.IsConfigured;
					//m_Latency = src.m_Latency;
					DsGuid = src.DsGuid;

					return *this;
				};

				void Configure();
				const HWND Create(){return NULL;};
			
				__declspec(property(get=GetNumBuffers,put=PutNumBuffers)) int NumBuffers;
				__declspec(property(get=GetBufferSize,put=PutBufferSize)) int BufferSize;
				__declspec(property(get=GetExclusive,put=PutExclusive)) bool IsExclusive;
				__declspec(property(get=GetDither,put=PutDither)) bool IsDither;
				__declspec(property(get=GetDeviceDescription,put=PutDeviceDescription)) string& DeviceDescription;
				__declspec(property(get=GetDsGuid,put=PutDsGuid)) GUID& DsGuid;
				__declspec(property(get=GetDeviceLists)) DeviceList& DeviceLists;
				__declspec(property(get=GetLatency)) int Latency;
				__declspec(property(get=GetDirectSound)) model::audiodriver::DirectSound& DirectSoundObject;

				const int GetLatency()
				{
					const int _totalbytes = m_NumBuffers * m_BufferSize;
					return (m_SamplesPerSec != 0 ? ((_totalbytes * (1000 / 4)) / m_SamplesPerSec):0);
				}

				void Apply(const configuration::DirectSound& backup);
				void Cancel(const configuration::DirectSound& backup) {*this = backup;};

				static BOOL CALLBACK EnumDSoundDevices
				(
					LPGUID lpGUID,
					const TCHAR* psDesc,
					const TCHAR* psDrvName,
					void* pDirectSound
				);

				PROP_IMP(int,NumBuffers);
				PROP_IMP(int,BufferSize);
				PROP_IMP(bool,Exclusive);
				PROP_IMP(bool,Dither);
				PROP_IMP(string&,DeviceDescription);
				PROP_IMP(GUID&,DsGuid);

				DeviceList& GetDeviceLists(){return m_DeviceLists;};

				model::audiodriver::DirectSound& GetDirectSound() const { return m_DirectSound;};

//				static const GUID GUID_NULL;

			private:
				model::audiodriver::DirectSound& m_DirectSound;

				configuration::DirectSound* m_pConfigBackup;
			
				GUID m_DsGuid;

				int m_NumBuffers;
				int m_BufferSize;
				bool m_Exclusive;
				bool m_Dither;
				string m_DeviceDescription;
				//int m_Latency;

				/** 初期化 */
				void Init()
				{
					// 上位クラスのInit()を呼ぶ
					AudioDriver::Init();

					//GUID dsGuid;
					//memcpy(&dsGuid, &m_DsGuid, sizeof(GUID));
					// DirectSound devices
					m_NumBuffers = 4;
					m_BufferSize = 4096;
					m_DsGuid = GUID_NULL;
					m_Dither = false;
					m_SamplesPerSec = DEFAULT_SAMPLES_PER_SEC;
					m_BitDepth = DEFAULT_BIT_DEPTH;
					m_ChannelMode = 3;
					m_Exclusive = false;
					boost::call_once(OnceInit,m_Once);
				};

				// Serialization 
				friend class boost::serialization::access;
				template<class Archive>
				void serialize(Archive& ar, const unsigned int version)
				{
					ar & boost::serialization::make_nvp("NumBuffers",m_NumBuffers);
					ar & boost::serialization::make_nvp("BufferSize",m_BufferSize);
					ar & boost::serialization::make_nvp("DeviceGuid",m_DsGuid);
					ar & boost::serialization::make_nvp("DeviceDescription",m_DeviceDescription);
					ar & boost::serialization::make_nvp("Dither",m_Dither);
					ar & boost::serialization::make_nvp("Exclusive",m_Exclusive);
					ar & boost::serialization::make_nvp("SamplesPerSec",m_SamplesPerSec);
				}

				// Static Class

				static DeviceList m_DeviceLists;
				static boost::once_flag m_Once;
				static void OnceInit()
				{
					DirectSoundEnumerate(EnumDSoundDevices, NULL);
				};
			};
			typedef boost::shared_ptr<DirectSound> DirectSoundPtr; 
		};


//		BOOST_CLASS_VERSION(::model::audiodriver::configuration::DirectSound, 0);

class DirectSound : public AudioDriver
{
public:
	explicit DirectSound(::model::iController *pController);
	virtual ~DirectSound();
//	void Release(){delete this;};
	void Initialize(const HWND hwnd,const AUDIODRIVERWORKFN pCallback,const void* context);

	::model::iCompornentConfiguration* const Configuration(){return &m_Config;};

	void Reset();
	const bool Start();
	const bool Stop();

	const bool Enable(const bool e);
	const int GetWritePos() const;
	const int GetPlayPos() const;

	const bool GetIsInitialized() const { return m_IsInitialized; };
	const bool GetIsConfigured() const { return m_Config.IsConfigured; };
	const int GetSamplesPerSec() const { return m_Config.SamplesPerSec; };

	__declspec(property(get=GetController)) iController& Controller;

	iController& GetController() const {return *m_pController;};

	void Error(TCHAR const *msg) const;

	const int GetNumBuffers() const {return m_Config.NumBuffers;};
	const int GetBufferSize() const {return m_Config.BufferSize;};

	const string& GetDriverName() const {return m_DriverName;};
	const string& GetDriverDescription() const {return m_DriverDescription;};

private :
	static string m_DriverName;
	static string m_DriverDescription;

	model::audiodriver::configuration::DirectSound m_Config;

	bool m_IsInitialized;

	HWND _hwnd;
	MMRESULT _timer;
	bool _running;
	bool _playing;
	bool _timerActive;

	static boost::mutex m_Mutex;
	static boost::thread m_PollerThread;

	LPDIRECTSOUND _pDs;
	LPDIRECTSOUNDBUFFER _pBuffer;
	void* _callbackContext;
	AUDIODRIVERWORKFN _pCallback;

	int _bytesPerSample;
	int _dsBufferSize;
	int _currentOffset;
	int _lowMark;
	int _highMark;
	int _buffersToDo;

	static void PollerThread(void* pDirectSound);
	static HANDLE m_Handle;

//	static void TimerCallback(UINT uTimerID, UINT uMsg, DWORD pDirectSound, DWORD dw1, DWORD dw2);
	void DoBlocks();

};


	};
};
#endif