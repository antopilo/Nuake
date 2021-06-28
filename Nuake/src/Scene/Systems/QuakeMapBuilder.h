#pragma once
#include <string>
class Entity;
class Scene;
struct brush;
struct brush_geometry;
class QuakeMapBuilder
{
private:
	void CreateTrigger(brush* brush, brush_geometry* brush_inst, Scene* scene, Entity& parent, std::string target, std::string targetname);
	void CreateBrush(brush* brush, brush_geometry* brush_inst, Scene* scene, Entity& parent, std::string target, std::string targetname);
	void CreateFuncBrush(brush* brush, brush_geometry* brush_inst, Scene* scene, Entity& parent, std::string target, std::string targetname);
public:
	QuakeMapBuilder(){}

	void BuildQuakeMap(Entity& ent, bool Collisions = true);


};