#include "AssimpModule.h"

#include "Nuake/Resource/Bakers/AssetBakerManager.h"

#include "GLTFBaker.h"

void AssimpModule_Startup()
{
	using namespace Nuake;

	AssetBakerManager& assetBakerMgr = AssetBakerManager::Get();
	assetBakerMgr.RegisterBaker(CreateRef<GLTFBaker>());
}

void AssimpModule_Shutdown()
{

}