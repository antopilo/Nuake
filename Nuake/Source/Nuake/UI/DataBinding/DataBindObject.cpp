#include "DataBindObject.h"

namespace NuakeUI
{
	DataBindObject::DataBindObject(const std::string& name, int* value)
		: Name(name), Data(value)
	{
		Type = DataBindType::Int;
	}

	DataBindObject::DataBindObject(const std::string& name, float* value)
		: Name(name), Data(value)
	{
		Type = DataBindType::Float;
	}

	DataBindObject::DataBindObject(const std::string& name, bool* value)
		: Name(name), Data(value)
	{
		Type = DataBindType::Bool;
	}

	DataBindObject::DataBindObject(const std::string& name, std::string* value)
		: Name(name), Data(value)
	{
		Type = DataBindType::String;
	}

	DataBindObject::DataBindObject(const std::string& name, char* value)
		: Name(name), Data(value)
	{
		Type = DataBindType::Char;
	}

	void DataBindObject::SetData(int* value)
	{
		Data = value;
	}

	void DataBindObject::SetData(bool* value)
	{
		Data = value;
	}
	
	void DataBindObject::SetData(std::string* value)
	{
		Data = value;
	}
	
	void DataBindObject::SetData(float* value)
	{
		Data = value;
	}

	void DataBindObject::SetData(char* value)
	{
		Data = value;
	}

	std::variant<int*, float*, bool*, std::string*, char*> DataBindObject::GetData()
	{
		return Data;
	}

	DataModel::DataModel(const std::string& name)
		: Name(name)
	{
	}

	bool DataModel::HasData(const std::string& dataName)
	{
		for (auto& dataBindObject : DataObjects)
		{
			if (dataBindObject.Name == dataName)
			{
				return true;
			}
		}

		return false;
	}
}