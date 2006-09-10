/***************************************************************************
 *   Copyright (C) 2006 by Stefan Nattkemper   *
 *   natti@linux   *
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
#include "ladspamachine.h"

#ifdef linux
	#include <dlfcn.h>
#endif

namespace psycle {
	namespace host {

		LADSPAMachine::LADSPAMachine( Machine::id_type id, Song* song )
		: Machine(MACH_LADSPA, MACHMODE_FX, id, song)
		{
			_audiorange = 1.0f;
			ladspa_path = std::getenv("LADSPA_PATH");
			std::cout << ladspa_path << std::endl;
			psDescriptor = 0;
		}


		LADSPAMachine::~LADSPAMachine() throw()
		{
		}

		bool LADSPAMachine::loadPlugin( const std::string & fileName )
		{			
			#ifdef linux
			pluginHandle_ = dlopen( std::string(ladspa_path + fileName).c_str() , RTLD_NOW);
			#endif
			if ( !pluginHandle_ ) {
				#ifdef linux
        std::cerr << "Cannot load library: " << dlerror() << std::endl;
				#endif
        return false;

				LADSPA_Descriptor_Function pfDescriptorFunction = 0;
  			unsigned long lPluginIndex;

				#ifdef linux
				pfDescriptorFunction
				= (LADSPA_Descriptor_Function)dlsym( pluginHandle_,
                                        "ladspa_descriptor");
				#endif

				if (!pfDescriptorFunction) {
					std::cerr << "Unable to  load : ladspa_descriptor" << std::endl;
					std::cerr << "Are you sure '"<< fileName.c_str() << "' is a ladspa file ?" << std::endl;
					#ifdef linux
						std::cerr << dlerror() << std::endl;
					#endif
					return false;
				}

				for (lPluginIndex = 0;; lPluginIndex++) {
					psDescriptor = pfDescriptorFunction(lPluginIndex);
    			if (psDescriptor == NULL) {
						std::cerr <<
              "Unable to find label in plugin library file" << std::endl;
     				return false;         
					}
    		}

				return true;
			}
		} // end of loadPlugin

	}
}


