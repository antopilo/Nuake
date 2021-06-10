class Engine { 
	foreign static Log(msg)
}

class Vector3 {
	construct new(x, y, z) {
		_x = x
		_y = y
		_z = z
	}
}

class Entity {
	foreign GetComponent(type)
}