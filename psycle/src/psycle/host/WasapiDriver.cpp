///\file
///\brief implementation file for psycle::host::AudioDriver.

#include "WasapiDriver.hpp"
#include "Constants.hpp"
namespace psycle
{
	namespace host
	{
		AudioDriverInfo WasapiDriver::_info = { "Windows WASAPI inteface" };

		WasapiDriver::WasapiDriver()
		{}

		WasapiDriver::~WasapiDriver() {};

		void WasapiDriver::RefreshAvailablePorts() {

		}
		 void WasapiDriver::Initialize(HWND hwnd, AUDIODRIVERWORKFN pCallback, void* context) {};
		 void WasapiDriver::Reset(void) {};
		//todo
		 bool WasapiDriver::Enable(bool e) { return false; }
		//todo
		 bool WasapiDriver::Enabled() { return false; }
		//todo
		 void WasapiDriver::GetCapturePorts(std::vector<std::string> &ports) { ports.resize(0); }
		//todo
		 void WasapiDriver::GetReadBuffers(int idx, float **pleft, float **pright,int numsamples) { pleft=0; pright=0; return; }
		//todo
		 bool WasapiDriver::AddCapturePort(int idx){ return false; };
		//todo
		 bool WasapiDriver::RemoveCapturePort(int idx){ return false; }
		//todo
		 int WasapiDriver::GetWritePos() { return 0; }
		//todo
		 int WasapiDriver::GetPlayPos() { return 0; }
		//todo
		 int WasapiDriver::GetInputLatency() { return 0; }
		//todo
		 int WasapiDriver::GetOutputLatency() { return 0; }
		//todo
		 void WasapiDriver::Configure(void) {};
		//todo
		 bool WasapiDriver::Initialized(void) { return true; }
		//todo
		 bool WasapiDriver::Configured(void) { return true; }
	}
}
