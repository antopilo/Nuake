#include "CanvasParser.h"

#include "../Nodes/Canvas.h"

#include "../Nodes/Text.h"
#include "../Nodes/Button.h"

#include "../FileSystem.h"
#include "StyleSheetParser.h"
#include "../StringHelper.h"

#include <iostream>
#include <charconv>

namespace NuakeUI
{
	CanvasParser::CanvasParser()
	{
		RegisterNodeType("div", Node::New);
		RegisterNodeType("text", Text::New);
		RegisterNodeType("button", Button::New);
	}

	void CanvasParser::RegisterNodeType(const std::string& name, refNew refConstructor)
	{
		NodeTypes[name] = refConstructor;
	}

	bool CanvasParser::HasNodeType(const std::string& name) const
	{
		return NodeTypes.find(name) != NodeTypes.end();
	}

	refNew CanvasParser::GetNodeType(const std::string& name) const
	{
		if (HasNodeType(name))
		{
			return NodeTypes.at(name);
		}

		return nullptr;
	}

	NodePtr CanvasParser::CreateNodeFromXML(tinyxml2::XMLElement* xml, const std::string& id)
	{
		NodePtr newNode;
		std::string nodeId = id;
		std::string type = xml->Value();
		std::string text = xml->GetText() ? xml->GetText() : "";

		if (HasNodeType(type))
		{
			newNode = GetNodeType(type)(nodeId, text);
			newNode->Type = type;

			if (!newNode->HasBeenInitialized())
			{
				newNode->InitializeNode();
			}
		}
		
		return newNode;
	}

	void CanvasParser::AddClassesToNode(tinyxml2::XMLElement* e, NodePtr node)
	{
		auto classAttribute = e->FindAttribute("class");
		if (!classAttribute)
			return;

		std::string strClasses = classAttribute->Value();
		node->Classes = StringHelper::Split(strClasses, ' ');
	}

	void CanvasParser::WriteValueFromString(std::variant<int, float, bool, std::string, char>& var, const std::string& str)
	{
		// Determine type of value
		if (str.find("'") != std::string::npos)
		{
			// Removing second bracket
			const auto& stringSplit = StringHelper::Split(str, "'");
			var = stringSplit[1];
		}
		else if (str.find(".") != std::string::npos)
		{
			const auto& begin = str.data();
			const auto& end = begin + std::size(str);
			float rightFloat;
			std::from_chars(begin, end, rightFloat);
			var = rightFloat;
		}
		else if (str.find("true") != std::string::npos)
		{
			var = true;
		}
		else if (str.find("false") != std::string::npos)
		{
			var = false;
		}
		else
		{
			const auto& begin = str.data();
			const auto& end = begin + std::size(str);
			int rightInt;
			std::from_chars(begin, end, rightInt);
			var = rightInt;
		}
	}

	void CanvasParser::AddModelIfToNode(tinyxml2::XMLElement* e, NodePtr node)
	{
		if (auto modelIf = e->FindAttribute("if"); modelIf)
		{
			std::string ifCondition = modelIf->Value();
			ifCondition = StringHelper::RemoveChar(ifCondition, ' ');

			ComparaisonType compType = ComparaisonType::None;
			std::vector<std::string> splits;

			const std::vector<std::string> operators { "==", "!=", ">=", "<=", ">", "<" };
			for (auto i = 0; i < std::size(operators); i++)
			{
				std::string operatorString = operators[i];
				if (ifCondition.find(operatorString) != std::string::npos)
				{
					compType = (ComparaisonType)i;
					splits = StringHelper::Split(ifCondition, operatorString);
				}
			}

			if (std::size(splits) < 2 || compType == ComparaisonType::None)
				return;

			auto operation = DataModelOperation::New(splits[0], OperationType::If, compType);
			WriteValueFromString(operation->Right, splits[1]);
			
			node->AddDataModelOperation(operation);
		}
	}

