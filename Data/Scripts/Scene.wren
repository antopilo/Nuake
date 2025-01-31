import "Nuake:Engine" for Engine
import "Nuake:Math" for Vector3

class Scene {
	foreign static GetEntityID(name)

	foreign static CreateEntity(name)
	foreign static AddPrefab(prefabPath)
	
	static GetEntity(name) {
		var entId = Scene.GetEntityID(name)
		var ent = Entity.new(entId)
		return ent
	}

	foreign static EntityHasComponent(id, name)
	static EntityGetComponent(id, component) {
		if(this.EntityHasComponent(id, component) == false) {
			Engine.Log("Tried getting a non-existent component of type: %(component) on entity with id: %(id)")
			return
		}

		if (component == "Light") {
			return Light.new(id)
		} else if (component == "CharacterController") {
			return CharacterController.new(id)
		} else if (component == "RigidBody") {
			return RigidBody.new(id)
		} else if (component == "Camera") {
			return Camera.new(id)
		} else if (component == "Transform") {
			return TransformComponent.new(id)
		} else if (component == "Script") {
			return this.GetScript_(id)
		} else if (component == "Trigger") {
			return Trigger.new(id)
		} else if (component == "Brush") {
			return Brush.new(id)
		} else if (component == "AudioEmitter") {
			return AudioEmitter.new(id)
		}
	}

    //
	// Components
	//
	// Transform

	foreign static GetScript_(e)
	foreign static GetLocalTranslation_(e)
	foreign static GetTranslation_(e)
	foreign static SetTranslation_(e, x, y, z)
	foreign static SetRotation_(e, x, y, z)
	foreign static SetLookAt_(e, x, y, z)
	foreign static GetRotation_(e)
	//foreign static SetScale_(e, x, y, z)

	// Light
	foreign static GetLightIntensity_(e)  // returns a float
	foreign static SetLightIntensity_(e, intensity)
	//foreign static SetLightIsVolumetric_(e, bool)
	//foreign static SetLightSyncDirectionWithSky_(e, bool)
	//foreign static SetLightColor_(e, r, g, b)
	//foreign static GetLightColor_(e)

	// Camera
	foreign static SetCameraDirection_(e, x, y, z)
	foreign static GetCameraDirection_(e) // returns a list x,y,z
	foreign static GetCameraRight_(e)     // returns a list x,y,z
	//foreign static SetcameraFov(e, fov)
	//foreign static GetCameraFov(e)        // returns a float

	// Audio Emitter
	foreign static SetAudioEmitterPlaying_(e, playing)

	// Character controller
	foreign static MoveAndSlide_(e, x, y, z)
	foreign static IsCharacterControllerOnGround_(e)
	//foreign static IsOnGround_(e)
	
	// RigidBody
	foreign static AddForce_(e, x, y, z)

	// Physics
	static RayCast(from, to) {
		// Fetch results from physics manager, returns a list of floats
		// that we need to unpack into vector3
		var results = Scene.RayCast_(from.x, from.y, from.z, to.x, to.y, to.z)

		var points = [] // Result array of vec3

		var size = results.count / 3
		var i = 0
		while(i < size) {
			// size is base 1 while array is base 0, offset by 1
			var listIndex = i - 1
			points.add(Vector3.new(results[listIndex], results[listIndex + 1], results[listIndex + 2]))
			i = i + 1
		}

		return points
	}

	foreign static RayCast_(fromX, fromY, fromZ, toX, toY, toZ)

	foreign static TriggerGetOverlappingBodyCount_(e)
	foreign static TriggerGetOverlappingBodies_(e)

	foreign static BrushGetTargets_(e)
	foreign static BrushGetTargetsCount_(e)
}

class Entity {
	construct new(id) {
		_entityId = id
	}

	HasComponent(component) {
		return Scene.EntityHasComponent(_entityId, component)
	}

	GetComponent(component) {
		return Scene.EntityGetComponent(_entityId, component)
	}
}

class TransformComponent {
	construct new(id) {
		_entityId = id
	}

	GetLocalTranslation() {
		var result = Scene.GetLocalTranslation_(_entityId)
		return Vector3.new(result[0], result[1], result[2])
	}

	GetTranslation() {
		var result = Scene.GetTranslation_(_entityId)
		return Vector3.new(result[0], result[1], result[2])
	}


	SetTranslation(t) {
		Scene.SetTranslation_(_entityId, t.x, t.y, t.z)
	}

	SetTranslation(x, y, z) {
		Scene.SetTranslation_(_entityId, x, y, z)
	}

	GetRotation() {
		var result = Scene.GetRotation_(_entityId)
		return Vector3.new(result[0], result[1], result[2])
	}

	SetRotation(t) {
		Scene.SetRotation_(_entityId, t.x, t.y, t.z)
	}

	SetLookAt(t) {
		Scene.SetLookAt_(_entityId, t.x, t.y, t.z)
	}
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
	}*/
	
}

class CharacterController {
	construct new(id) {
		_entityId = id
	}

	MoveAndSlide(vel) {
		Scene.MoveAndSlide_(_entityId, vel.x, vel.y, vel.z)
	}

	IsOnGround() {
		return Scene.IsCharacterControllerOnGround_(_entityId)
	}
}

class RigidBody {
	construct new(id) {
		_entityId = id
	}

	AddForce(force) {
		Scene.AddForce_(_entityId, force.x, force.y, force.z)
	}
}

class Camera {
	construct new(id) {
		_entityId = id
	}

	SetDirection(dir) {
		Scene.SetCameraDirection_(_entityId, dir.x, dir.y, dir.z)
	}

	GetDirection() {
		var dir = Scene.GetCameraDirection_(_entityId)
		return Vector3.new(dir[0], dir[1], dir[2])
	}

	GetRight() {
		var dir = Scene.GetCameraRight_(_entityId)
		return Vector3.new(dir[0], dir[1], dir[2])
	}
}

class AudioEmitter {
	construct new(id) {
		_entityId = id
	}

	SetPlaying(playing) {
		Scene.SetAudioEmitterPlaying_(_entityId, playing)
	}
}

class Trigger {
	construct new(id) {
		_entityId = id
	}

	GetOverlappingBodyCount() {
		return Scene.TriggerGetOverlappingBodyCount_(_entityId)
	}

	GetOverlappingBodies() {
		if(this.GetOverlappingBodyCount() <= 0) {
			return []
		}

		var bodies = Scene.TriggerGetOverlappingBodies_(_entityId)

		var entities = []
		for(b in bodies) {
			entities.add(Entity.new(b))
		}
		return entities
	}
}

class Brush {
	construct new(id) {
		_entityId = id
	}

	GetTargetsCount() {
		return Scene.BrushGetTargetsCount_(_entityId)
	}

	GetTargets() {
		if(this.GetTargetsCount() <= 0) {
			return []
		}

		var entities = []
		var targets = Scene.BrushGetTargets_(_entityId)
		
		for(t in targets) {
			entities.add(Entity.new(t))
		}

		return entities
	}
}