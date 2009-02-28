#pragma once

#include "Constants.hpp"

namespace psycle {
	namespace host {

		class SequencerView : public CDialogBar
		{
		public:
			SequencerView(class CMainFrame* main_frame);
			~SequencerView();

			void SetProject(class Project* project);

			void OnInclen();
			void OnDeclen();
			void OnDeclong();
			void OnInclong();			
			void OnSelchangeSeqlist();
			void OnDblclkSeqlist();
			void OnIncshort();
			void OnDecshort();
			void OnSeqins();
			void OnSeqnew();
			void OnSeqcut();
			void OnSeqcopy();
			void OnSeqpaste();
			void OnSeqdelete();
			void OnSeqduplicate();
			void OnSeqclr();
			void OnSeqsort();
			void OnSeqShowpattername();
		
			void UpdateSequencer(int bottom = -1);
			void UpdatePlayOrder(bool mode);

		private:
			CMainFrame* main_frame_;
			Project* project_;
			int seqcopybuffer[MAX_SONG_POSITIONS];
			int seqcopybufferlength;
		};

	}
}
