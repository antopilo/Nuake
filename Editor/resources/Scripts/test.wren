import "Scripts/Engine" for Engine
import "Scripts/Scene" for Scene, Entity

class Test {
	construct new() {}

	init() {
		System.print("hello init")
	}

	// Gets called on the click event.
	static hello() {
		// Get the entity named Light
		var entity = Scene.GetEntity("Light")

		// Get the component light
		var light = entity.GetComponent("Light")
		//light.SetIntensity(222.0) // Change intensity

		var currentIntensity = light.GetIntensity() // Get new intensity
		Engine.Log("Current intensity %(currentIntensity)")
	}
}
