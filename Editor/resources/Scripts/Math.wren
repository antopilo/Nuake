class Math {
	foreign static Sqrt_(x, y, z)
}

class Vector3 {
	construct new(x, y, z) {
		_x = x
		_y = y
		_z = z
	}

	Sqrt() {
		return Math.Sqrt_(_x, _y, _z)
	}

	Normalize() {
		var length = this.Sqrt()
		var x = _x / length
		var y = _y / length
		var z = _z / length
		return Vector3.new(x, y, z)
	}
}