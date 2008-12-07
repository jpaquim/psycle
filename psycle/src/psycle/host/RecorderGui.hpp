#pragma once

#include "MachineGui.hpp"

namespace psycle {
	namespace host {

		class RecorderGui : public MachineGui {
		public:
			RecorderGui(class MachineView* view,
					    class Machine* mac);
			~RecorderGui();

			virtual bool OnEvent(TestCanvas::Event* ev);

			virtual void BeforeDeleteDlg();

		private:
			void ShowDialog();

			class CWaveInMacDlg* dialog_;
		};
	}
}