#pragma once
#include <glm\ext\vector_float3.hpp>
#include <glm\ext\vector_float2.hpp>
#include "TransformComponent.h"
#include "../Rendering/Camera.h"
#include "../../../Rendering/Framebuffer.h"
#include "BaseComponent.h"
#include "../Resource/Serializable.h"

enum LightType {
	Directional, Point, Spot
};

class LightComponent  {
	
public:
	glm::vec2 yes = glm::vec2(2, 2);
	
	LightType Type = Point;
	glm::vec3 Direction = glm::vec3(0, -1, 0);
	glm::vec3 Color;
	bool IsVolumetric = false;
	float Strength;
	bool SyncDirectionWithSky = false;
	Ref<FrameBuffer> m_Framebuffer;

	bool CastShadows = true;
	float Attenuation = 0.0f;
	float LinearAttenuation = 0.0f;
	float QuadraticAttenuation = 0.0f;

	LightComponent();

	glm::mat4 GetProjection();

	glm::mat4 GetLightTransform();
	void SetDirection(glm::vec3 dir);
	glm::vec3 GetDirection();

	void BeginDrawShadow();

	void EndDrawShadow();
	void DrawShadow();

	void Draw(TransformComponent transformComponent, Ref<Camera> cam);
	void DrawDeferred(TransformComponent transformComponent, Camera* cam);
	void DrawEditor();

	void SetType(LightType type);


	json Serialize()
	{
		BEGIN_SERIALIZE();
		SERIALIZE_VAL(Type);
		SERIALIZE_VEC3(Direction);
		SERIALIZE_VEC3(Color);
		SERIALIZE_VAL(IsVolumetric);
		SERIALIZE_VAL(Strength);
		SERIALIZE_VAL(SyncDirectionWithSky);
		SERIALIZE_VAL(CastShadows);
		END_SERIALIZE();
	}

	bool Deserialize(std::string str)
	{
		BEGIN_DESERIALIZE();
		if (j.contains("Type"))
			Type = (LightType)j["Type"];
		if (j.contains("IsVolumetric"))
			IsVolumetric = j["IsVolumetric"];
		if (j.contains("Strength"))
			Strength = j["Strength"];
		if (j.contains("SyncDirectionWithSky"))
			SyncDirectionWithSky = j["SyncDirectionWithSky"];
		if (j.contains("CastShadows"))
			SyncDirectionWithSky = j["CastShadows"];
		if (j.contains("Direction"))
		{
			float x = j["Direction"]["x"];
			float y = j["Direction"]["y"];
			float z = j["Direction"]["z"];
			this->Direction = Vector3(x, y, z);
		}

		return true;
	}
};