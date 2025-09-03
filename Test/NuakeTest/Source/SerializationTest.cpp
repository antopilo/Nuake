#include "catch2/catch_amalgamated.hpp"
#include "Nuake/Scene/Components/Component.h"
#include "Nuake/Resource/Serializer/ComponentSerializer.h"

#include "entt/entt.hpp"

namespace Serialization
{
	using namespace Nuake;

	enum class TestEnum : int32_t
	{
		One,
		Two,
		Three,
		Four
	};

	class TestData : public Component
	{
		NUAKECOMPONENT(TestData, "TestData");

	public:
		int myInt;
		bool myBool;
		std::string myString;
		Vector2 myVec2;
		Vector3 myVec3;
		Vector4 myVec4;
		TestEnum myEnum;

		static void InitializeComponentClass()
		{
			BindComponentField<&TestData::myInt>("myInt", "myInt");
			BindComponentField<&TestData::myBool>("myBool", "myBool");
			BindComponentField<&TestData::myString>("myString", "myString");
			BindComponentField<&TestData::myVec2>("myVec2", "myVec2");
			BindComponentField<&TestData::myVec3>("myVec3", "myVec3");
			BindComponentField<&TestData::myVec4>("myVec4", "myVec4");
			BindComponentField<&TestData::myEnum>("myEnum", "myEnum");
		}
	};

	TEST_CASE("Serialize Struct", "[Serialization]")
	{
		// Initialize component
		TestData::InternalInitializeClass();
		TestData testData =
		{
			.myInt = 1337,
			.myBool = true,
			.myString = "Hello World",
			.myVec2 = Vector2(1, 2),
			.myVec3 = Vector3(3, 4, 5),
			.myVec4 = Vector4(6, 7, 8, 9),
			.myEnum = TestEnum::Two
		};

		// Serialize into json
		ComponentSerializer serializer;
		json result = serializer.Serialize(testData);

		// Test JSON result
		REQUIRE(result.contains("TestData"));

		REQUIRE(result["TestData"].contains("myInt"));
		REQUIRE(result["TestData"]["myInt"] == testData.myInt);

		REQUIRE(result["TestData"].contains("myBool"));
		REQUIRE(result["TestData"]["myBool"] == testData.myBool);

		REQUIRE(result["TestData"].contains("myString"));
		REQUIRE(result["TestData"]["myString"] == testData.myString);

		REQUIRE(result["TestData"].contains("myVec2"));
		REQUIRE(result["TestData"]["myVec2"]["x"] == testData.myVec2.x);
		REQUIRE(result["TestData"]["myVec2"]["y"] == testData.myVec2.y);
		
		REQUIRE(result["TestData"].contains("myVec3"));
		REQUIRE(result["TestData"]["myVec3"]["x"] == testData.myVec3.x);
		REQUIRE(result["TestData"]["myVec3"]["y"] == testData.myVec3.y);
		REQUIRE(result["TestData"]["myVec3"]["z"] == testData.myVec3.z);
		
		REQUIRE(result["TestData"].contains("myVec4"));
		REQUIRE(result["TestData"]["myVec4"]["x"] == testData.myVec4.x);
		REQUIRE(result["TestData"]["myVec4"]["y"] == testData.myVec4.y);
		REQUIRE(result["TestData"]["myVec4"]["z"] == testData.myVec4.z);
		REQUIRE(result["TestData"]["myVec4"]["w"] == testData.myVec4.w);
	}

	TEST_CASE("Deserialize Struct", "[Serialization]")
	{
		// Initialize component
		TestData::InternalInitializeClass();
		TestData testData =
		{
			.myInt = 1337,
			.myBool = true,
			.myString = "Hello World",
			.myVec2 = Vector2(1, 2),
			.myVec3 = Vector3(3, 4, 5),
			.myVec4 = Vector4(6, 7, 8, 9)
		};

		// Serialize into json
		ComponentSerializer serializer;
		json result = serializer.Serialize(testData);

		// Deserialize
		TestData inTestData = TestData{ };
		serializer.Deserialize(result, inTestData);

		// Test JSON result
		REQUIRE(inTestData.myInt == testData.myInt);
		REQUIRE(inTestData.myBool == testData.myBool);
		REQUIRE(inTestData.myString == testData.myString);
		REQUIRE(inTestData.myVec2 == testData.myVec2);
		REQUIRE(inTestData.myVec3 == testData.myVec3);
		REQUIRE(inTestData.myVec4 == testData.myVec4);
	}
}