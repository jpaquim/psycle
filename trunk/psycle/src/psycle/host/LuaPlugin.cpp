// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007-2015 members of the psycle project http://psycle.sourceforge.net

///\file
///\brief implementation file for psycle::host::LuaPlugin.

#include <psycle/host/detail/project.private.hpp>
#include <psycle/host/LuaPlugin.hpp>

#include <psycle/host/Global.hpp>
#include <psycle/host/Configuration.hpp>
#include <psycle/host/Song.hpp>
#include <psycle/host/Player.hpp>
#include <psycle/helpers/math.hpp>
#include "Zap.hpp"

#include <lua.hpp>
#include <boost/algorithm/string.hpp>
#include <algorithm>
#include <sstream>
#include <iostream>
#include <fstream>


namespace psycle { namespace host {

  //////////////////////////////////////////////////////////////////////////
  // Lua

  LuaPlugin::LuaPlugin(lua_State* state, int index, bool full)
    : curr_prg_(0), proxy_(this, state)
  {		
    _macIndex = index;
    _type = MACH_LUA;
    _mode = MACHMODE_FX;
    usenoteon_ = false;
    std::sprintf(_editName, "native plugin");		
    InitializeSamplesVector();
    for(int i(0) ; i < MAX_TRACKS; ++i) {
      trackNote[i].key = 255; // No Note.
      trackNote[i].midichan = 0;
    }
    try {
      proxy_.call_run();
      if (full) {
        proxy_.call_init();
      }
    } catch(std::exception &e) { 
      AfxMessageBox(e.what());  
    e; } //do nothing.
  }

  LuaPlugin::~LuaPlugin() {    
    Free();    
  }

  void LuaPlugin::Free() {
    try {
      proxy_.free_state();
    } catch(std::exception &e) { e; } //do nothing.
  }

  void LuaPlugin::OnReload() {
    try {
      proxy_.reload();
    } CATCH_WRAP_AND_RETHROW(*this)
    /*PluginInfo info = CallPluginInfo();
    _mode = info.mode;*/
  }

  bool LuaPlugin::OnEvent(canvas::Event* ev) {
    try {
       return proxy_.call_event(ev); 
      } catch(std::exception &e) { e;
       return false;
    } 
  }

