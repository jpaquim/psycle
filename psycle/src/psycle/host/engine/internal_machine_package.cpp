#include <packageneric/pre-compiled.private.hpp>
#include PACKAGENERIC
#include <psycle/host/engine/internal_machines_package.hpp>
#include <psycle/host/engine/machine.hpp>
#include <psycle/host/engine/internal_machines.hpp>
#include <psycle/host/engine/sampler.hpp>
#include <psycle/host/engine/XMSampler.hpp>
#include <psycle/host/engine/plugin.hpp>
#include <psycle/host/engine/VSTHost.hpp>

namespace psycle{
	namespace host{
		internal_machine_package::internal_machine_package()
		{
			infomap[MACH_MASTER]=&Master::minfo;
		//	infomap[MACH_SINE]=&ConvertedPlugin::minfo;
		//	infomap[MACH_DIST]=&ConvertedPlugin::minfo;
			infomap[MACH_SAMPLER]=&Sampler::minfo;
		//	infomap[MACH_DELAY]=&ConvertedPlugin::minfo;
		//	infomap[MACH_2PFILTER]=&ConvertedPlugin::minfo;
		//	infomap[MACH_GAIN]=&ConvertedPlugin::minfo;
		//	infomap[MACH_FLANGER]=&ConvertedPlugin::minfo;
			infomap[MACH_PLUGIN]=&Plugin::minfo;
			infomap[MACH_VST]=&vst::plugin::minfo;
			infomap[MACH_VSTFX]=&vst::plugin::minfo2;
			infomap[MACH_SCOPE]=&Dummy::minfo;
			infomap[MACH_XMSAMPLER]=&XMSampler::minfo;
			infomap[MACH_DUPLICATOR]=&DuplicatorMac::minfo;
			infomap[MACH_MIXER]=&Mixer::minfo;
			infomap[MACH_LFO]=&LFO::minfo;
			infomap[MACH_AUTOMATOR]=&Automator::minfo;
			infomap[MACH_DUMMY]=&Dummy::minfo;
		}

		internal_machine_package::~internal_machine_package()
		{
			infomap.clear();
		}

		InternalMachineInfo &internal_machine_package::getInfo(int type) const
		{
			return *infomap[type];
		}
	}
}
