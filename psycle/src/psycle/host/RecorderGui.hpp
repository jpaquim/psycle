#pragma once
#include "Psycle.hpp"
#include "GeneratorGui.hpp"

namespace psycle { namespace host {

class CWaveInMacDlg;

class RecorderGui : public GeneratorGui {
	public:
		RecorderGui(MachineView* view, Machine* mac);
		~RecorderGui();

		virtual void BeforeDeleteDlg();
	protected:
		virtual void ShowDialog(double x, double y);
	private:

		class CWaveInMacDlg* dialog_;
};

}}
