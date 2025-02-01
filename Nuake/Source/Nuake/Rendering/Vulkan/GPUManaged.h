#pragma once

#include <functional>
#include <stack>
#include <string>

namespace Nuake
{
	using CleanUpFunc = std::function<void()>;
	using CleanUpStack = std::stack<CleanUpFunc>;

	class GPUManaged
	{
	protected:
		std::string debugName;
		CleanUpStack cleanUpQueue;

	public:
		virtual void SetDebugName(const std::string& inName);
		std::string_view GetDebugName() const;

		void AddGPUCleanUpFunc(CleanUpFunc cleanUpFunc);
		CleanUpStack GetGPUCleanUpStack();
	};
}