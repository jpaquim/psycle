#include <psycle/host/detail/project.private.hpp>
#include "DummyGui.hpp"

namespace psycle { namespace host {

DummyGenGui::DummyGenGui(MachineView* view, Machine* mac)
: GeneratorGui(view, mac) {}

bool DummyGenGui::OnEvent(canvas::Event* ev) {
	if ( ev->type == canvas::Event::BUTTON_2PRESS)
		return true;
	return GeneratorGui::OnEvent(ev);					
}

DummyEffectGui::DummyEffectGui(MachineView* view, Machine* mac)
	: EffectGui(view, mac) {
}

bool DummyEffectGui::OnEvent(canvas::Event* ev) {
	if ( ev->type == canvas::Event::BUTTON_2PRESS)
		return true;
	return EffectGui::OnEvent(ev);					
}

}}