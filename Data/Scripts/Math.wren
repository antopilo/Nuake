class Math {
	foreign static Sqrt_(x, y, z)
	foreign static Sin(s)
	foreign static Cos(s)
	foreign static Radians(s)
	foreign static Degrees(s)

	static Cross(vec1, vec2) {
		var result = this.Cross_(vec1.x, vec1.y, vec1.z, vec2.x, vec2.y, vec2.z)
		return Vector3.new(result[0], result[1], result[2])
	}

	static Dot(vec1, vec2) {
		return this.Dot_(vec1.x, vec1.y, vec1.z, vec2.x, vec2.y, vec2.z)
	}

	static Lerp(a, b, t) {
		return a + t * (b - a)
	}

	foreign static Dot_(x, y, z, x1, y2, z2)
	foreign static Cross_(x, y, z, x1, y2, z2)
	foreign static Length_(x, y, z)
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

	/(other) {
		if(other is Vector3) {
			return Vector3.new(_x / other.x, 
								_y / other.y,
								_z / other.z)
		} else if(other is Num) {
			return Vector3.new(_x / other, _y / other, _z / other)
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

	-(other) {
		if(other is Vector3) {
			return Vector3.new(_x - other.x, 
								_y - other.y, 
								_z - other.z)
		} else if(other is Num) {
			return Vector3.new(_x - other, 
								_y - other, 
								_z - other)
		}
	}

	construct new(x, y, z) {
		_x = x
		_y = y
		_z = z
	}

	Duplicate() {
		return Vector3.new(_x, _y, _z)
	}

	Sqrt() {
		return Math.Sqrt_(_x, _y, _z)
	}

	Cross(vec) {
		return Math.Cross(this, vec)
	}

	Normalize() {
		var length = this.Length()
		if(length > 0.0) {
			var x = _x / length
			var y = _y / length
			var z = _z / length
			return Vector3.new(x, y, z)
		}
		
		return Vector3.new(0, 0, 0)
	}

	Angle(vec) {
		this.Normalize() * vec.Normalize()
	}

	Dot(vec) {
		return Math.Dot(this, vec)
	}

	Length() {
		return Math.Length_(_x, _y, _z)
	}
}