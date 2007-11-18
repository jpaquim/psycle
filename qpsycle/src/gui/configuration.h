#pragma once
#include "inputhandler.h"
#include <string>
#include <map>
/// configuration for the user interface
namespace psy {
	namespace core {
		class AudioDriver;
	}
}
using psy::core::AudioDriver;


enum KnobMode
{
	QDialMode, ///< Old QDial behaviour
	QSynthAngularMode, ///< Knob moves angularly as the mouse around the widget center
	QSynthLinearMode, ///< Knob moves proportonally to drag distance in one ortogonal axis
	PsycleLinearMode,
	FixedLinearMode
};



/// configuration for the user interface
class Configuration
{
	public:
		Configuration();
	
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
			const std::string & samplePath() const { return samplePath_; }
		private:
			std::string configFilePath_;
			std::string iconPath_;
			std::string pluginPath_;
			std::string prsPath_;
			std::string hlpPath_;
			std::string ladspaPath_;
			std::string songPath_;
			std::string samplePath_;
	///\}

	///\name misc behaviour flavour settings
	///\{
		public:
			bool ft2HomeEndBehaviour() const { return ft2HomeEndBehaviour_; }
			bool shiftArrowForSelect() const { return shiftArrowForSelect_; }
			bool wrapAround() const { return wrapAround_; }
			bool centerCursor() const { return centerCursor_; }

			KnobMode knobBehaviour() const { return knobBehaviour_; }
			void setKnobBehaviour( KnobMode behaviourType );

			void setFT2HomeEndBehaviour( bool state );
			void setShiftKeyBehaviour( bool state );
			void setWrapAround( bool state );
			void setCenterCursor( bool state );
		private:
			bool ft2HomeEndBehaviour_;
			bool shiftArrowForSelect_;
			bool wrapAround_;
			bool centerCursor_;
			KnobMode knobBehaviour_;
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
