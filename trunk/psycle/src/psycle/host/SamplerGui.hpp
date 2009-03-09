#pragma once

#include "GeneratorGui.hpp"

namespace psycle {
	namespace host {

		class SamplerGui : public GeneratorGui {
		public:
			SamplerGui(class MachineView* view,
#ifdef use_psycore
					   class psy::core::Machine* mac);
#else
					   class Machine* mac);
#endif
			~SamplerGui();

			virtual void BeforeDeleteDlg();

		protected:
			virtual void ShowDialog(double x, double y);
		private:
			class CGearTracker* dialog_;
		};
	}
}