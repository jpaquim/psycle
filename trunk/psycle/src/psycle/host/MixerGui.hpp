#pragma once
#include "Psycle.hpp"
#include "EffectGui.hpp"

namespace psycle {
	namespace host {
		
		class CFrameMixerMachine;

		class MixerGui : public EffectGui {
		public:
			MixerGui(class MachineView* view, Machine* mac);
			~MixerGui();

			virtual void BeforeDeleteDlg();
			void UpdateVU(CDC* devc);

		protected:
			virtual void ShowDialog(double x, double y);
		private:

			class CFrameMixerMachine* dialog_;
		};

	}	// namespace host
}	// namespace psycle
