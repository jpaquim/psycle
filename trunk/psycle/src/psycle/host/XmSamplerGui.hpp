#pragma once

#include "MachineGui.hpp"

namespace psycle {
	namespace host {

		class XmSamplerGui : public MachineGui {
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