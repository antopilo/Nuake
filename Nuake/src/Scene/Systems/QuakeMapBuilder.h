#pragma once

class Entity;
class QuakeMapBuilder
{
public:
	QuakeMapBuilder(){}

	void BuildQuakeMap(Entity& ent, bool Collisions = true);
};