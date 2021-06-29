import "Scripts/ScriptableEntity" for ScriptableEntity
import "Scripts/Engine" for Engine
import "Scripts/Scene" for Scene
import "Scripts/Math" for Vector3, Math
import "Scripts/Input" for Input

class CamScript is ScriptableEntity {
    construct new() {
       _Pitch = 0
       _Yaw = 0

       _mouseLastX = 0
       _mouseLastY = 0

        _BobHeight = 0.1
        _BobSpeed = 5.0
        _CamHeight = 0.5

        _deltaTime = 0

        Input.HideMouse()
    }

    init() {}

    update(ts) {
        _deltaTime = _deltaTime + ts
        var x = Input.GetMouseX()
        var y = Input.GetMouseY()

        var diffx = x - _mouseLastX
        var diffy =  _mouseLastY - y
        _mouseLastX = x
        _mouseLastY = y

        var sens = 0.1

        diffx = diffx * sens
        diffy = diffy * sens

        _Yaw = _Yaw + diffx
        _Pitch = _Pitch + diffy

        if(_Pitch > 89) _Pitch = 89
        if(_Pitch < -89) _Pitch = -89

        var cam = this.GetComponent("Camera")

        var rad_yaw = Math.Radians(_Yaw)
        var rad_pitch = Math.Radians(_Pitch)
        var camX = Math.Cos(rad_yaw) * Math.Cos(rad_pitch)
        var camY = Math.Sin(rad_pitch)
        var camZ = Math.Sin(rad_yaw) * Math.Cos(rad_pitch)

        var newDir = Vector3.new(camX, camY, camZ)


        var player = Scene.GetEntity("Player")

        var playerScript = player.GetComponent("Script")
        var velocity = playerScript.Velocity.Duplicate()
        velocity.y = 0 // Dont count gravity in bob.
        velocity = velocity.Sqrt()

        var amount = (Math.Sin(_deltaTime * (velocity * 0.01)) / 2 + 1)  * _BobHeight
        //Engine.Log("BobAmount: %(amount)")

        var pTransform = player.GetComponent("Transform")
        var pPos = pTransform.GetTranslation()
        if(amount < 1 && amount > -1) amount = 0
            
        var transform = this.GetComponent("Transform")
        var newPos = Vector3.new(pPos.x, pPos.y + _CamHeight + amount , pPos.z)
        transform.SetTranslation(newPos)

        cam.SetDirection(newDir)
    }

    exit() {}
}