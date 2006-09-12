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
#include "player.h"
#include <dlfcn.h>

namespace psycle {
	namespace host {

		LADSPAMachine::LADSPAMachine( Machine::id_type id, Song* song )
		: Machine(MACH_LADSPA, MACHMODE_FX, id, song)
		{
			_audiorange = 1.0f;
			_editName = "ladspa plug";
			psDescriptor = 0;
			pluginHandle = 0;
			libHandle_=0;
		}


		LADSPAMachine::~LADSPAMachine() throw()
        {
                ///\todo: deactivate if activated
           if ( ppfValues ) delete[] ppfValues;
            if ( pluginHandle )
            {
                psDescriptor->deactivate(pluginHandle);
                psDescriptor->cleanup(pluginHandle);
             }
            if ( libHandle_ ) dlclose(libHandle_);
		}
		
/*****************************************************************************
* This function provides a wrapping of dlopen(). When the filename is
*   not an absolute path (i.e. does not begin with / character), this
*   routine will search the LADSPA_PATH for the file.
*****************************************************************************/
        void * LADSPAMachine::dlopenLADSPA(const char * pcFilename, int iFlag)
        {
            std::string filename_(pcFilename);
            char *pcBuffer;
            const char * pcStart, * pcEnd ,* pcLADSPAPath;
            bool endsInSO, needsSlash;
            size_t iFilenameLength;
            void * pvResult(NULL);
            
            if (filename_.compare(filename_.length()-3,3,".so"))
                filename_.append(".so");
            
            if (filename_.c_str()[0] == '/') {
                    /* The filename is absolute. Assume the user knows what he/she is
                           doing and simply dlopen() it. */
                pvResult = dlopen(filename_.c_str(), iFlag);
                if (pvResult != NULL)
                  return pvResult;

            }
            else {
                /* If the filename is not absolute then we wish to check along the
                   LADSPA_PATH path to see if we can find the file there. We do
                   NOT call dlopen() directly as this would find plugins on the
                   LD_LIBRARY_PATH, whereas the LADSPA_PATH is the correct place
                   to search. */

              pcLADSPAPath = std::getenv("LADSPA_PATH");
              if ( !pcLADSPAPath) pcLADSPAPath = "/usr/lib/ladspa/";
              std::string directory(pcLADSPAPath);
              int dotindex(0),dotpos(0),prevdotpos(0);

              dotpos = directory.find(':',dotindex++);
              while (dotpos != directory.npos) {
              	std::string fullname = directory.substr(prevdotpos,dotpos);
              	if (fullname.c_str()[fullname.length()-1] != '/' )
               	   fullname.append("/");
               	fullname.append(filename_);
            	
                pvResult = dlopen(fullname.c_str(), iFlag);
            	
                if (pvResult != NULL)
                  return pvResult;
                prevdotpos = dotpos;            	
              }
            }

            /* If nothing has worked, then at least we can make sure we set the
               correct error message - and this should correspond to a call to
               dlopen() with the actual filename requested. */
            return dlopen(pcFilename, iFlag);
        }
		

        bool LADSPAMachine::loadDll( const std::string & fileName, int pluginIndex )
		{	
		      // Step one: Open the shared library.
			libHandle_ = dlopenLADSPA( fileName.c_str() , RTLD_NOW);
			if ( !libHandle_ ) {
				std::cerr << "Cannot load library: " << dlerror() << std::endl;
		        return false;
			}
            // Step two: Get the entry function.
			LADSPA_Descriptor_Function pfDescriptorFunction =
				 (LADSPA_Descriptor_Function)dlsym( libHandle_, "ladspa_descriptor");

			if (!pfDescriptorFunction) {
				std::cerr << "Unable to  load : ladspa_descriptor" << std::endl;
				std::cerr << "Are you sure '"<< fileName.c_str() << "' is a ladspa file ?" << std::endl;
				std::cerr << dlerror() << std::endl;
				dlclose(libHandle_); libHandle_=0;
				return false;
			}
			/*Step three: Get the descriptor of the selected plugin (a shared library can have
			     several plugins*/
			psDescriptor = pfDescriptorFunction(pluginIndex);
            if (psDescriptor == NULL) {
				std::cerr <<  "Unable to find the selected plugin  in the library file" << std::endl;
    			dlclose(libHandle_); libHandle_=0;
   				return false;       
			}
			// Step four: Create (instantiate) the plugin, so that we can use it.
			pluginHandle = psDescriptor->instantiate(psDescriptor,Player::Instance()->timeInfo().sampleRate());
			if ( !pluginHandle) 
    			return false;

   			// Step five: Prepare the structures to use the plugin with the program.
   			prepareStructures();
   			
   			// and switch on:
            psDescriptor->activate(pluginHandle);
   			return true;
		} // end of loadPlugin


