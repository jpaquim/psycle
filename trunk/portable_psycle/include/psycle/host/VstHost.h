#pragma once
#include "Machine.h"
#include "Vst\AEffectx.h"
#include "constants.h"
#include "Helpers.h"
#include "Configuration.h"
#if !defined(_WINAMP_PLUGIN_)
	#include "NewMachine.h"
#endif
#include <operating_system/exception.h>
#include <operating_system/exceptions/code_description.h>
///\file
///\brief interface file for psycle::host::VSTPlugin
namespace psycle
{
	namespace host
	{
		#define MAX_INOUTS 16

		/// Dialog max ticks for parameters.
		#define VST_QUANTIZATION 65535

		#define VSTINSTANCE_NO_ERROR 0
		#define VSTINSTANCE_ERR_NO_VALID_FILE -1
		#define VSTINSTANCE_ERR_NO_VST_PLUGIN -2
		#define VSTINSTANCE_ERR_REJECTED -3
		#define VSTINSTANCE_ERR_EXCEPTION -4

		typedef AEffect* (*PVSTMAIN)(audioMasterCallback audioMaster);

		class VSTinote
		{
		public:
			unsigned char note;
			unsigned char midichan;
		};

		/// vst plugin.
		class VSTPlugin : public Machine
		{
		public:
			VSTPlugin();
			// <bohan> I added throw() to the signature of the desctructor to make it clear.
			// <bohan> It's just informative and is implicit because
			// <bohan> any object of any class is disallowed to throw any exception from its destructor.
			~VSTPlugin() throw();

			void Free() throw(...);
			void Instance(const char dllname[], const bool overwriteName = true) throw(...);
			//void Create(VSTPlugin *plug);
			//virtual void Init(void);
			virtual bool Load(RiffFile* pFile);

			///\todo To be removed when changing the fileformat.
			bool LoadChunk(RiffFile* pFile);
			inline virtual bool LoadSpecificFileChunk(RiffFile* pFile, int version)
			{
				UINT size;
				pFile->Read(&size,sizeof(size));

				if (size)
				{
					if (version > CURRENT_FILE_VERSION_MACD)
					{
						// data is from a newer format of psycle, it might be unsafe to load.
						pFile->Skip(size);
						return FALSE;
					}

					pFile->Read(&_program,sizeof(_program));
					// set the program
					SetCurrentProgram(_program);

					UINT count;

					pFile->Read(&count,sizeof(count));  // size of vars

					for (UINT i = 0; i < count; i++)
					{
						float temp;
						pFile->Read(&temp,sizeof(temp));
						SetParameter(i,temp);
					}

					size -= sizeof(_program)+sizeof(count)+(sizeof(float)*count);

					if(size)
					{
						if(_pEffect->flags & effFlagsProgramChunks)
						{
							char * const data = new char[size];
							pFile->Read(data, size); // Number of parameters
							try 
							{
								Dispatch(effSetChunk, 0, size, data);
							}
							catch(...)
							{
								std::ostringstream s; s
									<< "Machine had an exception while loading its chunk:" << std::endl
									<< "It has crashed and will be removed.";
								::MessageBox(0, s.str().c_str(), GetDllName(), 0);
								delete data;
								return false;
							}
							delete data;
						}
						else
						{
							// there is a data chunk, but this machine does not want one.
							pFile->Skip(size);
							return false;
						}
					}
				}
				return TRUE;
			};

