import "Scripts/ScriptableEntity" for ScriptableEntity
import "Scripts/Engine" for Engine
import "Scripts/Scene" for Scene, Trigger
import "Scripts/Math" for Vector3
import "Scripts/Input" for Input
import "Scripts/Physics" for Physics, CollisionResult

class TriggerScript is ScriptableEntity {
    construct new() {
        _Activated = false
    }

    init() {
    }

    update(ts) {
    }
    
    fixedUpdate(ts) {
        var trigger = this.GetComponent("Trigger") 
        Engine.Log("Hello")
        if(trigger.GetOverlappingBodyCount() > 0) {
            var bodies = trigger.GetOverlappingBodies()
            var brush = this.GetComponent("Brush")
            var isPlayer = bodies[0].HasComponent("CharacterController")
            if(isPlayer) {
                _Activated = true
            }
        }
        if(_Activated) {
            var brush = this.GetComponent("Brush")
            for(t in brush.GetTargets()) {
                var transform = t.GetComponent("Transform")
                var pos = transform.GetTranslation()
                Engine.Log("Hello")
                pos.x = pos.x + 1.0 * ts
                
                transform.SetTranslation(pos)
            } 
        }
    }

    exit() {

    }
}