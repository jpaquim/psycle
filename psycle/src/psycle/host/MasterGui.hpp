#pragma once

#include "MachineGui.hpp"

namespace psycle {
	namespace host {

		class MasterGui : public MachineGui {
		public:
			MasterGui(class MachineView* view,
					  class Machine* mac);
			~MasterGui();

			virtual bool OnEvent(TestCanvas::Event* ev);

		private:
			void ShowDialog();

			class CMasterDlg* dialog_;
		};
	}
}