#include "plugin_interface.h"

namespace psycle
{
  namespace plugin_interface
  {
    CFxCallback::~CFxCallback() throw() {}

    void CFxCallback::MessBox(char* ptxt,char*caption,unsigned int type){}
    int CFxCallback::CallbackFunc(int cbkID,int par1,int par2,int par3){return 0;}
    float * CFxCallback::unused0(int, int){return 0;}
    float * CFxCallback::unused1(int, int){return 0;}
    int CFxCallback::GetTickLength(){return 2048;}
    int CFxCallback::GetSamplingRate(){return 44100;}
    int CFxCallback::GetBPM(){return 125;}
    int CFxCallback::GetTPB(){return 4;}
    // Don't get fooled by the above return values.
    // You get a pointer to a subclass of this one that returns the correct ones.


    CMachineInterface::~CMachineInterface() {}
    void CMachineInterface::Init() {}
    void CMachineInterface::SequencerTick() {}
    void CMachineInterface::ParameterTweak(int par, int val) {}
    void CMachineInterface::Work(float *psamplesleft, float *psamplesright , int numsamples, int tracks) {}
    void CMachineInterface::Stop() {}

    ///\name Export / Import
    ///\{
    void CMachineInterface::PutData(void * pData) {}
    void CMachineInterface::GetData(void * pData) {}
    int CMachineInterface::GetDataSize() { return 0; }
    ///\}
    
    void CMachineInterface::Command() {}
    void CMachineInterface::MuteTrack(int const i) {}
    bool CMachineInterface::IsTrackMuted(int const i) const { return false; }
    void CMachineInterface::MidiNote(int const channel, int const value, int const velocity) {}
    void CMachineInterface::Event(uint32 const data) {}
    bool CMachineInterface::DescribeValue(char* txt,int const param, int const value) { return false; }
    bool CMachineInterface::PlayWave(int const wave, int const note, float const volume) { return false; }
    void CMachineInterface::SeqTick(int channel, int note, int ins, int cmd, int val) {}
    void CMachineInterface::StopWave() {}
  }
}
