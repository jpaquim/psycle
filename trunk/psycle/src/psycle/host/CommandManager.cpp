#include "CommandManager.hpp"


namespace psycle {
	namespace host {

		CommandManager::CommandManager()
			: max_cmds_(30) {
		}

		CommandManager::~CommandManager() {
			while (!command_queue_.empty()) {
				CommandUndoable* cmd = command_queue_.back();
				command_queue_.pop_back();
				delete cmd;
			}
		}

		void CommandManager::InvokeCommand(Command* cmd) {
		}

		void CommandManager::InvokeCommand(CommandUndoable* cmd) {			
			command_queue_.push_back(cmd);
			if ( command_queue_.size() > max_cmds_ ) {
				CommandUndoable* cmd = command_queue_.front();
				command_queue_.pop_front();
				delete cmd;
			}
		}

		void CommandManager::Undo() {
			if (!command_queue_.empty()) {
				CommandUndoable* cmd = command_queue_.back();
				command_queue_.pop_back();
				cmd->Undo();
				delete cmd;
			}
		}

	}	// namespace host
}	// namespace psycle