import "Scripts/Engine" for Engine
import "Scripts/Input" for Input
import "Scripts/Scene" for Scene, Entity

class Test {
	init() {
		System.print("hello init")
	}

	update(t) {
		System.print("hello update %(t)")
	}

	exit() {
		System.print("hello exit ")
	}

	static hello() {
		var entity = Scene.GetEntity("Light")
		var light = entity.GetComponent("Light")
		light.SetIntensity(1.0)

		if(Input.IsMouseButtonPressed(2) == true) {
			Engine.Log("RIGHT CLICK!!!!!!")
		}
	}
}