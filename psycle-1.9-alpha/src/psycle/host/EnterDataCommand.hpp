#pragma once
#include <psycle/host/detail/project.hpp>
#include "Command.hpp"
#include <psycle/core/pattern.h>

namespace psycle { namespace host {

class EnterDataCommand : public CommandUndoable {
	public:
		EnterDataCommand(class PatternView* pat_view, unsigned int n_char, unsigned int n_flags);

		virtual void Execute();
		virtual void Undo();
		virtual void Redo();

	private:
		unsigned int n_char_;
		unsigned int n_flags_;
		PatternView* pat_view_;
		PatternEvent prev_ev_;
		double prev_pos_;
		int prev_track_;
		bool prev_has_ev_;
		PatternEvent next_ev_;
		double next_pos_;
		int next_track_;
		bool next_has_ev_;
};

}}