  int LuaPlugin::GenerateAudioInTicks(int /*startSample*/, int numSamples) throw(psycle::host::exception)
  {
    if (crashed()) {
      return numSamples;
    }
    if(_mode == MACHMODE_GENERATOR) {
      Standby(false);
    }

    if (!_mute) 
    {
      if ((_mode == MACHMODE_GENERATOR) || (!Bypass() && !Standby()))
      {										
        int ns = numSamples;
        int us = 0;
        while (ns)
        {
          int nextevent = (TWSActive)?TWSSamples:ns+1;
          for (int i=0; i < Global::song().SONGTRACKS; i++)
          {
            if (TriggerDelay[i]._cmd)
            {
              if (TriggerDelayCounter[i] < nextevent)
              {
                nextevent = TriggerDelayCounter[i];
              }
            }
          }
          if (nextevent > ns)
          {
            if (TWSActive)
            {
              TWSSamples -= ns;
            }
            for (int i=0; i < Global::song().SONGTRACKS; i++)
            {
              // come back to this
              if (TriggerDelay[i]._cmd)
              {
                TriggerDelayCounter[i] -= ns;
              }
            }
            try
            {								
              //proxy().Work(samplesV[0]+us, samplesV[1]+us, ns, Global::song().SONGTRACKS);
              proxy_.call_work(ns, us);
            }
            catch(const std::exception & e)
            {
              e;
              return 0;
            }
            ns = 0;
          }
          else
          {
            if(nextevent)
            {
              ns -= nextevent;
              try
              {
                //todo: this should change if we implement multi-io for native plugins (complicated right now. needs new API calls)
                //proxy().Work(samplesV[0]+us, samplesV[1]+us, nextevent, Global::song().SONGTRACKS);
                proxy_.call_work(nextevent, us);
              }
              catch(const std::exception &e)
              {
                e;
                return 0;
              }
              us += nextevent;
            }
            if (TWSActive)
            {
              if (TWSSamples == nextevent)
              {
                int activecount = 0;
                TWSSamples = TWEAK_SLIDE_SAMPLES;
                for (int i = 0; i < MAX_TWS; i++)
                {
                  if (TWSDelta[i] != 0)
                  {
                    TWSCurrent[i] += TWSDelta[i];

                    if (((TWSDelta[i] > 0) && (TWSCurrent[i] >= TWSDestination[i]))
                      || ((TWSDelta[i] < 0) && (TWSCurrent[i] <= TWSDestination[i])))
                    {
                      TWSCurrent[i] = TWSDestination[i];
                      TWSDelta[i] = 0;
                    }
                    else
                    {
                      activecount++;
                    }
                    try
                    {
                      proxy_.call_parameter(TWSInst[i], int(TWSCurrent[i])/(double)0xFFFF);
                    }
                    catch(const std::exception &e)
                    {
                      e;
                      return 0;
                    }
                  }
                }
                if(!activecount) TWSActive = false;
              }
            }
            for (int i=0; i < Global::song().SONGTRACKS; i++)
            {
              // come back to this
              if (TriggerDelay[i]._cmd == PatternCmd::NOTE_DELAY)
              {
                if (TriggerDelayCounter[i] == nextevent)
                {
                  // do event
                  try
                  {
                    proxy_.call_seqtick(i ,TriggerDelay[i]._note, TriggerDelay[i]._inst, 0, 0);											
                  }
                  catch(const std::exception &e)
                  {
                    e;
                    return 0;

                  }
                  TriggerDelay[i]._cmd = 0;
                }
                else
                {
                  TriggerDelayCounter[i] -= nextevent;
                }
              }
              else if (TriggerDelay[i]._cmd == PatternCmd::RETRIGGER)
              {
                if (TriggerDelayCounter[i] == nextevent)
                {
                  // do event
                  try
                  {
                    proxy_.call_seqtick(i, TriggerDelay[i]._note, TriggerDelay[i]._inst, 0, 0);
                  }
                  catch(const std::exception &e)
                  {

                    e;
                    return 0;

                  }
                  TriggerDelayCounter[i] = (RetriggerRate[i]*Global::player().SamplesPerRow())/256;
                }
                else
                {
                  TriggerDelayCounter[i] -= nextevent;
                }
              }
              else if (TriggerDelay[i]._cmd == PatternCmd::RETR_CONT)
              {
                if (TriggerDelayCounter[i] == nextevent)
                {
                  // do event
                  try
                  {
                    proxy_.call_seqtick(i ,TriggerDelay[i]._note, TriggerDelay[i]._inst, 0, 0);
                  }
                  catch(const std::exception &e)
                  {
                    e;
                    return 0;
                  }
                  TriggerDelayCounter[i] = (RetriggerRate[i]*Global::player().SamplesPerRow())/256;
                  int parameter = TriggerDelay[i]._parameter&0x0f;
                  if (parameter < 9)
                  {
                    RetriggerRate[i]+= 4*parameter;
                  }
                  else
                  {
                    RetriggerRate[i]-= 2*(16-parameter);
                    if (RetriggerRate[i] < 16)
                    {
                      RetriggerRate[i] = 16;
                    }
                  }
                }
                else
                {
                  TriggerDelayCounter[i] -= nextevent;
                }
              }
              else if (TriggerDelay[i]._cmd == PatternCmd::ARPEGGIO)
              {
                if (TriggerDelayCounter[i] == nextevent)
                {
                  PatternEntry entry =TriggerDelay[i];
                  switch(ArpeggioCount[i])
                  {
                  case 0: 
                    try
                    {
                      proxy_.call_seqtick(i ,TriggerDelay[i]._note, TriggerDelay[i]._inst, 0, 0);
                    }
                    catch(const std::exception &e)
                    {
                      e;
                      return 0;
                    }
                    ArpeggioCount[i]++;
                    break;
                  case 1:
                    entry._note+=((TriggerDelay[i]._parameter&0xF0)>>4);
                    try
                    {
                      proxy_.call_seqtick(i ,entry._note, entry._inst, 0, 0);
                    }
                    catch(const std::exception &e)
                    {
                      e;
                      return 0;
                    }
                    ArpeggioCount[i]++;
                    break;
                  case 2:
                    entry._note+=(TriggerDelay[i]._parameter&0x0F);
                    try
                    {
                      proxy_.call_seqtick(i ,entry._note, entry._inst, 0, 0);
                    }
                    catch(const std::exception &e)
                    {
                      e;
                      return 0;
                    }
                    ArpeggioCount[i]=0;
                    break;
                  }
                  TriggerDelayCounter[i] = Global::player().SamplesPerTick();
                }
                else
                {
                  TriggerDelayCounter[i] -= nextevent;
                }
              }
            }
          }
        }
        UpdateVuAndStanbyFlag(numSamples);
      }
    }
    else Standby(true);
    return numSamples;
  }

