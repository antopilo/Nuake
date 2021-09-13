#include <string>
#include "Node.h"

namespace Nuake
{
	namespace UI 
	{
		class TextInput : public Node
		{
		private:
			std::string content = "";
			bool IsFocus = false;

			Ref<Node> Background;
		public:
			TextInput()
			{
				Background = CreateRef<Node>();
				Background->NormalStyle.MinWidth = {200, Layout::Unit::PIXEL };
				Background->NormalStyle.MinHeight = { 30, Layout::Unit::PIXEL };

				Childrens.push_back(Background);
			}

			void Draw()
			{
				Renderer2D::DrawRect()
			}
		};
	}
}