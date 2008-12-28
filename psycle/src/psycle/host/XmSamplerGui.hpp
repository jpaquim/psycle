#pragma once

#include "GeneratorGui.hpp"

namespace psycle {
	namespace host {

		class XmSamplerGui : public GeneratorGui {
		public:
			XmSamplerGui(class MachineView* view,
					     class Machine* mac);
			~XmSamplerGui();

			virtual void BeforeDeleteDlg();

		protected:
			virtual void ShowDialog(double x, double y);
		private:

			class XMSamplerUI* dialog_;
		};
	}
}