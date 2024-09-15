#include "Canvas.h"

#include "src/UI/Renderer.h"
#include "src/FileSystem/File.h"
#include "Node.h"

#include <yoga/Yoga.h>
#include <yoga/YGConfig.h>


namespace NuakeUI
{
	CanvasPtr Canvas::New()
	{
		return std::make_shared<Canvas>();
	}
	
	Canvas::Canvas() : mInputManager(nullptr), mDirty(false)
	{
		mYogaConfig = YGConfigNew();
		YGConfigSetUseWebDefaults(mYogaConfig, true);
		YGConfigSetPointScaleFactor(mYogaConfig, 1.0f);
	}

	Canvas::~Canvas()
	{
		YGConfigFree(mYogaConfig);
		if (mRootNode)
		{
			YGNodeFreeRecursive(mRootNode->GetYogaNode());
		}
	}

	void Canvas::Tick()
	{
		if (!mRootNode)
			return;
		
		mRootNode->Tick(mInputManager);
		mRootNode->UpdateInput(mInputManager);

		mInputManager->ScrollX = 0.f;
		mInputManager->ScrollY = 0.f;
	}

	void Canvas::Draw()
	{
		if (!mRootNode)
			return;
		
		Renderer::Get().BeginDraw();
		Renderer::Get().DrawNode(mRootNode, 0);
		mRootNode->Draw(0);
	}
	
	void Canvas::ComputeLayout(Vector2 size)
	{
		if (!mRootNode)
			return;
		
		Renderer::Get().SetViewportSize(size);

		float x, y;
		x = mInputManager->GetMouseX();
		y = mInputManager->GetMouseY();

		auto root = mRootNode->GetYogaNode();

		// Recompute the node tree.
		if (mDirty)
			ComputeStyle(mRootNode);

		mRootNode->Calculate();

		YGNodeStyleSetMaxWidth(root, size.x);
		// This enfores that the root max size is the viewport.
		mRootNode->ComputedStyle.MaxWidth = { size.x, LengthType::Pixel };
		mRootNode->ComputedStyle.MaxHeight = { size.y, LengthType::Pixel };

		if (root)
		{
			YGNodeSetAlwaysFormsContainingBlock(root, true /*alwaysFormsContainingBlock*/);
			YGNodeCalculateLayout(root, size.x, size.y, YGDirectionLTR);
		}
	}

	void Canvas::ComputeStyle(NodePtr node, const std::vector<StyleRule>& inheritedRules)
	{
		std::vector<StyleRule> relationStyles;

		auto allRules = mStyleSheet->Rules;
		for (auto& i : inheritedRules)
		{
			allRules.push_back(i);
		}

		if (node->Classes.size() == 1 && node->Classes[0] == "dropdown")
		{
 			Logger::Log("Found dropdown");
		}

		for (auto& rule : allRules)
		{
			bool respectSelector = true;
			bool containsRelation = false;
			std::vector<StyleRule> relationalStyleInRule;
			// TODO: Apply descendant selectors to child nodes.
			for (StyleSelector& selector : rule.Selector)
			{
				bool foundSelector = false;
				switch (selector.Type)
				{
				case StyleSelectorType::Class:
				{
					for (auto& c : node->Classes)
					{
						if (c == selector.Value)
							foundSelector = true;
					}
					break;
				}
				case StyleSelectorType::Pseudo:
				{
					const bool isHover = selector.Value == "hover" && node->State == NodeState::Hover;
					const bool isActive = selector.Value == "active" && node->State == NodeState::Clicked;
					if (isHover || isActive)
					{
						foundSelector = true;
					}
					break;
				}
				case StyleSelectorType::Id:
				{
					if (selector.Value == node->GetID())
					{
						foundSelector = true;
					}
					break;
				}
				case StyleSelectorType::Tag:
				{
					if (selector.Value == node->GetType())
					{
						foundSelector = true;
					}
					break;
				}
				}

				if (respectSelector && selector.SelectorRelation != Relation::None)
				{
					if (selector.SelectorRelation == Relation::Descendant)
					{
						containsRelation = true;
						foundSelector = true;
						auto ruleSelector = StyleSelector{ selector.Type, selector.Value, Relation::None };
						StyleRule newStyleRule = StyleRule({ ruleSelector });
						newStyleRule.Properties = rule.Properties;
						relationalStyleInRule.push_back(std::move(newStyleRule));
					}
				}
				else if(!foundSelector)
				{
					respectSelector = false;
				}

				if (respectSelector)
				{
					
				}
			}

			if (respectSelector && !containsRelation)
			{
				node->ApplyStyleProperties(rule.Properties);
			}

			if (relationalStyleInRule.size() > 0 && respectSelector)
			{
				for (auto& s : relationalStyleInRule)
				{
					relationStyles.push_back(s);
				}
			}

		}

		for (auto& c : node->GetChildrens())
		{
			ComputeStyle(c, relationStyles);
		}
	}
	

	// Getters & Setters
	NodePtr Canvas::GetRoot() const
	{
		return mRootNode;
	}
	
	void Canvas::SetRoot(NodePtr root)
	{
		mRootNode = root;
		root->canvasOwner = this;
	}

	void Canvas::SetInputManager(InputManager* inputManager)
	{
		mInputManager = inputManager;
	}

	StyleSheetPtr Canvas::GetStyleSheet() const
	{
		return mStyleSheet;
	}

	void Canvas::SetStyleSheet(StyleSheetPtr styleSheet)
	{
		mDirty = true;
		mStyleSheet = styleSheet;
	}

	Ref<Node> Canvas::GetNodeByUUID(const UUID& uuid)
	{
		if (!nodeCache.contains(uuid))
		{
			return nullptr;
		}

		return nodeCache[uuid];
	}
	void Canvas::AddSourceFile(Ref<File> file)
	{
		sourceFiles.push_back(file);
	}

	std::vector<Ref<File>> Canvas::GetSourceFiles() const
	{
		return sourceFiles;
	}
}
