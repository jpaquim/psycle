#pragma once

#include "GeneratorGui.hpp"

namespace psycle {
	namespace host {

		class VstGenGui : public GeneratorGui {
		public:
			VstGenGui(class MachineView* view,
#ifdef use_psycore
					  class psy::core::Machine* mac);
#else
					  class Machine* mac);
#endif
			~VstGenGui();

			virtual void BeforeDeleteDlg();

		protected:
			virtual void ShowDialog(double x, double y);
		private:
			class CVstEffectWnd* dialog_;
		};
	}
}