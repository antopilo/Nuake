#include "UserInterface.h"
#include "yoga/Yoga.h"
#include <src/Vendors/pugixml/pugixml.hpp>
#include "Stylesheet.h"
namespace UI
{
	struct simple_walker : pugi::xml_tree_walker
	{
		virtual bool for_each(pugi::xml_node& node)
		{
			for (int i = 0; i < depth(); ++i) std::cout << "  "; // indentation

			std::cout << node.type() << ": name='" << node.name() << "', value='" << node.value() << "'\n";

			return true; // continue traversal
		}
	};

	UserInterface::UserInterface(const std::string& name)
	{
		m_Name = name;
		pugi::xml_document doc;

		// Parse xml document
		pugi::xml_parse_result result = doc.load_file("resources/Interface/Testing.interface");
		std::cout << "Load result: " << result.description() << ", mesh name: " << doc.child("Canvas").attribute("size").value() << std::endl;
		
		auto childrends = doc.child("Canvas").children();
		simple_walker walker;
		doc.traverse(walker);
		Logger::Log("Hello");

		// Create engine structure from document


		// Convert engine structure to yoga


		// Calculate layout


		// Load css styling

		//Ref<StyleSheet> stylesheet = StyleSheet::New("resources/Interface/Testing.css");
		// Iterate through css file

		// Create style sheet data structure

		// When rendering load styles to shader

		// Render

	}

	Ref<UserInterface> UserInterface::New(const std::string& name)
	{
		return CreateRef<UserInterface>(name);
	}

	void UserInterface::Calculate()
	{

	}

	void UserInterface::Draw()
	{
		Renderer2D::BeginDraw();
		for (auto r : m_Rects)
		{
			r->Draw();
		}
	}

	void UserInterface::Update(Timestep ts)
	{

	}
}
