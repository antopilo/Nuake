#pragma once
#include "src/Core/Core.h"
#include "src/UI/UserInterface.h"
#include "src/Core/Maths.h"
#include "src/Core/Timestep.h"
namespace Nuake
{
	class NewEditor
	{
	public:
		NewEditor();

		void Update(Timestep ts);
		void Draw(Vector2 screenSize);

	private:
		void FetchNodes();
		Ref<UI::UserInterface> m_UserInterface;
		Ref<FrameBuffer> m_SceneFramebuffer;
		Ref<Node> m_ViewportNode;

		Ref<Node> m_FileMenu;
		Ref<Node> m_Splitter;
		Ref<Node> m_Splitter2;
		Ref<Node> m_Left;
		Ref<Node> m_Right;
		Ref<Node> m_Inspector;
		
		bool fileOpened = false;
	};
}