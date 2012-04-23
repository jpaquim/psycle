///\file
///\brief interface file for psycle::host::CSkinDlg.
#pragma once
#include <psycle/host/detail/project.hpp>
#include "Global.hpp"
#include "PsycleConfig.hpp"
#include "Preset.hpp"
namespace psycle { namespace host {

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
