#pragma once

#include "machine.h"

namespace psycle {
	namespace host {

		// dummy machine.
		class Dummy : public Machine
		{
		public:
			Dummy(int index, Song* song);
            virtual Dummy* clone() const;
			~Dummy();            

			virtual int GenerateAudio(int numSamples);
			virtual std::string GetName() const { return _psName; };
			virtual bool LoadSpecificChunk(RiffFile* pFile, int version);
			/// Marks that the Dummy was in fact a VST plugin that couldn't be loaded
			bool wasVST;
		protected:
			static std::string _psName;
		};


		// note duplicator machine.
		class DuplicatorMac : public Machine
		{
		public:
			DuplicatorMac();
			DuplicatorMac(int index, Song* song);
            virtual DuplicatorMac* clone() const;
			~DuplicatorMac();

			virtual void Init(void);
			virtual void Tick( int channel, const PatternEvent & pData );
			virtual void PreWork(int numSamples);
			virtual int GenerateAudio( int numSamples );
			virtual std::string GetName() const { return _psName; };
			virtual void GetParamName(int numparam,char *name);
			virtual void GetParamRange(int NUMPARSE,int &minval,int &maxval);
			virtual void GetParamValue(int numparam,char *parVal);
			virtual int GetParamValue(int numparam);
			virtual bool SetParameter(int numparam,int value);
			virtual bool LoadSpecificChunk(RiffFile * pFile, int version);
			virtual void SaveSpecificChunk(RiffFile * pFile);

		protected:
			short macOutput[8];
			short noteOffset[8];
			static std::string _psName;
			bool bisTicking;
		};


		/// master machine.
		class Master : public Machine
		{
		public:
			Master();
			Master(int index, Song* song);
            virtual Master* clone() const;
			~Master();

			virtual void Init(void);
			virtual void Stop();
			virtual void Tick(int channel, const PatternEvent & data );
			virtual int GenerateAudio( int numSamples );
			virtual std::string GetName() const { return _psName; };
			/// Loader for psycle fileformat version 2.
			virtual bool LoadPsy2FileFormat(RiffFile* pFile);
			virtual bool LoadSpecificChunk(RiffFile * pFile, int version);
			virtual void SaveSpecificChunk(RiffFile * pFile);

			/// this is for the VstHost
			double sampleCount;
			bool _clip;
			bool decreaseOnClip;
			static float* _pMasterSamples;
			int peaktime;
			float currentpeak;
			bool vuupdated;
		protected:
			static std::string _psName;
		};

}
}