#pragma once

class SelectableElement
{
	virtual void Draw();
};

class SceneFile : public SelectableElement
{
	void Draw() override;
};

class ProjectFile : public SelectableElement
{
	void Draw() override;
};

class MaterialFile : public SelectableElement
{
	void Draw() override;
};

class Entity : public SelectableElement
{
	void Draw() override;
};

class PropretiesWindow
{
	SelectableElement selected;

	static void Draw();
};
