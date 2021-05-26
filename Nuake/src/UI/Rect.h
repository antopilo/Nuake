#pragma once
#include <algorithm>
#include <src/Core/Core.h>
#include <src/Core/Maths.h>
#include <src/Rendering/Textures/Texture.h>
#include <src/Rendering/Shaders/Shader.h>
#include <src/Rendering/Renderer2D.h>


namespace UI
{
	// Styling props
	class RectProp
	{
	public:
		float BorderRadius = 0.0f;
		float BorderMinXWidth = 0.0f;
		float BorderMinYWidth = 0.0f;
		float BorderMaxXWidth = 0.0f;
		float BorderMaxYWidth = 0.0f;
		Color BorderColor = Color(0, 0, 0, 1);
		Color BackgroundColor = Color(1, 0, 0, 1);
		Ref<Texture> BackgroundTexture;
	};
	
	// This is the base component for every UI layout.
	class Rect
	{
	private:
		std::string id; // This is similar to html ids
		std::vector<std::string> groups; // this is similar to html classes
		
	public:
		RectProp Props;

		float MinX;
		float MinY;
		float MaxX;
		float MaxY;

		static Ref<Rect> New(float minx, float miny, float maxx, float maxy)
		{
			return CreateRef<Rect>(minx, miny, maxx, maxy);
		}

		Rect(float minx, float miny, float maxx, float maxy)
		{
			this->MinX = minx;
			this->MinY = miny;
			this->MaxX = maxx;
			this->MaxY = maxy;

			this->id = "";
			this->groups = std::vector<std::string>();
		}

		// Check if a point is inside the rectangle.
		bool IsInside(Vector2 point)
		{
			return point.x > MinX && point.x < MaxX && point.y > MinY && point.y < MaxY;
		}

		bool HasGroup(const std::string& group)
		{
			for (auto g : groups)
				if (g == group)
					return true;
			return false;
		}

		std::vector<std::string> GetGroups() { return groups; }
		void AddGroup(const std::string& group)
		{
			// Check if already has group.
			for (auto g : groups)
				if (g == group)
					return;
			groups.push_back(group);
		}

		std::string GetID() { return id; }
		void SetID(const std::string& id) { this->id = id; }

		// Cuts a rectangle left and returns a new one.
		Ref<Rect> CutLeft(float a)
		{
			float minx = this->MinX;
			this->MinX = std::min(this->MaxX, this->MinX + a);
			return CreateRef<Rect>(minx, this->MinY, this->MinX, this->MaxY);
		}

		void Draw()
		{
			Renderer2D::UIShader->SetUniform4f("background_color", Props.BackgroundColor.r,
				Props.BackgroundColor.g,
				Props.BackgroundColor.b,
				Props.BackgroundColor.a);

			// Get transform
			Matrix4 transform = Matrix4(1.0f);
			transform = glm::translate(transform, Vector3(MinX, MinY, 0));
			Vector3 scale = Vector3(MaxX - MinX, MaxY - MinY, 0);
			transform = glm::scale(transform, scale);
			Renderer2D::UIShader->SetUniformMat4f("model", transform);

			Renderer2D::DrawRect();
		}
	};
}