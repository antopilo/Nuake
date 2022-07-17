#pragma once
#include <src/Core/Core.h>
#include <src/Resource/Model.h>

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