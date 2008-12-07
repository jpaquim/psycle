#pragma once

#include "MachineGui.hpp"

namespace psycle {
	namespace host {

		class VstFxGui : public MachineGui {
		public:
			VstFxGui(class MachineView* view,
				     class Machine* mac);
			~VstFxGui();

			virtual bool OnEvent(TestCanvas::Event* ev);

			virtual void BeforeDeleteDlg();

		private:
			void ShowDialog();

			class CVstEffectWnd* dialog_;
		};
	}
}