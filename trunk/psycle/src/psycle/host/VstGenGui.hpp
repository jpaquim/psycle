#pragma once

#include "GeneratorGui.hpp"

namespace psycle {
	namespace host {

		class VstGenGui : public GeneratorGui {
		public:
			VstGenGui(class MachineView* view,
				      class Machine* mac);
			~VstGenGui();

			virtual void BeforeDeleteDlg();

		protected:
			virtual void ShowDialog(double x, double y);
		private:
			class CVstEffectWnd* dialog_;
		};
	}
}