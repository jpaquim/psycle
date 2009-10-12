#ifndef PSYCLE__GUI__CONFIGURATION
#define PSYCLE__GUI__CONFIGURATION

#include "inputhandler.hpp"
#include <string>
#include <map>
/// configuration for the user interface
namespace psy {
	namespace core {
		class AudioDriver;
	}
}

using psy::core::AudioDriver;

namespace qpsycle {

	enum KnobMode {
		QDialMode, ///< Built-in QDial behaviour
		QSynthAngularMode, ///< Knob moves angularly as the mouse around the widget center
		PsycleLinearMode,
		FixedLinearMode
	};



	/// configuration for the user interface
	class Configuration {
		public:
			Configuration();
			~Configuration();
	
		///\name configuration file
		///\{
			public:
				void loadConfig();
				void loadConfig( const std::string & path );
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
			private:
				std::string configFilePath_;
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

		public://private: ///\ todo private access
			///\ todo put this in player ..
			bool _RecordTweaks;
			///\ todo put this in player ..
			bool _RecordUnarmed;
	};
}

#endif
