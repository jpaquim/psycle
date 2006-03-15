///\file
///\brief implementation file for psycle::host::Global.
#include <packageneric/pre-compiled.private.hpp>
#include PACKAGENERIC
#include <psycle/host/machine.hpp>
#include <psycle/host/plugin.hpp>
#include <psycle/host/VSTHost.hpp>

///\todo bad coupling with the gui
#include <psycle/host/NewMachine.hpp>

#include <cstdint>
namespace psycle
{
	namespace host
	{
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// old file format vomit, don't look at it.

		bool Machine::Load(RiffFile* pFile)
		{
			char junk[256];
			std::memset(junk, 0, sizeof junk);

			pFile->Read(&_editName, 16); _editName[15] = 0;

			pFile->Read(_inputMachines);
			pFile->Read(_outputMachines);
			pFile->Read(_inputConVol);
			pFile->Read(_connection);
			pFile->Read(_inputCon);
			pFile->Read(_connectionPoint);
			pFile->Read(_connectedInputs);
			pFile->Read(_connectedOutputs);

			pFile->Read(_panning);
			Machine::SetPan(_panning);

			pFile->Skip(4*8); // SubTrack[]
			pFile->Skip(4); // numSubtracks
			pFile->Skip(4); // interpol

			pFile->Skip(4); // outwet
			pFile->Skip(4); // outdry

			pFile->Skip(4); // distPosThreshold
			pFile->Skip(4); // distPosClamp
			pFile->Skip(4); // distNegThreshold
			pFile->Skip(4); // distNegClamp

			pFile->Skip(1); // sinespeed
			pFile->Skip(1); // sineglide
			pFile->Skip(1); // sinevolume
			pFile->Skip(1); // sinelfospeed
			pFile->Skip(1); // sinelfoamp

			pFile->Skip(4); // delayTimeL
			pFile->Skip(4); // delayTimeR
			pFile->Skip(4); // delayFeedbackL
			pFile->Skip(4); // delayFeedbackR

			pFile->Skip(4); // filterCutoff
			pFile->Skip(4); // filterResonance
			pFile->Skip(4); // filterLfospeed
			pFile->Skip(4); // filterLfoamp
			pFile->Skip(4); // filterLfophase
			pFile->Skip(4); // filterMode

			return true;
		}

		bool Master::Load(RiffFile* pFile)
		{
			char junk[256];
			std::memset(junk, 0, sizeof junk);
			
			pFile->Read(&_editName, 16); _editName[15] = 0;
			
			pFile->Read(_inputMachines);
			pFile->Read(_outputMachines);
			pFile->Read(_inputConVol);
			pFile->Read(_connection);
			pFile->Read(_inputCon);
			pFile->Read(_connectionPoint);
			pFile->Read(_connectedInputs);
			pFile->Read(_connectedOutputs);
			
			pFile->Read(_panning);
			Machine::SetPan(_panning);

			pFile->Skip(4*8); // SubTrack[]
			pFile->Skip(4); // numSubtracks
			pFile->Skip(4); // interpol

			/////////////
			_outDry = 0; pFile->Read(_outDry); // outdry
			/////////////

			pFile->Skip(4); // outwet
			
			pFile->Skip(4); // distPosThreshold
			pFile->Skip(4); // distPosClamp
			pFile->Skip(4); // distNegThreshold
			pFile->Skip(4); // distNegClamp

			pFile->Skip(1); // sinespeed
			pFile->Skip(1); // sineglide
			pFile->Skip(1); // sinevolume
			pFile->Skip(1); // sinelfospeed
			pFile->Skip(1); // sinelfoamp

			pFile->Skip(4); // delayTimeL
			pFile->Skip(4); // delayTimeR
			pFile->Skip(4); // delayFeedbackL
			pFile->Skip(4); // delayFeedbackR

			pFile->Skip(4); // filterCutoff
			pFile->Skip(4); // filterResonance
			pFile->Skip(4); // filterLfospeed
			pFile->Skip(4); // filterLfoamp
			pFile->Skip(4); // filterLfophase
			pFile->Skip(4); // filterMode

			return true;
		}

