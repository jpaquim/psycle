#pragma once
#include "Psycle.hpp"
#include "EffectGui.hpp"

namespace psycle {
	namespace host {

		class CVstEffectWnd;

		class VstFxGui : public EffectGui {
		public:
			VstFxGui(MachineView* view, Machine* mac);
			~VstFxGui();

			virtual void BeforeDeleteDlg();

		protected:
			virtual void ShowDialog(double x, double y);

		private:
			class CVstEffectWnd* dialog_;
		};
	}
}
