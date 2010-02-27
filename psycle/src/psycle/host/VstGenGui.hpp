#pragma once
#include "Psycle.hpp"
#include "GeneratorGui.hpp"

namespace psycle { namespace host {

class CVstEffectWnd;

class VstGenGui : public GeneratorGui {
	public:
		VstGenGui(MachineView* view, Machine* mac);
		~VstGenGui();

		virtual void BeforeDeleteDlg();

	protected:
		virtual void ShowDialog(double x, double y);
	private:
		class CVstEffectWnd* dialog_;
};

}}
