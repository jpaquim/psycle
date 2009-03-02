#pragma once

#include "EffectGui.hpp"
#include "GeneratorGui.hpp"

namespace psycle {
	namespace host {

		class DummyGenGui : public GeneratorGui {
		public:
			DummyGenGui(class MachineView* view,
#ifdef use_psycore							 
  				        class psy::core::Machine* mac);
#else
						class Machine* mac);
#endif

			~DummyGenGui();

			virtual bool OnEvent(PsycleCanvas::Event* ev);
		};

		class DummyEffectGui : public EffectGui {
		public:
			DummyEffectGui(class MachineView* view,
#ifdef use_psycore							 
  	  			           class psy::core::Machine* mac);
#else
						   class Machine* mac);
#endif
			~DummyEffectGui();

			virtual bool OnEvent(PsycleCanvas::Event* ev);
		};
	}
}