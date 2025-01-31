#pragma once
#include <memory>
#include <map>
#include <unordered_map>
#include <string>
#include <vector>
#include <queue>
#include <stdexcept>
#include <cassert>

#define ASSERT(x) if (!(x)) assert(false)

template<typename T>
using Ref = std::shared_ptr<T>;

template<typename T>
using Scope = std::unique_ptr<T>;

template<typename T>
using Weak = std::weak_ptr<T>;

template<typename T, typename ... Args>
constexpr Ref<T> CreateRef(Args&& ... args)
{
	return std::make_shared<T>(std::forward<Args>(args)...);
}

template<typename T, typename ... Args>
constexpr Scope<T> CreateScope(Args&& ... args)
{
	return std::make_unique<T>(std::forward<Args>(args)...);
}
