#include "Canvas.h"
#include <yoga/Yoga.h>
#include <yoga/YGConfig.h>

#include "../Renderer.h"

#include "Node.h"

namespace NuakeUI
{
	CanvasPtr Canvas::New()
	{
		return std::make_shared<Canvas>();
	}
	
	Canvas::Canvas() : mInputManager(nullptr), mDirty(false)
	{
		mYogaConfig = YGConfigNew();
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
		
		mRootNode->UpdateInput(mInputManager);
		mRootNode->Tick(mInputManager);

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

		if (root)
			YGNodeCalculateLayout(root, size.x, size.y, YGDirectionLTR);
	}

	void Canvas::ComputeStyle(NodePtr node)
	{
		for (auto& s : node->GetDataModelOperations())
		{
			if (s->Type != OperationType::IfClass)
				continue;


			if (auto dataModel = node->GetDataModel(); s->Compare(dataModel))
			{
				node->AddClass(s->ClassName);
			}
			else
			{
				node->RemoveClass(s->ClassName);
			}
		}
		for (auto& rule : mStyleSheet->Rules)
		{
			bool respectSelector = true;

			for (StyleSelector& selector : rule.Selector)
			{
				bool foundSelector = false;
				if (selector.Type == StyleSelectorType::Class)
				{
					for (auto& c : node->Classes)
					{
						if (c == selector.Value)
							foundSelector = true;
					}
				}
				else if (selector.Type == StyleSelectorType::Pseudo)
				{
					if (selector.Value == "hover" && node->State == NodeState::Hover)
						foundSelector = true;
					if (selector.Value == "active" && node->State == NodeState::Pressed)
						foundSelector = true;
				}
				else if (selector.Type == StyleSelectorType::Id)
				{
					if (node->GetID() == selector.Value)
						foundSelector = true;
				}
				else if (selector.Type == StyleSelectorType::Tag)
				{
					if (selector.Value == node->GetType())
					{
						foundSelector = true;

					}
				}

				if (!foundSelector)
					respectSelector = false;
			}

			if (respectSelector)
				node->ApplyStyleProperties(rule.Properties);
		}

		for (auto& c : node->GetChildrens())
		{
			ComputeStyle(c);
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
}
