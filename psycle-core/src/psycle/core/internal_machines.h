// -*- mode:c++; indent-tabs-mode:t -*-

/**********************************************************************************************
	Copyright 2007-2008 members of the psycle project http://psycle.sourceforge.net

	This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.
	This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
	You should have received a copy of the GNU General Public License along with this program; if not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
**********************************************************************************************/

#pragma once
#include "machine.h"

namespace psy { namespace core {

/****************************************************************************************************/
/// dummy machine.
class Dummy : public Machine {
	protected:
		Dummy(MachineCallbacks* callbacks, Machine::id_type index); friend class InternalHost;
	public:
		virtual ~Dummy() throw();
		/// Loader for psycle fileformat version 2.
		void CopyFrom(Machine* mac);
		virtual bool LoadSpecificChunk(RiffFile*,int version);
		virtual int GenerateAudio(int numSamples);
		virtual MachineKey getMachineKey() const { return MachineKey::dummy(); }
		virtual std::string GetName() const { return _psName; }
	protected:
		static std::string _psName;
};

/****************************************************************************************************/
/// note duplicator machine.
class DuplicatorMac : public Machine {
	protected:
		DuplicatorMac(MachineCallbacks* callbacks, Machine::id_type index); friend class InternalHost;
	public:
		virtual ~DuplicatorMac() throw();
		virtual void Init(void);
		virtual void Tick( int channel, const PatternEvent & pData );
		virtual void Stop();
		virtual void PreWork(int numSamples);
		virtual int GenerateAudio( int numSamples );
		virtual MachineKey getMachineKey() const { return MachineKey::duplicator(); }
		virtual std::string GetName() const { return _psName; }
		virtual void GetParamName(int numparam,char *name) const;
		virtual void GetParamRange(int numparam,int &minval,int &maxval) const;
		virtual void GetParamValue(int numparam,char *parVal) const;
		virtual int GetParamValue(int numparam) const;
		virtual bool SetParameter(int numparam,int value);
		virtual bool LoadSpecificChunk(RiffFile * pFile, int version);
		virtual void SaveSpecificChunk(RiffFile * pFile) const;

	protected:
		static const int NUM_MACHINES=8;
		void AllocateVoice(int channel, int machine);
		void DeallocateVoice(int channel, int machine);
		std::int16_t macOutput[NUM_MACHINES];
		std::int16_t noteOffset[NUM_MACHINES];
		static std::string _psName;
		bool bisTicking;
		// returns the allocated channel of the machine, for the channel (duplicator's channel) of this tick.
		int allocatedchans[MAX_TRACKS][NUM_MACHINES];
		// indicates if the channel of the specified machine is in use or not
		bool availablechans[MAX_MACHINES][MAX_TRACKS];
};

/****************************************************************************************************/
/// master machine.
class Master : public Machine {
	protected:
		Master(MachineCallbacks* callbacks, Machine::id_type index); friend class InternalHost;
	public:
		virtual ~Master() throw();
		virtual void Init(void);
		virtual void Stop();
		virtual void Tick(int channel, const PatternEvent & data );
		virtual int GenerateAudio( int numSamples );
		virtual MachineKey getMachineKey() const { return MachineKey::master(); }
		virtual std::string GetName() const { return _psName; }
		/// Loader for psycle fileformat version 2.
		virtual bool LoadPsy2FileFormat(RiffFile* pFile);
		virtual bool LoadSpecificChunk(RiffFile * pFile, int version);
		virtual void SaveSpecificChunk(RiffFile * pFile) const;

		/// this is for the VstHost
		double sampleCount;
		bool _clip;
		bool decreaseOnClip;
		static float* _pMasterSamples;
		int peaktime;
		float currentpeak;
		float _lMax;
		float _rMax;
		int _outDry;
		bool vuupdated;
	protected:
		static std::string _psName;
};

/****************************************************************************************************/
/// LFO machine
class LFO : public Machine {
	protected:
		LFO(MachineCallbacks* callbacks, Machine::id_type index); friend class InternalHost;
	public:
		virtual ~LFO() throw();
		virtual void Init(void);
		virtual void Tick( int channel, const PatternEvent & pData );
		virtual void PreWork(int numSamples);
		virtual int GenerateAudio( int numSamples );
		virtual MachineKey getMachineKey() const { return MachineKey::lfo(); }
		virtual std::string GetName() const { return _psName; }
		virtual void GetParamName(int numparam,char *name) const;
		virtual void GetParamRange(int numparam,int &minval,int &maxval) const;
		virtual void GetParamValue(int numparam,char *parVal) const;
		virtual int GetParamValue(int numparam) const;
		virtual bool SetParameter(int numparam,int value);
		virtual bool LoadSpecificChunk(RiffFile * pFile, int version);
		virtual void SaveSpecificChunk(RiffFile * pFile) const;


		///\name constants
		///\{
			int const static LFO_SIZE = 2048;
			int const static MAX_PHASE = LFO_SIZE;
			int const static MAX_SPEED = 100000;
			int const static MAX_DEPTH = 100;
			int const static NUM_CHANS = 6;
		///\}

		struct lfo_types {
			enum lfo_type {
				sine, tri, saw, /*sawdown,*/ square, ///< depth ranges from -100% to 100% -- inverse saw is redundant
				num_lfos
			};
		};

		struct prms {
			/// our parameter indices
			enum prm {
				wave, speed,
				mac0,   mac1,   mac2,   mac3,   mac4,   mac5,
				prm0,   prm1,   prm2,   prm3,   prm4,   prm5,
				level0, level1, level2, level3, level4, level5,
				phase0, phase1, phase2, phase3, phase4, phase5,
				num_params
			};
		};

	protected:
		/// fills the lfo table based on the value of waveform
		virtual void FillTable();
		/// initializes data to start modulating the param given by 'which'
		virtual void ParamStart(int which);
		/// resets a parameter that's no longer being modulated
		virtual void ParamEnd(int which);

		// parameter settings
	
		std::int16_t waveform;
		std::int32_t lSpeed;
		std::int16_t macOutput[NUM_CHANS];
		std::int16_t paramOutput[NUM_CHANS];
		std::int32_t phase[NUM_CHANS];
		std::int32_t level[NUM_CHANS];

		// internal state vars
	
		/// position in our lfo
		float lfoPos;
		/// our lfo
		float waveTable[LFO_SIZE];
		/// value of knob when last seen-- used to compensate for outside changes
		std::int16_t prevVal[NUM_CHANS];
		/// where knob should be at lfo==0
		std::int32_t centerVal[NUM_CHANS];

		static std::string _psName;
		bool bisTicking;
};

}}
