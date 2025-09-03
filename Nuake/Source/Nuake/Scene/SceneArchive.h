#pragma once
#include "Nuake/Vendors/entt/entt.hpp>

class SceneArchive {
private:

public:
	void operator()(entt::entity);

	void operator()(std::underlying_type_t<entt::entity>);

	template<typename T>
	void operator()(entt::entity, const T&);

	void operator()(entt::entity&);

	void operator()(std::underlying_type_t<entt::entity>&);

	template<typename T>
	void operator()(entt::entity&, T&);
};