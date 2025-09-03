import "Nuake:Math" for Vector3
import "Nuake:Engine" for Engine

class Keys {
    static SPACE { 32 }
    static APOSTROPHE { 39 }  /* ' */
    static COMMA { 44 }  /* } */
    static MINUS {  5 }  /* - */
    static PERIOD { 46 } /* . */
    static SLASH { 47 }  /* / */
    static NUM0 { 48 }
    static NUM1 { 49 }
    static NUM2 { 50 }
    static NUM3 { 51 }
    static NUM4 { 52 }
    static NUM5 { 53 }
    static NUM6 { 54 }
    static NUM7 { 55 }
    static NUM8 { 56 }
    static NUM9 { 57 }
    static SEMICOLON { 59 }  /* ; */
    static EQUAL { 61 } /* {  */
    static A { 65 }
    static B { 66 }
    static C { 67 }
    static D { 68 }
    static E { 69 }
    static F { 70 }
    static G { 71 }
    static H { 72 }
    static I { 73 }
    static J { 74 }
    static K { 75 }
    static L { 76 }
    static M { 77 }
    static N { 78 }
    static O { 79 }
    static P { 80 }
    static Q { 81 }
    static R { 82 }
    static S { 83 }
    static T { 84 }
    static U { 85 }
    static V { 86 }
    static W { 87 }
    static X { 88 }
    static Y { 89 }
    static Z { 90 }
    static LEFT_BRACKET { 91 }  /* [ */
    static BACKSLASH { 92 }  /* \ */
    static RIGHT_BRACKET { 93 }  /* ] */
    static GRAVE_ACCENT { 96 }  /* ` */
    static WORLD_1 { 161 }/* non-US #1 */
    static WORLD_2 { 162 }/* non-US #2 */
    static ESCAPE { 256 }
    static ENTER { 257 }
    static TAB { 258 }
    static BACKSPACE { 259 }
    static INSERT { 260 }
    static DELETE { 261 }
    static RIGHT { 262 }
    static LEFT { 263 }
    static DOWN { 264 }
    static UP { 265 }
    static PAGE_UP { 266 }
    static PAGE_DOWN { 267 }
    static HOME { 268 }
    static END { 269 }
    static CAPS_LOCK { 280 }
    static SCROLL_LOCK { 281 }
    static NUM_LOCK { 282 }
    static PRINT_SCREEN { 283 }
    static PAUSE { 284 }
    static F1 { 290 }
    static F2 { 291 }
    static F3 { 292 }
    static F4 { 293 }
    static F5 { 294 }
    static F6 { 295 }
    static F7 { 296 }
    static F8 { 297 }
    static F9 { 298 }
    static F10 { 299 }
    static F11 { 300 }
    static F12 { 301 }
    static F13 { 302 }
    static F14 { 303 }
    static F15 { 304 }
    static F16 { 305 }
    static F17 { 306 }
    static F18 { 307 }
    static F19 { 308 }
    static F20 { 309 }
    static F21 { 310 }
    static F22 { 311 }
    static F23 { 312 }
    static F24 { 313 }
    static F25 { 314 }
    static KP_0 { 320 }
    static KP_1 { 321 }
    static KP_2 { 322 }
    static KP_3 { 323 }
    static KP_4 { 324 }
    static KP_5 { 325 }
    static KP_6 { 326 }
    static KP_7 { 327 }
    static KP_8 { 328 }
    static KP_9 { 329 }
    static KP_DECIMAL { 330 }
    static KP_DIVIDE { 331 }
    static KP_MULTIPLY { 332 }
    static KP_SUBTRACT { 333 }
    static KP_ADD {  334 }
    static KP_ENTER {  335 }
    static KP_EQUAL {  336 }
    static LEFT_SHIFT {  340 }
    static LEFT_CONTROL { 341 }
    static LEFT_ALT { 342 }
    static LEFT_SUPER { 343 }
    static RIGHT_SHIFT { 344 }
    static RIGHT_CONTROL { 345 }
    static RIGHT_ALT { 346 }
    static RIGHT_SUPER { 347 }
    static MENU { 348 }
}

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