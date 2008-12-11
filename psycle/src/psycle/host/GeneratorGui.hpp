#pragma once

#include "MachineGui.hpp"

namespace psycle {
	namespace host {

		class GeneratorGui : public MachineGui {
		public:
			GeneratorGui(class MachineView* view,
					  class Machine* mac);
			~GeneratorGui();

			virtual bool OnEvent(TestCanvas::Event* ev);

			virtual void BeforeDeleteDlg();

			virtual void SetSelected(bool on);
			virtual bool IsSelected() const;

		private:
			void ShowDialog();
			class CFrameMachine* dialog_;
			TestCanvas::Line sel_line_left_top_1;
			TestCanvas::Line sel_line_left_top_2;
			TestCanvas::Line sel_line_right_top_1;
			TestCanvas::Line sel_line_right_top_2;
			TestCanvas::Line sel_line_left_bottom_1;
			TestCanvas::Line sel_line_left_bottom_2;
			TestCanvas::Line sel_line_right_bottom_1;
			TestCanvas::Line sel_line_right_bottom_2;
		};
	}
}