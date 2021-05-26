#include "UserInterface.h"

namespace UI
{
	UserInterface::UserInterface(const std::string& name)
	{
		m_Name = name;
	}

	Ref<UserInterface> UserInterface::New(const std::string& name)
	{
		return CreateRef<UserInterface>(name);
	}

	void UserInterface::Draw()
	{
		Renderer2D::BeginDraw();
		for (auto r : m_Rects)
		{
			r->Draw();
		}
	}

	void UserInterface::Update(Timestep ts)
	{

	}
}
