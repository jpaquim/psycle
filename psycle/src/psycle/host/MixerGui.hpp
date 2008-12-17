#pragma once

#include "MachineGui.hpp"

namespace psycle {
	namespace host {

		class MixerGui : public MachineGui {
		public:
			MixerGui(class MachineView* view,
				     class Machine* mac);
			~MixerGui();

			virtual bool OnEvent(TestCanvas::Event* ev);
			virtual void BeforeDeleteDlg();

		private:
			void ShowDialog();

			class CFrameMixerMachine* dialog_;
		};
	}
}