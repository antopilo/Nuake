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
       Input.HideMouse()
    }

    init() {
       
    }

    update(ts) {
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

        cam.SetDirection(newDir)
    }

    CheckInput() {
        
    }

    exit() {
    }
}