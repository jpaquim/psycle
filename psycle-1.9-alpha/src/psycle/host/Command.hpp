#pragma once
#include <psycle/host/detail/project.hpp>

namespace psycle { namespace host {

class Command {
	public:
		Command() {}
		virtual ~Command() {}

		virtual void Execute() = 0;
};

class CommandUndoable  {
	public:
		CommandUndoable() {}
		virtual ~CommandUndoable() {}

		virtual void Execute() = 0;
		virtual void Undo() {}
		virtual void Redo() {}
};

}}
