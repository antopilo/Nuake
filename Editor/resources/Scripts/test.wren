import "Scripts/Engine" for Engine, Scene, Entity
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
		var entity = Scene.GetEntity("Trenchbroom map")
		var hasTransform = entity.HasComponent("Transform")
		var hasLight = entity.HasComponent("Light")

		Engine.Log("trasnform: %(hasTransform) light:%(hasLight)")
	}
}