#pragma once

enum ActionType
{
	SAVE, TRANSLATE
};

class EditorAction
{
private:

public:
	virtual void Do() = 0;
	virtual void Undo() = 0;
};