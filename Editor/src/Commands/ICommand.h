#pragma once

namespace NuakeEditor
{
	class ICommand
	{
	public:
		ICommand() = default;

		virtual bool Execute()
		{
			return false;
		}

		virtual bool Undo() 
		{
			return false;
		}
	};
}