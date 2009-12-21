#include "SeqPasteCommand.hpp"
#include "SeqView.hpp"
#include "Project.hpp"

namespace psycle { namespace host {		

SeqPasteCommand::SeqPasteCommand(SequencerView* seq_view) 
: seq_view_(seq_view) {
}

SeqPasteCommand::~SeqPasteCommand() {
}

void SeqPasteCommand::Execute() {
	// Undo storage
	#if PSYCLE__CONFIGURATION__USE_PSYCORE
		psycle::core::SequenceLine* line = (*seq_view_->project()->song().patternSequence().begin());
		prev_line_.clear();
		psycle::core::SequenceLine::iterator it = line->begin();
		for ( ; it != line->end(); ++it) {
			psycle::core::SequenceEntry* entry = new psycle::core::SequenceEntry(&prev_line_);
			entry->setPattern(it->second->pattern());
			prev_line_.insert(it->first, entry);
		}
	#endif
	// Execute Command
	seq_view_->OnSeqdelete();
	#if PSYCLE__CONFIGURATION__USE_PSYCORE
		// Redo storage
		line = (*seq_view_->project()->song().patternSequence().begin());
		next_line_.clear();
		it = line->begin();
		for ( ; it != line->end(); ++it) {
			psycle::core::SequenceEntry* entry = new psycle::core::SequenceEntry(&next_line_);
			entry->setPattern(it->second->pattern());
			next_line_.insert(it->first, entry);
		}
	#endif
}

void SeqPasteCommand::Undo() {
	#if PSYCLE__CONFIGURATION__USE_PSYCORE
		psycle::core::SequenceLine* line = (*seq_view_->project()->song().patternSequence().begin());
		line->clear();
		psycle::core::SequenceLine::iterator it = prev_line_.begin();
		for ( ; it != prev_line_.end(); ++it) {
			psycle::core::SequenceEntry* entry = new psycle::core::SequenceEntry(line);
			entry->setPattern(it->second->pattern());
			line->insert(it->first, entry);
		}
		seq_view_->UpdateSequencer();
	#endif
}

void SeqPasteCommand::Redo() {
	#if PSYCLE__CONFIGURATION__USE_PSYCORE
		psycle::core::SequenceLine* line = (*seq_view_->project()->song().patternSequence().begin());
		line->clear();
		psycle::core::SequenceLine::iterator it = next_line_.begin();
		for ( ; it != next_line_.end(); ++it) {
			psycle::core::SequenceEntry* entry = new psycle::core::SequenceEntry(line);
			entry->setPattern(it->second->pattern());
			line->insert(it->first, entry);
		}
		seq_view_->UpdateSequencer();
	#endif
}

}}
