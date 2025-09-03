#pragma once

namespace NuakeEditor
{
	class ICommand
	{
	protected:
		bool mIsUndoable = false;

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

		bool IsUndoable() const { return mIsUndoable; }
	};
}