		bool Plugin::LoadOldFileFormat(RiffFile* pFile)
		{
			bool result = true;
			char junk[256];
			std::memset(junk, 0, sizeof junk);

			char sDllName[256];
			int numParameters;

			pFile->Read(sDllName); // Plugin dll name
			///\todo would be nicer with std::string and std::transform
			for(int i(std::strlen(sDllName) - 1) ; i > 0  ; --i) sDllName[i] = std::tolower(sDllName[i]);

			//Patch: Automatically replace old AS's by AS2F.
			bool wasAB=false;
			bool wasAS1=false;
			bool wasAS2=false;
			if (std::strcmp(sDllName,"arguru bass.dll" ) == 0)
			{
				strcpy(sDllName,"arguru synth 2f.dll");
				wasAB=true;
			}
			if (std::strcmp(sDllName,"arguru synth.dll" ) == 0)
			{
				strcpy(sDllName,"arguru synth 2f.dll");
				wasAS1=true;
			}
			if (!std::strcmp(sDllName,"arguru synth 2.dll" ))
			{
				strcpy(sDllName,"arguru synth 2f.dll");
				wasAS2=true;
			}
			if (!std::strcmp(sDllName,"synth21.dll" ))
			{
				strcpy(sDllName,"arguru synth 2f.dll");
				wasAS2=true;
			}

			std::string sPath;
			if ( !CNewMachine::lookupDllName(sDllName,sPath) ) 
			{
				// Check Compatibility Table.
				// Probably could be done with the dllNames lockup.
				//GetCompatible(sDllName,sPath) // If no one found, it will return a null string.
				sPath = sDllName;
			}
			if ( !CNewMachine::TestFilename(sPath) ) 
			{
				result = false;
			}
			else 
			{
				try
				{
					Instance(sPath.c_str());
				}
				catch(...)
				{
					char sError[_MAX_PATH];
					sprintf(sError,"Missing or corrupted native Plug-in \"%s\" - replacing with Dummy.",sDllName);
					MessageBox(NULL,sError, "Error", MB_OK);
					result = false;
				}
			}

			Init();

			pFile->Read(&_editName, 16); _editName[15] = 0;

			pFile->Read(numParameters);
			if(result)
			{
				std::int32_t * Vals = new std::int32_t[numParameters];
				pFile->Read(Vals, numParameters * sizeof(std::int32_t));
				try
				{
					if ( wasAB ) // Patch to replace Arguru Bass by Arguru Synth 2f
					{
						proxy().ParameterTweak(0,Vals[0]);
						for (int i=1;i<15;i++)
						{
							proxy().ParameterTweak(i+4,Vals[i]);
						}
						proxy().ParameterTweak(19,0);
						proxy().ParameterTweak(20,Vals[15]);
						if (numParameters>16)
						{
							proxy().ParameterTweak(24,Vals[16]);
							proxy().ParameterTweak(25,Vals[17]);
						}
						else
						{
							proxy().ParameterTweak(24,0);
							proxy().ParameterTweak(25,0);
						}
					}
					else for (int i=0; i<numParameters; i++)
					{
						proxy().ParameterTweak(i,Vals[i]);
					}
				}
				catch(const std::exception &)
				{
					loggers::warning(UNIVERSALIS__COMPILER__LOCATION);
				}
				try
				{
					int size = proxy().GetDataSize();
					//pFile->Read(&size,sizeof(int));	// This SHOULD be the right thing to do
					if(size)
					{
						char * pData = new char[size];
						pFile->Read(pData, size); // Number of parameters
						try
						{
							proxy().PutData(pData); // Internal load
						}
						catch(const std::exception &)
						{
						}
						delete[] pData;
					}
				}
				catch(std::exception const &)
				{
					loggers::warning(UNIVERSALIS__COMPILER__LOCATION);
				}
				if(wasAS1) // Patch to replace Synth1 by Arguru Synth 2f
				{
					try
					{
						proxy().ParameterTweak(17,Vals[17]+10);
						proxy().ParameterTweak(24,0);
						proxy().ParameterTweak(25,0);
					}
					catch(const std::exception &)
					{
					}
				}
				if(wasAS2)
				{
					try
					{
						proxy().ParameterTweak(24,0);
						proxy().ParameterTweak(25,0);
					}
					catch(const std::exception&)
					{
					}
				}
				delete[] Vals;
			}
			else
			{
				for (int i=0; i<numParameters; i++)
				{
					pFile->Read(&junk[0], 4);
				}
			}

			pFile->Read(_inputMachines);
			pFile->Read(_outputMachines);
			pFile->Read(_inputConVol);
			pFile->Read(_connection);
			pFile->Read(_inputCon);
			pFile->Read(_connectionPoint);
			pFile->Read(_connectedInputs);
			pFile->Read(_connectedOutputs);

			pFile->Read(_panning);
			Machine::SetPan(_panning);

			pFile->Skip(4*8); // SubTrack[]
			pFile->Skip(4); // numSubtracks
			pFile->Skip(4); // interpol

			pFile->Skip(4); // outwet
			pFile->Skip(4); // outdry

			pFile->Skip(4); // distPosThreshold
			pFile->Skip(4); // distPosClamp
			pFile->Skip(4); // distNegThreshold
			pFile->Skip(4); // distNegClamp

			pFile->Skip(1); // sinespeed
			pFile->Skip(1); // sineglide
			pFile->Skip(1); // sinevolume
			pFile->Skip(1); // sinelfospeed
			pFile->Skip(1); // sinelfoamp

			pFile->Skip(4); // delayTimeL
			pFile->Skip(4); // delayTimeR
			pFile->Skip(4); // delayFeedbackL
			pFile->Skip(4); // delayFeedbackR

			pFile->Skip(4); // filterCutoff
			pFile->Skip(4); // filterResonance
			pFile->Skip(4); // filterLfospeed
			pFile->Skip(4); // filterLfoamp
			pFile->Skip(4); // filterLfophase
			pFile->Skip(4); // filterMode

			return result;
		}

