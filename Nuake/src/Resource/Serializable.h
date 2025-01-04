#pragma once
#include <string>
#include <json/json.hpp>
using json = nlohmann::json;

#define BEGIN_SERIALIZE() json j;
#define SERIALIZE_VAL_LBL(lbl, v) j[lbl] = v;
#define SERIALIZE_VAL(v) j[#v] = this->v;
#define SERIALIZE_RES_FILE(v) \
	bool validFile = this->v.file != nullptr && this->v.file->Exist(); \
	j["validFile"#v] = validFile; \
	if (validFile) \
	{ \
		j["file"#v] = this->v.file->GetRelativePath(); \
	}

#define SERIALIZE_VEC2(v) \
		j[#v]["x"] = v.x; \
		j[#v]["y"] = v.y;

#define SERIALIZE_VEC3(v) \
		SERIALIZE_VEC2(v) \
		j[#v]["z"] = v.z;

#define SERIALIZE_VEC4(v) \
		SERIALIZE_VEC3(v) \
		j[#v]["w"] = v.w;

#define DESERIALIZE_VAL(p) \
if(j.contains(#p)) \
{ \
p = j[#p]; \
}

#define DESERIALIZE_RES_FILE(v) \
	if (j.contains("validFile"#v)) \
	{ \
		if (bool validFile = j["validFile"#v]) \
		{ \
			const std::string filePath = j["file"#v]; \
			(v).file = FileSystem::GetFile(filePath); \
		} \
	}

#define DESERIALIZE_VEC4(v, p) \
	p = Vector4(v["x"], v["y"], v["z"], v["w"]);

#define DESERIALIZE_VEC3_INTO4(v, p) \
	p = Vector4(v["x"], v["y"], v["z"], 0.0f);

#define DESERIALIZE_VEC3(v, p) \
	p = Vector3(v["x"], v["y"], v["z"]);

#define DESERIALIZE_VEC2(v, p) \
	p = Vector2(v["x"], v["y"]);

#define SERIALIZE_MAT4(lbl, m) \
{ \
	int i = 0; \
	for (int l = 0; l < 4; l++) { \
		for (int k = 0; k < 4; k++) { \
				j[lbl][i] = m[l][k]; \
				i++; \
		} \
	} \
}

#define DESERIALIZE_MAT4(lbl, m) \
{ \
	int i = 0; \
	for(int l = 0; l < 4; l++) { \
		for(int k = 0; k < 4; k++) { \
			m[l][k] = j[lbl][i];\
			i++; \
		} \
	} \
}

#define SERIALIZE_OBJECT(v)				 j[#v] = v->Serialize();
#define SERIALIZE_OBJECT_REF(v)			 j[#v] = v.Serialize();
#define SERIALIZE_OBJECT_REF_LBL(lbl, v) j[lbl] = v.Serialize();
#define END_SERIALIZE() return j;
#define DUMP_SERIALIZE() j.dump(4);

#define BEGIN_DESERIALIZE() json j = json::parse(str);
#define DESERIALIZE_COMPONENT(c) \
		if(j.contains(#c)) \
			AddComponent<c>().Deserialize(j[#c]);
#define POSTDESERIALIZE_COMPONENT(c) \
		if(HasComponent<c>()) \
			GetComponent<c>().PostDeserialize(*m_Scene);

class ISerializable
{
public:
	virtual json Serialize() = 0;
	virtual bool Deserialize(const json& j) = 0;
};
