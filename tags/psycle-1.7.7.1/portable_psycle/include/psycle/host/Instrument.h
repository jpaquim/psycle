#pragma once
#if defined(_WINAMP_PLUGIN_)
	//#include <afxmt.h>
#endif
#include "Constants.h"
#include "FileIO.h"
///\file
///\brief interface file for psycle::host::Filter.
namespace psycle
{
	namespace host
	{
		/// an instrument is a waveform with some extra features added around it.
		class Instrument
		{
		public:
			Instrument();
			~Instrument();
			void Delete();
			void DeleteLayer(int c);
			void LoadFileChunk(RiffFile* pFile,int version,bool fullopen=true);
			void SaveFileChunk(RiffFile* pFile);
			bool Empty();

			///\name Loop stuff
			///\{
			bool _loop;
			int _lines;
			///\}

			///\verbatim
			/// NNA values overview:
			///
			/// 0 = Note Cut      [Fast Release 'Default']
			/// 1 = Note Release  [Release Stage]
			/// 2 = Note Continue [No NNA]
			///\endverbatim
			unsigned char _NNA;
			
			///\name Amplitude Envelope overview:
			///\{
			/// Attack Time [in Samples at 44.1Khz]
			int ENV_AT;	
			/// Decay Time [in Samples at 44.1Khz]
			int ENV_DT;	
			/// Sustain Level [in %]
			int ENV_SL;	
			/// Release Time [in Samples at 44.1Khz]
			int ENV_RT;	
			///\}
			
			///\name Filter 
			///\{
			/// Attack Time [in Samples at 44.1Khz]
			int ENV_F_AT;	
			/// Decay Time [in Samples at 44.1Khz]
			int ENV_F_DT;	
			/// Sustain Level [0..128]
			int ENV_F_SL;	
			/// Release Time [in Samples at 44.1Khz]
			int ENV_F_RT;	

			/// Cutoff Frequency [0-127]
			int ENV_F_CO;	
			/// Resonance [0-127]
			int ENV_F_RQ;	
			/// EnvAmount [-128,128]
			int ENV_F_EA;	
			/// Filter Type [0-4]
			int ENV_F_TP;	
			///\}

			int _pan;
			bool _RPAN;
			bool _RCUT;
			bool _RRES;

			char _sName[32];

			///\name wave stuff
			///\{
			unsigned int waveLength[MAX_WAVES];
			unsigned short waveVolume[MAX_WAVES];
			unsigned int waveLoopStart[MAX_WAVES];
			unsigned int waveLoopEnd[MAX_WAVES];
			int waveTune[MAX_WAVES];
			int waveFinetune[MAX_WAVES];	
			bool waveLoopType[MAX_WAVES];
			bool waveStereo[MAX_WAVES];
			char waveName[MAX_WAVES][32];
			signed short *waveDataL[MAX_WAVES];
			signed short *waveDataR[MAX_WAVES];
			///\}

		};
	}
}
