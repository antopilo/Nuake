class Math {
	foreign static Sqrt_(x, y, z)
}

class Vector3 {

	x {_x}
	y {_y}
	z {_z}
	x=(value) {
		_x = value
	}
	y=(value) {
		_y = value
	}
	z=(value) {
		_z = value
	}

	mul(other) {
		if(other is Vector3) {
			return Vector3.new(_x * other.x, 
								_y * other.y,
								_z * other.z)
		} else {
			return Vector3.new(_x * other, _y * other, _z * other)
		}
	}

	*(other) {
		if(other is Vector3) {
			return Vector3.new(_x * other.x, 
								_y * other.y,
								_z * other.z)
		} else if(other is Num) {
			return Vector3.new(_x * other, _y * other, _z * other)
		}
	}

	+(other) {
		if(other is Vector3) {
			return Vector3.new(_x + other.x, 
								_y + other.y, 
								_z + other.z)
		} else if(other is Num) {
			return Vector3.new(_x + other, 
								_y + other, 
								_z + other)
		}
	}

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