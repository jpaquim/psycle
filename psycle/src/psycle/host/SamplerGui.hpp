#pragma once

#include "MachineGui.hpp"

namespace psycle {
	namespace host {

		class SamplerGui : public MachineGui {
		public:
			SamplerGui(class MachineView* view,
					   class Machine* mac);
			~SamplerGui();

			virtual bool OnEvent(TestCanvas::Event* ev);

			virtual void BeforeDeleteDlg();

		private:
			void ShowDialog();

			class CGearTracker* dialog_;
		};
	}
}