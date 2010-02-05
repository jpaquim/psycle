// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007-2010 members of the psycle project http://psycle.sourceforge.net

#include "Command.hpp"

// a class for multi-level undo/redo

namespace psycle {
	namespace host {

		class CommandManager {
		public:
			CommandManager();
			~CommandManager();

			template<typename T>
			void ExecuteCommand(T* cmd) {
				cmd->Execute();
				InvokeCommand(cmd);
			}

			void Undo();
			void Redo();
			void Clear();

			std::deque<CommandUndoable*>::size_type UndoSize() const {
				return undo_queue_.size();
			}

			std::deque<CommandUndoable*>::size_type RedoSize() const {
				return redo_queue_.size();
			}
    
    		private:
				void InvokeCommand(Command* cmd);
				void InvokeCommand(CommandUndoable* cmd);

				std::deque<CommandUndoable*> undo_queue_;
				std::deque<CommandUndoable*> redo_queue_;
				int max_undo_;
				int max_redo_;
		};


	}	// namespace host
}	// namespace psycle
