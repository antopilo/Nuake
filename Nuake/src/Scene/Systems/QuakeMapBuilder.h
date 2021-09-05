#pragma once
#include <string>

struct brush;
struct brush_geometry;

namespace Nuake {
	class Entity;
	class Scene;
	class FGDBrushEntity;

	class QuakeMapBuilder
	{
	private:
		float ScaleFactor = 1.f;
		void CreateTrigger(brush* brush, brush_geometry* brush_inst,
			Scene* scene, Entity& parent,
			const std::string& target, const std::string& targetname);

		void CreateBrush(brush* brush, brush_geometry* brush_inst,
			Scene* scene, Entity& parent,
			const std::string& target, const std::string& targetname);

		void CreateFuncBrush(brush* brush, brush_geometry* brush_inst,
			Scene* scene, Entity& parent,
			const std::string& target, const std::string& targetname, FGDBrushEntity fgdBrush);

	public:
		QuakeMapBuilder() {}

		void BuildQuakeMap(Entity& ent, bool Collisions = true);
	};
}



