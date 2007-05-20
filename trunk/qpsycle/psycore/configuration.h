/***************************************************************************
*   Copyright (C) 2007 Psycledelics Community   *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
*   This program is distributed in the hope that it will be useful,       *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
*   GNU General Public License for more details.                          *
*                                                                         *
*   You should have received a copy of the GNU General Public License     *
*   along with this program; if not, write to the                         *
*   Free Software Foundation, Inc.,                                       *
*   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
***************************************************************************/
#ifndef PSYCLE__CORE__CONFIGURATION
#define PSYCLE__CORE__CONFIGURATION

#include "inputhandler.h"

#include <string>

namespace psy {
	namespace core {
		class AudioDriver;  // FIXME: doesn't belong in psycore

		/// configuration for the core
		class CoreConfiguration
		{
		};
		
		/// configuration for the user interface
		class UIConfiguration : public CoreConfiguration
		{
			public:
				UIConfiguration();
				~UIConfiguration();
				void loadConfig();
	            void loadConfig( const std::string & path );
				void setDriverByName( const std::string & driverName );        

			public://private: ///\ todo private access
				AudioDriver* _pOutputDriver;
				AudioDriver* _pSilentDriver;
				///\ todo put this in player ..
				bool _RecordTweaks;
				bool _RecordUnarmed;
				
			public:
				std::map<std::string, AudioDriver*> & driverMap() { return driverMap_; }

				///\name paths
				///\{
					const std::string & iconPath() const;
					const std::string & pluginPath() const;
					const std::string & ladspaPath() const;
					const std::string & prsPath() const;
					const std::string & hlpPath() const;
					const std::string & songPath() const;
				///\}

				bool enableSound() const;
				bool ft2HomeEndBehaviour() const;
				bool shiftArrowForSelect() const;
	            bool wrapAround() const;
	            bool centerCursor() const;

				InputHandler & inputHandler();

			private:
				InputHandler inputHandler_;

				std::map<std::string, AudioDriver*> driverMap_;
				bool enableSound_;
				bool doEnableSound;

				std::string iconPath_;
				std::string pluginPath_;
				std::string prsPath_;
				std::string hlpPath_;
				std::string ladspaPath_;
				std::string songPath_;

				void addAudioDriver(AudioDriver* driver);
				void setDefaults();
				void configureKeyBindings();
				//void setXmlDefaults();

	            // Settings.
	            bool ft2HomeEndBehaviour_;
    	        bool shiftArrowForSelect_;
    	        bool wrapAround_;
    	        bool centerCursor_;
		};
	
		/// the actual configuration class used by qpsycle. it's simply the UIConfiguration class
		/// note that a simple typedef won't work due to the configuration class being forward-declared, as a class and not a typedef.
		class Configuration : public UIConfiguration
		{
		};
	}
}
#endif