  bool LuaPlugin::LoadSpecificChunk(RiffFile* pFile, int version)
  {
    if (proxy_.prsmode() == LuaMachine::NATIVEPRS) {
    uint32_t size;
    pFile->Read(size); // size of whole structure
    if(size)
    {
      uint32_t count;
      pFile->Read(count);  // size of vars
      //Read vals and names to do SetParameter.
      //It is done this way to allow parameters to change without disrupting the loader.
      std::vector<int> vals;
      std::map<std::string, int> ids;
      for (uint32_t i = 0; i < count; i++) {
        int temp;
        pFile->Read(temp);
        vals.push_back(temp);
      }
      for (uint32_t i = 0; i < count; i++) {
        std::string id;
        pFile->ReadString(id);
        ids[id] = i;
      }
      int num = GetNumParams();
      for (int i = 0; i < num; ++i) {
        std::string id = proxy_.get_parameter_id(i);
        std::map<std::string, int>::iterator it = ids.find(id);
        if (it != ids.end()) {
          int idx = ids[id];
          SetParameter(i, vals[idx]);
        } else {
          // parameter not found
        }
      }
      uint32_t size2=0;
      pFile->Read(size2);
      if(size2)
      {
        byte* pData = new byte[size2];
        pFile->Read(pData, size2); // Number of parameters
        try
        {
          proxy_.call_putdata(pData, size2);
          delete[] pData;
        }
        catch(const std::exception &e)
        {
          e;
          delete[] pData;
          return false;
        }
      }
    }
    return true;
    } else {
      try {
					UINT size;
					unsigned char _program;
					pFile->Read(&size, sizeof size );
					if(size)
					{
						UINT count;
						pFile->Read(&_program, sizeof _program);
						pFile->Read(&count, sizeof count);
						size -= sizeof _program + sizeof count + sizeof(float) * count;
						if(!size)
						{
							/*BeginSetProgram();
							SetProgram(_program);
							for(UINT i(0) ; i < count ; ++i)
							{
								float temp;
								pFile->Read(&temp, sizeof temp);
								SetParameter(i, temp);
							}
							EndSetProgram();*/
						}
						else
						{
							/*BeginSetProgram();
							SetProgram(_program);
							EndSetProgram();*/
							pFile->Skip(sizeof(float) *count);
              bool b = proxy_.prsmode() == LuaMachine::CHUNKPRS;
							if(b)
							{
								unsigned char * data(new unsigned char[size]);
								pFile->Read(data, size); // Number of parameters
                proxy_.call_putdata(data, size);
								zapArray(data);
							}
							else
							{
								// there is a data chunk, but this machine does not want one.
								pFile->Skip(size);
								return false;
							}
						}
					}
					return true;
				}
				catch(...){return false;}
    }
  }

  void LuaPlugin::SaveSpecificChunk(RiffFile * pFile)
  {
    if (proxy_.prsmode() == LuaMachine::NATIVEPRS) {
      uint32_t count = GetNumParams();
      uint32_t size2(0);
      unsigned char * pData = 0;
      try
      {
        size2 = proxy_.call_data(&pData, false);
      }
      catch(const std::exception &e)
      {
        e;
        // data won't be saved
      }
      uint32_t size = size2 + sizeof(count) + sizeof(int)*count;
      std::vector<std::string> ids;
      for (UINT i = 0; i < count; i++) {
        std::string id = proxy_.get_parameter_id(i);
        ids.push_back(id);
        size += id.length()+1;
      }
      pFile->Write(size);
      pFile->Write(count);
      for (uint32_t i = 0; i < count; i++)
      {
        int temp = GetParamValue(i);
        pFile->Write(temp);
      }
      // ids
      for (uint32_t i = 0; i < count; i++) {
        pFile->WriteString(ids[i]);
      }
      pFile->Write(size2);
      if(size2)
      {        
        pFile->Write(pData, size2); // Number of parameters
        zapArray(pData);
      }
    } else {
    try {
			    UINT count(GetNumParams());
					unsigned char _program=0;
					UINT size(sizeof _program + sizeof count);
					UINT chunksize(0);
					unsigned char * pData(0);
          bool b = proxy_.prsmode() == LuaMachine::CHUNKPRS;
					if(b)
					{
						count=0;
            chunksize = proxy_.call_data(&pData, true);
						size+=chunksize;
					}
					else
					{
						 size+=sizeof(float) * count;
					}
					pFile->Write(&size, sizeof size);
					// _program = static_cast<unsigned char>(GetProgram());
					pFile->Write(&_program, sizeof _program);
					pFile->Write(&count, sizeof count);

					if(b)
					{
						pFile->Write(pData, chunksize);
					}
					else
					{
						for(UINT i(0); i < count; ++i)
						{
              float temp = proxy_.get_parameter_value(i);
							pFile->Write(&temp, sizeof temp);
						}
					}
				} catch(...) {
			}
    }
  }

