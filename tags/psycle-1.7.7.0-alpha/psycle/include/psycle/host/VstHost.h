#pragma once
#include "Machine.h"
#include "Vst\AEffectx.h"
#include "constants.h"
#include "Helpers.h"
#include "Configuration.h"
#if !defined(_WINAMP_PLUGIN_)
	#include "NewMachine.h"
#endif
///\file
///\brief interface file for psycle::host::VSTPlugin
namespace psycle
{
	namespace host
	{
		#define MAX_INOUTS 16

		/// Dialog max ticks for parameters.
		#define VST_QUANTIZATION 65535

		#define VSTINSTANCE_ERR_NO_VALID_FILE -1
		#define VSTINSTANCE_ERR_NO_VST_PLUGIN -2
		#define VSTINSTANCE_ERR_REJECTED -3
		#define VSTINSTANCE_ERR_EXCEPTION -4
		#define VSTINSTANCE_NO_ERROR 0

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
			~VSTPlugin();

			void Free();
			int Instance(char *dllname,bool overwriteName=true);
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

					if (size)
					{
						if(_pEffect->flags & effFlagsProgramChunks)
						{
							char* pData = new char[size];
							pFile->Read(pData, size); // Number of parameters
							try 
							{
								Dispatch(effSetChunk,0, size, pData,0.0f);
								delete pData;
							}
							catch (...)
							{
								MessageBox(NULL,"Machine had an exception while loading it's chunk.\nIt has crashed and will be removed.",GetDllName(),NULL);
								return FALSE;
							}
						}
						else
						{
							// there is a data chunk, but this machine does not want one.
							pFile->Skip(size);
							return FALSE;
						}
					}
				}
				return TRUE;
			};

			bool LoadDll(char* psFileName)
			{
				_strlwr(psFileName);
				char sPath2[_MAX_PATH];
				CString sPath;
				#if defined(_WINAMP_PLUGIN_)
					sPath = Global::pConfig->GetVstDir();
					if ( FindFileinDir(psFileName,sPath) )
					{
						strcpy(sPath2,sPath);
						if (Instance(sPath2,false) != VSTINSTANCE_NO_ERROR)
						{
							char sError[128];
							sprintf(sError,"Missing or Corrupted VST plug-in \"%s\" - replacing with Dummy.",sPath2);
							::MessageBox(NULL,sError, "Loading Error", MB_OK);
							return FALSE;
						}
					}
					else
					{
						char sError[128];
						sprintf(sError,"Missing VST plug-in \"%s\" - replacing with Dummy.",psFileName);
						::MessageBox(NULL,sError, "Loading Error", MB_OK);
						return FALSE;
					}
				#else // if !_WINAMP_PLUGIN_
					if ( CNewMachine::dllNames.Lookup(psFileName,sPath) )
					{
						strcpy(sPath2,sPath);
						if ( !CNewMachine::TestFilename(sPath2) ) 
						{
							return false;
						}
						if (Instance(sPath2,false) != VSTINSTANCE_NO_ERROR)
						{
							char sError[128];
							sprintf(sError,"Missing or Corrupted VST plug-in \"%s\" - replacing with Dummy.",sPath2);
							::MessageBox(NULL,sError, "Loading Error", MB_OK);
							return FALSE;
						}
					}
					else
					{
						char sError[128];
						sprintf(sError,"Missing VST plug-in \"%s\" - replacing with Dummy.",psFileName);
						::MessageBox(NULL,sError, "Loading Error", MB_OK);
						return FALSE;
					}
				#endif // _WINAMP_PLUGIN_
				return TRUE;
			};

			#if !defined(_WINAMP_PLUGIN_)
				bool SaveChunk(RiffFile* pFile,bool &isfirst);
				inline virtual void SaveSpecificChunk(RiffFile* pFile) 
				{
					UINT count = GetNumParams();
					UINT size = sizeof(_program)+sizeof(count)+(sizeof(float)*count);
					char* pData = NULL;

					if(_pEffect->flags & effFlagsProgramChunks)
					{
						try 
						{
							size += Dispatch( effGetChunk,0,0, &pData,0.0f);
						}
						catch (...)
						{
							MessageBox(NULL,"Machine had an exception while saving it's chunk.\nIt has crashed and will probably take psycle down with it.",GetDllName(),NULL);
							size = sizeof(_program)+sizeof(count)+(sizeof(float)*count);
						}
					}

					pFile->Write(&size,sizeof(size));

					pFile->Write(&_program,sizeof(_program));

					pFile->Write(&count,sizeof(count));
					for (UINT i = 0; i < count; i++)
					{
						float temp = GetParameter(i);
						pFile->Write(&temp,sizeof(temp));
					}

					size-=sizeof(_program)+sizeof(count)+(sizeof(float)*count);
					if (size > 0)
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
			inline long Dispatch(long opCode, long index, long value, void *ptr, float opt)
			{
				return _pEffect->dispatcher(_pEffect, opCode, index, value, ptr, opt);
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
