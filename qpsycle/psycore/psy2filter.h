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
#ifndef PSY2FILTER_H
#define PSY2FILTER_H

#include "psyfilter.h"

/**
@author  Psycledelics  
*/

namespace psy
{
	namespace core
	{

		class RiffFile;
    class PatternCategory;
    class SequenceLine;
    class Machine;
    class PatternEvent;
    class MachineCallbacks;

		#ifdef __unix__
		namespace convert_internal_machines
		{
			class Converter;
		}
				#endif

		class Psy2Filter : public PsyFilter
		{
		protected:
				class VSTLoader
				{
					public:
						bool valid;
						char dllName[128];
						int numpars;
						float * pars;
				};


			// Singleton Pattern
		protected:
	  	Psy2Filter();          
			virtual ~Psy2Filter();

		private:
			Psy2Filter( Psy2Filter const & );
			Psy2Filter& operator=(Psy2Filter const&);

		public:
			static Psy2Filter* Instance() {
					// don`t use multithreaded
					static Psy2Filter s;
						return &s; 
			}
			// Singleton pattern end
	
			protected:

				virtual bool testFormat(const std::string & fileName);
				virtual bool load(const std::string & fileName, CoreSong & song, MachineCallbacks* callbacks);

				virtual bool LoadINFO(RiffFile* file,CoreSong& song);
				virtual bool LoadSNGI(RiffFile* file,CoreSong& song);
				virtual bool LoadSEQD(RiffFile* file,CoreSong& song);
				virtual bool LoadPATD(RiffFile* file,CoreSong& song,int index);
				virtual bool LoadINSD(RiffFile* file,CoreSong& song);
				virtual bool LoadWAVD(RiffFile* file,CoreSong& song);
				virtual bool PreLoadVSTs(RiffFile* file,CoreSong& song);
				#ifdef __unix__
				virtual bool LoadMACD(RiffFile* file,CoreSong& song,convert_internal_machines::Converter* converter, MachineCallbacks* callbacks);
				virtual bool TidyUp(RiffFile* file,CoreSong &song,convert_internal_machines::Converter* converter);
								#endif				
	

		protected:

			static std::string const FILE_FOURCC;
			/// PSY2-fileformat Constants
			static int const PSY2_MAX_TRACKS;
			static int const PSY2_MAX_WAVES;
			static int const PSY2_MAX_INSTRUMENTS;
			static int const PSY2_MAX_PLUGINS;

		private:

				std::vector<int> seqList;
				PatternCategory* singleCat;
				SequenceLine* singleLine;
				Machine* pMac[128];
				bool _machineActive[128];
				unsigned char busMachine[64];
				unsigned char busEffect[64];
				VSTLoader vstL[256];

				void preparePatternSequence(CoreSong & song);
				PatternEvent convertEntry( unsigned char * data ) const;
	
	
		};

	}
}

#endif	//_PSY2FILTER_H_
