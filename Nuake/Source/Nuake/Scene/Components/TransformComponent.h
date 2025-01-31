#pragma once
#include "Nuake/Core/Maths.h"
#include "Nuake/Resource/Serializable.h"

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

		Matrix4 PreviousTransform;
	public:
		bool Dirty = true;
		bool GlobalDirty = true;

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
			BEGIN_SERIALIZE();
			SERIALIZE_VAL_LBL("Type", "TransformComponent");
			SERIALIZE_VEC3(Translation);
			SERIALIZE_VEC3(GlobalTranslation);
			SERIALIZE_VEC4(Rotation);
			SERIALIZE_VEC4(GlobalRotation);
			SERIALIZE_VEC3(GlobalScale);
			SERIALIZE_VEC3(Scale);
			END_SERIALIZE();
		}

		bool Deserialize(const json& j)
		{
			this->Translation = Vector3(j["Translation"]["x"], j["Translation"]["y"], j["Translation"]["z"]);
			
			if (j.contains("GlobalTranslation"))
			{
				float x = j["GlobalTranslation"]["x"];
				float y = j["GlobalTranslation"]["y"];
				float z = j["GlobalTranslation"]["z"];

				this->GlobalTranslation = Vector3(x, y, z);
			}

			if (j.contains("Rotation"))
			{
				float w = j["Rotation"]["w"];
				float x = j["Rotation"]["x"];
				float y = j["Rotation"]["y"];
				float z = j["Rotation"]["z"];

				this->Rotation = Quat(w, x, y, z);
			}

			if (j.contains("GlobalRotation"))
			{
				float w = j["GlobalRotation"]["w"];
				float x = j["GlobalRotation"]["x"];
				float y = j["GlobalRotation"]["y"];
				float z = j["GlobalRotation"]["z"];

				this->GlobalRotation = Quat(w, x, y, z);
			}

			if (j.contains("GlobalScale"))
			{
				float x = j["GlobalScale"]["x"];
				float y = j["GlobalScale"]["y"];
				float z = j["GlobalScale"]["z"];

				this->GlobalScale = Vector3(x, y, z);
			}
				
			this->Scale = Vector3(j["Scale"]["x"], j["Scale"]["y"], j["Scale"]["z"]);

			LocalTransform = Matrix4(1);
			GlobalTransform = Matrix4(1);
			this->Dirty = true;
			return true;
		}
	};
}
