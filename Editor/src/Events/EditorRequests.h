#include <src/Core/Core.h>
#include <src/Core/MulticastDelegate.h>

#include <string>

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
	MulticastDelegate<std::string> requestCloseEditorWindow;

	EditorRequests() = default;
	~EditorRequests() = default;

public:
	static EditorRequests& Get()
	{
		static EditorRequests instance;
		return instance;
	}

public:
	// Broadcast requests
	void RequestLoadScene(Ref<Nuake::File> sceneFile)
	{
		requestLoadScene.Broadcast(sceneFile);
	}

	void RequestCloseEditorWindow(std::string windowId)
	{
		requestCloseEditorWindow.Broadcast(windowId.c_str());
	}
	
	// Subcribe to requests
	auto& OnRequestCloseEditorWindow() { return requestCloseEditorWindow; }
	auto& OnRequestLoadScene() { return requestLoadScene; }
};