	void CanvasParser::AddModelClasses(tinyxml2::XMLElement* e, NodePtr node)
	{
		auto currentAttribute = e->FirstAttribute();

		while (currentAttribute)
		{
			std::string attributeName = currentAttribute->Name();
			if (attributeName == "modelClass")
			{
				std::string attributeValue = currentAttribute->Value();
				auto attributeValueSplits = StringHelper::Split(attributeValue, ':');

				if (std::size(attributeValueSplits) < 2)
					continue;

				std::string className = StringHelper::RemoveChar(attributeValueSplits[0], '[');
				className = StringHelper::RemoveChar(className, ']');

				ComparaisonType compType = ComparaisonType::None;
				std::vector<std::string> splits;

				// TODO: Move to another reusable method.
				const std::vector<std::string> operators{ "==", "!=", ">=", "<=", ">", "<" };
				for (auto i = 0; i < std::size(operators); i++)
				{
					std::string operatorString = operators[i];
					std::string logicalExpression = attributeValueSplits[1];
					if (logicalExpression.find(operatorString) != std::string::npos)
					{
						compType = (ComparaisonType)i;
						splits = StringHelper::Split(logicalExpression, operatorString);
					}
				}

				if (std::size(splits) < 2 || compType == ComparaisonType::None)
					return;

				std::string dataProp = StringHelper::RemoveChar(splits[0], ' ');
				auto operation = DataModelOperation::New(dataProp, OperationType::IfClass, compType);
				WriteValueFromString(operation->Right, StringHelper::RemoveChar(splits[1], ' '));
				operation->ClassName = className;
				node->AddDataModelOperation(operation);
			}
			currentAttribute = currentAttribute->Next();
		}
	}

	void CanvasParser::ScanFragment(tinyxml2::XMLElement* e, NodePtr node)
	{
		// We have a <fragment> with a src path.
		const std::string nodeType = e->Value();
		if (nodeType == "fragment")
		{
			if (auto srcAttr = e->FindAttribute("src"); srcAttr)
			{
				std::string fragmentPath =  _parsingPath + "/../" + srcAttr->Value();
				if (FileSystem::FileExists(fragmentPath))
				{
					tinyxml2::XMLDocument doc;
					if (tinyxml2::XMLError error = doc.LoadFile(fragmentPath.c_str()))
					{
						doc.PrintError();
					}

					auto firstNode = doc.FirstChildElement();
					IterateOverElement(firstNode, node);
				}
				else
				{
					std::cout << "Fragment src attributes error. Cant find file at: " << fragmentPath << std::endl;
				}
			}
		}
	}

	void CanvasParser::IterateOverElement(tinyxml2::XMLElement* e, NodePtr node)
	{
		tinyxml2::XMLElement* current = e;
		while (current)
		{
			std::string id = "Node";

			// Look if the node has an id.
			auto idAttribute = current->FindAttribute("id");
			if (idAttribute)
			{
				id = idAttribute->Value();
			}

			ScanFragment(current, node);
			

			NodePtr newNode = CreateNodeFromXML(current, id);
			if (newNode)
			{
				AddClassesToNode(current, newNode);
				AddModelIfToNode(current, newNode);
				AddModelClasses(current, newNode);

				// Insert in the tree
				node->InsertChild(newNode);

				// Recursivity on the childs of the current node.
				IterateOverElement(current->FirstChildElement(), newNode);
			}

			// Continue to the sibbling after going Depth first.
			current = current->NextSiblingElement();
		}
	}

	CanvasPtr CanvasParser::Parse(const std::string& path)
	{
		_parsingPath = path;

		tinyxml2::XMLDocument doc;
		if (tinyxml2::XMLError error = doc.LoadFile(path.c_str()))
		{
			doc.PrintError();
			return nullptr;
		}

		CanvasPtr canvas = Canvas::New();
		NodePtr root = Node::New("root");

		auto firstNode = doc.FirstChildElement();
		if (!firstNode)
		{
			return canvas;
		}

		// Look for stylesheet attribute in root.
		auto styleSheet = firstNode->FindAttribute("stylesheet");
		if (styleSheet)
		{
			std::string relativePath = path + "/../" + styleSheet->Value();
			if (FileSystem::FileExists(relativePath))
			{
				auto styleSheet = StyleSheetParser::Get().Parse(relativePath);
				canvas->SetStyleSheet(styleSheet);
			}
		}

		IterateOverElement(firstNode, root);

		canvas->SetRoot(root);
		return canvas;
	}
}