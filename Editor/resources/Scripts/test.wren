import "Scripts/Engine" for Engine
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
		Engine.Log("Hello from foreign")
	}
}