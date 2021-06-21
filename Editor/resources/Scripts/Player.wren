import "Scripts/ScriptableEntity" for ScriptableEntity
import "Scripts/Engine" for Engine
import "Scripts/Scene" for Scene
import "Scripts/Math" for Vector3
import "Scripts/Input" for Input
import "Scripts/Physics" for Physics, CollisionResult

class PlayerScript is ScriptableEntity {


    Velocity {_Velocity}
    construct new() {
        _InputDir = Vector3.new(0, 0, 0)
        _Velocity = Vector3.new(0, 0, 0)
        _Accel = 150
        _Deccel = 0.85
        _AirDeccel = 0.7
        _Gravity = 20

        _MaxSpeed = 20
        _Jump = 400
        //Input.HideMouse()
    }

    init() {
        Engine.Log("Player init")
    }

    fixedUpdate(ts) {
        this.CheckInput()

        var camEntity = Scene.GetEntity("Camera")
        var cam = camEntity.GetComponent("Camera")

        var dir = cam.GetDirection()
        dir.y = 0
        dir = dir.Normalize()

        var right = cam.GetRight()
        right.y = 0
        right = right.Normalize()

        var move = _InputDir
        var new_Velocity = (dir * move.z) + (right * move.x)

        var controller = this.GetComponent("CharacterController")
        var transform = this.GetComponent("Transform")
        
        var from = transform.GetTranslation()
        var to = from - Vector3.new(0, 1, 0)

        var normal = Physics.Raycast(from, to).Normal
 
        if(!controller.IsOnGround())  {
            _Velocity.y = _Velocity.y -_Gravity
            //_Velocity.x = _Velocity.x * _AirDeccel
            //_Velocity.z = _Velocity.z * _AirDeccel
        } else {
            _Velocity.y = -10
            

            if(Input.IsKeyPressed(32)) _Velocity.y = _Jump
                

            _Velocity.x = _Velocity.x + new_Velocity.x * _Accel
            _Velocity.z = _Velocity.z + new_Velocity.z * _Accel
            _Velocity.x = _Velocity.x * _Deccel
            _Velocity.z = _Velocity.z * _Deccel
        }

        controller.MoveAndSlide(_Velocity * ts)
    }

    CheckInput() {
        /* 
        87 = W 
        65 = A
        83 = S
        68 = D
        32 = SPACE
        */
        if(Input.IsKeyDown(87)) {
            _InputDir.z = 1
        } else if(Input.IsKeyDown(83)) {
            _InputDir.z = -1
        } else {
            _InputDir.z = 0
        }
        if(Input.IsKeyDown(65)) {
            _InputDir.x = 1
        } else if(Input.IsKeyDown(68)) {
            _InputDir.x = -1
        } else {
            _InputDir.x = 0
        }

    }

    exit() {
        Engine.Log("Player exit")
    }
}