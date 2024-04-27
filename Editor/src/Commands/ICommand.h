#pragma once

namespace NuakeEditor
{
	class ICommand
	{
	public:
		virtual void Execute() = 0;
		virtual void Undo() = 0;
	};
}