			bool LoadDll(char* psFileName)
			{
				_strlwr(psFileName);
				char sPath2[1 << 10];
				CString sPath;
				#if defined(_WINAMP_PLUGIN_)
					sPath = Global::pConfig->GetVstDir();
					if(FindFileinDir(psFileName, sPath))
					{
						std::strcpy(sPath2, sPath);
						try
						{
							Instance(sPath2, false);
						}
						catch(const std::exception & e)
						{
							std::ostringstream s; s
								<< "exception while instanciating:" << sPath2 << std::endl
								<< "replacing with dummy." << std::endl
								<< e.what();
							::MessageBox(0, s.str().c_str(), "Loading Error", 0);
							return false;
						}
						catch(...)
						{
							std::ostringstream s; s
								<< "exception while instanciating:" << sPath2 << std::endl
								<< "replacing with dummy." << std::endl
								<< "unkown type";
							::MessageBox(0, s.str().c_str(), "Loading Error", 0);
							return false;
						}
					}
					else
					{
						std::ostringstream s; s
							<< "missing:" << psFileName << std::endl
							<< "replacing with dummy."
						::MessageBox(0, s.str().c_str(), "Loading Error", 0);
						return false;
					}
				#else // if !_WINAMP_PLUGIN_
					if(CNewMachine::dllNames.Lookup(psFileName, sPath))
					{
						std::strcpy(sPath2, sPath);
						if(!CNewMachine::TestFilename(sPath2)) return false;
						try
						{
							Instance(sPath2, false);
						}
						catch(const std::exception & e)
						{
							std::ostringstream s; s
								<< "exception while instanciating:" << sPath2 << std::endl
								<< "replacing with dummy." << std::endl
								<< e.what();
							::MessageBox(0, s.str().c_str(), "Loading Error", 0);
							return false;
						}
						catch(...)
						{
							std::ostringstream s; s
								<< "exception while instanciating:" << sPath2 << std::endl
								<< "replacing with dummy." << std::endl
								<< "unkown type";
							::MessageBox(0, s.str().c_str(), "Loading Error", 0);
							return false;
						}
					}
					else
					{
						std::ostringstream s; s
							<< "missing:" << psFileName << std::endl
							<< "replacing with dummy.";
						::MessageBox(0, s.str().c_str(), "Loading Error", 0);
						return false;
					}
				#endif // _WINAMP_PLUGIN_
				return true;
			};

			#if !defined(_WINAMP_PLUGIN_)
				bool SaveChunk(RiffFile* pFile,bool &isfirst);
				inline virtual void SaveSpecificChunk(RiffFile* pFile) 
				{
					UINT count = GetNumParams();
					UINT size = sizeof(_program)+sizeof(count)+(sizeof(float)*count);
					char * pData = 0;
					if(_pEffect->flags & effFlagsProgramChunks)
					{
						try 
						{
							size += Dispatch( effGetChunk,0,0, &pData,0.0f);
						}
						catch(...)
						{
							std::ostringstream s; s
								<< "Machine had an exception while saving its chunk:" << std::endl
								<< "It has crashed and will probably take psycle down with it.";
							::MessageBox(0, s.str().c_str(), GetDllName(), 0);
							size = sizeof _program  + sizeof count  + sizeof(float) * count;
						}
					}
					pFile->Write(&size,sizeof(size));
					pFile->Write(&_program,sizeof(_program));
					pFile->Write(&count,sizeof(count));
					for(UINT i = 0; i < count; i++)
					{
						float temp = GetParameter(i);
						pFile->Write(&temp,sizeof(temp));
					}
					size-=sizeof(_program)+sizeof(count)+(sizeof(float)*count);
					if(size > 0)
					{
						pFile->Write(pData,size);
					}
				};

				inline virtual void SaveDllName(RiffFile* pFile) 
				{
					CString str = GetDllName();
					char str2[256];
					strcpy(str2,str.Mid(str.ReverseFind('\\')+1));
					pFile->Write(str2,strlen(str2)+1);
				};
			#endif // ndef _WINAMP_PLUGIN_

			inline virtual int GetNumParams(void) { return _pEffect->numParams; }

			inline virtual void GetParamName(int numparam,char* name)
			{
				if ( numparam < _pEffect->numParams ) Dispatch(effGetParamName,numparam,0,name,0.0f);
				else strcpy(name,"Out of Range");

			}

			inline virtual void GetParamValue(int numparam,char* parval)
			{
				if ( numparam < _pEffect->numParams ) DescribeValue(numparam,parval);
				else strcpy(parval,"Out of Range");
			}

			inline virtual int GetParamValue(int numparam)
			{
				if ( numparam < _pEffect->numParams ) return f2i(_pEffect->getParameter(_pEffect, numparam)*65535);
				else return -1;
			}

			inline virtual char* GetName(void) { return _sProductName; }

			inline virtual void SetSampleRate(int sr) {Dispatch( effSetSampleRate, 0, 0, NULL, (float)sr);};

			inline long GetVersion() { return _version; }
			inline char* GetVendorName(void) { return _sVendorName; }
			inline char* GetDllName(void) { return _sDllName; }

