#pragma once

#include "GeneratorGui.hpp"

namespace psycle {
	namespace host {

		class RecorderGui : public GeneratorGui {
		public:
			RecorderGui(class MachineView* view,
					    class Machine* mac);
			~RecorderGui();

			virtual bool OnEvent(TestCanvas::Event* ev);

			virtual void BeforeDeleteDlg();

		private:
			void ShowDialog(double x, double y);

			class CWaveInMacDlg* dialog_;
		};
	}
}