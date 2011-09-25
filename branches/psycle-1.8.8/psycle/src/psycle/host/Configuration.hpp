///\file
///\interface psycle::host::Configuration.
#pragma once
#include <psycle/host/detail/project.hpp>
#include "Global.hpp"
#include <string>

namespace psycle
{
	namespace host
	{
		class AudioDriver;
		class ConfigStorage;

		/// configuration.
		class Configuration
		{
		public:
			Configuration();
			virtual ~Configuration();
			
			//Actions
			virtual void SetDefaultSettings(bool include_others=true);
			virtual void Load(ConfigStorage &);
			virtual void Save(ConfigStorage &);
			virtual void RefreshSettings() {}

			//Members

			std::string const & appPath              () const { return program_executable_dir_; }

			std::string const & GetPluginDir         () const { return plugin_dir_; }
			               void SetPluginDir         (std::string const &d) { plugin_dir_ = d; }
			std::string const & GetVst32Dir          () const { return vst32_dir_; }
			               void SetVst32Dir          (std::string const &d) { vst32_dir_ = d;}
			std::string const & GetVst64Dir          () const { return vst64_dir_; }
			               void SetVst64Dir          (std::string const &d) { vst64_dir_ = d;}
						   bool SupportsJBridge      () const;
			               void UseJBridge           (bool use);
			               bool UsesJBridge          () const;
			               void UsePsycleVstBridge   (bool use);
			               bool UsesPsycleVstBridge  () const;
			AudioDriver const & audioDriver          () const { return *_pOutputDriver; }
			               void SetDefaultPatLines   (int);
			               int  GetDefaultPatLines   () const;
			               void UseAutoStopMachines  (bool use);
			               bool UsesAutoStopMachines () const;
			               void LoadNewBlitz         (bool use);
			               bool LoadsNewBlitz        () const;

		public:
			AudioDriver* _pOutputDriver;
		private:
			bool prefferNewBlitz;
			std::string program_executable_dir_;
			std::string plugin_dir_;
			std::string plugin_dir_other;
			std::string vst32_dir_;
			std::string vst64_dir_;
		};
	}
}
