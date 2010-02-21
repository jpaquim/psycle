// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007-2010 members of the psycle project http://psycle.sourceforge.net

#include "SeqHelperCommand.hpp"

#include "Project.hpp"
#include "SeqView.hpp"


namespace psycle { 
	namespace host {
	
		SeqHelperCommand::SeqHelperCommand(SequencerView* seq_view) 
			: seq_view_(seq_view) {
		}

		SeqHelperCommand::~SeqHelperCommand() {
		}

		void SeqHelperCommand::PrepareUndoStorage() {
			// Undo store
			core::SequenceLine & line = **(seq_view_->project()->song().sequence().begin() + 1);
			prev_line_.clear();
			core::SequenceLine::iterator it = line.begin();
			for ( ; it != line.end(); ++it) {
				core::SequenceEntry & entry = *new core::SequenceEntry(prev_line_, it->second->pattern());
				prev_line_.insert(it->first, entry);
			}	
		}

		void SeqHelperCommand::PrepareRedoStorage() {
			// Redo store
			core::SequenceLine & line = **(seq_view_->project()->song().sequence().begin() + 1);
			next_line_.clear();
			psycle::core::SequenceLine::iterator it = line.begin();
			for ( ; it != line.end(); ++it) {
				core::SequenceEntry & entry = *new core::SequenceEntry(next_line_, it->second->pattern());
				next_line_.insert(it->first, entry);
			}
		}

		void SeqHelperCommand::Undo() {
			core::SequenceLine & line = **(seq_view_->project()->song().sequence().begin() + 1);
			line.clear();
			core::SequenceLine::iterator it = prev_line_.begin();
			for (; it != prev_line_.end(); ++it) {
				psycle::core::SequenceEntry & entry = *new core::SequenceEntry(line, it->second->pattern());
				line.insert(it->first, entry);
			}
			seq_view_->UpdateSequencer();
		}

		void SeqHelperCommand::Redo() {
			core::SequenceLine & line = **(seq_view_->project()->song().sequence().begin() + 1);
			line.clear();
			core::SequenceLine::iterator it = next_line_.begin();
			for ( ; it != next_line_.end(); ++it) {
				core::SequenceEntry & entry = *new core::SequenceEntry(line, it->second->pattern());
				line.insert(it->first, entry);
			}
			seq_view_->UpdateSequencer();
		}

	}  // namespace host
}  // namespace psycle
