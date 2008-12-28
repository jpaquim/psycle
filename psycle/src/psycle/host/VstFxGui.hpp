#pragma once

#include "EffectGui.hpp"

namespace psycle {
	namespace host {

		class VstFxGui : public EffectGui {
		public:
			VstFxGui(class MachineView* view,
				     class Machine* mac);
			~VstFxGui();

			virtual void BeforeDeleteDlg();

		protected:
			virtual void ShowDialog(double x, double y);
		private:

			class CVstEffectWnd* dialog_;
		};
	}
}