#include "DummyGui.hpp"

#ifdef use_psycore
namespace psy {
	namespace core {
		class Machine;
	}
}
using namespace psy::core;
#endif

namespace psycle {
	namespace host {

		class MachineView;

#ifndef use_psycore
		class Machine;
#endif

		DummyGenGui::DummyGenGui(MachineView* view, Machine* mac)
			: GeneratorGui(view, mac)
		{
		}

		DummyGenGui::~DummyGenGui() {}

		bool DummyGenGui::OnEvent(PsycleCanvas::Event* ev)
		{
			if ( ev->type == PsycleCanvas::Event::BUTTON_2PRESS)
				return true;
			return GeneratorGui::OnEvent(ev);					
		}

		DummyEffectGui::DummyEffectGui(MachineView* view, Machine* mac)
			: EffectGui(view, mac)
		{
		}

		DummyEffectGui::~DummyEffectGui() {}

		bool DummyEffectGui::OnEvent(PsycleCanvas::Event* ev)
		{
			if ( ev->type == PsycleCanvas::Event::BUTTON_2PRESS)
				return true;
			return EffectGui::OnEvent(ev);					
		}

	}  // namespace host
}  // namespace psycle

