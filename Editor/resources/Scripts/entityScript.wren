import "Scripts/Engine" for Engine
import "Scripts/ScriptableEntity" for ScriptableEntity


class TestScript is ScriptableEntity {
    construct new() {
        _deltaTime = 0
    }

    init() {
        Engine.Log("Hello init")
    }

    update(ts) {
        _deltaTime = _deltaTime + ts


    }

    fixedUpdate(ts) {
        
    }

    exit() {
        Engine.Log("Hello exit")
    }
}