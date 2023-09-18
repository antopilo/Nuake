#pragma once
#include "src/Core/Core.h"
#include "src/Core/Maths.h"
#include "src/Resource/Serializable.h"

namespace Nuake
{
	struct SkeletonNode
	{
		Matrix4 Transform;
		Matrix4 Offset;
		std::string Name;
		int ChildrenCount;
		std::vector<SkeletonNode> Children;
		Matrix4 FinalTransform = Matrix4(1.0f);
		int32_t Id = -1;
		int32_t EntityHandle = 0;

		json Serialize()
		{
			BEGIN_SERIALIZE();
			SERIALIZE_MAT4("Transform", Transform);
			SERIALIZE_MAT4("Offset", Offset);
			SERIALIZE_VAL(Name);
			SERIALIZE_VAL(ChildrenCount);
			SERIALIZE_VAL(Id);
			SERIALIZE_VAL(EntityHandle);

			uint32_t i = 0;
			for (auto& c : Children)
			{
				j["Children"][i] = c.Serialize();
				i++;
			}

			END_SERIALIZE();
		}

		bool Deserialize(json j)
		{
			DESERIALIZE_MAT4("Transform", Transform);
			DESERIALIZE_MAT4("Offset", Offset);
			DESERIALIZE_VAL(Name);
			DESERIALIZE_VAL(ChildrenCount);
			DESERIALIZE_VAL(Id);
			DESERIALIZE_VAL(EntityHandle);

			if (j.contains("Children"))
			{
				for (uint32_t i = 0; i < ChildrenCount; i++)
				{
					SkeletonNode newChildren;
					newChildren.Deserialize(j["Children"][i]);
					Children.push_back(std::move(newChildren));
				}
			}
			
			return true;
		}
	};
}