import "Scripts/Math" for Vector3

class CollisionResult {

    LocalPosition { _Local}
    WorldPosition { _World}
    Normal { _Normal}

    construct new(l, w, n) {
        _Local = l
        _World = w
        _Normal = n
    }
}

class Physics {
    foreign static Raycast_(x, y, z, x2, y2, z2)

    static Raycast(v1, v2) {
        var result = this.Raycast_(v1.x, v1.y, v1.z, v2.x, v2.y, v2.z)

        var local = Vector3.new(result[0], result[1], result[2])
        var world = Vector3.new(result[3], result[4], result[5])
        var normal = Vector3.new(result[6], result[7], result[8])

        return CollisionResult.new(local, world, normal)
    }
}