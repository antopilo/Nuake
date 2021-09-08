#pragma once
#include <src/Core/Timestep.h>
#include <src/Rendering/Buffers/Framebuffer.h>
#include <src/UI/Nodes/Rect.h>
#include "yoga/Yoga.h"
#include "Nodes/Node.h"
#include "Nodes/Canvas.h"
#include "src/Core/Maths.h"
#include "Styling/Stylesheet.h"
#include "Font/Font.h"
#include <vector>
#include <string>

namespace Nuake {
	namespace UI {
		class UserInterface
		{
		private:
			std::string m_Name;
			std::string m_Path;

			Ref<FrameBuffer> m_Framebuffer; // Texture of the interface.

			Ref<Canvas> Root;

			YGConfigRef yoga_config;
			YGNodeRef yoga_root;
			
			std::vector<std::string> m_Scripts;

			Ref<Node> GetNodeByIDRecurse(Ref<Node> node, const std::string& id);
			void RecursiveHover(Ref<Node> node, Vector2 pos);
		public:
			Ref<Font> font;
			const int Width = 1920;
			const int Height = 1080;

			Vector2 Size = Vector2();

			UserInterface(const std::string& name, const std::string& path);
			~UserInterface();

			void Reload();

			static Ref<UserInterface> New(const std::string& name, const std::string& path);
			void Calculate(int available_width, int available_height);

			void CreateYogaLayout();
			void CreateYogaLayoutRecursive(Ref<Node> node, YGNodeRef yoga_node);
			void Draw(Vector2 size);
			void DrawRecursive(Ref<Node> node, float z);
			void Update(Timestep ts);

			void RecursiveStyle(Ref<Node> node);
			void RecursiveMouseClick(Ref<Node> node, Vector2 pos);

			Ref<Node> GetNodeByID(const std::string& id);

			void ConsumeMouseClick(Vector2 pos);

			std::string GetPath() const { return m_Path; }
		};
	}
}
