import "Scripts/Engine" for Engine

class Scene {
	foreign static GetEntityID(name)


	static GetEntity(name) {
		var entId = Scene.GetEntityID(name)
		var ent = Entity.new(entId)
		return ent
	}

	foreign static EntityHasComponent(id, name)

	// Component specific private setter and getter.
	foreign static GetLightIntensity_(e)
	foreign static SetLightIntensity_(e, intensity)


	/* 
	// Transform
	foreign static SetTranslation_(e, x, y, z)
	foreign static SetRotation_(e, x, y, z)
	foreign static SetScale_(e, x, y, z)

	// Character controller
	foreign static SetVelocity(e, x, y, z)
	foreign static SetStepHeight(e, x, y, z)
	foreign static IsOnGround(e)
	*/
	// Light
	//
	/* 
	foreign static SetLightIsVolumetric_(e, bool)
	foreign static SetLightSyncDirectionWithSky_(e, bool)
	foreign static SetLightColor_(e, r, g, b)

	// Camera
	foreign static SetCameraFov_(e, fov)
	foreign static SetCameraType(e, type)
	foreign static SetCameraDirection_(e, x, y, z)
	*/
}

class Entity {
	construct new(id) {
		_entityId = id
	}

	HasComponent(component) {
		return Scene.EntityHasComponent(_entityId, component)
	}

	GetComponent(component) {
		if(this.HasComponent(component) == false) {
			Engine.Log("Tried getting a non-existent component of type: %(component) on entity with id: %(_entityId)")
			return
		}

		if (component == "Light") {
			return Light.new(_entityId)
		}
	}

	// Foreign engine functions
/* 
	// Transform
	foreign static SetTranslation_(e, x, y, z)
	foreign static SetRotation_(e, x, y, z)
	foreign static SetScale_(e, x, y, z)

	// Character controller
	foreign static SetVelocity(e, x, y, z)
	foreign static SetStepHeight(e, x, y, z)
	foreign static IsOnGround(e)
	*/
	// Light
	//
	/* 
	foreign static SetLightIsVolumetric_(e, bool)
	foreign static SetLightSyncDirectionWithSky_(e, bool)
	foreign static SetLightColor_(e, r, g, b)

	// Camera
	foreign static SetCameraFov_(e, fov)
	foreign static SetCameraType(e, type)
	foreign static SetCameraDirection_(e, x, y, z)
	*/
}

class Light {
	construct new(id) {
		_entityId = id
	}


	SetIntensity(intensity) {
		Scene.SetLightIntensity_(_entityId, intensity)
	}

	GetIntensity() {
		return Scene.GetLightIntensity_(_entityId)
	}

	
	/*
	SetType(type) {
		this.SetType_(_entityId, type)
	}

	SetColor(color) {
		this.SetColor_(_entityId, color.r, color.g, color.b, color.a)
	}

	SetDirection(direction) {
		var normalized = direction.Normalize()
		this.SetDirection_(_entityId, normalized.x, normalized.y, normalized.z)
	}
	
	foreign static SetType_(id, type)
	foreign static SetColor_(id, r, g, b)
	
	foreign static SetDirection_(id, x, y, z)
	*/
	
}