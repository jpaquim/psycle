#pragma once

#include "GeneratorGui.hpp"

namespace psycle {
	namespace host {

		class RecorderGui : public GeneratorGui {
		public:
			RecorderGui(class MachineView* view,
#ifdef use_psycore
			            class psy::core::Machine* mac);
#else
				        class Machine* mac);
#endif
			~RecorderGui();

			virtual void BeforeDeleteDlg();
		protected:
			virtual void ShowDialog(double x, double y);
		private:

			class CWaveInMacDlg* dialog_;
		};
	}
}