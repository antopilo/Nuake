#pragma once


namespace Nuake
{
	class ScriptingEngineNet
	{
	private:
		ScriptingEngineNet() = default;
		~ScriptingEngineNet() = default;

	public:
		static ScriptingEngineNet& Get();

		void Initialize();
	};
}