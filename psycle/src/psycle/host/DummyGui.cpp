#include "DummyGui.hpp"

using namespace psycle::core;

namespace psycle {
	namespace host {

		DummyGenGui::DummyGenGui(MachineView* view, Machine* mac)
			: GeneratorGui(view, mac) {
		}

		bool DummyGenGui::OnEvent(PsycleCanvas::Event* ev) {
			if ( ev->type == PsycleCanvas::Event::BUTTON_2PRESS)
				return true;
			return GeneratorGui::OnEvent(ev);					
		}

		DummyEffectGui::DummyEffectGui(MachineView* view, Machine* mac)
			: EffectGui(view, mac) {
		}

		bool DummyEffectGui::OnEvent(PsycleCanvas::Event* ev) {
			if ( ev->type == PsycleCanvas::Event::BUTTON_2PRESS)
				return true;
			return EffectGui::OnEvent(ev);					
		}

	}  // namespace host
}  // namespace psycle
