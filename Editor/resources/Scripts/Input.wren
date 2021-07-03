import "Nuake:Math" for Vector3
import "Nuake:Engine" for Engine
class Input {
    foreign static GetMouseX()
    foreign static GetMouseY()

    // Gets the mouse position of X & Y and returns a Vector3
    static GetMousePos() {
        var result = Vector3.new(this.GetMouseX_(), this.GetMouseY_(), 0)
        return result
    }

    // Keys
    foreign static IsKeyDown_(key)
    static IsKeyDown(key) {
        if(key is Num) {
            return this.IsKeyDown_(key)
        }
        Engine.Log("IsKeyDown expects a number. Got: %(key.type)")
    }
    
    foreign static IsKeyPressed_(key)
    static IsKeyPressed(key) {
        if(key is Num){
            return this.IsKeyPressed_(key)
            }
            
        Engine.Log("IsKeyPressed expects a number. Got: %(key.type)")
    }

    foreign static IsKeyReleased_(key)
    static IsKeyReleased(key) {
        if(key is Num) {
            return this.IsKeyReleased_(key)
            }
            
        Engine.Log("IsKeyReleased expects a number. Got: %(key.type)")
    }

    // Mouse
    foreign static IsMouseButtonDown_(button)
    static IsMouseButtonDown(button) {
        if(button is Num){ 
            return this.IsMouseButtonDown_(button)
            }
            
    }

    foreign static IsMouseButtonPressed_(button)
    static IsMouseButtonPressed(button) {
        if(button is Num) {
            Engine.Log("IsmouseButtonPressed: %(button)")
            return this.IsMouseButtonPressed_(button)
        }
            
    }

    foreign static IsMouseButtonReleased_(button)
    static IsMouseButtonReleased(button) {
        if(button is Num){
            return this.IsMouseButtonReleased_(button)
            }
            
    }

    foreign static HideMouse()
    foreign static ShowMouse()
    foreign static IsMouseHidden()
    
}