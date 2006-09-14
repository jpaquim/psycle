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
			pValues=0;
			psDescriptor = 0;
			pluginHandle = 0;
			libHandle_=0;
		}


		LADSPAMachine::~LADSPAMachine() throw()
        {
                ///\todo: deactivate if activated
           if ( pValues ) delete[] pValues;
            if ( pluginHandle )
            {
                if ( psDescriptor->deactivate ) psDescriptor->deactivate(pluginHandle);
                psDescriptor->cleanup(pluginHandle);
			    pluginHandle=0;
				psDescriptor=0;
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
            std::cout << filename_ << std::endl;
            if (filename_.compare(filename_.length()-3,3,".so"))
                filename_.append(".so");
            std::cout << filename_ << std::endl;
            
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

            std::cout << directory << std::endl;
              dotpos = directory.find(':',dotindex++);
              do{
              	std::string fullname = directory.substr(prevdotpos,dotpos);
              	if (fullname.c_str()[fullname.length()-1] != '/' )
               	   fullname.append("/");
               	fullname.append(filename_);
            	            std::cout << fullname << std::endl;

                pvResult = dlopen(fullname.c_str(), iFlag);
            	
                if (pvResult != NULL)
                  return pvResult;
                prevdotpos = dotpos;            	
               dotpos = directory.find(':',dotindex++);
              } while (dotpos != directory.npos);
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
			std::cout << "step two" << std::endl;
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
			std::cout << "step three" << std::endl;
			psDescriptor = pfDescriptorFunction(pluginIndex);
            if (psDescriptor == NULL) {
				std::cerr <<  "Unable to find the selected plugin  in the library file" << std::endl;
    			dlclose(libHandle_); libHandle_=0;
   				return false;       
			}
			// Step four: Create (instantiate) the plugin, so that we can use it.
			std::cout << "step four" << std::endl;
		
			pluginHandle = psDescriptor->instantiate(psDescriptor,Player::Instance()->timeInfo().sampleRate());
			if ( !pluginHandle) 
    			return false;

   			// Step five: Prepare the structures to use the plugin with the program.
			std::cout << "step five" << std::endl;
   			prepareStructures();
   			
   			// and switch on:
   						std::cout << "step six" << std::endl;

            if (psDescriptor->activate) psDescriptor->activate(pluginHandle);
			libName_ = fileName;
   			return true;
		} // end of loadPlugin


		LADSPA_Descriptor_Function LADSPAMachine::loadDescriptorFunction( const std::string & fileName ) {
			// Step one: Open the shared library.
			libHandle_ = dlopenLADSPA( fileName.c_str() , RTLD_NOW);
			if ( !libHandle_ ) {
				std::cerr << "Cannot load library: " << dlerror() << std::endl;
		        return 0;
			}
			std::cout << "step two" << std::endl;
            // Step two: Get the entry function.
			LADSPA_Descriptor_Function pfDescriptorFunction =
				 (LADSPA_Descriptor_Function)dlsym( libHandle_, "ladspa_descriptor");

			if (!pfDescriptorFunction) {
				std::cerr << "Unable to  load : ladspa_descriptor" << std::endl;
				std::cerr << "Are you sure '"<< fileName.c_str() << "' is a ladspa file ?" << std::endl;
				std::cerr << dlerror() << std::endl;
				dlclose(libHandle_); libHandle_=0;
				return 0;
			}

			return pfDescriptorFunction;

		}

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
            pValues = new LADSPA_Data[psDescriptor->PortCount];
            lBufferIndex = 0;
            for (int lPortIndex = 0; lPortIndex < psDescriptor->PortCount; lPortIndex++) {
                LADSPA_PortDescriptor iPortDescriptor = psDescriptor->PortDescriptors[lPortIndex];
                if (LADSPA_IS_PORT_CONTROL(iPortDescriptor)) {
                    if (LADSPA_IS_PORT_INPUT(iPortDescriptor))
                        psDescriptor->connect_port(pluginHandle,lPortIndex,
                    	   &pValues[lBufferIndex++]);
                    else if (LADSPA_IS_PORT_OUTPUT(iPortDescriptor))
                        psDescriptor->connect_port(pluginHandle,lPortIndex,
                    	   &pValues[lBufferIndex++]);
                }
            }
		}
		
        void LADSPAMachine::Init()
        {
            // Not sure what should we do here.
			SetDefaultsForControls();
        }
        void LADSPAMachine::Tick(int channel, const PatternEvent & pEntry )
        {
            if ( pEntry.note() == notecommands::tweak || pEntry.note() == notecommands::tweakslide)
            {
                SetParameter(pEntry.instrument(),pEntry.command()*0x100+pEntry.parameter());
            }
        }
	

		int LADSPAMachine:: GenerateAudio(int numSamples )
		{
	           psDescriptor->run(pluginHandle,numSamples);
	           return numSamples;
		}
         void  LADSPAMachine::GetParamName(int numparam, char * name)
		{
			strcpy(name,psDescriptor->PortNames[numparam]);
		}
         void  LADSPAMachine::GetParamRange(int numparam,int &minval, int &maxval)
		{
			LADSPA_PortRangeHintDescriptor iHintDescriptor = psDescriptor->PortRangeHints[numparam].HintDescriptor;
			minval=GetMinValue(numparam,iHintDescriptor)*32768;
			maxval=GetMaxValue(numparam,iHintDescriptor)*32768;
			
		}
		///\ todo: use the hings in the get/setparamvalue
         int  LADSPAMachine::GetParamValue(int numparam) { return pValues[numparam]; } 
         void LADSPAMachine:: GetParamValue(int numparam,char* parval) {}
         bool  LADSPAMachine::SetParameter(int numparam,int value) {pValues[numparam]=value; return true;}

		bool  LADSPAMachine::LoadSpecificChunk(RiffFile * pFile, int version) {return false;}
		void  LADSPAMachine::SaveSpecificChunk(RiffFile * pFile) {}
		void  LADSPAMachine::SaveDllName      (RiffFile * pFile){}

		LADSPA_Data LADSPAMachine::GetMinValue(int lPortIndex, LADSPA_PortRangeHintDescriptor iHintDescriptor)
		{
				if (LADSPA_IS_HINT_BOUNDED_BELOW(iHintDescriptor)) {
					LADSPA_Data fBound = psDescriptor->PortRangeHints[lPortIndex].LowerBound;
/*					if (LADSPA_IS_HINT_SAMPLE_RATE(iHintDescriptor) && fBound != 0) 
					  printf("%g*srate", fBound);
					else
					  printf("%g", fBound);
*/				  
					  return fBound;
				}
				else if (LADSPA_IS_HINT_TOGGLED(iHintDescriptor)) {
					return 0;
				}
		}
		LADSPA_Data LADSPAMachine::GetMaxValue(int lPortIndex, LADSPA_PortRangeHintDescriptor iHintDescriptor)
		{
				  if (LADSPA_IS_HINT_BOUNDED_ABOVE(iHintDescriptor)) {
					LADSPA_Data fBound = psDescriptor->PortRangeHints[lPortIndex].UpperBound;
/*					if (LADSPA_IS_HINT_SAMPLE_RATE(iHintDescriptor) && fBound != 0)
					  printf("%g*srate", fBound);
					else
					  printf("%g", fBound);
*/				  
			  }
				else if (LADSPA_IS_HINT_TOGGLED(iHintDescriptor)) {
					return 1;
				}
		}

		void LADSPAMachine::SetDefaultsForControls()
		{
            pValues = new LADSPA_Data[psDescriptor->PortCount];
            int lBufferIndex = 0;
			LADSPA_Data fDefault=0;
            for (int lPortIndex = 0; lPortIndex < psDescriptor->PortCount; lPortIndex++) {
                LADSPA_PortDescriptor iPortDescriptor = psDescriptor->PortDescriptors[lPortIndex];
                if (LADSPA_IS_PORT_CONTROL(iPortDescriptor)) {
                    if (LADSPA_IS_PORT_INPUT(iPortDescriptor))
					{
						LADSPA_PortRangeHintDescriptor iHintDescriptor = psDescriptor->PortRangeHints[lPortIndex].HintDescriptor;					
						switch (iHintDescriptor & LADSPA_HINT_DEFAULT_MASK) {
						case LADSPA_HINT_DEFAULT_NONE:
						  break;
						case LADSPA_HINT_DEFAULT_MINIMUM:
						  fDefault = psDescriptor->PortRangeHints[lPortIndex].LowerBound;
						  break;
						case LADSPA_HINT_DEFAULT_LOW:
						  if (LADSPA_IS_HINT_LOGARITHMIC(iHintDescriptor)) {
							fDefault 
							  = exp(log(psDescriptor->PortRangeHints[lPortIndex].LowerBound) 
								* 0.75
								+ log(psDescriptor->PortRangeHints[lPortIndex].UpperBound) 
								* 0.25);
						  }
						  else {
							fDefault 
							  = (psDescriptor->PortRangeHints[lPortIndex].LowerBound
							 * 0.75
							 + psDescriptor->PortRangeHints[lPortIndex].UpperBound
							 * 0.25);
						  }
						  break;
						case LADSPA_HINT_DEFAULT_MIDDLE:
						  if (LADSPA_IS_HINT_LOGARITHMIC(iHintDescriptor)) {
							fDefault 
							  = sqrt(psDescriptor->PortRangeHints[lPortIndex].LowerBound
								 * psDescriptor->PortRangeHints[lPortIndex].UpperBound);
						  }
						  else {
							fDefault 
							  = 0.5 * (psDescriptor->PortRangeHints[lPortIndex].LowerBound
								   + psDescriptor->PortRangeHints[lPortIndex].UpperBound);
						  }
						  break;
						case LADSPA_HINT_DEFAULT_HIGH:
						  if (LADSPA_IS_HINT_LOGARITHMIC(iHintDescriptor)) {
							fDefault 
							  = exp(log(psDescriptor->PortRangeHints[lPortIndex].LowerBound) 
								* 0.25
								+ log(psDescriptor->PortRangeHints[lPortIndex].UpperBound) 
								* 0.75);
						  }
						  else {
							fDefault 
							  = (psDescriptor->PortRangeHints[lPortIndex].LowerBound
							 * 0.25
							 + psDescriptor->PortRangeHints[lPortIndex].UpperBound
							 * 0.75);
						  }
						  break;
						case LADSPA_HINT_DEFAULT_MAXIMUM:
						  fDefault = psDescriptor->PortRangeHints[lPortIndex].UpperBound;
						  break;
						case LADSPA_HINT_DEFAULT_0:
						  fDefault=0;
						  break;
						case LADSPA_HINT_DEFAULT_1:
						  fDefault=1;
						  break;
						case LADSPA_HINT_DEFAULT_100:
						  fDefault=100;
						  break;
						case LADSPA_HINT_DEFAULT_440:
						  fDefault=440;
						  break;
						default:
						  break;
						}
						if (LADSPA_IS_HINT_SAMPLE_RATE(iHintDescriptor) && fDefault != 0) {
							// fDefault*=samplerate;
						}
						pValues[lPortIndex]=fDefault;
						
					}
                }
            }
		}

	}
}