  bool LuaPlugin::SetParameter(int numparam, int value) {
    if (crashed() || numparam < 0) {
      return false;
    }
    try {
      int minval; int maxval;
      proxy_.get_parameter_range(numparam, minval, maxval);
      int quantization = (maxval-minval);
      proxy_.call_parameter(numparam,double(value)/double(quantization));
      return true;
    } catch(std::exception &e) { e; } //do nothing.
    return false;
  }

  void LuaPlugin::GetParamRange(int numparam,int &minval, int &maxval) { 
    if (crashed() || numparam < 0) {
      minval = 0; maxval = 0xFFFF;
      return;
    }
    try {
      if( numparam < GetNumParams() ) {
        proxy_.get_parameter_range(numparam, minval, maxval);
      }
    } catch(std::exception &e) { e; }
  }

  int LuaPlugin::GetParamType(int numparam) { 
    if (crashed() || numparam < 0) {			  
      return 0;
    }
    int mpf = 0 ;
    try {
      if( numparam < GetNumParams() ) {
        mpf = proxy_.get_parameter_type(numparam);
      }
    } catch(std::exception &e) { e; }
    return mpf;
  }

  void LuaPlugin::GetParamName(int numparam, char * parval) {
    if (crashed() || numparam < 0) {
      std::strcpy(parval, "");
      return;
    }
    try {
      if( numparam < GetNumParams() ) {
        std::string name = proxy_.get_parameter_name(numparam);
        std::strcpy(parval, name.c_str());
      } else std::strcpy(parval, "Out of Range");
    } catch(std::exception &e) { e; std::strcpy(parval, ""); }
  }

  int LuaPlugin::GetParamValue(int numparam){
    if (crashed() || numparam < 0) {
      return 0;
    }
    if (numparam < GetNumParams()) {
      int minval; int maxval;			  
      try {
        proxy_.get_parameter_range(numparam, minval, maxval);
        int quantization = (maxval-minval);
        return proxy_.get_parameter_value(numparam) * quantization;
      } catch(std::exception &e) { e; } //do nothing.
    } else {
      // out of range
    }
    return 0;
  }

  bool LuaPlugin::DescribeValue(int numparam, char * psTxt){			
    if (crashed() || numparam < 0) {
      std::string par_display("Out of range or Crashed");
      std::sprintf(psTxt, "%s", par_display);
      return false;
    }
    if(numparam >= 0 && numparam < GetNumParams()) {
      try {
        std::string par_display = proxy_.get_parameter_display(numparam);
        std::string par_label = proxy_.get_parameter_label(numparam);
        if (par_label == "")
          std::sprintf(psTxt, "%s", par_display.c_str());
        else {
          std::sprintf(psTxt, "%s(%s)", par_display.c_str(), par_label.c_str());
        }
        return true;
      } catch(std::exception &e) {
        e;
        std::string par_display("Out of range");
        std::sprintf(psTxt, "%s", par_display);
        return true;
      } //do nothing.
    }
    return false;
  }

  void LuaPlugin::GetParamValue(int numparam, char * parval) {
    if (crashed() || numparam < 0) {
      std::strcpy(parval, "Out of Range or Crashed");
      return;
    }
    if(numparam < GetNumParams()) {
      try {
        if(!DescribeValue(numparam, parval)) {
          std::sprintf(parval,"%.0f",GetParamValue(numparam) * 1); // 1 = Plugin::quantizationVal())
        }					
      }
      catch(const std::exception &e) {
        e;
        return;
      }
    }
    else std::strcpy(parval,"Out of Range");
  }

