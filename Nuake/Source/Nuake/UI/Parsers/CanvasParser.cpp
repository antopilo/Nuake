#include "CanvasParser.h"
#include "StyleSheetParser.h"

#include "Nuake/FileSystem/FileSystem.h"
#include "Nuake/UI/StringHelper.h"

// Built-in widgets
#include "Nuake/UI/Nodes/Canvas.h"
#include "Nuake/UI/Nodes/Text.h"
#include "Nuake/UI/Nodes/Button.h"

#include "Nuake/Scripting/ScriptingEngineNet.h"

#include <iostream>
#include <charconv>
#include <thread>


using namespace NuakeUI;

CanvasParser::CanvasParser()
{
	// Built-in node types
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

	// Check if built-in uiWidget or custom one
	if (HasNodeType(type))
	{
		newNode = GetNodeType(type)(nodeId, text);
		newNode->Type = type;

		if (!newNode->HasBeenInitialized())
		{
			newNode->InitializeNode();
		}
	}
	else if(ScriptingEngineNet::Get().HasUIWidget(type))
	{
		// Look in C# if the user has defined custom widgets
		auto widget = ScriptingEngineNet::Get().GetUIWidget(type);
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
#ifdef _LIBCPP_VERSION // Temporary hack until llvm libc++ finally implement this
		float rightFloat = std::stof(str);
#else
		const auto& begin = str.data();
		const auto& end = begin + std::size(str);
		float rightFloat;
		std::from_chars(begin, end, rightFloat);
#endif
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

bool CanvasParser::ScanCustomWidgets(tinyxml2::XMLElement* e, NodePtr node, const std::string& inheritedId)
{
	const std::string nodeTypeName = e->Value();
	std::string id = inheritedId;

	// Look if the node has an id.
	auto idAttribute = e->FindAttribute("id");
	if (idAttribute)
	{
		id = idAttribute->Value();
	}

	if (!ScriptingEngineNet::Get().HasUIWidget(nodeTypeName))
	{
		return false; 
	}

	// Is template file valid?
	UIWidgetObject& customWidget = ScriptingEngineNet::Get().GetUIWidget(nodeTypeName);
	if (!FileSystem::FileExists(customWidget.htmlPath))
	{
		Logger::Log("Custom widget html file doesnt exist: " + nodeTypeName + " with HTML path: " + customWidget.htmlPath, "ui", CRITICAL);
		return false;
	}

	currentParsingCanvas->AddSourceFile(FileSystem::GetFile(customWidget.htmlPath));

	// Parse load HTML file now
	const std::string& absoluteFilePath = FileSystem::RelativeToAbsolute(customWidget.htmlPath);

	// This is a workaround in case the file is locked by a code editor, retry 5 times.
	tinyxml2::XMLDocument doc;
	tinyxml2::XMLError error;
	int attempt = 0;
	error = doc.LoadFile(absoluteFilePath.c_str());
	while (error && attempt < 5)
	{
		error = doc.LoadFile(absoluteFilePath.c_str());
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
		attempt++;
	}

	if (tinyxml2::XMLError error = doc.LoadFile(absoluteFilePath.c_str()))
	{
		doc.PrintError();
	}

	// Let's parse the file
	auto firstNode = doc.FirstChildElement();
	IterateOverElement(firstNode, node, nodeTypeName, id);

	return true;
}

void CanvasParser::IterateOverElement(tinyxml2::XMLElement* e, NodePtr node, const std::string& customNodeName, const std::string& inheritedID)
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
		

		// Let's keep fragments for now as they remove 
		// the need to create a C# class for simple templating.
		ScanFragment(current, node);
		ScanCustomWidgets(current, node, id);
		
		// Let's add custom widgets to the DOM.
		NodePtr newNode = CreateNodeFromXML(current, id);
		if (newNode)
		{
			if (!customNodeName.empty())
			{
				if (!inheritedID.empty())
				{
					newNode->ID = inheritedID;
				}

				// Allow to link between C# script and node using a UUID
				UUID scriptingId = UUID();
				newNode->SetScriptingID(scriptingId);
				customWidgetIDs.push_back(std::make_pair(std::make_pair(node->canvasOwner->uuid, scriptingId), customNodeName));
			}

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

Ref<Canvas> CanvasParser::Parse(CanvasPtr canvas, const std::string& path)
{
	_parsingPath = path;

	currentParsingCanvas = canvas;

	tinyxml2::XMLDocument doc;
	tinyxml2::XMLError error;
	bool fileLoaded = false;
	for (int i = 0; i < 5; ++i) {  // Try 5 times
		error = doc.LoadFile(path.c_str());
		if (error == tinyxml2::XML_SUCCESS) {
			fileLoaded = true;
			break;
		}
		else if (error == tinyxml2::XML_ERROR_FILE_NOT_FOUND) {
			std::this_thread::sleep_for(std::chrono::milliseconds(100));  // Wait before retrying
		}
	}

	if (error)
	{
		doc.PrintError();
		return nullptr;
	}

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
		std::string relativePath = styleSheet->Value();
		if (FileSystem::FileExists(relativePath))
		{
			auto styleSheet = StyleSheetParser::Get().Parse(canvas, relativePath);
			currentParsingCanvas->AddSourceFile(FileSystem::GetFile(relativePath));
			canvas->SetStyleSheet(styleSheet);
		}
	}

	canvas->AddSourceFile(FileSystem::GetFile(FileSystem::AbsoluteToRelative(path)));
	canvas->SetRoot(root);
	IterateOverElement(firstNode, root);

	return canvas;
}
