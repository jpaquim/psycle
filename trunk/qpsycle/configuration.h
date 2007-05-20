#pragma once
#include <psycore/configuration.h>
#include "inputhandler.h"
#include <string>
/// configuration for the user interface
namespace psy {
	namespace core {
		class AudioDriver;
	}
}
using psy::core::AudioDriver;
/// configuration for the user interface
class Configuration : public psy::core::Configuration
{
	public:
		Configuration();
	protected:
		/*override*/ void setDefaults();
	
	///\name configuration file
	///\{
		public:
			void loadConfig();
			void loadConfig( const std::string & path );
		private:
			/// creates an initial default xml config file
			//void setXmlDefaults();
	///\}
	
	///\name input handler
	///\{
		public:
			InputHandler const & inputHandler() const { return inputHandler_; }
		private:
			InputHandler inputHandler_;
			void configureKeyBindings();
	///\}

	///\name paths
	///\{
		public:
			const std::string & iconPath() const { return iconPath_; }
			const std::string & pluginPath() const { return pluginPath_; } // could be in core?
			const std::string & ladspaPath() const { return ladspaPath_; } // could be in core?
			const std::string & prsPath() const { return prsPath_; }
			const std::string & hlpPath() const { return hlpPath_; }
			const std::string & songPath() const { return songPath_; }
		private:
			std::string iconPath_;
			std::string pluginPath_;
			std::string prsPath_;
			std::string hlpPath_;
			std::string ladspaPath_;
			std::string songPath_;
	///\}

	///\name misc behaviour flavour settings
	///\{
		public:
			bool ft2HomeEndBehaviour() const { return ft2HomeEndBehaviour_; }
			bool shiftArrowForSelect() const { return shiftArrowForSelect_; }
			bool wrapAround() const { return wrapAround_; }
			bool centerCursor() const { centerCursor_; }
		private:
			bool ft2HomeEndBehaviour_;
			bool shiftArrowForSelect_;
			bool wrapAround_;
			bool centerCursor_;
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
