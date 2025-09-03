#pragma once
#include <Nuake/Core/Maths.h>

#include <vector>

class IAnimatedValue
{
public:
	static std::vector<IAnimatedValue*> Instances;

	static void UpdateAll (float ts)
	{
		for (auto& instance : Instances)
		{
			instance->Update(ts);
		}
	}

	virtual void Update(float ts) = 0;
};

template<typename T>
class AnimatedValue : public IAnimatedValue
{
public:
	T Value;
	T TargetValue;
	float Speed = 20.0f;
	AnimatedValue(T value = T(), T targetValue = T())
		: Value(value), TargetValue(targetValue)
	{
		IAnimatedValue::Instances.push_back(this);
	}

	~AnimatedValue()
	{
		auto it = std::remove(Instances.begin(), Instances.end(), this);
		IAnimatedValue::Instances.erase(it, Instances.end());
	}

	virtual void Update(float ts) override
	{
		Value = glm::mix(Value, TargetValue, glm::clamp(Speed * ts, 0.0f, 1.0f));
	}

	void SetValue(T value)
	{
		Value = value;
		TargetValue = value;
	}

	operator T() const
	{
		return Value;
	}

	AnimatedValue<T>& operator =(T value)
	{
		TargetValue = value;
		return *this;
	}
};