#pragma once

#include "DataBindObject.h"

#include <string>
#include <variant>

namespace NuakeUI
{
	enum class OperationType
	{
		If, IfClass
	};

	enum class ComparaisonType
	{
		Equal,
		NotEqual,
		GreaterOrEqual,
		LessOrEqual,
		Greater,
		Less,
		None
	};

	class DataModelOperation;
	typedef std::shared_ptr<DataModelOperation> DataModelOperationPtr;
	typedef std::vector<DataModelOperationPtr> DataModelOperationCollection;
	class DataModelOperation
	{
	public:
		std::string Name;
		std::string ClassName;

		OperationType Type;
		ComparaisonType CompType;

		std::variant<int, float, bool, std::string, char> Right;

		static DataModelOperationPtr New(const std::string& name, OperationType type, ComparaisonType compType);
		DataModelOperation(const std::string& name, OperationType type, ComparaisonType compType);
		~DataModelOperation() = default;

		template<typename T> 
		bool CompareLeftAndRight(const T& left, const T& right, const ComparaisonType& compType)
		{
			if (CompType == ComparaisonType::Equal)
			{
				return left == right;
			}
			else if (CompType == ComparaisonType::NotEqual)
			{
				return left != right;
			}
			else if (CompType == ComparaisonType::GreaterOrEqual)
			{
				return left >= right;
			}
			else if (CompType == ComparaisonType::LessOrEqual)
			{
				return left <= right;
			}
			else if (CompType == ComparaisonType::Greater)
			{
				return left > right;
			}
			else if (CompType == ComparaisonType::Less)
			{
				return left < right;
			}
		}

		bool Compare(DataModelPtr object);
	};
}