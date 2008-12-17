#pragma once

#include "GeneratorGui.hpp"

namespace psycle {
	namespace host {

		class VstGenGui : public GeneratorGui {
		public:
			VstGenGui(class MachineView* view,
				      class Machine* mac);
			~VstGenGui();

			virtual bool OnEvent(TestCanvas::Event* ev);

			virtual void BeforeDeleteDlg();

		private:
			void ShowDialog();
			class CVstEffectWnd* dialog_;
		};
	}
}