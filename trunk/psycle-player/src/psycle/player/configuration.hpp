#pragma once
#include <string>
#include <map>
namespace psy {
	namespace core {
		class AudioDriver;
	}
}
using psy::core::AudioDriver;
/// configuration for the user interface
class Configuration
{
	public:
		Configuration();
		~Configuration();
	
	///\name configuration file
	///\{
		public:
			void loadConfig();
			void loadConfig( const std::string & path );
		private:
			/// creates an initial default xml config file
			//void setXmlDefaults();
	///\}
	
	///\name paths
	///\{
		public:
			const std::string & pluginPath() const { return pluginPath_; } // could be in core?
			const std::string & ladspaPath() const { return ladspaPath_; } // could be in core?
		private:
			std::string pluginPath_;
			std::string ladspaPath_;
	///\}

	///\name audio drivers
	///\{
		public:
			void setDriverByName( const std::string & driverName );        
			std::map<std::string, AudioDriver*> & driverMap() { return driverMap_; }
			bool enableSound() const { return enableSound_; }
		public://private: ///\ todo private access
			AudioDriver* _pOutputDriver;
			AudioDriver* _pSilentDriver;
		private:
			std::map<std::string, AudioDriver*> driverMap_;
			bool enableSound_;
			bool doEnableSound;
			void addAudioDriver(AudioDriver* driver);
	///\}
};
