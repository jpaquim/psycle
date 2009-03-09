#include "DummyGui.hpp"
#include "Song.hpp"

#ifdef use_psycore
#include <psycle/core/machine.h>
#endif

namespace psycle {
	namespace host {

		DummyGenGui::DummyGenGui(class MachineView* view,
#ifdef use_psycore							 
			 				     psy::core::Machine* mac)
#else
					             class Machine* mac)
#endif
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

		DummyEffectGui::DummyEffectGui(class MachineView* view,
#ifdef use_psycore							 
						     class psy::core::Machine* mac)
#else
							 class Machine* mac)
#endif

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
