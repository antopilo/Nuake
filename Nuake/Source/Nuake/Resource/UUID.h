#pragma once
#include <cstdint>
#include <functional>

namespace Nuake 
{
	class UUID 
	{
	private:
		uint64_t m_UUID;

	public:
		UUID();
		UUID(uint64_t uuid);
		UUID(const UUID&) = default;

		operator uint64_t() const { return m_UUID; }
	};
}

namespace std 
{
	template<>
	struct hash<Nuake::UUID>
	{
		std::size_t operator()(const Nuake::UUID& uuid) const
		{
			return hash<uint64_t>()((uint64_t)uuid);
		}
	};
}