/***************************************************************************
 *   Copyright (C) 2006 by  Stefan Nattkemper   *
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
#include "ladspa.h"

/**
@author  Stefan Nattkemper
*/

namespace psycle {
	namespace host {

		class LadspaParam
		{
		public:
			LadspaParam(LADSPA_PortDescriptor descriptor,LADSPA_PortRangeHint hint, const char *newname);
			LADSPA_PortDescriptor descriptor() { return descriptor_; }
			LADSPA_PortRangeHintDescriptor hint() { return hint_.HintDescriptor; }
			const char* name() { return portName_; }
			LADSPA_Data rawvalue() { return value_; }
			LADSPA_Data* valueaddress() { return &value_; }
			int value() ;
			void setValue(int data);
			void setrawvalue(LADSPA_Data data) { value_ = data; }
			void setDefault();
			LADSPA_Data minval() { return integer_ ? (minVal_*rangeMultiplier_) : 0; }
			LADSPA_Data maxval() { return integer_ ? (maxVal_-minVal_) : 65535; }
		private:
			LADSPA_PortDescriptor descriptor_;
			LADSPA_PortRangeHint hint_;
			const char * portName_;
			LADSPA_Data value_;
			LADSPA_Data minVal_;
			LADSPA_Data maxVal_;
			float rangeMultiplier_;
			bool integer_;
			bool logaritmic_;
		};
	
		class LADSPAMachine: public Machine {
		public:
            
            LADSPAMachine( int id, Song* song );
			virtual ~LADSPAMachine();

        public:
            virtual void Init();
			virtual void PreWork(int numSamples);
            virtual int GenerateAudio(int numSamples );
            virtual void Tick(int channel, const PatternEvent & pEntry );
            virtual void Stop(){}
            inline virtual std::string GetDllName() const  { return libName_.c_str(); }
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
