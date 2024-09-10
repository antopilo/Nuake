#include "Button.h"
#include "Text.h"

namespace NuakeUI
{
	std::shared_ptr<Button> Button::New(const std::string& name, const std::string& label)
	{
		return std::make_shared<Button>(name, label);
	}

	Button::Button(const std::string& name, const std::string& label) :
		Label(label)
	{
		ID = name;
		//InsertChild(Text::New(ID + "lbl", Label));
	}

	void Button::UpdateInput(InputManager* inputManager)
	{
		float mx = inputManager->GetMouseX();
		float my = inputManager->GetMouseY();
		bool isHover = IsMouseHover(mx, my);
		if (isHover)
		{
			bool isMouseDown = inputManager->IsMouseInputDown();
			if (isMouseDown)
			{
				State = NodeState::Pressed;
			}
			else
			{
				if (State == NodeState::Pressed && !isMouseDown)
				{
					// Calling the click callback.
					if (mHasCallback)
						ClickCallback(*this);
				}
				State = NodeState::Hover;
			}
		}
		else
		{
			State = NodeState::Idle;
		}

		for (auto& c : Childrens)
			c->UpdateInput(inputManager);
	}
}