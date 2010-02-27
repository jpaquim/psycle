#pragma once
#include "Psycle.hpp"
#include "EffectGui.hpp"
#include "GeneratorGui.hpp"

namespace psycle { namespace host {

class DummyGenGui : public GeneratorGui {
	public:
		DummyGenGui(MachineView* view, psycle::core::Machine* mac);
		virtual bool OnEvent(canvas::Event* ev);
};

class DummyEffectGui : public EffectGui {
	public:
		DummyEffectGui(MachineView* view, psycle::core::Machine* mac);
		virtual bool OnEvent(canvas::Event* ev);
};

}}
