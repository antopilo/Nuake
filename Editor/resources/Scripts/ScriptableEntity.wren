import "Nuake:Scene" for Scene 

class ScriptableEntity {
    SetEntityId(id) {
        _EntityID = id
    }

    GetComponent(component) {
        return Scene.EntityGetComponent(_EntityID, component)
    }

    HasComponent(component) {
        return Scene.EntityHasComponent(_EntityID, component)
    }
}