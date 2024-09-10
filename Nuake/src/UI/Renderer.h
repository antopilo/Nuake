#pragma once
#include <src/Rendering/Shaders/Shader.h>
#include <src/Rendering/Buffers/VertexBuffer.h>
#include <src/Rendering/Buffers/VertexArray.h>

#include "Nodes/Node.h"
#include "Nodes/Text.h"
#include "Font/Font.h"

#include <msdf-atlas-gen/msdf-atlas-gen.h>

#include <memory>
#include <string>

namespace NuakeUI
{
	class Renderer
	{
	public:
		static Renderer& Get() {
			static Renderer instance;
			return instance;
		}

		void SetViewportSize(const Vector2& size);

		void BeginDraw();
		void DrawNode(std::shared_ptr<Node> node, int z);
		void DrawString(const std::string& string, NodeStyle& nodeStyle, std::shared_ptr<Font> font = mDefaultFont, Vector3 position = Vector3());
		void DrawChar(std::shared_ptr<Font> font, int letter);
		void DrawRect();

		void ReloadShaders();
		static std::shared_ptr<Font> mDefaultFont;
	private:
		Vector2 mSize;
		Matrix4 mView;

		Renderer();
		~Renderer() = default;

		std::shared_ptr<Shader> mShader;
		std::shared_ptr<Shader> mSDFShader;
		std::shared_ptr<VertexArray> mVertexArray;
		std::shared_ptr<VertexBuffer> mVertexBuffer;
	};
}