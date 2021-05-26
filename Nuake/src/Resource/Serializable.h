#pragma once
#include <string>
#include <json/json.hpp>
using json = nlohmann::json;

#define BEGIN_SERIALIZE() json j;
#define SERIALIZE_VAL_LBL(lbl, v) j[lbl] = v;
#define SERIALIZE_VAL(v) j[#v] = this->v;
#define SERIALIZE_VEC3(v) \
		j[#v]["x"] = this->v.x; \
		j[#v]["y"] = this->v.y; \
		j[#v]["z"] = this->v.z;

#define SERIALIZE_VEC4(v) \
		SERIALIZE_VEC3(v) \
		j[#v]["w"] = this->v.z;

#define SERIALIZE_OBJECT(v) j[#v] = v->Serialize();
#define SERIALIZE_OBJECT_REF(v) j[#v] = v.Serialize();
#define SERIALIZE_OBJECT_REF_LBL(lbl, v) j[lbl] = v.Serialize();
#define END_SERIALIZE() return j;
#define DUMP_SERIALIZE() j.dump(4);

#define BEGIN_DESERIALIZE() json j = json::parse(str);
#define DESERIALIZE_COMPONENT(c) \
		if(j.contains(#c)) \
			AddComponent<c>().Deserialize(j[#c].dump());


class ISerializable
{
public:
	virtual json Serialize() = 0;
	virtual bool Deserialize(const std::string& str) = 0;

};