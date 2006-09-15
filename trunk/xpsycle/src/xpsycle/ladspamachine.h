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

		class LadspaParam
		{
		public:
			LadspaParam(LADSPA_PortDescriptor descriptor,LADSPA_PortRangeHint hint, const char *newname)
			:descriptor_(descriptor)
			,hint_(hint)
			,portName_(newname)
			,value_(0)
			{;}
			LADSPA_PortDescriptor descriptor() { return descriptor_; }
			LADSPA_PortRangeHintDescriptor hint() { return hint_.HintDescriptor; }
			LADSPA_Data minval() { return hint_.LowerBound; }
			LADSPA_Data maxval() { return hint_.UpperBound; }
			LADSPA_Data value() { return value_; }
			LADSPA_Data* valueaddress() { return &value_; }
			void setValue(LADSPA_Data data) { value_ =  data; }
			const char* name() { return portName_; }
		private:
			LADSPA_PortDescriptor descriptor_;
			LADSPA_PortRangeHint hint_;
			const char * portName_;
			LADSPA_Data value_;
		};
	
		class LADSPAMachine: public Machine {
		public:
            
            LADSPAMachine( Machine::id_type id, Song* song );
			virtual ~LADSPAMachine() throw();

        public:
            virtual void Init();
			virtual void PreWork(int numSamples);
            virtual int GenerateAudio(int numSamples );
            virtual void Tick(int channel, const PatternEvent & pEntry );
            virtual void Stop(){}
            inline virtual std::string GetDllName() const throw() { return libName_.c_str(); }
            virtual std::string GetName() const { return (char *) (psDescriptor)?psDescriptor->Name:""; };

            virtual void GetParamName(int numparam, char * name);
            virtual void GetParamRange(int numparam,int &minval, int &maxval);
            virtual int GetParamValue(int numparam);
            virtual void GetParamValue(int numparam,char* parval);
            virtual bool SetParameter(int numparam,int value);

			virtual bool LoadSpecificChunk(RiffFile * pFile, int version);
			virtual void SaveSpecificChunk(RiffFile * pFile);
			virtual void SaveDllName      (RiffFile * pFile);


        public:

			LADSPA_Descriptor_Function loadDescriptorFunction( const std::string & fileName );

			bool loadDll( const std::string & fileName ,int pluginIndex=0);
            const LADSPA_Descriptor* pluginDescriptor() { return psDescriptor; }
			std::string label() const
			{
			     return (psDescriptor)?psDescriptor->Label:"";
			}

		private:
		
            void *dlopenLADSPA(const char * pcFilename, int iFlag);
            void prepareStructures(void);
			LADSPA_Data GetMinValue(int lPortIndex, LADSPA_PortRangeHintDescriptor iHintDescriptor);
			LADSPA_Data GetMaxValue(int lPortIndex, LADSPA_PortRangeHintDescriptor iHintDescriptor);
			void SetDefaultsForControls();
			void* libHandle_;
			std::string libName_;

			const LADSPA_Descriptor * psDescriptor;
			/*const*/ LADSPA_Handle pluginHandle;
			std::vector<LadspaParam> values_;
			std::vector<LadspaParam> controls_;
			float* pOutSamplesL;
			float* pOutSamplesR;
		};
	}
}

#endif
