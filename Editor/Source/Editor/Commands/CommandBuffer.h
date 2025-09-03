#pragma once
#include "ICommand.h"

#include <vector>

namespace NuakeEditor
{
	class CommandBuffer
	{
	private:
		std::vector<ICommand> mCommands;

	public:
		CommandBuffer() : mCommands(std::vector<ICommand>()) {}

		~CommandBuffer() = default;

		void PushCommand(ICommand& command)
		{
			command.Execute();

			mCommands.push_back(command);
		}

		void PopCommand()
		{

		}
	};
}