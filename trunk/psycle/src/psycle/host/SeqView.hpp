// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007-2009 members of the psycle project http://psycle.sourceforge.net

#pragma once
#include <psycle/host/detail/project.hpp>
#include "Psycle.hpp"

namespace psycle {
namespace core {
	class SequenceEntry;
	class SequenceLine;
	class Pattern;
}
namespace host {

class CMainFrame;
class Project;

class SequencerView : public CDialogBar {
	public:
		SequencerView(CMainFrame* main_frame);
		~SequencerView();

		void SetProject(Project* project) { project_ = project; }
		Project* project() { return project_; }
		/// To be used when changing the selected entry from outside of SequencerView.
		/// Example: from playback, or via hotkey.
		void SetEntry(psycle::core::SequenceEntry* entry);
		SequenceEntry* GetEntry(int list_position);		
		SequenceEntry* selected_entry() { return selected_entry_; } 
		// this entry is currently used by the patternview
		SequenceLine & ComputeSelblockLine();
		std::map<int,SequenceEntry*>& pos_map() { return pos_map_; }
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
		void DecCurPattern();
		void IncCurPattern();
		void DecPosition();
		void IncPosition(bool bRepeat=false);

		void BuildPositionMap();
		void BuildListBox();
		void SelectItems();
		void BuildSelectionList();
		void BuildCopyList();
		void SwitchToSelBlockPlay();
		void SwitchToNormalPlay();
		bool sel_block_play() const { return selblock_play_; }

	private:
		CMainFrame* main_frame_;
		Project* project_;
		int seqcopybuffer[MAX_SONG_POSITIONS];
		int seqcopybufferlength;
		SequenceEntry* selected_entry_;
		std::map<int,SequenceEntry*> pos_map_; // Relation between the list position and the patterns			
		std::vector<psycle::core::Pattern*> copy_list_; // list to store copy/cut/paste entries
		psycle::core::SequenceLine* seq_sel_play_line_;
		psycle::core::SequenceLine* seq_main_play_line_; // mainline between normal play and bar selblock play
		bool selblock_play_;
		std::map<SequenceEntry*, int> sel_pos_map_;
		std::vector<int> selection_; // Vector of the selected indexes
	};

}}
