#pragma once
#include <psycle/host/detail/project.hpp>
#include "PatHelperCommand.hpp"

namespace psycle { namespace host {

class PatPasteCommand : public PatHelperCommand {
	public:
		PatPasteCommand(class PatternView* pat_view, bool mix = false);
		virtual void Execute();
	private:
		bool mix_;
};

}}
