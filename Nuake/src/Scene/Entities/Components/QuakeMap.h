#pragma once
#include <string>
#include <vector>
#include "../Rendering/Mesh/Mesh.h"
#include "../Resource/TrenchbroomMap.h"
class __declspec(dllexport) QuakeMap  {
private:
	

public:
	std::vector<Ref<Mesh>> m_Meshes;
	Ref<TrenchbroomMap> Map;
	std::string Path;
	bool HasCollisions = false;
	void Load(std::string path, bool collisions);
	void Build();
	void Rebuild();
	void Draw();
	void DrawEditor();
	
};