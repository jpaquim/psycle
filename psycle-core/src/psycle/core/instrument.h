// This program is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
// You should have received a copy of the GNU General Public License along with this program; if not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
// copyright 2007-2009 members of the psycle project http://psycle.sourceforge.net

///\interface psycle::core::Instrument

#ifndef PSYCLE__CORE__INSTRUMENT__INCLUDED
#define PSYCLE__CORE__INSTRUMENT__INCLUDED
#pragma once

#include <psycle/core/config.hpp>
#include <universalis/stdlib/cstdint.hpp>
#include <string>

namespace psycle { namespace core {

class RiffFile;

/// an instrument is a waveform with some extra features added around it.
class PSYCLE__CORE__DECL Instrument {
	public:
		//PSYCLE__STRONG_TYPEDEF(int, id_type);
		typedef int id_type;

		Instrument();
		~Instrument();
		void Reset();
		void DeleteLayer(void);
		void LoadFileChunk(RiffFile* pFile,int version,bool fullopen=true);
		void SaveFileChunk(RiffFile* pFile);
		bool Empty();

		///\name Loop stuff
		///\{
			bool _loop;
			std::int32_t _lines;
		///\}

		///\verbatim
		/// NNA values overview:
		///
		/// 0 = Note Cut      [Fast Release 'Default']
		/// 1 = Note Release  [Release Stage]
		/// 2 = Note Continue [No NNA]
		///\endverbatim
		unsigned char _NNA;

		///\name Instrument assignation to a specific sampler
		///\{
		int _locked_machine_index; //-1 means not locked
		bool _locked_to_machine;
		///\}

		
		///\name Amplitude Envelope overview:
		///\{
			/// Attack Time [in Samples at 44.1Khz]
			std::int32_t ENV_AT;
			/// Decay Time [in Samples at 44.1Khz]
			std::int32_t ENV_DT;
			/// Sustain Level [in %]
			std::int32_t ENV_SL;
			/// Release Time [in Samples at 44.1Khz]
			std::int32_t ENV_RT;
		///\}
		
		///\name Filter 
		///\{
			/// Attack Time [in Samples at 44.1Khz]
			std::int32_t ENV_F_AT;
			/// Decay Time [in Samples at 44.1Khz]
			std::int32_t ENV_F_DT;
			/// Sustain Level [0..128]
			std::int32_t ENV_F_SL;
			/// Release Time [in Samples at 44.1Khz]
			std::int32_t ENV_F_RT;

			/// Cutoff Frequency [0-127]
			std::int32_t ENV_F_CO;
			/// Resonance [0-127]
			std::int32_t ENV_F_RQ;
			/// EnvAmount [-128,128]
			std::int32_t ENV_F_EA;
			/// Filter Type [0-4]
			std::int32_t ENV_F_TP;
		///\}

		std::int32_t _pan;
		bool _RPAN;
		bool _RCUT;
		bool _RRES;

		char _sName[32];

		///\name wave stuff
		///\{
			std::uint32_t waveLength;
			std::uint16_t waveVolume;
			std::uint32_t waveLoopStart;
			std::uint32_t waveLoopEnd;
			std::int32_t waveTune;
			std::int32_t waveFinetune;
			bool waveLoopType;
			bool waveStereo;
			char waveName[32];
			std::int16_t *waveDataL;
			std::int16_t *waveDataR;

			// xml copy paste methods
			std::string toXml() const;
			void setName( const std::string & name );
			void createHeader( const std::string & header );
			void createWavHeader( const std::string & name, const std::string & header );
			void setCompressedData( unsigned char* left, unsigned char* right );

			void getData( unsigned char* data, const std::string & dataStr);
		///\}
};

/// Instrument index of the wave preview.
Instrument::id_type const PREV_WAV_INS(255);

}}
#endif
