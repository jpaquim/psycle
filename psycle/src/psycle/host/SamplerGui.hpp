#pragma once

#include "GeneratorGui.hpp"

namespace psycle {
	namespace host {

		class SamplerGui : public GeneratorGui {
		public:
			SamplerGui(class MachineView* view,
					   class Machine* mac);
			~SamplerGui();

			virtual void BeforeDeleteDlg();

		protected:
			virtual void ShowDialog(double x, double y);
		private:
			class CGearTracker* dialog_;
		};
	}
}