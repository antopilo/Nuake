#pragma once
#include "src/Core/Core.h"
#include "src/Resource/UUID.h"
#include "../Nodes/Canvas.h"

#include <functional>
#include <memory>
#include <map>
#include <string>

#include <msdfgen/include/tinyxml2.h>

namespace NuakeUI
{
	typedef std::function<NodePtr(std::string, std::string)> refNew;

	class CanvasParser
	{
	private:
		std::map<std::string, refNew> NodeTypes;
		std::string _parsingPath;
		std::vector<std::pair<std::pair<UUID, UUID>, std::string>>  customWidgetIDs;

	public:
		static CanvasParser& Get()
		{
			static CanvasParser instance;
			return instance;
		}

	private:
		CanvasParser();
		~CanvasParser() = default;

	public:
		UUID lastCanvasID = UUID(0);

		/// <summary>
		/// Register a custom node type.
		/// </summary>
		/// <param name="name">XML tag</param>
		/// <param name="refConstructor">pointer to method that returns a shared pointer.</param>
		void RegisterNodeType(const std::string& name, refNew refConstructor);
		bool HasNodeType(const std::string& name) const;
		refNew GetNodeType(const std::string& name) const;

		Ref<Canvas> Parse(CanvasPtr canvas, const std::string& file);

		std::vector<std::pair<std::pair<UUID, UUID>, std::string>> GetAllCustomWidgetInstance() { return customWidgetIDs; }

	private:
		void ScanFragment(tinyxml2::XMLElement* e, NodePtr node);
		bool ScanCustomWidgets(tinyxml2::XMLElement* e, NodePtr node);

		void WriteValueFromString(std::variant<int, float, bool, std::string, char>& var, const std::string& str);
		void IterateOverElement(tinyxml2::XMLElement* e, NodePtr node, const std::string& customNodeName = "");
		NodePtr CreateNodeFromXML(tinyxml2::XMLElement* xml, const std::string& id = "Node");
		void AddClassesToNode(tinyxml2::XMLElement* e, NodePtr node);
		void AddModelIfToNode(tinyxml2::XMLElement* e, NodePtr node);
		void AddModelClasses(tinyxml2::XMLElement* e, NodePtr node);
	};
}