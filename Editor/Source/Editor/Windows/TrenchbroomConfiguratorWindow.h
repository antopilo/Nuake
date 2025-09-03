#pragma once

namespace Nuake
{
	class FGDPointEntity;
}

class TrenchbroomConfiguratorWindow
{
public:
	TrenchbroomConfiguratorWindow() = default;
	~TrenchbroomConfiguratorWindow() = default;

	void Update();
	void Draw();
	void DrawPrefabItem(const Nuake::FGDPointEntity& pointEntity);
private:

};