  void LuaPlugin::GetParamId(int numparam, std::string& id) {
    if (crashed() || numparam < 0) {      
      return;
    }
    if(numparam < GetNumParams()) {
      try {
        id = proxy_.get_parameter_id(numparam);
      } catch(const std::exception &e) {
        e;
        return;
      }
    }
  }

  void LuaPlugin::AfterTweaked(int numparam) {
    if (crashed() || numparam < 0) {      
      return;
    }
    if(numparam < GetNumParams()) {
      try {
        proxy_.call_aftertweaked(numparam);
      } catch(const std::exception &e) {
        e;
        return;
      }
    }
  }
  
  std::string LuaPlugin::help() {
    if (crashed()) {
      return "saucer section missing";
    }		
    try {
      return proxy_.call_help();
    } catch(const std::exception &e) { e; }
    return "";
  }

  void LuaPlugin::NewLine() {
    if (crashed()) {
      return;
    }		
    try {
      proxy_.call_newline();
    } catch(const std::exception &e) { e; }
  }

  void LuaPlugin::Tick(int channel, PatternEntry * pData){
    if (crashed()) {
      return;
    }
    if(pData->_note == notecommands::tweak || pData->_note == notecommands::tweakeffect)
    {
      if(pData->_inst < proxy_.num_parameter())
      {
        int nv = (pData->_cmd<<8)+pData->_parameter;
        int const min = 0; // always range 0 .. FFFF like vst tweak
        int const max = 0xFFFF;
        nv += min;        
        if(nv > max) nv = max;                        
        // quantization done in parameter.lua
        try
        {
          proxy_.call_parameter(pData->_inst, double(nv)/0xFFFF);
        }
        catch(const std::exception &e)
        {
          e;
          return;
        }
      }
    } else if(pData->_note == notecommands::tweakslide)
    {
      if(pData->_inst < proxy_.num_parameter())
      {
        int i;
        if(TWSActive)
        {
          // see if a tweak slide for this parameter is already happening
          for(i = 0; i < MAX_TWS; i++)
          {
            if((TWSInst[i] == pData->_inst) && (TWSDelta[i] != 0))
            {
              // yes
              break;
            }
          }
          if(i == MAX_TWS)
          {
            // nope, find an empty slot
            for (i = 0; i < MAX_TWS; i++)
            {
              if (TWSDelta[i] == 0)
              {
                break;
              }
            }
          }
        }
        else
        {
          // wipe our array for safety
          for (i = MAX_TWS-1; i > 0; i--)
          {
            TWSDelta[i] = 0;
          }
        }
        if (i < MAX_TWS)
        {
          TWSDestination[i] = float(pData->_cmd<<8)+pData->_parameter;
          float min = 0; // float(_pInfo->Parameters[pData->_inst]->MinValue);
          float max = 0xFFFF; //float(_pInfo->Parameters[pData->_inst]->MaxValue);
          TWSDestination[i] += min;
          if (TWSDestination[i] > max)
          {
            TWSDestination[i] = max;
          }
          TWSInst[i] = pData->_inst;
          try
          {
            TWSCurrent[i] = proxy_.get_parameter_value(TWSInst[i])*0xFFFF;
          }
          catch(const std::exception &e)
          {
            e;
            return;
          }
          TWSDelta[i] = float((TWSDestination[i]-TWSCurrent[i])*TWEAK_SLIDE_SAMPLES)/Global::player().SamplesPerRow();
          TWSSamples = 0;
          TWSActive = TRUE;
        }
        else
        {
          // we have used all our slots, just send a twk
          int nv = (pData->_cmd<<8)+pData->_parameter;
          int const min = 0; //_pInfo->Parameters[pData->_inst]->MinValue;
          int const max = 0xFFFF; //_pInfo->Parameters[pData->_inst]->MaxValue;
          nv += min;
          if (nv > max) nv = max;
          // quantization done in parameter.lua
          try
          {
            proxy_.call_parameter(pData->_inst, nv/(double)0xFFFF);
          }
          catch(const std::exception &e)
          {
            e;
            return;
          }
        }
      }
    } else
      try {
        const int note = pData->_note;
        if (usenoteon_==0) {
          proxy_.call_seqtick(channel, pData->_note, pData->_inst, pData->_cmd,
            pData->_parameter);
        } else {
          // noteon modus				
          if (note < notecommands::release)  { // Note on
            SendNoteOn(channel, note, pData->_inst, pData->_cmd, pData->_parameter);
          } else if (note == notecommands::release) { // Note Off. 				
            SendNoteOff(channel, note, notecommands::empty, pData->_inst,
              pData->_cmd, pData->_parameter);
          } else {
            SendCommand(channel, pData->_inst, pData->_cmd, pData->_parameter);
          }
        }
    } catch(const std::exception &e) { e; }
  }

