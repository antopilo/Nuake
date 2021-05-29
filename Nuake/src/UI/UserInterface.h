#pragma once
#include <src/Core/Timestep.h>
#include <src/Rendering/Framebuffer.h>
#include <src/UI/Rect.h>

namespace UI
{
	class UserInterface
	{
	private:
		std::vector<Ref<Rect>> m_Rects;
		Ref<FrameBuffer> m_Framebuffer; // Texture of the interface.
		std::string m_Name;

	public:
		const int Width = 1920;
		const int Height = 1080;

		UserInterface(const std::string& name);


		static Ref<UserInterface> New(const std::string& name);
		void Calculate();
		void Draw();
		void Update(Timestep ts);

		std::vector<Ref<Rect>> GetRects()
		{
			return m_Rects;
		}

		void AddRect(Ref<Rect> rect)
		{
			m_Rects.push_back(rect);
		}

		Ref<Rect> GetRectByID(const std::string& id)
		{
			for (auto r : m_Rects)
			{
				if (r->GetID() == id)
					return r;
			}
			return nullptr;
		}

		std::vector<Ref<Rect>> GetRectsByGroup(const std::string& group)
		{
			std::vector<Ref<Rect>> rects = std::vector<Ref<Rect>>();
			for (auto r : m_Rects)
			{
				if(r->HasGroup(group))
					rects.push_back(r);
			}
			return rects;
		}
	};
}