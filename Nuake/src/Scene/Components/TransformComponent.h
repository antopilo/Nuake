#pragma once
#include "src/Core/Maths.h"
#include "src/Resource/Serializable.h"

namespace Nuake
{
	class TransformComponent {
	public:
		Vector3 Translation;
		Quat Rotation;
		Vector3 Scale;

		Vector3 GlobalTranslation;
		Quat GlobalRotation;
		Vector3 GlobalScale;

		Matrix4 LocalTransform;
		Matrix4 GlobalTransform;
	public:
		bool Dirty = true;
		TransformComponent();

		Matrix4 GetGlobalTransform() const;
		void SetGlobalTransform(const Matrix4& transform);

		Matrix4 GetLocalTransform() const;
		void SetLocalTransform(const Matrix4& transform);

		void SetLocalRotation(const Quat& quat);
		Quat GetLocalRotation() const;

		void SetGlobalRotation(const Quat& quat);
		Quat GetGlobalRotation() const;

		Vector3 GetLocalPosition() const;
		void SetLocalPosition(const Vector3& position);

		Vector3 GetGlobalPosition() const;
		void SetGlobalPosition(const Vector3& position);

		void SetLocalScale(const Vector3& scale);
		Vector3 GetLocalScale() const;

		void SetGlobalScale(const Vector3& scale);
		Vector3 GetGlobalScale() const;

		json Serialize()
		{
			Rotation = glm::normalize(Rotation);
			BEGIN_SERIALIZE();
			SERIALIZE_VAL_LBL("Type", "TransformComponent");
			SERIALIZE_VEC3(Translation);
			SERIALIZE_VEC4(Rotation);
			SERIALIZE_VEC3(Scale);
			END_SERIALIZE();
		}

		bool Deserialize(std::string str)
		{
			BEGIN_DESERIALIZE();
			this->Translation = Vector3(j["Translation"]["x"], j["Translation"]["y"], j["Translation"]["z"]);
			if (j.contains("Rotation"))
			{
				float w = j["Rotation"]["w"];
				float x = j["Rotation"]["x"];
				float y = j["Rotation"]["y"];
				float z = j["Rotation"]["z"];
				this->Rotation = Quat(w, x, y, z);
			}

				
			this->Scale = Vector3(j["Scale"]["x"], j["Scale"]["y"], j["Scale"]["z"]);

			LocalTransform = Matrix4(1);
			GlobalTransform = Matrix4(1);
			Dirty = true;
			return true;
		}
	};
}
