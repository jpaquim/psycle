#include <packageneric/pre-compiled.private.hpp>
#include <packageneric/module.private.hpp>
#include <psycle/engine/internal_machine_package.hpp>
#include <psycle/engine/internal_machines.hpp>
#include <psycle/engine/sampler.hpp>
#include <psycle/engine/XMSampler.hpp>
#include <psycle/engine/plugin.hpp>
#include <psycle/engine/VSTHost.hpp>

namespace psycle{
	namespace host{

		InternalMachinePackage::InternalMachinePackage()
		{
			//\todo: Implement the machine creators of the converted internal machines (SINE, DIST, and so on).
			infomap[MACH_MASTER]=InternalMachineInfo(MACH_MASTER,MACHMODE_MASTER,Master::CreateFromType,false,false,"Master Machine","Master","Arguru",0,1000,0);
			infomap[MACH_SINE]=InternalMachineInfo(MACH_SINE,MACHMODE_FX,Dummy::CreateFromType,false,true,"Ayeternal Sine","SineLFO","Arguru and bohan",0,1000,0);
			infomap[MACH_DIST]=InternalMachineInfo(MACH_DIST,MACHMODE_FX,Dummy::CreateFromType,false,true,"Ayeternal Dist!","Dist!","Arguru and bohan",0,1000,0);
			infomap[MACH_SAMPLER]=InternalMachineInfo(MACH_SAMPLER,MACHMODE_GENERATOR,Sampler::CreateFromType,false,false,"Basic Sampler","Sampler","Arguru",0,500,0);
			infomap[MACH_DELAY]=InternalMachineInfo(MACH_DELAY,MACHMODE_FX,Dummy::CreateFromType,false,true,"Ayeternal Delay","Delay","Arguru and bohan",0,1000,0);
			infomap[MACH_2PFILTER]=InternalMachineInfo(MACH_2PFILTER,MACHMODE_FX,Dummy::CreateFromType,false,true,"Ayeternal 2Pfilter","2pfilter","Arguru and bohan",0,1000,0);
			infomap[MACH_GAIN]=InternalMachineInfo(MACH_GAIN,MACHMODE_FX,Dummy::CreateFromType,false,true,"Ayeternal Gain","Gain","Arguru and bohan",0,1000,0);
			infomap[MACH_FLANGER]=InternalMachineInfo(MACH_FLANGER,MACHMODE_FX,Dummy::CreateFromType,false,true,"Ayeternal Flanger","Flanger","Arguru and bohan",0,1000,0);
			infomap[MACH_PLUGIN]=InternalMachineInfo(MACH_PLUGIN,MACHMODE_UNDEFINED,Plugin::CreateFromType,true,false,"Native Plugin Host","Plugin","Psycledelics",0,1100,0);
			// The original VST host separated vst gens and vst fx. Nowadays, the difference is minimal, and the "FX" one could be removed with no problems.
			infomap[MACH_VST]=InternalMachineInfo(MACH_VST,MACHMODE_UNDEFINED,vst::plugin::CreateFromType,true,false,"VST 2.x Plugin Host","Vst Plugin","Psycledelics",0,1200,0);
			infomap[MACH_VSTFX]=InternalMachineInfo(MACH_VSTFX,MACHMODE_FX,vst::plugin::CreateFromType,true,true,"VST Host","Vst fx","Psycledelics",0,1200,0);
			infomap[MACH_SCOPE]=InternalMachineInfo(MACH_DUMMY,MACHMODE_FX,Dummy::CreateFromType,false,true,"Dummy Machine","Dummy","Arguru",0,1000,0);
			infomap[MACH_XMSAMPLER]=InternalMachineInfo(MACH_XMSAMPLER,MACHMODE_GENERATOR,XMSampler::CreateFromType,false,false,"Sampulse Sampler V2","Sampulse","JosepMa",0,600,0);
			//\todo: Change mode from Generator to controller.
			infomap[MACH_DUPLICATOR]=InternalMachineInfo(MACH_DUPLICATOR,MACHMODE_GENERATOR,DuplicatorMac::CreateFromType,false,false,"Note Duplicator","Dupe it!","JosepMa",0,1000,16);
			infomap[MACH_MIXER]=InternalMachineInfo(MACH_MIXER,MACHMODE_FX,Mixer::CreateFromType,false,false,"Send/Return Mixer","Mixer","JosepMa",0,500,255);
			//\todo: Change mode from generator to controller
			infomap[MACH_LFO]=InternalMachineInfo(MACH_LFO,MACHMODE_GENERATOR,LFO::CreateFromType,false,false,"LFO","LFO","dw",0,100,LFO::prms::num_params);
			//\todo: Change mode from generator to controller
			infomap[MACH_AUTOMATOR]=InternalMachineInfo(MACH_AUTOMATOR,MACHMODE_GENERATOR,Automator::CreateFromType,false,false,"Automator","Automator","dw",0,100,Automator::prms::num_params);
			infomap[MACH_DUMMY]=InternalMachineInfo(MACH_DUMMY,MACHMODE_FX,Dummy::CreateFromType,false,false,"Dummy Machine","Dummy","Arguru",0,1000,0);

			pos = infomap.end();
		}

		InternalMachinePackage::~InternalMachinePackage()
		{
			infomap.clear();
		}

		const InternalMachineInfo* InternalMachinePackage::GetInfo(Machine::class_type subclass) const
		{
			std::map<Machine::class_type,InternalMachineInfo>::const_iterator iterator
				= infomap.find(subclass);
			if(iterator != infomap.end())
			{
				return &iterator->second;
			}
			else return 0;
		}
		void InternalMachinePackage::MoveFirst()
		{
			pos = infomap.begin();
		}
		void InternalMachinePackage::MoveNext()
		{
			pos++;
		}
		const InternalMachineInfo* InternalMachinePackage::GetInfoAtPos()
		{
			if ( pos != infomap.end())
			{
				return &pos->second;
			}
			else return 0;
		}
		bool InternalMachinePackage::end() const
		{
			return (pos == infomap.end());
		}
		std::uint32_t InternalMachinePackage::size() const
		{
			return infomap.size();

		}
	}
}
