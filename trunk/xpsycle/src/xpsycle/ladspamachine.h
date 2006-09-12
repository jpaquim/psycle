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
#ifndef LADSPAMACHINE_H
#define LADSPAMACHINE_H

#include "machine.h"
#include <ladspa.h>
/**
@author Stefan Nattkemper
*/

namespace psycle {
	namespace host {

		class LADSPAMachine: public Machine {
		public:
            
            LADSPAMachine( Machine::id_type id, Song* song );
			virtual ~LADSPAMachine() throw();

        public:
            virtual void Init();
            virtual int GenerateAudioInTicks( int startSample, int numSamples );
            virtual void Tick(int channel, const PatternEvent & pEntry );
            virtual void Stop(){}
            inline virtual std::string GetDllName() const throw() { return libName_.c_str(); }
            virtual std::string GetName() const { return (char *) psDescriptor->Name; };

            virtual int GetNumParams() { return psDescriptor->PortCount; } // This is not correct, but for now it's ok.
            virtual int GetNumCols() { (GetNumParams()/24)+1; } 
            virtual void GetParamName(int numparam, char * name);
            virtual void GetParamRange(int numparam,int &minval, int &maxval);
            virtual int GetParamValue(int numparam);
            virtual void GetParamValue(int numparam,char* parval);
            virtual bool SetParameter(int numparam,int value);

			virtual bool LoadSpecificChunk(RiffFile * pFile, int version);
			virtual void SaveSpecificChunk(RiffFile * pFile);
			virtual void SaveDllName      (RiffFile * pFile);


        public:
			bool loadDll( const std::string & fileName ,int pluginIndex=0);
            const LADSPA_Descriptor* pluginDescriptor() { return psDescriptor; }
			std::string label() const
			{
			     return (psDescriptor)?psDescriptor->Label:"";
			}

		private:
		
            void *dlopenLADSPA(const char * pcFilename, int iFlag);
            void prepareStructures(void);

			void* libHandle_;
			std::string libName_;

			const LADSPA_Descriptor * psDescriptor;
			/*const*/ LADSPA_Handle pluginHandle;
			LADSPA_Data **ppfValues;

		};
	}
}

#endif
