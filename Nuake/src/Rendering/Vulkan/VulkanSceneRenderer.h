#pragma once
namespace Nuake
{
	class VkSceneRenderer
	{
	public:
		VkSceneRenderer();
		~VkSceneRenderer();

		void Init();
		void BeginScene();
		void EndScene();
	};
}