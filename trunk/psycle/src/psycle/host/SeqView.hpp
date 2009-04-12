#pragma once

#include "Psycle.hpp"
#if PSYCLE__CONFIGURATION__USE_PSYCORE
namespace psy {
	namespace core {
	class SequenceEntry;
	}
}
#endif
namespace psycle {
	namespace host {

		class CMainFrame;
		class Project;

		class SequencerView : public CDialogBar
		{
		public:
			SequencerView(CMainFrame* main_frame);
			~SequencerView();

			void SetProject(Project* project);
			/// To be used when changing the selected entry from outside of SequencerView.
			/// Example: from playback, or via hotkey.
#if PSYCLE__CONFIGURATION__USE_PSYCORE
			void SetSelectedEntry(SequenceEntry* entry);
			SequenceEntry* selectedEntry();
#else
			void SetSelectedEntry(int entry);
			int selecteEntry();
#endif

			/// Change pattern under current position
			void OnDeclong();
			void OnInclong();			
			void OnIncshort();
			void OnDecshort();
			/// change sequence length (Synonim of adding or deleting at the end)
			void OnInclen();
			void OnDeclen();

			void OnSelchangeSeqlist();
			void OnDblclkSeqlist();
			// Add the selected one again.
			void OnSeqins();
			// Add new, after selection
			void OnSeqnew();
			// Add cloned from selection, after selection
			void OnSeqduplicate();

			void OnSeqcut();
			void OnSeqcopy();
			void OnSeqpaste();
			void OnSeqdelete();

			/// Remove the whole sequence. Does not delete the patterns, just the sequence.
			void OnSeqclr();
			/// Changes the indexes of the patterns, so that the sequence indexes become ordered.
			void OnSeqsort();
			void OnSeqShowpattername();
			/// Update the contents of the sequence list.
			void UpdateSequencer(int selectedpos = -1);
			/// Should be just Update selected position, but does some more things.
			/// mode = false sets the selection to playOrderSel.
			/// mode = true sets selection to editPosition and refreshes the name of that position.
			void UpdatePlayOrder(bool mode);

		private:
			CMainFrame* main_frame_;
			Project* project_;
			int seqcopybuffer[MAX_SONG_POSITIONS];
			int seqcopybufferlength;

#if PSYCLE__CONFIGURATION__USE_PSYCORE
			SequenceEntry* selectedEntry_;
			std::map<int,SequenceEntry*> id_map_; // Relation between the list position and the patterns
#else
			int selectedEntry_;
			std::map<int,int> id_map_; // Relation between the list position and the patterns
#endif
			std::vector<int> selection_; // Vector of the selected indexes
		};

	}
}
