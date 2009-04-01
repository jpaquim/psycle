#pragma once
#include "Psycle.hpp"
#include "EffectGui.hpp"
#include "GeneratorGui.hpp"

namespace psycle {
	namespace host {

		class DummyGenGui : public GeneratorGui {
		public:
			DummyGenGui(MachineView* view, Machine* mac);
			~DummyGenGui();

			virtual bool OnEvent(PsycleCanvas::Event* ev);
		};

		class DummyEffectGui : public EffectGui {
		public:
			DummyEffectGui(MachineView* view, Machine* mac);
			~DummyEffectGui();

			virtual bool OnEvent(PsycleCanvas::Event* ev);
		};
	}
}
