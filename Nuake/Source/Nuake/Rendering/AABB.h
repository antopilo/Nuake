#pragma once
#include "Nuake/Core/Maths.h"

namespace Nuake {
	class AABB
	{
	public:
		Vector3 Min;
		Vector3 Max;

		AABB() = default;
		~AABB() = default;

		AABB(const Vector3& min, const Vector3& max)
		{
			Min = min;
			Max = max;
		}

		// Transforms the bounding box and recalculate an axis aligned box
		void Transform(Matrix4 transform)
		{
			Min = Vector3(transform * Vector4(Min, 0.0f));
			Max = Vector3(transform * Vector4(Max, 0.0f));

			if (Min.x > Max.x)
			{
				float max = Max.x;
				Max.x = Min.x;
				Min.x = max;
			}

			if (Min.y > Max.y)
			{
				float max = Max.y;
				Max.y = Min.y;
				Min.y = max;
			}

			if (Min.z > Max.z)
			{
				float max = Max.z;
				Max.z = Min.z;
				Min.z = max;
			}
		}
	};
}