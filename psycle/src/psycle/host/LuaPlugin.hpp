///\file
///\brief interface file for psycle::host::LuaPlugin.
#pragma once
#include <psycle/host/detail/project.hpp>
#include "Global.hpp"
#include "Machine.hpp"
#include "LuaArray.hpp"
#include "LuaHost.hpp"


struct lua_State;
namespace psycle { namespace host { namespace canvas { struct Event; }}}

namespace psycle { namespace host {

  class LuaPlugin : public Machine
  {
  public:
    LuaPlugin(lua_State* state, int index, bool full=true);
    virtual ~LuaPlugin();
    void Free();

    virtual int GenerateAudioInTicks( int startSample, int numSamples );
    virtual float GetAudioRange() const { return 1.0f; }
    virtual const char* const GetName(void) const { return proxy_.info().name.c_str(); }
    // todo other paramter save that doesnt invalidate tweaks when changing num parameters
    virtual bool LoadSpecificChunk(RiffFile* pFile, int version);
    virtual void SaveSpecificChunk(RiffFile * pFile);
    // todo testing
    virtual const char * const GetDllName() const throw() { return dll_path_.c_str(); }
    //PluginInfo
    virtual const std::string GetAuthor() { return proxy_.info().vendor; }
    virtual const uint32_t GetAPIVersion() { return proxy_.info().APIversion; }
    virtual const uint32_t GetPlugVersion() { return atoi(proxy_.info().version.c_str()); }
    bool IsSynth() const throw() { return (proxy_.info().mode == MACHMODE_GENERATOR); }

    //TODO: testing
    virtual void NewLine();
    //TODO testing
    virtual void Tick(int track, PatternEntry * pData);
    virtual void Stop();
    //TODO: testing
    virtual int GetNumCols() { return proxy_.num_cols(); }
    virtual void GetParamRange(int numparam,int &minval, int &maxval);
    virtual int GetNumParams() { return proxy_.num_parameter(); }
    virtual int GetParamType(int numparam);
    virtual void GetParamName(int numparam, char * parval);
    virtual void GetParamValue(int numparam, char * parval);
    virtual int GetParamValue(int numparam);
    virtual bool SetParameter(int numparam, int value); //{ return false;}
    virtual bool DescribeValue(int parameter, char * psTxt);
    virtual int GetNumInputPins() const { return this->IsSynth() ? 0 : samplesV.size(); }
    virtual int GetNumOutputPins() const { return samplesV.size(); }
    PluginInfo CallPluginInfo() { return proxy_.call_info(); }
    virtual void SetSampleRate(int sr) { try {Machine::SetSampleRate(sr); proxy_.call_sr_changed((float)sr); }catch(...){} }
    virtual void AfterTweaked(int idx) { proxy_.call_aftertweaked(idx); };
    template<class T> void GetMenu(T& f) { proxy_.get_menu(f); }	
    std::string help();
    virtual int GetGuiType() const { return proxy_.gui_type(); }
    void OnMenu(const std::string& id) { proxy_.call_menu(id); }
    canvas::Canvas* GetCanvas() { return proxy_.call_canvas(); }
    void OnEvent(canvas::Event* ev) { proxy_.call_event(ev); }

    virtual void OnReload();

    std::string dll_path_;
    bool usenoteon_;

  protected:
    LuaProxy proxy_;

  private:
    // additions if noteon mode is used
    struct note { 
      unsigned char key;
      unsigned char midichan;
    };
    note trackNote[MAX_TRACKS];
    void SendNoteOn(unsigned char channel,
      unsigned char key,
      unsigned char inst,
      unsigned char cmd,
      unsigned char val);
    void SendNoteOff(unsigned char channel,
      unsigned char key,
      unsigned char lastkey,
      unsigned char inst,
      unsigned char cmd,
      unsigned char val);
    void SendCommand(unsigned char channel,
      unsigned char inst,
      unsigned char cmd,
      unsigned char val);
  };


}   // namespace
}   // namespace
