/***************************************************************************
*   Copyright (C) 2007 Psycledelics     *
*   psycle.sf.net   *
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
#include <psycle/core/psycleCorePch.hpp>

#include "ladspamachine.h"

#include "dsp.h"
#include "player.h"

#include <iostream> // only for debug output
#include <sstream>

#if defined __unix__ || defined __APPLE__
	#include <dlfcn.h>
#else
	#include <windows.h>
#endif

namespace psy {
	namespace core {
	
		///\todo: Improve the case where no min/max limit is given by the plugin (Example: the amp.so doesn't have a max value).
		LadspaParam::LadspaParam(LADSPA_PortDescriptor descriptor,LADSPA_PortRangeHint hint, const char *newname)
		:descriptor_(descriptor)
		,hint_(hint)
		,portName_(newname)
		,integer_(false)
		,logaritmic_(false)
		,rangeMultiplier_(1)
		{
			if (LADSPA_IS_HINT_TOGGLED(hint.HintDescriptor)) {
				minVal_= 0; maxVal_ = 1; integer_= true;
			}
			else 
			{
				if (LADSPA_IS_HINT_BOUNDED_BELOW(hint.HintDescriptor)) {
					minVal_= hint.LowerBound;
				}
				else minVal_ = 0;
				if (LADSPA_IS_HINT_BOUNDED_ABOVE(hint.HintDescriptor)) {
					maxVal_ = hint.UpperBound;
				}
				else maxVal_ = 1;
				if (LADSPA_IS_HINT_SAMPLE_RATE(hint.HintDescriptor)) {
					maxVal_*=Player::Instance()->timeInfo().sampleRate();
				}

				if ( LADSPA_IS_HINT_LOGARITHMIC(hint.HintDescriptor) ){
					logaritmic_ = true;
					// rangeMultiplier_ =   9 / (maxVal_ - minVal_);
					rangeMultiplier_ =   (exp(1.0)-1) / (maxVal_ - minVal_);
				}
				else if ( LADSPA_IS_HINT_INTEGER(hint.HintDescriptor) ){
					integer_ = true;
				} else {
					rangeMultiplier_ =   65535.0f / (maxVal_ - minVal_);
				}
			}
			setDefault();
			std::cout << "min/max/def" << minVal_ << "/" << maxVal_ << "/" << value_ << std::endl;
		}
		
		void LadspaParam::setDefault()
		{
			LADSPA_Data fDefault=0.0f;
			switch (hint_.HintDescriptor & LADSPA_HINT_DEFAULT_MASK) {
			case LADSPA_HINT_DEFAULT_NONE:
				break;
			case LADSPA_HINT_DEFAULT_MINIMUM:
				fDefault = hint_.LowerBound * (float)((LADSPA_IS_HINT_SAMPLE_RATE(hint_.HintDescriptor)) ? (float)Player::Instance()->timeInfo().sampleRate() : 1.0f);
				break;
			case LADSPA_HINT_DEFAULT_LOW:
				if (LADSPA_IS_HINT_LOGARITHMIC(hint_.HintDescriptor)) {
					fDefault 
					= exp(log(hint_.LowerBound) * 0.75
					+ log(hint_.UpperBound) * 0.25) * (float)((LADSPA_IS_HINT_SAMPLE_RATE(hint_.HintDescriptor)) ? (float)Player::Instance()->timeInfo().sampleRate() : 1.0f);
				}
				else {
					fDefault 
					= (hint_.LowerBound * 0.75
					+ hint_.UpperBound * 0.25)* (float)((LADSPA_IS_HINT_SAMPLE_RATE(hint_.HintDescriptor)) ? (float)Player::Instance()->timeInfo().sampleRate() : 1.0f);
				}
				break;
			case LADSPA_HINT_DEFAULT_MIDDLE:
				if (LADSPA_IS_HINT_LOGARITHMIC(hint_.HintDescriptor)) {
					fDefault 
					= sqrt(hint_.LowerBound
					* hint_.UpperBound) * (float)((LADSPA_IS_HINT_SAMPLE_RATE(hint_.HintDescriptor)) ? (float)Player::Instance()->timeInfo().sampleRate() : 1.0f);
				}
				else {
					fDefault 
					= 0.5 * (hint_.LowerBound
					+ hint_.UpperBound) * (float)((LADSPA_IS_HINT_SAMPLE_RATE(hint_.HintDescriptor)) ? (float)Player::Instance()->timeInfo().sampleRate() : 1.0f);
				}
				break;
			case LADSPA_HINT_DEFAULT_HIGH:
				if (LADSPA_IS_HINT_LOGARITHMIC(hint_.HintDescriptor)) {
					fDefault 
					= exp(log(hint_.LowerBound) * 0.25
					+ log(hint_.UpperBound) * 0.75) * (float)((LADSPA_IS_HINT_SAMPLE_RATE(hint_.HintDescriptor)) ? (float)Player::Instance()->timeInfo().sampleRate() : 1.0f);
				}
				else {
					fDefault 
					= (hint_.LowerBound * 0.25
					+ hint_.UpperBound * 0.75) * (float)((LADSPA_IS_HINT_SAMPLE_RATE(hint_.HintDescriptor)) ? (float)Player::Instance()->timeInfo().sampleRate() : 1.0f);
				}
				break;
			case LADSPA_HINT_DEFAULT_MAXIMUM:
				fDefault = hint_.UpperBound* (float)((LADSPA_IS_HINT_SAMPLE_RATE(hint_.HintDescriptor)) ? (float)Player::Instance()->timeInfo().sampleRate() : 1.0f);
				break;
			case LADSPA_HINT_DEFAULT_0:
				fDefault=0.0f;
				break;
			case LADSPA_HINT_DEFAULT_1:
				fDefault=1.0f;
				break;
			case LADSPA_HINT_DEFAULT_100:
				fDefault=100.0f;
				break;
			case LADSPA_HINT_DEFAULT_440:
				fDefault=440.0f;
				break;
			default:
				break;
			}
			value_ = fDefault;
		}
		
		int LadspaParam::value() const
		{ 
			return (integer_)? value_ :
				// (logaritmic_) ? log10(1+((value_-minVal_)*rangeMultiplier_))*65535.0f:
				(logaritmic_) ?  log(1 + ((value_ - minVal_) * rangeMultiplier_)) * 65535.0f:
				(value_- minVal_)*rangeMultiplier_;
		}
		void LadspaParam::setValue(int data)
		{
			value_ = (integer_) ? data :
				// (logaritmic_) ? minVal_ + (pow(10, data/65535.0f)-1)/ rangeMultiplier_ :
				(logaritmic_) ? minVal_ + (exp(data / 65535.0f) - 1) / rangeMultiplier_ :
				minVal_+ (data/rangeMultiplier_);
		}
		
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
		LADSPAMachine::LADSPAMachine(MachineCallbacks* callbacks, Machine::id_type id, CoreSong* song )
		:
			Machine(callbacks,MACH_LADSPA, MACHMODE_FX, id, song)
		{
			SetEditName("ladspa plug");
			SetAudioRange(1.0f);
			psDescriptor = 0;
			pluginHandle = 0;
			libHandle_=0;
			pOutSamplesL= new LADSPA_Data[STREAM_SIZE];
			pOutSamplesR= new LADSPA_Data[STREAM_SIZE];
		}
		
		LADSPAMachine::~LADSPAMachine() throw()
		{
			if ( pluginHandle )
			{
				if ( psDescriptor->deactivate ) psDescriptor->deactivate(pluginHandle);
				psDescriptor->cleanup(pluginHandle);
				pluginHandle=0;
				psDescriptor=0;
				}
			if ( libHandle_ ) {
				#if defined __unix__ || defined __APPLE__
				dlclose(libHandle_);
				#else
				::FreeLibrary( static_cast<HINSTANCE> ( libHandle_ ) ) ;
				#endif

			}
			delete[] pOutSamplesL;
			delete[] pOutSamplesR;
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
			//std::cout << filename_ << std::endl;
			#if defined __unix__ || defined __APPLE__
			if (filename_.compare(filename_.length()-3,3,".so"))
				filename_.append(".so");
			#else
			if (filename_.compare(filename_.length()-3,3,".dll"))
				filename_.append(".dll");
			#endif
			//std::cout << filename_ << std::endl;
			
			#if defined __unix__ || defined __APPLE__
			if (filename_.c_str()[0] == '/') {
			#else
			if (filename_.c_str()[0] == '\\') {
			#endif
					/* The filename is absolute. Assume the user knows what he/she is
						doing and simply dlopen() it. */
				#if defined __unix__ || defined __APPLE__
				pvResult = dlopen(filename_.c_str(), iFlag);
				#else
				// Set error mode to disable system error pop-ups (for LoadLibrary)
				UINT uOldErrorMode = SetErrorMode(SEM_FAILCRITICALERRORS);
				pvResult = LoadLibraryA( filename_.c_str() );
				// Restore previous error mode
				SetErrorMode( uOldErrorMode );
				#endif
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
				if ( !pcLADSPAPath) {
				#if defined __unix__ || defined __APPLE__
				pcLADSPAPath = "/usr/lib/ladspa/";
				#else
				pcLADSPAPath = "C:\\Programme\\Audacity\\Plug-Ins\\";
				#endif
				}
				std::string directory(pcLADSPAPath);
				int dotindex(0),dotpos(0),prevdotpos(0);
		
			std::cout << directory << std::endl;
				dotpos = directory.find(':',dotindex++);
				do{
				std::string fullname = directory.substr(prevdotpos,dotpos);
				#if defined __unix__ || defined __APPLE__
				if (fullname.c_str()[fullname.length()-1] != '/' )
					fullname.append("/");
				#else
				if (fullname.c_str()[fullname.length()-1] != '\\' )
					fullname.append("\\");
				#endif
				fullname.append(filename_);
				std::cout << fullname << std::endl;
		
				#if defined __unix__ || defined __APPLE__
				pvResult = dlopen(fullname.c_str(), iFlag);
				#else
				// Set error mode to disable system error pop-ups (for LoadLibrary)
				UINT uOldErrorMode = SetErrorMode(SEM_FAILCRITICALERRORS);
				pvResult = LoadLibraryA( fullname.c_str() );
				// Restore previous error mode
				SetErrorMode( uOldErrorMode );
				#endif

				if (pvResult != NULL)
					return pvResult;
				prevdotpos = dotpos;
				dotpos = directory.find(':',dotindex++);
				} while (dotpos != directory.npos);
			}
		
			/* If nothing has worked, then at least we can make sure we set the
				correct error message - and this should correspond to a call to
				dlopen() with the actual filename requested. */

			#if defined __unix__ || defined __APPLE__
			pvResult = dlopen( pcFilename, iFlag);
			#else
			// Set error mode to disable system error pop-ups (for LoadLibrary)
			UINT uOldErrorMode = SetErrorMode(SEM_FAILCRITICALERRORS);
			pvResult = LoadLibraryA( pcFilename );
			// Restore previous error mode
			SetErrorMode( uOldErrorMode );
			#endif
			return pvResult;
		}
		
		
		bool LADSPAMachine::loadDll( const std::string & fileName, int pluginIndex )
		{
			// Step one: Open the shared library.
			#if defined __unix__ || defined __APPLE__
				libHandle_ = dlopenLADSPA( fileName.c_str() , RTLD_NOW);
				if ( !libHandle_ ) {
					//std::cerr << "Cannot load library: " << dlerror() << std::endl;
					return false;
				}
			#else
				// Set error mode to disable system error pop-ups (for LoadLibrary)
				UINT uOldErrorMode = SetErrorMode(SEM_FAILCRITICALERRORS);
				libHandle_ = LoadLibraryA( fileName.c_str() );
				// Restore previous error mode
				SetErrorMode( uOldErrorMode );
			#endif

			// Step two: Get the entry function.
			#if defined __unix__ || defined __APPLE__
				LADSPA_Descriptor_Function pfDescriptorFunction =
					(LADSPA_Descriptor_Function)dlsym( libHandle_, "ladspa_descriptor");
			#else
				LADSPA_Descriptor_Function pfDescriptorFunction =
					(LADSPA_Descriptor_Function)GetProcAddress(  static_cast<HINSTANCE>( libHandle_ ), "ladspa_descriptor" );
			#endif
		
			if (!pfDescriptorFunction) {
				//std::cerr << "Unable to  load : ladspa_descriptor" << std::endl;
				//std::cerr << "Are you sure '"<< fileName.c_str() << "' is a ladspa file ?" << std::endl;
				#if defined __unix__ || defined __APPLE__
					//std::cerr << dlerror() << std::endl;
					dlclose( libHandle_ ); 
				#else
					::FreeLibrary( static_cast<HINSTANCE>( libHandle_ ) ) ;
				#endif
				libHandle_=0;
				return false;
			}
			/*Step three: Get the descriptor of the selected plugin (a shared library can have
				several plugins*/
			std::cout << "step three" << std::endl;
			psDescriptor = pfDescriptorFunction(pluginIndex);
			if (psDescriptor == NULL) {
				//std::cerr <<  "Unable to find the selected plugin  in the library file" << std::endl;
				#if defined __unix__ || defined __APPLE__
					dlclose(libHandle_);
				#else
					::FreeLibrary( static_cast<HINSTANCE>( libHandle_ ) );
				#endif
				libHandle_=0;
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
			SetEditName(label());
			return true;
		} // end of loadPlugin
		
		
		LADSPA_Descriptor_Function LADSPAMachine::loadDescriptorFunction( const std::string & fileName ) {
			// Step one: Open the shared library.
			#if defined __unix__ || defined __APPLE__
				libHandle_ = dlopenLADSPA( fileName.c_str() , RTLD_NOW);
			#else
				// Set error mode to disable system error pop-ups (for LoadLibrary)
				UINT uOldErrorMode = SetErrorMode(SEM_FAILCRITICALERRORS);
				libHandle_ = LoadLibraryA( fileName.c_str() );
			// Restore previous error mode
				SetErrorMode( uOldErrorMode );
			#endif
			if ( !libHandle_ ) {
				#if 0 ///\todo
					std::cerr << "Cannot load library: "
					#if defined __unix__ || defined __APPLE__
						<<  dlerror() 
					#endif
					<< std::endl;
				#endif
				return 0;
			}
			// Step two: Get the entry function.
			LADSPA_Descriptor_Function pfDescriptorFunction =
					(LADSPA_Descriptor_Function)
				#if defined __unix__ || defined __APPLE__
					dlsym(  libHandle_, "ladspa_descriptor");
				#else
					GetProcAddress( static_cast<HINSTANCE>( libHandle_), "ladspa_descriptor");
				#endif
				
		
			if (!pfDescriptorFunction) {
				//std::cerr << "Unable to  load : ladspa_descriptor" << std::endl;
				//std::cerr << "Are you sure '"<< fileName.c_str() << "' is a ladspa file ?" << std::endl;
				#if defined __unix__ || defined __APPLE__
					//std::cerr << dlerror() << std::endl;
					dlclose(libHandle_);
				#else
					::FreeLibrary( static_cast<HINSTANCE>( libHandle_ ) ) ;
				#endif
				libHandle_=0;
				return 0;
			}
		
			return pfDescriptorFunction;
		
		}
		
		void LADSPAMachine::prepareStructures()
		{
			// Controls
			LADSPA_Data *ppbuffersIn[]={_pSamplesL,_pSamplesR};
			LADSPA_Data *ppbuffersOut[]={pOutSamplesL,pOutSamplesR};
		
			_numPars=0;
			int indexinput(0),indexoutput(0);
			for (int lPortIndex = 0; lPortIndex < psDescriptor->PortCount; lPortIndex++) {
				LADSPA_PortDescriptor iPortDescriptor = psDescriptor->PortDescriptors[lPortIndex];
				if (LADSPA_IS_PORT_CONTROL(iPortDescriptor)) {
					LadspaParam parameter(iPortDescriptor,psDescriptor->PortRangeHints[lPortIndex],psDescriptor->PortNames[lPortIndex]);
					
					values_.push_back(parameter);
					if (LADSPA_IS_PORT_INPUT(iPortDescriptor))
					{
						psDescriptor->connect_port(pluginHandle,lPortIndex,
							values_[_numPars].valueaddress());
					}
					else if (LADSPA_IS_PORT_OUTPUT(iPortDescriptor))
					{
						psDescriptor->connect_port(pluginHandle,lPortIndex,
							controls_[_numPars].valueaddress());
					}
					_numPars++;
				}
				else if (LADSPA_IS_PORT_AUDIO(iPortDescriptor))
				{
					// Only Stereo for now.
					// note, the connections are inverted because we do an inversion in PreWork() (in order to avoid the BROKEN_INPLACE problems)
					if (LADSPA_IS_PORT_INPUT(iPortDescriptor)  && indexoutput < 2 ) {
						psDescriptor->connect_port(pluginHandle,lPortIndex,
							ppbuffersOut[indexoutput++]);
						}
					else if (LADSPA_IS_PORT_OUTPUT(iPortDescriptor)  && indexinput < 2 ) {
						psDescriptor->connect_port(pluginHandle,lPortIndex,
							ppbuffersIn[indexinput++]);
						}
				}
			}
			_nCols = (GetNumParams()/12)+1;
		}
		
		void LADSPAMachine::Init()
		{
			// Not sure what should we do here.
			SetDefaultsForControls();
		}
		void LADSPAMachine::Tick(int channel, const PatternEvent & pEntry )
		{
			if ( pEntry.note() == psy::core::commands::tweak || pEntry.note() == psy::core::commands::tweak_slide)
			{
				SetParameter(pEntry.instrument(),pEntry.command()*0x100+pEntry.parameter());
			}
		}
		
		void LADSPAMachine::PreWork(int numSamples)
		{
				std::swap(_pSamplesL,pOutSamplesL);
				std::swap(_pSamplesR,pOutSamplesR);
			Machine::PreWork(numSamples);
		}
		
		int LADSPAMachine::GenerateAudio(int numSamples )
		{
			psDescriptor->run(pluginHandle,numSamples);
				std::swap(_pSamplesL,pOutSamplesL);
				std::swap(_pSamplesR,pOutSamplesR);
			return numSamples;
		}
		void  LADSPAMachine::GetParamName(int numparam, char * name) const
		{
			strcpy(name,values_[numparam].name());
		}
		void  LADSPAMachine::GetParamRange(int numparam,int &minval, int &maxval) const
		{
			minval=values_[numparam].minval();
			maxval=values_[numparam].maxval();
		}
		
		int LADSPAMachine::GetParamValue(int numparam) const { return values_[numparam].value(); } 
		
		void LADSPAMachine::GetParamValue(int numparam,char* parval) const
		{
			LADSPA_PortRangeHintDescriptor iHintDescriptor = values_[numparam].hint();
			float value = values_[numparam].rawvalue();
			if (LADSPA_IS_HINT_TOGGLED(iHintDescriptor))
			{
				std::strcpy(parval, (value>0.0)?"on":"off");
			}
			else if (LADSPA_IS_HINT_INTEGER(iHintDescriptor)) 
			{
				std::sprintf(parval, "%.0f", value);
			}
			else
			{
				std::sprintf(parval, "%.4f", value);
			}
		}
		
		bool  LADSPAMachine::SetParameter(int numparam,int value)
		{
			values_[numparam].setValue(value); return true;
		}
		
		void LADSPAMachine::SetDefaultsForControls()
		{
			int lBufferIndex = 0;
			for (int lPortIndex = 0; lPortIndex < _numPars; lPortIndex++) {
				values_[lPortIndex].setDefault();
			}
		}
		
		void LADSPAMachine::SaveDllName(RiffFile * pFile) const
		{
			std::string::size_type extpos=0;
			std::string withoutSuffix;
			if ( (extpos= libName_.find(".so"))!= std::string::npos) {
				withoutSuffix = libName_.substr(0,extpos);
			}
			else if ( (extpos = libName_.find(".dll"))!= std::string::npos) {
				withoutSuffix = libName_.substr(0,extpos);
			}
			pFile->WriteArray(withoutSuffix.c_str(), withoutSuffix.length() + 1);
		}
		
		bool LADSPAMachine::LoadSpecificChunk(RiffFile* pFile, int version)
		{
			std::uint32_t size;
			pFile->Read(size); // size of whole structure
			if(size)
			{
				if(version > CURRENT_FILE_VERSION_MACD)
				{
					pFile->Skip(size);
					std::ostringstream s; s
						<< version << " > " << CURRENT_FILE_VERSION_MACD << std::endl
						<< "Data is from a newer format of psycle, it might be unsafe to load." << std::endl;
					//MessageBox(0, s.str().c_str(), "Loading Error", MB_OK | MB_ICONWARNING);
					return false;
				}
				else
				{
					std::uint32_t count;
					pFile->Read(count);  // size of vars
					for(unsigned int i(0) ; i < count ; ++i)
					{
						float temp;
						pFile->Read(temp);
						values_[i].setrawvalue(temp);
					}
				}
			}
			return true;
		}
		
		void LADSPAMachine::SaveSpecificChunk(RiffFile* pFile) const
		{
			std::uint32_t count = GetNumParams();
			std::uint32_t size = sizeof count  + sizeof(std::uint32_t) * count;
			pFile->Write(size);
			pFile->Write(count);
			for(unsigned int i(0) ; i < count ; ++i) {
		float temp = values_[i].rawvalue();
		pFile->Write(temp);
		}
		}
	}
}