		void LADSPAMachine::prepareStructures()
		{
		      // Audio Buffers
		    LADSPA_Data *ppbuffers[]={_pSamplesL,_pSamplesR};
            int lBufferIndex = 0;
            for (int lPortIndex = 0; lPortIndex < psDescriptor->PortCount; lPortIndex++) {
                LADSPA_PortDescriptor iPortDescriptor = psDescriptor->PortDescriptors[lPortIndex];
                if (LADSPA_IS_PORT_INPUT(iPortDescriptor) 
                    && LADSPA_IS_PORT_AUDIO(iPortDescriptor))
                        psDescriptor->connect_port(pluginHandle,lPortIndex,
                    	   ppbuffers[lBufferIndex++]);
                 // Only Stereo for now.
                 if (lBufferIndex==2) break;
            }
            
            lBufferIndex = 0;
            for (int lPortIndex = 0; lPortIndex < psDescriptor->PortCount; lPortIndex++) {
                LADSPA_PortDescriptor iPortDescriptor = psDescriptor->PortDescriptors[lPortIndex];
                if (LADSPA_IS_PORT_OUTPUT(iPortDescriptor) 
                    && LADSPA_IS_PORT_AUDIO(iPortDescriptor))
                        psDescriptor->connect_port(pluginHandle,lPortIndex,
                    	   ppbuffers[lBufferIndex++]);
                 // Only Stereo for now.
                 if (lBufferIndex==2) break;
            }

            // Controls
            ppfValues = new LADSPA_Data*[psDescriptor->PortCount];
            lBufferIndex = 0;
            for (int lPortIndex = 0; lPortIndex < psDescriptor->PortCount; lPortIndex++) {
                LADSPA_PortDescriptor iPortDescriptor = psDescriptor->PortDescriptors[lPortIndex];
                if (LADSPA_IS_PORT_CONTROL(iPortDescriptor)) {
                    if (LADSPA_IS_PORT_INPUT(iPortDescriptor))
                        psDescriptor->connect_port(pluginHandle,lPortIndex,
                    	   ppfValues[lBufferIndex++]);
                    if (LADSPA_IS_PORT_OUTPUT(iPortDescriptor))
                        psDescriptor->connect_port(pluginHandle,lPortIndex,
                    	   ppfValues[lBufferIndex++]);
                }
            }
		}
		
        void LADSPAMachine::Init()
        {
            // Not sure what should we do here.
        }
        void LADSPAMachine::Tick(int channel, const PatternEvent & pEntry )
        {
            if ( pEntry.note() == notecommands::tweak || pEntry.note() == notecommands::tweakslide)
            {
                // modify parameters?
            }
        }
	

		int LADSPAMachine:: GenerateAudioInTicks( int startSample, int numSamples )
		{
	           psDescriptor->run(pluginHandle,numSamples);
	           return numSamples;
		}
         void  LADSPAMachine::GetParamName(int numparam, char * name) {}
         void  LADSPAMachine::GetParamRange(int numparam,int &minval, int &maxval) {}
         int  LADSPAMachine::GetParamValue(int numparam) { return 0; } 
         void LADSPAMachine:: GetParamValue(int numparam,char* parval) {}
         bool  LADSPAMachine::SetParameter(int numparam,int value) {return false;}

		bool  LADSPAMachine::LoadSpecificChunk(RiffFile * pFile, int version) {return false;}
		void  LADSPAMachine::SaveSpecificChunk(RiffFile * pFile) {}
		void  LADSPAMachine::SaveDllName      (RiffFile * pFile){}



	}
}


