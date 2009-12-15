#include "Command.hpp"

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

			std::deque<CommandUndoable*>::size_type Size() const {
				return command_queue_.size();
			}
    
    		private:
				void InvokeCommand(Command* cmd);
				void InvokeCommand(CommandUndoable* cmd);

				std::deque<CommandUndoable*> command_queue_;
				int max_cmds_;
		};


	}	// namespace host
}	// namespace psycle