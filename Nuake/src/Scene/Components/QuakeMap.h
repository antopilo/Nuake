#pragma once
#include <string>
#include <vector>
#include "../Rendering/Mesh/Mesh.h"
#include "../Resource/TrenchbroomMap.h"
#include "../Resource/Serializable.h"

class QuakeMapComponent  {
private:
	
public:
	std::vector<Ref<Mesh>> m_Meshes;
	Ref<TrenchbroomMap> Map;
	std::string Path;
	bool HasCollisions = false;
	void Load(std::string path, bool collisions);

	void Draw();
	void DrawEditor();
	
	json Serialize()
	{
		BEGIN_SERIALIZE();
		SERIALIZE_VAL(HasCollisions);
		SERIALIZE_VAL(Path);
		END_SERIALIZE();
	}

	bool Deserialize(std::string str)
	{
		BEGIN_DESERIALIZE();
		this->Path = j["Path"];
		this->HasCollisions = j["HasCollisions"];
		return true;
	}
};