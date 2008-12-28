#pragma once

#include "GeneratorGui.hpp"

namespace psycle {
	namespace host {

		class XmSamplerGui : public GeneratorGui {
		public:
			XmSamplerGui(class MachineView* view,
					     class Machine* mac);
			~XmSamplerGui();

			virtual bool OnEvent(TestCanvas::Event* ev);

			virtual void BeforeDeleteDlg();

		private:
			void ShowDialog();

			class XMSamplerUI* dialog_;
		};
	}
}