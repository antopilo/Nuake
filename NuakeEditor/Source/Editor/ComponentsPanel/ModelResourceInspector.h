#pragma once
#include <Nuake/Core/Core.h>
#include <Nuake/Resource/Model.h>

class ModelResourceInspector
{
private:
	Ref<Nuake::Model> _model;

public:
	ModelResourceInspector(Ref<Nuake::Model> model);
	ModelResourceInspector() = default;
	~ModelResourceInspector() = default;

	void Draw();
};