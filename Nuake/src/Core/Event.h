#pragma once
#include "Core.h"


namespace Nuake {

	enum class EventType 
	{
		None = 0,
		WindowClose, WindowResize, WindowFocus, WindowLostFocus, WindowMoved,
		AppUpdate, AppRender, AppEvent, AppCloseEvent
	};

	class Event
	{
		virtual EventType GetEventType() const = 0;
		virtual const std::string_view& GetName() const = 0;
		
	};
}