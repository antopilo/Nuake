#include "IAssetBaker.h"

namespace Nuake
{
	class GLTFBaker : public IAssetBaker
	{
	public:
		GLTFBaker() : IAssetBaker(".glb") {}

		Ref<File> Bake(const Ref<File>& file) override;
	};
}