		namespace vst
		{
			bool plugin::LoadChunkOldFileFormat(RiffFile * pFile)
			{
				bool b;
				try
				{
					b = proxy().flags() & effFlagsProgramChunks;
				}
				catch(const std::exception &)
				{
					b = false;
				}
				if(!b) return false;

				// read chunk size
				std::uint32_t chunk_size;
				pFile->Read(chunk_size);

				// read chunk data
				char * chunk(new char[chunk_size]);
				pFile->Read(chunk, chunk_size);

				try
				{
					proxy().dispatcher(effSetChunk, 0, chunk_size, chunk);
				}
				catch(const std::exception &)
				{
					// [bohan] hmm, so, data just gets lost?
					delete[] chunk;
					return false;
				}

				delete[] chunk;
				return true;
			}

			bool plugin::Load(RiffFile * pFile)
			{
				Machine::Init();

				pFile->Read(&_editName, 16); _editName[15] = 0;

				pFile->Read(_inputMachines);
				pFile->Read(_outputMachines);
				pFile->Read(_inputConVol);
				pFile->Read(_connection);
				pFile->Read(_inputCon);
				pFile->Read(_connectionPoint);
				pFile->Read(_connectedInputs);
				pFile->Read(_connectedOutputs);

				pFile->Read(_panning);
				Machine::SetPan(_panning);

				pFile->Skip(4*8); // SubTrack[]
				pFile->Skip(4); // numSubtracks
				pFile->Skip(4); // interpol

				pFile->Skip(4); // outwet
				pFile->Skip(4); // outdry

				pFile->Skip(4); // distPosThreshold
				pFile->Skip(4); // distPosClamp
				pFile->Skip(4); // distNegThreshold
				pFile->Skip(4); // distNegClamp

				pFile->Skip(1); // sinespeed
				pFile->Skip(1); // sineglide
				pFile->Skip(1); // sinevolume
				pFile->Skip(1); // sinelfospeed
				pFile->Skip(1); // sinelfoamp

				pFile->Skip(4); // delayTimeL
				pFile->Skip(4); // delayTimeR
				pFile->Skip(4); // delayFeedbackL
				pFile->Skip(4); // delayFeedbackR

				pFile->Skip(4); // filterCutoff
				pFile->Skip(4); // filterResonance
				pFile->Skip(4); // filterLfospeed
				pFile->Skip(4); // filterLfoamp
				pFile->Skip(4); // filterLfophase
				pFile->Skip(4); // filterMode

				bool old;
				pFile->Read(old); // old format
				pFile->Read(_instance); // ovst.instance
				if(old)
				{
					char mch;
					pFile->Read(mch);
					_program = 0;
				}
				else
				{
					pFile->Read(_program);
				}
				return true;
			}
		}
	}
}
