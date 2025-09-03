#pragma once
#include "NodeState.h"

#include "Nuake/UI/DataBinding/DataBindObject.h"
#include "Nuake/UI/DataBinding/DataModelOperations.h"
#include "Nuake/UI/Styles/StyleSheet.h"
#include "Nuake/UI/Nodes/NodeStyle.h"
#include "Nuake/UI/InputManager.h"

#include "Nuake/Core/Maths.h"
#include "Nuake/Resource/UUID.h"
#include "Nuake/Scripting/ScriptingEngineNet.h"

#include <yoga/yoga.h>

#include <any>
#include <map>
#include <string>
#include <vector>


#define SetLength(name) \
if (ComputedStyle.##name.type == LengthType::Auto)  \
YGNodeStyleSet##name##Auto(mNode); \
else if (ComputedStyle.##name.type == LengthType::Pixel) YGNodeStyleSet##name(mNode, ComputedStyle.##name.value); \
else if (ComputedStyle.##name.type == LengthType::Percentage) YGNodeStyleSet##name##Percent(mNode, ComputedStyle.##name.value); \

#define SetLengthBorder(name, border) \
if (ComputedStyle.##name##border.type == LengthType::Auto)  \
YGNodeStyleSet##name##Auto(mNode, YGEdge##border); \
else if (ComputedStyle.##name##border.type == LengthType::Pixel) YGNodeStyleSet##name(mNode, YGEdge##border, ComputedStyle.##name##border.value); \
else if (ComputedStyle.##name##border.type == LengthType::Percentage) YGNodeStyleSet##name##Percent(mNode,  YGEdge##border, ComputedStyle.##name##border.value); \

#define SetLengthBorderNoAuto(name, border) \
if (ComputedStyle.##name##border.type == LengthType::Pixel) YGNodeStyleSet##name(mNode, YGEdge##border, ComputedStyle.##name##border.value); \
else if (ComputedStyle.##name##border.type == LengthType::Percentage) YGNodeStyleSet##name##Percent(mNode,  YGEdge##border, ComputedStyle.##name##border.value); \


#define SetLengthNoAuto(name) \
if (ComputedStyle.##name.type == LengthType::Pixel) YGNodeStyleSet##name(mNode, ComputedStyle.##name.value); \
else if (ComputedStyle.##name.type == LengthType::Percentage) YGNodeStyleSet##name##Percent(mNode, ComputedStyle.##name.value); \

#define EnumProp(name) EnumPropEx(name, ##name##Type)

#define EnumPropEx(name, enums) \
case StyleProperties::name: \
{ \
auto type = value.value.Enum; \
ComputedStyle.##name## = (##enums##)type; \
} \
break;\

#define LengthProp(name)  \
				case StyleProperties::name: \
				{ \
					switch (value.type) \
					{ \
					case PropValueType::Pixel: \
					{ \
						ComputedStyle.name.type = LengthType::Pixel; \
						ComputedStyle.name.value = value.value.Number; \
					} \
					break; \
					case PropValueType::Percent: \
					{ \
						ComputedStyle.name.type = LengthType::Percentage;  \
						ComputedStyle.name.value = value.value.Number; \
					} \
					break; \
					case PropValueType::Auto: \
					{ \
						ComputedStyle.name.type = LengthType::Auto; \
					} \
					break; \
					} \
				} \
break; \


using namespace NuakeUI;

namespace NuakeUI
{
	class Node;
	class CanvasParser;
	class Renderer;
	class Canvas;

	typedef std::shared_ptr<Node> NodePtr;

	class Node
	{
		friend NuakeUI::Canvas;
		friend NuakeUI::CanvasParser;
		friend NuakeUI::Renderer;

	private:
		static Node* mFocused;
		UUID scriptingId;

	protected:
		Canvas* canvasOwner;

		std::string Type = "node";
		float ScrollDelta = 0.0f;
		std::string ID = "";
		Node* Parent = nullptr;
		std::vector<NodePtr> Childrens = std::vector<NodePtr>();

		YGNodeRef mNode;

		DataModelOperationCollection mDataModelOperations;
		DataModelPtr mDataModel;

		bool mHasBeenInitialized = false;
		void InitializeNode();

	public:
		bool CanGrabFocus = false;
		std::any UserData;
		NodeState State = NodeState::Idle;

		std::vector<std::string> Classes = std::vector<std::string>();
		Vector2 ComputedSize = { 0, 0 };
		Vector2 ComputedPosition = { 0, 0 };
		int32_t ComputedZIndex = 0;
		NodeStyle ComputedStyle;
		NodeStyle TargetStyle;


		static NodePtr New(const std::string id, const std::string& value = "");
		Node(const std::string& id, const std::string& value = "");
		Node() = default;
		~Node() = default;

		bool HasBeenInitialized() const;

		virtual void Draw(int z);
		virtual void UpdateInput(InputManager* manager, const Vector2& overrideMousePos = Vector2(-1, -1));
		virtual void Tick(InputManager* manager, const Vector2& overrideMousePos = Vector2(-1, -1));
		virtual void Calculate();

		void OnMouseHover(InputManager* inputManager) {};
		void OnMouseExit(InputManager* inputManager) {};
		void OnClick(InputManager* inputManager);
		void OnTick(InputManager* manager) {};
		void OnClickReleased(InputManager* inputManager) {};
		void OnScroll(InputManager* inputManager) {};

		void SetScriptingID(const UUID& uuid)
		{
			scriptingId = uuid;
		}

		UUID GetScriptingID() const
		{
			return scriptingId;
		}

		bool HasFocus() const;
		void GrabFocus();
		void ReleaseFocus();

		void ApplyStyleProperties(std::map<StyleProperties, PropValue> properties);

		void AddClass(const std::string& c)
		{
			bool containClass = false;
			for (auto& classe : Classes)
			{
				if (c == classe)
				{
					containClass = true;
				}
			}

			if (!containClass)
			{
				Classes.push_back(c);
			}
		}
		void RemoveClass(const std::string& c)
		{
			bool found = false;
			int i = 0;
			for (auto& cc : Classes)
			{
				if (cc == c)
				{
					found = true;
					break;
				}
				i++;
			}

			if (found)
				Classes.erase(Classes.begin() + i);

		}
		bool HasClass(const std::string& c) const
		{
			bool found = false;
			for (auto& cc : Classes)
				if (cc == c) found = true;
			return found;
		}

		// Getter Setter
		std::string GetType() const;
		std::string GetID() const;
		uint32_t GetIndex() const;
		YGNodeRef GetYogaNode() const;
		float GetScroll() const;
		bool IsMouseHover(float x, float y);

		bool HasDataModel() const;
		DataModelPtr GetDataModel() const;
		void SetDataModel(const DataModelPtr& dataModel);

		DataModelOperationCollection& GetDataModelOperations();
		void AddDataModelOperation(DataModelOperationPtr& operation);

		std::vector<NodePtr> GetChildrens() const;
		void InsertChild(NodePtr child);

		template<class T>
		std::shared_ptr<T> GetChild(unsigned int index)
		{
			assert(index < Childrens.size()); // No childrens.
			return std::static_pointer_cast<T>(Childrens[index]);
		}

		template<class T>
		std::shared_ptr<T> GetChildByID(const std::string& id)
		{
			assert(Childrens.size() > 0); // No childrens.
			for (auto& c : Childrens)
			{
				if (c->ID == id)
					return std::static_pointer_cast<T>(c);
			}

			assert(false);  // Node not found.
		}

		template<class T>
		bool FindChildByID(const std::string& id, std::shared_ptr<T>& node)
		{
			for (auto& c : Childrens)
			{
				if (c->ID == id)
				{
					node = std::static_pointer_cast<T>(c);
					return true;
				}

				if (c->FindChildByID<T>(id, node))
					return true;
			}

			return false;
		}
	};
}
