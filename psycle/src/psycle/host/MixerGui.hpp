#pragma once

#include "EffectGui.hpp"

namespace psycle {
	namespace host {

		class MixerGui : public EffectGui {
		public:
			MixerGui(class MachineView* view,
				     class Machine* mac);
			~MixerGui();

			virtual void BeforeDeleteDlg();
			void UpdateVU(CDC* devc);

		protected:
			virtual void ShowDialog(double x, double y);
		private:

			class CFrameMixerMachine* dialog_;
		};
	}
}