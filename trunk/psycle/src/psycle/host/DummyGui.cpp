#include "DummyGui.hpp"
#include "Song.hpp"

namespace psycle {
	namespace host {

		DummyGenGui::DummyGenGui(class MachineView* view,
								 class Machine* mac)
			: GeneratorGui(view, mac)
		{
		}

		DummyGenGui::~DummyGenGui() {}

		bool DummyGenGui::OnEvent(TestCanvas::Event* ev)
		{
			if ( ev->type == TestCanvas::Event::BUTTON_2PRESS)
				return true;
			return GeneratorGui::OnEvent(ev);					
		}

		DummyEffectGui::DummyEffectGui(class MachineView* view,
									   class Machine* mac)
			: EffectGui(view, mac)
		{
		}

		DummyEffectGui::~DummyEffectGui() {}

		bool DummyEffectGui::OnEvent(TestCanvas::Event* ev)
		{
			if ( ev->type == TestCanvas::Event::BUTTON_2PRESS)
				return true;
			return EffectGui::OnEvent(ev);					
		}

	}  // namespace host
}  // namespace psycle
