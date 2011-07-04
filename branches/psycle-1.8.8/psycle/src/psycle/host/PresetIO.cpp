///\file
///\brief implementation file for psycle::host::CSkinDlg.

#include "PresetIO.hpp"
#include <sstream>
#include <boost/algorithm/string.hpp>
namespace psycle { namespace host {

		CPreset::CPreset()
		:
			params(),
			data(),
			numPars(-1),
			dataSize()
		{
			std::memset(name, 0, sizeof name * sizeof *name);
		}

		CPreset::CPreset(const CPreset& preset)
		:
			params(),
			data(),
			numPars(-1),
			dataSize()
		{
			std::memset(name, 0, sizeof name * sizeof *name);
			operator=(preset);
		}
		CPreset::~CPreset()
		{
			delete[] params;
			delete[] data;
		}

		void CPreset::Clear()
		{
			delete[] params; params = 0;
			numPars =-1;
			delete[] data; data = 0;
			dataSize = 0;
			std::memset(name, 0, sizeof name * sizeof *name);
		}

		void CPreset::Init(int num)
		{
			if ( num > 0 )
			{
				delete[] params; params = new int[num];
				numPars=num;
			}
			else
			{
				delete[] params; params = 0;
				numPars =-1;
			}
			delete[] data; data = 0;
			dataSize = 0;

			std::memset(name,0, sizeof name * sizeof *name);
		}

		void CPreset::Init(int num,const char* newname, int const * parameters,int size, void* newdata)
		{
			if ( num > 0 )
			{
				delete[] params; params = new int[num];
				numPars=num;
				std::memcpy(params,parameters,numPars * sizeof *params);
			}
			else
			{
				delete[] params; params = 0;
				numPars=-1;
			}

			if ( size > 0 )
			{
				delete[] data; data = new unsigned char[size];
				std::memcpy(data,newdata,size);
				dataSize = size;
			}
			else
			{
				delete[] data; data = 0;
				dataSize=0;
			}
			std::strcpy(name,newname);
		}

		void CPreset::Init(int num,const char* newname, float const * parameters)
		{
			if ( num > 0 )
			{
				delete[] params; params = new int[num];
				numPars=num;
				for(int x=0;x<num;x++) params[x]= helpers::math::lround<int,float>(parameters[x]*65535.0f);
			}
			else
			{
				delete[] params; params = 0;
				numPars=-1;
			}

			delete[] data; data = 0;
			dataSize = 0;

			std::strcpy(name,newname);
		}
		bool CPreset::operator <(const CPreset& b) const
		{
			return name < b.name;
		}
		CPreset& CPreset::operator=(const CPreset& newpreset)
		{
			if ( newpreset.numPars > 0 )
			{
				numPars=newpreset.numPars;
				delete[] params; params = new int[numPars];
				std::memcpy(params,newpreset.params,numPars * sizeof *params);
			}
			else
			{
				delete[] params; params = 0;
				numPars=-1;
			}

			if ( newpreset.dataSize > 0 )
			{
				dataSize = newpreset.dataSize;
				delete[] data; data = new unsigned char[dataSize];
				std::memcpy(data,newpreset.data,dataSize);
			}
			else
			{
				delete[] data; data = 0;
				dataSize = 0;
			}

			strcpy(name,newpreset.name);
			return *this;
		}

		int CPreset::GetParam(const int n) const
		{
			if (( numPars != -1 ) && ( n < numPars )) return params[n];
			return -1;
		}

		void CPreset::SetParam(const int n,int val)
		{
			if (( numPars != -1 ) && ( n < numPars )) params[n] = val;
		}

		////////////////////////////////////////////////////////////////////////////
		//
		//////////////////////////////////////////////////////////////////////////////

		void PresetIO::LoadPresets(const char* szFile, int numParameters, int dataSizeStruct,  std::list<CPreset>& presets, bool warn_if_not_exists)
		{
			std::FILE* hfile;
			if(hfile=std::fopen(szFile,"rb"))
			{
				int numpresets;
				int filenumpars;
				if ( std::fread(&numpresets,sizeof(int),1,hfile) != 1 ||
					std::fread(&filenumpars,sizeof(int),1,hfile) != 1 )
				{
					::MessageBox(0,"Couldn't read from file. Operation aborted","Preset File Error",MB_OK);
				}
				else if (numpresets >= 0)
				{
					// ok so we still support old file format by checking for a positive numpresets
					if (( filenumpars != numParameters )  || (dataSizeStruct))
					{
						::MessageBox(0,"The current preset File is not up-to-date with the plugin.","Preset File Error",MB_OK);
					}
					else {
						presets.clear();
						LoadVersion0(hfile, numpresets, numParameters, presets);
					}
				}
				else
				{
					// new preset file format since numpresets was < 0
					// we will use filenumpars already read as version #
					if (filenumpars == 1)
					{
						presets.clear();
						LoadVersion1(hfile, numParameters, dataSizeStruct, presets);
					}
					else
					{
						::MessageBox(0,"The current preset file is from a newer version of psycle than you are currently running.","Preset File Error",MB_OK);
					}
				}
				std::fclose(hfile);
			}
			else if(warn_if_not_exists) {
				::MessageBox(0,"Couldn't open file. Operation aborted","Preset File Error",MB_OK);
			}
		}

