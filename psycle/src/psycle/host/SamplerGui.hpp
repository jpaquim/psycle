#pragma once

#include "GeneratorGui.hpp"

namespace psycle {
	namespace host {

		class SamplerGui : public GeneratorGui {
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