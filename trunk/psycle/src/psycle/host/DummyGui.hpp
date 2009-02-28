#pragma once

#include "EffectGui.hpp"
#include "GeneratorGui.hpp"

namespace psycle {
	namespace host {

		class DummyGenGui : public GeneratorGui {
		public:
			DummyGenGui(class MachineView* view,
						class Machine* mac);
			~DummyGenGui();

			virtual bool OnEvent(PsycleCanvas::Event* ev);
		};

		class DummyEffectGui : public EffectGui {
		public:
			DummyEffectGui(class MachineView* view,
						class Machine* mac);
			~DummyEffectGui();

			virtual bool OnEvent(PsycleCanvas::Event* ev);
		};
	}
}