		void PresetIO::SavePresets(const char* szFile,  std::list<CPreset>& presets)
		{
			std::FILE* hfile;
			if(!(hfile=std::fopen(szFile,"wb")))
			{
				::MessageBox(0,"The File couldn't be opened for Writing. Operation Aborted","File Save Error",MB_OK);
				return;
			}
			if(presets.size() > 0) {
				if (false)
				{
					SaveVersion0(hfile, presets);
				}
				else if (true)
				{
					SaveVersion1(hfile, presets);
				}
			}
			std::fclose(hfile);
		}


		void PresetIO::LoadVersion0(FILE* hfile, int numpresets, int numParameters, std::list<CPreset>& presets)
		{			
			char name[32];
			int* ibuf;
			ibuf= new int[numParameters];

			for (int i=0; i< numpresets && !std::feof(hfile) && !std::ferror(hfile); i++ )
			{
				std::fread(name,sizeof(name),1,hfile);
				std::fread(ibuf,numParameters*sizeof(int),1,hfile);
				CPreset preset;
				preset.Init(numParameters,name,ibuf,0,0);
				presets.push_back(preset);
			}
			delete[] ibuf;
		}

		void PresetIO::LoadVersion1(FILE* hfile, int numParameters, int dataSizeStruct, std::list<CPreset>& presets)
		{
			int numpresets;
			int filenumpars;
			int filepresetsize;
			// new preset format version 1
			std::fread(&numpresets,sizeof(int),1,hfile);
			std::fread(&filenumpars,sizeof(int),1,hfile);
			std::fread(&filepresetsize,sizeof(int),1,hfile);
			// now it is time to check our file for compatability
			if (( filenumpars != numParameters )  || (filepresetsize != dataSizeStruct))
			{
				::MessageBox(0,"The current preset File is not up-to-date with the plugin.","Preset File Error",MB_OK);
				return;
			}
			// ok that works, so we should now load the names of all of the presets
			char name[32];
			int* ibuf= new int[numParameters];
			unsigned char * dbuf = 0;
			if ( dataSizeStruct > 0 ) dbuf = new unsigned char[dataSizeStruct];

			for (int i=0; i< numpresets && !feof(hfile) && !ferror(hfile); i++)
			{
				std::fread(name,sizeof(name),1,hfile);
				std::fread(ibuf,numParameters*sizeof(int),1,hfile);
				if ( dataSizeStruct > 0 )  std::fread(dbuf,dataSizeStruct,1,hfile);
				CPreset preset;
				preset.Init(numParameters,name,ibuf,dataSizeStruct,dbuf);
				presets.push_back(preset);
			}
			delete[] ibuf;
			delete[] dbuf;
		}

		void PresetIO::SaveVersion0(FILE* hfile, std::list<CPreset>& presets)
		{
			int numpresets=presets.size();
			std::list<CPreset>::iterator preset = presets.begin();
			int numParameters = preset->GetNumPars();

			if ( std::fwrite(&numpresets,sizeof(int),1,hfile) != 1 ||
				std::fwrite(&numParameters,sizeof(int),1,hfile) != 1 )
			{
				::MessageBox(0,"Couldn't write to File. Operation Aborted","File Save Error",MB_OK);
				return;
			}
			
			char cbuf[32];
			int* ibuf = new int[numParameters];

			for (int i=0; i< numpresets && !feof(hfile) && !ferror(hfile); i++, preset++ )
			{
				preset->GetName(cbuf);
				preset->GetParsArray(ibuf);
				std::fwrite(cbuf,sizeof(cbuf),1,hfile);
				std::fwrite(ibuf,numParameters*sizeof(int),1,hfile);
			}
			delete[] ibuf;
		}

		void PresetIO::SaveVersion1(FILE* hfile, std::list<CPreset>& presets)
		{
			int temp1 = -1;
			int fileversion = 1;
			if ( std::fwrite(&temp1,sizeof(int),1,hfile) != 1 ||
				std::fwrite(&fileversion,sizeof(int),1,hfile) != 1 )
			{
				::MessageBox(0,"Couldn't write to File. Operation Aborted","File Save Error",MB_OK);
				return;
			}
			std::list<CPreset>::iterator preset = presets.begin();
			int numpresets=presets.size();
			int numParameters = preset->GetNumPars();
			int dataSizeStruct = preset->GetDataSize();
			std::fwrite(&numpresets,sizeof(int),1,hfile);
			std::fwrite(&numParameters,sizeof(int),1,hfile);
			std::fwrite(&dataSizeStruct,sizeof(int),1,hfile);
			
			char cbuf[32];
			int* ibuf= new int[numParameters];
			unsigned char * dbuf = 0;
			if ( dataSizeStruct > 0 ) dbuf = new byte[dataSizeStruct];

			for (int i=0; i< numpresets && !std::feof(hfile) && !std::ferror(hfile); i++, preset++)
			{
				preset->GetName(cbuf);
				preset->GetParsArray(ibuf);
				preset->GetDataArray(dbuf);
				std::fwrite(cbuf,sizeof(cbuf),1,hfile);
				std::fwrite(ibuf,numParameters*sizeof(int),1,hfile);
				if ( dataSizeStruct > 0 ) std::fwrite(dbuf,dataSizeStruct,1,hfile);
			}
			delete[] ibuf;
			delete[] dbuf;
		}

	}   // namespace
}   // namespace
