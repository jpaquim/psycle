#pragma once

#include "MachineGui.hpp"

namespace psycle {
	namespace host {

		class EffectGui : public MachineGui {
		public:
			EffectGui(class MachineView* view,
					  class Machine* mac);
			~EffectGui();

			virtual bool OnEvent(TestCanvas::Event* ev);

			virtual void BeforeDeleteDlg();

		private:
			void ShowDialog();

			class CFrameMachine* dialog_;
		};
	}
}