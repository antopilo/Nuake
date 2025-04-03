#pragma once

#include <functional>
#include <vector>

#define DECLARE_MULTICAST_DELEGATE(multicastDelegateName, ...) typedef MulticastDelegate<__VA_ARGS__> multicastDelegateName;

struct DelegateHandle
{
    size_t id = InvalidHandle;
    
    static inline size_t InvalidHandle = static_cast<size_t>(-1);

    bool IsValid() const { return id != InvalidHandle; }
    void Reset() { id = InvalidHandle; }

    // Comparison operators for convenience
    bool operator==(const DelegateHandle& other) const { return id == other.id; }
    bool operator!=(const DelegateHandle& other) const { return id != other.id; }
};

template<typename... Args>
class MulticastDelegate
{
public:
    // Add a callable with bound variables (supports no arguments as well)
    template<typename Callable, typename... BoundArgs>
    DelegateHandle AddStatic(Callable&& func, BoundArgs&&... boundArgs)
    {
        size_t id = GetNextID();
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
        SetDelegate(id, boundFunction);
        return DelegateHandle{ id };
    }
    
    template<typename Obj, typename Callable, typename... BoundArgs>
    DelegateHandle AddRaw(Obj* object, Callable&& func, BoundArgs&&... boundArgs)
    {
        size_t id = GetNextID();
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
        SetDelegate(id, boundFunction);
        return DelegateHandle{ id };
    }

    // Remove a callable using the token returned by Add()
    void Remove(DelegateHandle& handle)
    {
        //assert(handle.IsValid());
        
        if (handle.IsValid() && handle.id < delegates.size())
        {
            delegates[handle.id].active = false;
            
            // Mark this slot as reusable
            freeIDs.push_back(handle.id);
        }

        // Invalidate the handle
        handle.Reset();
    }

    // Clear all delegates
    void Clear()
    {
        delegates.clear();
        freeIDs.clear();
        nextID = 0;
    }

    // Invoke all callables
    void Broadcast(Args... args)
    {
        for (auto& delegate : delegates)
        {
            if (delegate.active)
            {
                delegate.function(std::forward<Args>(args)...);
            }
        }
    }

private:
    struct Delegate
    {
        bool active = false;
        std::function<void(Args...)> function;
    };

    // A vector of delegates with active state
    std::vector<Delegate> delegates;
    // List of reusable slots
    std::vector<size_t> freeIDs;
    size_t nextID = 0;

    // Get the next available ID, either by reusing a free slot or by creating a new one
    size_t GetNextID()
    {
        if (!freeIDs.empty())
        {
            size_t id = freeIDs.back();
            freeIDs.pop_back();
            return id;
        }
        
        return nextID++;
    }

    // Set the delegate in the vector, makes the array larger if necessary
    void SetDelegate(size_t id, const std::function<void(Args...)>& func)
    {
        if (id >= delegates.size())
            delegates.resize(id + 1);
        
        delegates[id] = { true, func };
    }
};
