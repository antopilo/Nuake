#include "DataModelOperations.h"
#include <iostream>

namespace NuakeUI
{
	DataModelOperation::DataModelOperation(const std::string& name, OperationType type, ComparaisonType compType)
	{
		Name = name;
		Type = type;
		CompType = compType;
	}

	DataModelOperationPtr DataModelOperation::New(const std::string& name, OperationType type, ComparaisonType compType)
	{
		return std::make_shared<DataModelOperation>(name, type, compType);
	}

	bool DataModelOperation::Compare(DataModelPtr object)
	{
		if (!object || !object->HasData(Name))
		{
			std::cout << "DataModel has no data named " << Name << std::endl;
			return false;
		}
		
		DataBindType dataType = object->GetDataType(Name);
		auto dataRight = Right;

		switch (dataType)
		{
			case DataBindType::Bool:
				return CompareLeftAndRight<bool>(object->GetData<bool>(Name), std::get<bool>(Right), CompType);
				break;
			case DataBindType::Int:
				return CompareLeftAndRight<int>(object->GetData<int>(Name), std::get<int>(Right), CompType);
				break;
			case DataBindType::Float:
				return CompareLeftAndRight<float>(object->GetData<float>(Name), std::get<float>(Right), CompType);
				break;
			case DataBindType::String:
				return CompareLeftAndRight<std::string>(object->GetData<std::string>(Name), std::get<std::string>(Right), CompType);
				break;
			case DataBindType::Char:
				return CompareLeftAndRight<char>(object->GetData<char>(Name), std::get<char>(Right), CompType);
				break;
		}

		return false;
	}
}