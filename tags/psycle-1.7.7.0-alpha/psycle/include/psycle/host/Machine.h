#pragma once
#if defined _WINAMP_PLUGIN_
	#include <stdio.h>
#endif
#include "SongStructs.h"
#include "Dsp.h"
#include "Helpers.h"
#include "Constants.h"
#include "FileIO.h"
///\file
///\brief interface file for psycle::host::Machine

namespace psycle
{
	namespace host
	{
		class RiffFile; // forward declaration

		#if !defined(_CYRIX_PROCESSOR_) && !defined(_WINAMP_PLUGIN_)
			///\todo make that an inline function
			#define CPUCOST_INIT(cost)	\
				ULONG cost;				\
				__asm rdtsc				\
				__asm mov cost, eax
			/*
			///\todo make that an inline function
			#define CPUCOST_CALC(cost, numSamples)	\
				__asm {								\
				__asm rdtsc							\
				__asm sub eax, cost					\
				__asm mov cost, eax					\
				}									\
				cost = cost* Global::pConfig->_pOutputDriver->_samplesPerSec/ numSamples
			//	cost = (cost*1000)/(numSamples*(Global::_cpuHz/Global::pConfig->_pOutputDriver->_samplesPerSec));
			*/
			///\todo make that an inline function
			#define CPUCOST_CALC(cost, numSamples)	\
				__asm rdtsc							\
				__asm sub eax, cost					\
				__asm mov cost, eax

			#else

			///\todo make that an inline function
			#define CPUCOST_INIT(cost)	\
				ULONG cost;				\
				cost = 0;

			///\todo make that an inline function
			#define CPUCOST_CALC(cost, numSamples)	cost = 0;
		#endif


		/// Internal Machines' Parameters' Class.
		class CIntMachParam			
		{
		public:
			/// Short name
			char const *name;		
			/// >= 0
			int minValue;
			/// <= 65535
			int maxValue;
		};

		/// well, what to say?
		class Machine
		{
		public:
			int _macIndex;

			MachineType _type;
			MachineMode _mode;
			bool _bypass;
			bool _mute;
			bool _waitingForSound;

			bool _stopped;
			bool _worked;

			/// left data
			float *_pSamplesL;
			/// right data
			float *_pSamplesR;						
			/// left chan volume
			float _lVol;							
			/// right chan volume
			float _rVol;							
			/// numerical value of panning.
			int _panning;							
			#if !defined(_WINAMP_PLUGIN_)
				/// output peak level for DSP
				float _volumeCounter;					
				/// output peak level for display
				int _volumeDisplay;	
				/// output peak level for display
				int _volumeMaxDisplay;
				/// output peak level for display
				int _volumeMaxCounterLife;
				unsigned long int _cpuCost;
				unsigned long int _wireCost;
				int _scopePrevNumSamples;
				int	_scopeBufferIndex;
				float *_pScopeBufferL;
				float *_pScopeBufferR;
			#endif // ndef _WINAMP_PLUGIN_

			int _x;
			int _y;
			char _editName[32];
			int _numPars;
			/// Incoming connections Machine number
			int _inputMachines[MAX_CONNECTIONS];	
			/// Outgoing connections Machine number
			int _outputMachines[MAX_CONNECTIONS];	
			/// Incoming connections Machine vol
			float _inputConVol[MAX_CONNECTIONS];	
			/// Value to multiply _inputConVol[] to have a 0.0...1.0 range
			float _wireMultiplier[MAX_CONNECTIONS];	
			/// Outgoing connections activated
			bool _connection[MAX_CONNECTIONS];      
			/// Incoming connections activated
			bool _inputCon[MAX_CONNECTIONS];		
			/// number of Incoming connections
			int _numInputs;							
			/// number of Outgoing connections
			int _numOutputs;						
			#if !defined(_WINAMP_PLUGIN_)
				/// point for wire? 
				CPoint _connectionPoint[MAX_CONNECTIONS];
			#endif
			PatternEntry	TriggerDelay[MAX_TRACKS];
			int				TriggerDelayCounter[MAX_TRACKS];
			int				RetriggerRate[MAX_TRACKS];

			bool TWSActive;
			int TWSInst[MAX_TWS];
			int TWSSamples;
			float TWSDelta[MAX_TWS];
			float TWSCurrent[MAX_TWS];
			float TWSDestination[MAX_TWS];
			
			Machine();
			virtual ~Machine();

