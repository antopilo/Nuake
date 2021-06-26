import "Scripts/ScriptableEntity" for ScriptableEntity
import "Scripts/Engine" for Engine
import "Scripts/Scene" for Scene, Trigger
import "Scripts/Math" for Vector3
import "Scripts/Input" for Input
import "Scripts/Physics" for Physics, CollisionResult

class TriggerScript is ScriptableEntity {
    construct new() {

    }

    init() {
        Engine.Log("Hello, I'm a trigger")
    }

    update(ts) {
        
    }
    fixedUpdate(ts) {
        var trigger = this.GetComponent("Trigger") 
        
        Engine.Log("Current overlap: %(trigger.GetOverlappingBodyCount())")
    }

    exit() {

    }
}