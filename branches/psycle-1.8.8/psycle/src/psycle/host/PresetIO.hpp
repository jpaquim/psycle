///\file
///\brief interface file for psycle::host::CSkinDlg.
#pragma once
#include "Psycle.hpp"

#include "PsycleConfig.hpp"
namespace psycle { namespace host {

		class CPreset  
		{
		public:
			CPreset();
			CPreset(const CPreset& newpreset);
			virtual ~CPreset();
			bool operator <(const CPreset& b) const;
			CPreset& operator=(const CPreset& newpreset);

			void Init(int num);
			void Init(int num,const char* newname,   int const * parameters,int size, void* newdata);
			void Init(int num,const char* newname, float const * parameters); // for VST .fxb's
			void Clear();
			int GetNumPars() const { return numPars; }
			void GetParsArray(int* destarray) const { if(numPars>0) std::memcpy(destarray, params, numPars * sizeof *params); }
			void GetDataArray(void* destarray) const {if(dataSize>0) std::memcpy(destarray, data, dataSize); }
			void* GetData() const {return data;}
			long int GetDataSize() const {return dataSize;}
			void SetName(const char *setname) { std::strcpy(name,setname); }
			void GetName(char *nname) const { std::strcpy(nname,name); }
			int GetParam(const int n) const;
			void SetParam(const int n,int val);
		private:
			int numPars;
			int* params;
			long int dataSize;
			unsigned char * data;
			char name[32];
		};

		/// skin file IO
		class PresetIO
		{
		protected:
			PresetIO();
			virtual ~PresetIO();
		public:
			static void LoadPresets(const char* szFile, int numParameters, int dataSizeStruct,std::list<CPreset>& presets, bool warn_if_notexists=true);
			static void SavePresets(const char* szFile, std::list<CPreset>& presets);

		protected:
			static void LoadVersion0(FILE* hfile, int numpresets, int numParameters, std::list<CPreset>& presets);
			static void LoadVersion1(FILE* hfile, int numParameters, int dataSizeStruct, std::list<CPreset>& presets);
			static void SaveVersion0(FILE* hfile, std::list<CPreset>& presets);
			static void SaveVersion1(FILE* hfile, std::list<CPreset>& presets);
		};
	}   // namespace
}   // namespace
