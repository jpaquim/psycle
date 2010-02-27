#pragma once
#include "Psycle.hpp"
#include "GeneratorGui.hpp"

namespace psycle { namespace host {

class CGearTracker;

class SamplerGui : public GeneratorGui {
	public:
		SamplerGui(MachineView* view, Machine* mac);
		~SamplerGui();

		virtual void BeforeDeleteDlg();

	protected:
		virtual void ShowDialog(double x, double y);
	private:
		class CGearTracker* dialog_;
};

}}
