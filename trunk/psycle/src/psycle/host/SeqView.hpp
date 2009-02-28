#pragma once

namespace psycle {
	namespace host {

		class SequencerView : public CDialogBar
		{
		public:
			SequencerView(class CMainFrame* main_frame);
			~SequencerView();

			void SetProject(class Project* project);

		protected:
			afx_msg void OnInclen();
			afx_msg void OnDeclen();
			DECLARE_MESSAGE_MAP();

		private:
			void UpdateSequencer(int bottom = -1);
			void UpdatePlayOrder(bool mode);

			CMainFrame* main_frame_;
			Project* project_;
		};

	}
}

