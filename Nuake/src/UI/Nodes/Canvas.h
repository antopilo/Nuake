#pragma once
#include "Node.h"
#include "src/Core/Maths.h"
#include "src/Resource/UUID.h"
#include "src/UI/InputManager.h"
#include "src/UI/Styles/StyleSheet.h"

#include "yoga/YGConfig.h"

#include <memory>


namespace Nuake
{
	class File;
}

namespace NuakeUI
{
	class Canvas;
	using CanvasPtr = std::shared_ptr<Canvas>;
	
	class Canvas
	{
		friend Node;

	private:
		YGConfigRef mYogaConfig;

		std::map<UUID, Ref<Node>> nodeCache;

		std::string mFilePath = "";
		
		InputManager* mInputManager;
		StyleSheetPtr mStyleSheet;

		NodePtr mRootNode;
		
		bool mDirty;

		std::vector<Ref<Nuake::File>> sourceFiles;

	public:
		static CanvasPtr New();
		Canvas();
		~Canvas();

		UUID uuid = UUID(0);
		UUID GetUUID() { return uuid; }

		void Tick();
		void Draw();
		void ComputeLayout(Vector2 size);
		void ComputeStyle(NodePtr node);
		
		NodePtr GetRoot() const;
		void SetRoot(NodePtr root);

		void SetInputManager(InputManager* manager);

		StyleSheetPtr GetStyleSheet() const;
		void SetStyleSheet(StyleSheetPtr stylesheet);

		template<class T>
		bool FindNodeByID(const std::string& id, std::shared_ptr<T>& node)
		{
			if (!this->mRootNode->FindChildByID<T>(id, node))
				return false;

			return true;
		}

		Ref<Node> GetNodeByUUID(const UUID& uuid);

		void AddSourceFile(Ref<Nuake::File> file);
		std::vector<Ref<Nuake::File>> GetSourceFiles() const;
	};
}