			virtual void Init(void);
			virtual void SetPan(int newpan);
			virtual bool SetDestWireVolume(int srcIndex, int WireIndex,float value);
			virtual void SetWireVolume(int wireIndex,float value)
			{
				_inputConVol[wireIndex] = value / _wireMultiplier[wireIndex];
			}
			virtual bool GetDestWireVolume(int srcIndex, int WireIndex,float &value);
			virtual void GetWireVolume(int wireIndex, float &value)
			{
				value = _inputConVol[wireIndex] * _wireMultiplier[wireIndex];
			}
			virtual void InitWireVolume(MachineType mType,int wireIndex,float value);
			virtual int FindInputWire(int macIndex);
			virtual int FindOutputWire(int macIndex);
			virtual void Tick(void) {};
			virtual void Tick(int track, PatternEntry* pData) {};
			virtual void PreWork(int numSamples);
			virtual void Work(int numSamples);
			virtual void Stop(void) {};
			virtual char* GetName(void) = 0;
			virtual int GetNumParams(void) { return _numPars; }
			virtual void GetParamName(int numparam,char* name) { name[0]='\0'; }
			virtual void GetParamValue(int numparam,char* parval) { parval[0]='\0'; };
			virtual int GetParamValue(int numparam) { return 0; };
			virtual bool SetParameter(int numparam,int value) { return false;}; 
			virtual bool Load(RiffFile* pFile);
			static Machine* LoadFileChunk(RiffFile* pFile, int index, int version,bool fullopen=true);
			/// versions can only be older than current or this won't get called 
			inline virtual bool LoadSpecificFileChunk(RiffFile* pFile, int version)
			{
				UINT size;
				pFile->Read(&size,sizeof(size)); // size of this part params to load

				UINT count;

				pFile->Read(&count,sizeof(count)); // num params to load
				for (UINT i = 0; i < count; i++)
				{
					int temp;
					pFile->Read(&temp,sizeof(temp));
					SetParameter(i,temp);
				}

				pFile->Skip(size-sizeof(count)-(count*sizeof(int)));

				return TRUE;
			};
			virtual void SetSampleRate(int sr) {};
			#if !defined(_WINAMP_PLUGIN_)
				void SaveFileChunk(RiffFile* pFile);
				virtual void SaveSpecificChunk(RiffFile* pFile) 
				{
					UINT count = GetNumParams();
					UINT size = sizeof(count)+(count*sizeof(int));
					pFile->Write(&size,sizeof(size));
					pFile->Write(&count,sizeof(count));
					for (UINT i = 0; i < count; i++)
					{
						int temp = GetParamValue(i);
						pFile->Write(&temp,sizeof(temp));
					}
				};
				virtual void SaveDllName(RiffFile* pFile) 
				{
					char temp=0;
					pFile->Write(&temp,1);
				};

			protected:
				inline void SetVolumeCounter(int numSamples)
				{
					_volumeCounter = Dsp::GetMaxVol(_pSamplesL, _pSamplesR, numSamples);
					if (_volumeCounter > 32768.0f)
					{
						_volumeCounter = 32768.0f;
					}
					int temp = (f2i(fast_log2(_volumeCounter)*78.0f*4/14.0f) - (78*3));// not 100% accurate, but looks as it sounds
					// prevent downward jerkiness
					if (temp > 97)
					{
						temp = 97;
					}
					if (temp > _volumeDisplay)
					{
						_volumeDisplay = temp;
					}
					_volumeDisplay--;
				};
				/*
				inline void SetVolumeCounterAccurate(int numSamples)
				{
					_volumeCounter = Dsp::GetMaxVolAccurate(_pSamplesL, _pSamplesR, numSamples);
				};
				*/
			#endif // ndef _WINAMP_PLUGIN_
		};

		/// dummy machine.
		class Dummy : public Machine
		{
		public:
			/// Marks that the Dummy was in fact a VST plugin that couldn't be loaded
			bool wasVST;

			void Work(int numSamples);
			Dummy(int index);
			virtual char* GetName(void) { return _psName; };
			/// versions can only be older than current or this won't get called 
			inline virtual bool LoadSpecificFileChunk(RiffFile* pFile, int version)
			{
				UINT size;
				pFile->Read(&size, sizeof size); // size of this part params to load
				pFile->Skip(size);
				return true;
			};
		protected:
			static char* _psName;
		};

		/// master machine.
		class Master : public Machine
		{
		public:
			Master(int index);
			/// this is for the VstHost
			double sampleCount; 
			int _outDry;
			bool _clip;
			bool decreaseOnClip;
			static float* _pMasterSamples;
			virtual bool Load(RiffFile* pFile);
			virtual bool LoadSpecificFileChunk(RiffFile* pFile, int version)
			{
				UINT size;
				pFile->Read(&size, sizeof size ); // size of this part params to load
				pFile->Read(&_outDry,sizeof _outDry);
				pFile->Read(&decreaseOnClip, sizeof decreaseOnClip); // numSubtracks
				return true;
			};
			#if !defined(_WINAMP_PLUGIN_)
				//int _LMAX;
				//int _RMAX;
				int peaktime;
				float currentpeak;
				float _lMax;
				float _rMax;
				bool vuupdated;
				virtual void SaveSpecificChunk(RiffFile* pFile) 
				{
					UINT size = sizeof _outDry + sizeof decreaseOnClip;
					pFile->Write(&size, sizeof size); // size of this part params to load
					pFile->Write(&_outDry,sizeof _outDry);
					pFile->Write(&decreaseOnClip, sizeof decreaseOnClip); 
				};
			#endif // ndef _WINAMP_PLUGIN_
			Master();
			virtual void Init(void);
			virtual void Work(int numSamples);
			virtual char* GetName(void) { return _psName; };
		protected:
			static char* _psName;
		};
	}
}