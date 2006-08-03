///\file
///\brief implementation file for psycle::host::Global.
#include <packageneric/pre-compiled.private.hpp>
#include PACKAGENERIC
#include <psycle/engine/global.hpp>
#include <psycle/helpers/dsp.hpp>
#include <psycle/engine/song.hpp>
#include <psycle/engine/player.hpp>
//todo:
//#include <psycle/engine/dllfinder.hpp>
//#include <psycle/engine/configuration.hpp>
#include <psycle/host/cacheddllfinder.hpp>
#include <psycle/host/uiconfiguration.hpp>

namespace psycle
{
	namespace host
	{
		Global::Global()
		{
			cpu_frequency_ = CalculateCPUFreq();
		}

		Global::~Global()
		{
		}

		cpu::cycles_type Global::CalculateCPUFreq()
		{
			return GetNaiveCPUFreq();
		}
	}
}
