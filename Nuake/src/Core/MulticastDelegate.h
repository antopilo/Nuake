#pragma once

#include <functional>
#include <vector>
#include <algorithm>

#define DECLARE_MULTICAST_DELEGATE(multicastDelegateName, ...) typedef MulticastDelegate<__VA_ARGS__> multicastDelegateName; 

template<typename... Args>
class MulticastDelegate
{
public:
    using DelegateID = size_t;

    // Add a callable with bound variables (supports no arguments as well)
    template<typename Callable, typename... BoundArgs>
    DelegateID AddStatic(Callable&& func, BoundArgs&&... boundArgs)
    {
        DelegateID id = nextID++;
        auto boundFunction = [=](Args... args) {
            if constexpr (sizeof...(Args) > 0)
            {
                func(boundArgs..., std::forward<Args>(args)...);
            }
            else
            {
                func(boundArgs...);
            }
        };
        delegates.push_back({id, boundFunction});
        return id;
    }
    
    template<typename Callable, typename Obj, typename... BoundArgs>
    DelegateID AddObject(Callable&& func, Obj* object, BoundArgs&&... boundArgs)
    {
        DelegateID id = nextID++;
        auto boundFunction = [=](Args... args) {
            if constexpr (sizeof...(Args) > 0)
            {
                (object->*func)(boundArgs..., std::forward<Args>(args)...);
            }
            else
            {
                (object->*func)(boundArgs...);
            }
        };
        delegates.push_back({id, boundFunction});
        return id;
    }

    // Remove a callable using the token returned by Add()
    void Remove(DelegateID id)
    {
        auto it = std::remove_if(delegates.begin(), delegates.end(), [id](const auto& pair)
        {
            return pair.first == id;
        });
        
        if (it != delegates.end())
        {
            delegates.erase(it, delegates.end());
        }
    }

    // Clear all delegates
    void Clear()
    {
        delegates.clear();
    }

    // Invoke all callables
    void Broadcast(Args... args)
    {
        for (auto& [id, delegate] : delegates)
        {
            delegate(std::forward<Args>(args)...);
        }
    }

private:
    using DelegatePair = std::pair<DelegateID, std::function<void(Args...)>>;
    
    std::vector<DelegatePair> delegates;  // Vector of (ID, callable) pairs
    DelegateID nextID = 0;  // Unique ID generator
};
