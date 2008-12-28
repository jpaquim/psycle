#pragma once

#include "EffectGui.hpp"

namespace psycle {
	namespace host {

		class MixerGui : public EffectGui {
		public:
			MixerGui(class MachineView* view,
				     class Machine* mac);
			~MixerGui();

			virtual bool OnEvent(TestCanvas::Event* ev);
			virtual void BeforeDeleteDlg();

		private:
			void ShowDialog(double x, double y);

			class CFrameMixerMachine* dialog_;
		};
	}
}