  void LuaPlugin::Stop(){
    if (crashed()) {
      return;
    }
    try {			  
      proxy_.call_stop();
      if (usenoteon_!=0) {
        for(int i(0) ; i < MAX_TRACKS; ++i) {
          trackNote[i].key = 255; // No Note.
          trackNote[i].midichan = 0;
        }
      }
    } catch(const std::exception &e) { e; }
  }

  // additions if noteon mode is used
  void LuaPlugin::SendCommand(unsigned char channel,
    unsigned char inst,
    unsigned char cmd,
    unsigned char val) {
      int oldnote = notecommands::empty;
      if(trackNote[channel].key != notecommands::empty) {
        oldnote = trackNote[channel].key;
      }
      proxy_.call_command(oldnote, inst, cmd, val);
  }

  void LuaPlugin::SendNoteOn(unsigned char channel,
    unsigned char key,
    unsigned char inst,
    unsigned char cmd,
    unsigned char val) {
      int oldnote = notecommands::empty;
      if(trackNote[channel].key != notecommands::empty) {
        oldnote = trackNote[channel].key;
        SendNoteOff(channel, trackNote[channel].key, oldnote, inst, cmd, val);
      }
      note thisnote;
      thisnote.key = key;
      thisnote.midichan = 0;
      trackNote[channel] = thisnote;
      proxy_.call_noteon(key, oldnote, inst, cmd, val);
  }

  void LuaPlugin::SendNoteOff(unsigned char channel,
    unsigned char key,
    unsigned char lastkey,
    unsigned char inst,
    unsigned char cmd,
    unsigned char val) {
      if (trackNote[channel].key == notecommands::empty)
        return;
      note thenote = trackNote[channel];
      proxy_.call_noteoff(thenote.key, lastkey, inst, cmd, val);
      trackNote[channel].key = 255;
      trackNote[channel].midichan = 0;
  }

  //Bank & Programs
  
   void LuaPlugin::SetCurrentProgram(int idx) {
     if (crashed()) {
       return;
     }
     try {		
       proxy_.call_setprogram(idx);
       curr_prg_ = idx;
     } catch(const std::exception &e) { e; }     
   }

   void LuaPlugin::GetIndexProgramName(int bnkidx, int prgIdx, char* val) {     
     if (crashed()) {
       std::strcpy(val, "");
       return;
     }
     try {			  
       std::string name = proxy_.get_program_name(bnkidx, prgIdx);
       std::strcpy(val, name.c_str());
     } catch(const std::exception &e) {
       e; std::strcpy(val, "Out of Range");
     }
   }          

   //Bank & Programs
   int LuaPlugin::GetNumPrograms() {     
     if (crashed()) {
       return 0;
     }
     try {			  
       return proxy_.call_numprograms();
     } catch(const std::exception &e) { e; }
     return 0;
   }

   int LuaPlugin::GetCurrentProgram() {     
     if (crashed()) {
       return 0;
     }
     try {			  
       return proxy_.get_curr_program();
     } catch(const std::exception &e) { e; }
     return 0;
   }

   void LuaPlugin::SaveBank(const std::string& filename) {
      unsigned char * pData(0);
      int chunksize = proxy_.call_data(&pData, true);
      using namespace std;
	  #if __cplusplus >= 201103L
        ofstream ofile(filename, ios::binary);
      #else
	    ofstream ofile(filename.c_str(), ios::binary);
      #endif
      ofile.write((char*)pData, chunksize);
      ofile.close();
   }

   bool LuaPlugin::LoadBank(const std::string& filename) {      
      using namespace std;
      streampos size;
      char* pData;
	  #if __cplusplus >= 201103L
        ifstream file (filename, ios::in|ios::binary|ios::ate);
      #else
	    ifstream file (filename.c_str(), ios::in|ios::binary|ios::ate);
      #endif
      if (file.is_open()) {
        size = file.tellg();
        pData = new char[size];
        file.seekg (0, ios::beg);
        file.read(pData, size);
        file.close();
        proxy_.call_putdata((unsigned char*)pData, size);
        delete[] pData;
        return true;
      }    
      return false;
   }

}   // namespace
}   // namespace
