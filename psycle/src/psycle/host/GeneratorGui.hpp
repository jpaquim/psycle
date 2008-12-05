#pragma once

#include "MachineGui.hpp"

namespace psycle {
	namespace host {

		class GeneratorGui : public MachineGui {
		public:
			GeneratorGui(class MachineView* view,
					  class Machine* mac);
			~GeneratorGui();

			virtual bool OnEvent(TestCanvas::Event* ev);

		private:
			void ShowDialog();

			class CMasterDlg* dialog_;
		};
	}
}