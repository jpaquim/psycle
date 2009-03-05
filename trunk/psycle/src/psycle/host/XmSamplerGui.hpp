#pragma once

#include "GeneratorGui.hpp"

namespace psycle {
	namespace host {

		class XmSamplerGui : public GeneratorGui {
		public:
			XmSamplerGui(class MachineView* view,
#ifdef use_psycore
						 class psy::core::Machine* mac);
#else
					     class Machine* mac);
#endif
			~XmSamplerGui();

			virtual void BeforeDeleteDlg();
			virtual void UpdateVU(CDC* devc);


		protected:
			virtual void ShowDialog(double x, double y);
		private:

			class XMSamplerUI* dialog_;
		};
	}
}