#pragma once
#include "Node.h"
#include "../InputManager.h"

#include <string>

namespace NuakeUI
{
	class Button;
	typedef std::shared_ptr<Button> ButtonPtr;
	class Button : public Node
	{
	private:
		bool mHasBeenClicked = false;
		bool mHasCallback = false;
	public:
		std::string Label = "";

		Button(const std::string& name, const std::string& label);
		~Button() {};

		static ButtonPtr New(const std::string& name, const std::string& label);

		std::function<void(Button&)> ClickCallback;

		void UpdateInput(InputManager* inputManager) override;

		void SetClickCallback(std::function<void(Button&)> callback)
		{
			mHasCallback = true;
			ClickCallback = callback;
		}
	};
}