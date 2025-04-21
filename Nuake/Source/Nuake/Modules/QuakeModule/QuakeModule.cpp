#include "QuakeModule.h"
#include "QuakeBaker.h"

#include "Nuake/Resource/Bakers/AssetBakerManager.h"


void QuakeModule_Startup()
{
	using namespace Nuake;

	AssetBakerManager& assetBakerMgr = AssetBakerManager::Get();
	assetBakerMgr.RegisterBaker(CreateRef<QuakeBaker>());
}

void QuakeModule_Shutdown()
{

}