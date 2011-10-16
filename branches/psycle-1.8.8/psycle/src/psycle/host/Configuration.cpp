///\file
///\implementation psycle::host::Configuration.

#include <psycle/host/detail/project.private.hpp>
#include "Configuration.hpp"
#include "ConfigStorage.hpp"
#include "Song.hpp"
#include "VstHost24.hpp"
namespace psycle { namespace host {

		/////////////////////////////////////////////
		Configuration::Configuration() : _pOutputDriver(0)
		{
			char c[MAX_PATH];
			c[0]='\0';
			::GetModuleFileName(0, c, sizeof c);
			program_executable_dir_ = c;
			program_executable_dir_ = program_executable_dir_.substr(0, program_executable_dir_.rfind('\\')) + '\\';

			SetDefaultSettings();
		}

		Configuration::~Configuration() throw()
		{
		}
		//////////////////////////////////
		void Configuration::SetDefaultSettings(bool include_others)
		{
			SetPluginDir(appPath()+"PsyclePlugins");
			#if PSYCLE__CONFIGURATION__USE_BUILT_PLUGINS
				if(std::getenv("PSYCLE__CONFIGURATION__USE_BUILT_PLUGINS"))
					SetPluginDir(appPath() + "psycle-plugins");
			#endif
			SetCacheDir(universalis::os::fs::home_app_local(PSYCLE__NAME));
			SetVst32Dir(appPath()+"VstPlugins");
			SetVst64Dir(appPath()+"VstPlugins64");
			UseJBridge(false);
			UsePsycleVstBridge(false);
			SetDefaultPatLines(64);
			UseAutoStopMachines(false);
			LoadNewBlitz(false);
		}
		void Configuration::Load(ConfigStorage & store)
		{
			#if PSYCLE__CONFIGURATION__USE_BUILT_PLUGINS
				if(!std::getenv("PSYCLE__CONFIGURATION__USE_BUILT_PLUGINS"))
			#endif
			#if defined _WIN64
				store.Read("PluginDir64",plugin_dir_);
				store.Read("PluginDir",plugin_dir_other);
			#elif defined _WIN32
				store.Read("PluginDir",plugin_dir_);
				store.Read("PluginDir64",plugin_dir_other);
			#endif
			store.Read("VstDir",vst32_dir_);
			store.Read("VstDir64",vst64_dir_);
			bool use=false;
			if(store.Read("jBridge", use)) {
				UseJBridge(use);
			}
			use=false;
			if(store.Read("psycleBridge", use)) {
				UsePsycleVstBridge(use);
			}
			int defaultPatLines=0;
			if(store.Read("defaultPatLines", defaultPatLines))
			{
				SetDefaultPatLines(defaultPatLines);
			}
			use=false;
			if(store.Read("autoStopMachines", use))
			{
				UseAutoStopMachines(use);
			}
			use=false;
			if(store.Read("prefferNewBlitz", use))
			{
				LoadNewBlitz(use);
			}
		}
		void Configuration::Save(ConfigStorage & store)
		{
			#if PSYCLE__CONFIGURATION__USE_BUILT_PLUGINS
				if(!std::getenv("PSYCLE__CONFIGURATION__USE_BUILT_PLUGINS"))
			#endif
				#if defined _WIN64
					store.Write("PluginDir64",plugin_dir_);
					store.Write("PluginDir",plugin_dir_other);
				#elif defined _WIN32
					store.Write("PluginDir",plugin_dir_);
					store.Write("PluginDir64",plugin_dir_other);
				#endif
			store.Write("VstDir",vst32_dir_);
			store.Write("VstDir64",vst64_dir_);

			bool use=UsesJBridge();
			store.Write("jBridge", use);

			use=UsesPsycleVstBridge();
			store.Write("psycleBridge", use);
				
			int defaultPatLines=GetDefaultPatLines();
			store.Write("defaultPatLines", defaultPatLines);

			use=UsesAutoStopMachines();
			store.Write("autoStopMachines", use);

			use=LoadsNewBlitz();
			store.Write("prefferNewBlitz", use);
		}

		bool Configuration::SupportsJBridge() const
		{
			char testjBridge[MAX_PATH];
			JBridge::getJBridgeLibrary(testjBridge, MAX_PATH);
			return testjBridge[0]!='\0';
		}
		void Configuration::UseJBridge(bool use) 
		{
			vst::host::UseJBridge(use);
		}
		bool Configuration::UsesJBridge() const 
		{
			return vst::host::UseJBridge();
		}
		void Configuration::UsePsycleVstBridge(bool use)
		{
			vst::host::UsePsycleVstBridge(use);
		}
		bool Configuration::UsesPsycleVstBridge() const
		{
			return vst::host::UsePsycleVstBridge();
		}

		void Configuration::SetDefaultPatLines(int defLines)
		{
			Song::SetDefaultPatLines(defLines);
		}
        int  Configuration::GetDefaultPatLines() const
		{
			return Song::GetDefaultPatLines();
		}
       void Configuration::UseAutoStopMachines(bool use)
	   {
		   Machine::autoStopMachine = use;
	   }
       bool Configuration::UsesAutoStopMachines() const
	   {
		   return Machine::autoStopMachine;
	   }
       void Configuration::LoadNewBlitz(bool use)
	   {
		   prefferNewBlitz = use;
	   }
       bool Configuration::LoadsNewBlitz() const
	   {
		   return prefferNewBlitz;
	   }
	}
}