			inline long NumParameters(void) { return _pEffect->numParams; }
			inline float GetParameter(long parameter) { return _pEffect->getParameter(_pEffect, parameter); }
			bool DescribeValue(int parameter,char* psTxt);
			bool SetParameter(int parameter, float value);
			bool SetParameter(int parameter, int value);
			void SetCurrentProgram(int prg);
			int GetCurrentProgram();
			inline int NumPrograms() { return _pEffect->numPrograms; }
			inline bool IsSynth() { return _isSynth; }

			inline bool AddMIDI(unsigned char data0,unsigned char data1=0,unsigned char data2=0);
			inline void SendMidi();

			AEffect *_pEffect;
			inline long int Dispatch(long opCode = 0, long index = 0, long value = 0, void * ptr = 0, float opt = 0.0f) throw(...)
			{
				try
				{
					///<bohan> \todo i don't know why we get a "warning C4702: unreachable code" with msvc7.1 here...
					return _pEffect->dispatcher(_pEffect, opCode, index, value, ptr, opt);
				}
				catch(const std::exception & e)
				{
					std::ostringstream s; s
						<< "Machine had an exception on opcode: " << opCode << std::endl
						<< typeid(e).name() << std::endl
						<< e.what();
					::MessageBox(0, s.str().c_str(), _editName, 0);
					throw std::runtime_error(s.str());
				}
				catch(const char e[])
				{
					std::ostringstream s; s
						<< "Machine had an exception on opcode: " << opCode << std::endl
						<< typeid(e).name() << std::endl
						<< e;
					::MessageBox(0, s.str().c_str(), _editName, 0);
					throw std::runtime_error(s.str());
				}
				catch(const int & e)
				{
					std::ostringstream s; s
						<< "Machine had an exception on opcode: " << opCode << std::endl
						<< typeid(e).name() << std::endl
						<< e;
					::MessageBox(0, s.str().c_str(), _editName, 0);
					throw std::runtime_error(s.str());
				}
				catch(const unsigned int & e)
				{
					std::ostringstream s; s
						<< "Machine had an exception on opcode: " << opCode << std::endl
						<< typeid(e).name() << std::endl
						<< e;
					::MessageBox(0, s.str().c_str(), _editName, 0);
					throw std::runtime_error(s.str());
				}
				catch(...)
				{
					std::ostringstream s; s
						<< "Machine had an exception on opcode: " << opCode << std::endl
						<< "unkown type";
					::MessageBox(0, s.str().c_str(), _editName, 0);
					throw std::runtime_error(s.str());
				}
			}

			static long AudioMaster(AEffect *effect, long opcode, long index, long value, void *ptr, float opt);

			unsigned char _program;
			bool instantiated;
			///\todo Remove when Changing the FileFormat.
			int _instance;

			/// It needs to use Process
			bool requiresProcess;
			/// It needs to use ProcessRepl
			bool requiresRepl;		

			#if !defined(_WINAMP_PLUGIN_)
				CFrameWnd* editorWnd;
			#endif
			
		protected:

			HMODULE h_dll;

			/// Contains dll name
			char *_sDllName;

			char _sProductName[64];
			char _sVendorName[64];
			long _version;
			bool _isSynth;
			bool wantidle;

			float * inputs[MAX_INOUTS];
			float * outputs[MAX_INOUTS];
			float junk[STREAM_SIZE];

			static VstTimeInfo _timeInfo;
			VstMidiEvent midievent[MAX_VST_EVENTS];
			VstEvents events;
			int	queue_size;

		};

		/// vst "instrument" (input) plugin.
		class VSTInstrument : public VSTPlugin
		{
		public:
			VSTInstrument(int index);
			virtual void Tick(int channel, PatternEntry* pEntry);
			virtual void Work(int numSamples);
			virtual void Stop(void);

			bool AddNoteOn(unsigned char channel, unsigned char note,unsigned char speed,unsigned char midichannel=0);
			bool AddNoteOff(unsigned char channel,unsigned char midichannel=0,bool addatStart=false);
		protected:
			VSTinote trackNote[MAX_TRACKS];
		};
		
		/// vst "fx" (filter) plugin.
		class VSTFX : public VSTPlugin
		{
		public:
			VSTFX(int index);
			~VSTFX();
			virtual void Tick(int channel, PatternEntry* pEntry);
			virtual void Work(int numSamples);
		protected:
			float *_pOutSamplesL;
			float *_pOutSamplesR;
		};
	}
}
