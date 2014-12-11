// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007-2010 members of the psycle project http://psycle.sourceforge.net

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


namespace psycle { namespace host {

  //////////////////////////////////////////////////////////////////////////
  // Lua

  LuaPlugin::LuaPlugin(lua_State* state, int index, bool full)
    : proxy_(this, state)
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

  void  LuaPlugin::OnEvent(canvas::Event* ev) {
      try {
        proxy_.call_event(ev); 
      } catch(std::exception &e) { e; } 
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
  }

  void LuaPlugin::SaveSpecificChunk(RiffFile * pFile)
  {
    uint32_t count = GetNumParams();
    uint32_t size2(0);
    try
    {
      size2 = proxy_.call_data_size();
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
      byte * pData = new byte[size2];
      try
      {
        proxy_.call_data(pData); // Internal save
      }
      catch(const std::exception &e)
      {
        e;
        // this sucks because we already wrote the size,
        // so now we have to write the data, even if they are corrupted.
        throw e;
      }
      pFile->Write(pData, size2); // Number of parameters
      zapArray(pData);
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
    }catch(std::exception &e) { e; }
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
    }catch(std::exception &e) { e; }
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
    }catch(std::exception &e) { e; std::strcpy(parval, ""); }
  }

  int LuaPlugin::GetParamValue(int numparam){
    if (crashed() || numparam < 0) {
      return 0;
    }
    if(numparam < GetNumParams()) {
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
        try
        {
          proxy_.call_parameter(pData->_inst, nv/(double)0xFFFF);
        }
        catch(const std::exception &e)
        {
#ifndef NDEBUG 
          throw e;
          return;
#else
          e;
          return;
#endif
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
#ifndef NDEBUG 
            throw e;
            return;
#else
            e;
            return;
#endif
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
          try
          {
            proxy_.call_parameter(pData->_inst, nv/(double)0xFFFF);
          }
          catch(const std::exception &e)
          {
#ifndef NDEBUG 
            throw e;
            return;
#else
            e;
            return;
#endif
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


}   // namespace
}   // namespace
