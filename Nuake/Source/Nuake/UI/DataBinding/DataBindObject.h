#pragma once

#include <string>
#include <vector>
#include <memory>
#include <variant>
#include <cassert>

namespace NuakeUI
{
	enum class DataBindType
	{
		Int, Float, Bool, String, Char
	};

	class DataBindObject
	{
	private:
		DataBindType Type;

	public:
		std::string Name;

		std::variant<int*, float*, bool*, std::string*, char*> Data;

		DataBindObject(const std::string& name, int* data);
		DataBindObject(const std::string& name, float* data);
		DataBindObject(const std::string& name, bool* data);
		DataBindObject(const std::string& name, std::string* data);
		DataBindObject(const std::string& name, char* data);

		void SetData(int* value);
		void SetData(float* value);
		void SetData(bool* value);
		void SetData(std::string* value);
		void SetData(char* value);

		DataBindType GetType() const { return Type; }

		std::variant<int*, float*, bool*, std::string*, char*> GetData();
	};

	class DataModel;
	typedef std::shared_ptr<DataModel> DataModelPtr;
	
	class DataModel
	{
	public:
		std::vector<DataBindObject> DataObjects;
		std::string Name;

		static DataModelPtr New(const std::string& name)
		{
			return std::make_shared<DataModel>(name);
		}

		DataModel(const std::string& modelName);
		~DataModel() = default;

		void Bind(const std::string& dataName, int* data)
		{
			DataBindObject dataBindObject = DataBindObject(dataName, data);
			DataObjects.push_back(dataBindObject);
		}

		void Bind(const std::string& dataName, float* data)
		{
			DataBindObject dataBindObject = DataBindObject(dataName, data);
			DataObjects.push_back(dataBindObject);
		}

		void Bind(const std::string& dataName, bool* data)
		{
			DataBindObject dataBindObject = DataBindObject(dataName, data);
			DataObjects.push_back(dataBindObject);
		}

		void Bind(const std::string& dataName, std::string* data)
		{
			DataBindObject dataBindObject = DataBindObject(dataName, data);
			DataObjects.push_back(dataBindObject);
		}

		void Bind(const std::string& dataName, char* data)
		{
			DataBindObject dataBindObject = DataBindObject(dataName, data);
			DataObjects.push_back(dataBindObject);
		}
		
		bool HasData(const std::string& dataName);

		DataBindType GetDataType(const std::string dataName)
		{
			assert(HasData(dataName) && "Model has no data with that name");

			for (auto& dataObject : DataObjects)
			{
				if (dataObject.Name == dataName)
				{
					return dataObject.GetType();
				}
			}
		}

		template<typename T>
		T& GetData(const std::string& dataName)
		{
			assert(false && "Unsupported data type");
		}

		template<>
		bool& GetData(const std::string& dataName)
		{
			assert(HasData(dataName) && "Model has no data with that name.");
			
			for (auto& dataBindObject : DataObjects)
			{
				if (dataBindObject.Name == dataName)
				{
					return *std::get<bool*>(dataBindObject.GetData());
				}
			}

			assert(false);
		}

		template<>
		int& GetData(const std::string& dataName)
		{
			assert(HasData(dataName) && "Model has no data with that name.");

			for (auto& dataBindObject : DataObjects)
			{
				if (dataBindObject.Name == dataName)
				{
					auto variant = dataBindObject.GetData();
					int* data = std::get<int*>(variant);
					return *data;
				}
			}
		}

		template<>
		float& GetData(const std::string& dataName)
		{
			assert(HasData(dataName) && "Model has no data with that name.");

			for (auto& dataBindObject : DataObjects)
			{
				if (dataBindObject.Name == dataName)
				{
					auto variant = dataBindObject.GetData();
					float* data = std::get<float*>(variant);
					return *data;
				}
			}
		}

		template<>
		std::string& GetData(const std::string& dataName)
		{
			assert(HasData(dataName) && "Model has no data with that name.");

			for (auto& dataBindObject : DataObjects)
			{
				if (dataBindObject.Name == dataName)
				{
					auto variant = dataBindObject.GetData();
					std::string* data = std::get<std::string*>(variant);
					return *data;
				}
			}
		}

		template<>
		char& GetData(const std::string& dataName)
		{
			assert(HasData(dataName) && "Model has no data with that name.");

			for (auto& dataBindObject : DataObjects)
			{
				if (dataBindObject.Name == dataName)
				{
					auto variant = dataBindObject.GetData();
					char* data = std::get<char*>(variant);
					return *data;
				}
			}
		}
	};
}