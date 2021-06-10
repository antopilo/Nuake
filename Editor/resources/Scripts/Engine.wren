class Engine { 
	foreign static Log(msg)
}

class Scene {
	foreign static GetEntityID(name)

	static GetEntity(name) {
		var entId = Scene.GetEntityID(name)
		var ent = Entity.new(entId)
		return ent
	}

	foreign static EntityHasComponent(id, name)
}

class Entity {
	construct new(id) {
		_entityId = id
	}

	HasComponent(component) {
		return Scene.EntityHasComponent(_entityId, component)
	}
}

