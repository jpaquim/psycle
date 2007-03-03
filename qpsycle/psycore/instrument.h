///\file
///\brief interface file for psy::core::Filter. based on Revision 2078
#pragma once
#include "constants.h"
#include "fileio.h"
#include <string>

namespace psy
{
	namespace core
	{
		/// an instrument is a waveform with some extra features added around it.
		class Instrument
		{
		public:
			//PSYCLE__STRONG_TYPEDEF(int, id_type);
			typedef int id_type;

			Instrument();
			~Instrument();
			void Delete();
			void DeleteLayer(void);
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
			unsigned int waveLength;
			unsigned short waveVolume;
			unsigned int waveLoopStart;
			unsigned int waveLoopEnd;
			int waveTune;
			int waveFinetune;
			bool waveLoopType;
			bool waveStereo;
			char waveName[32];
			signed short *waveDataL;
			signed short *waveDataR;

			// xml copy paste methods
			std::string toXml() const;
			void setName( const std::string & name );
			void createHeader( const std::string & header );
			void createWavHeader( const std::string & name, const std::string & header );
			void setCompressedData( unsigned char* left, unsigned char* right );

			void getData( unsigned char* data, const std::string & dataStr);

			private:


			///\}
		};

		/// Instrument index of the wave preview.
		Instrument::id_type const PREV_WAV_INS(255);
	}
}
