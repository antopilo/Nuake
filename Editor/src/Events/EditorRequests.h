#include <src/Core/Core.h>
#include <src/Core/MulticastDelegate.h>

namespace Nuake
{
	class Scene;
	class File;
}

// This is all the commands that the editor can receive from anywhere in the UI
class EditorRequests
{
private:
	MulticastDelegate<Ref<Nuake::File>> requestLoadScene;

	EditorRequests() = default;
	~EditorRequests() = default;

public:
	static EditorRequests& Get()
	{
		static EditorRequests instance;
		return instance;
	}

public:
	void RequestLoadScene(Ref<Nuake::File> sceneFile)
	{
		requestLoadScene.Broadcast(sceneFile);
	}
	
	auto& OnRequestLoadScene() { return requestLoadScene; }
};