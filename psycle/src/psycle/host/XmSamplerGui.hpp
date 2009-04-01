#pragma once
#include "Psycle.hpp"
#include "GeneratorGui.hpp"

namespace psycle {
	namespace host {

		class XMSamplerUI;

		class XmSamplerGui : public GeneratorGui {
		public:
			XmSamplerGui(MachineView* view, Machine* mac);
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
