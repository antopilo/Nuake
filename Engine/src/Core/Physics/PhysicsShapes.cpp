#include "PhysicsShapes.h"
#include <btBulletDynamicsCommon.h>
Physics::Box::Box()
{
	Size = glm::vec3(1);
	bShape = new btBoxShape(btVector3(Size.x, Size.y, Size.z));
	m_Type = BOX;
}
// Sphere
Physics::Box::Box(glm::vec3 size) {
	Size = size;
	bShape = new btBoxShape(btVector3(size.x, size.y, size.z));
	m_Type = BOX;
}

Physics::Box::Box(float x, float y, float z) {
	Size = glm::vec3(x, y, z);
	bShape = new btBoxShape(btVector3(x, y, z));
	m_Type = BOX;
}

btCollisionShape* Physics::Box::GetBulletShape() 
{ 
	return bShape; 
}

// Sphere
Physics::Sphere::Sphere(float radius) {
	Radius = radius;
	bShape = new btSphereShape(Radius);
	
	m_Type = SPHERE;
}

void Physics::Sphere::SetRadius(float radius) {
	((btSphereShape*)bShape)->setUnscaledRadius(radius);
	Radius = radius;
}

btCollisionShape* Physics::Sphere::GetBulletShape()
{
	return bShape;
}

Physics::MeshShape::MeshShape(Ref<Mesh> mesh)
{
	m_Mesh = mesh;
	btConvexHullShape* trimesh = new btConvexHullShape();

	for (Vertex i : mesh->m_Vertices)
	{
		trimesh->addPoint(btVector3(i.position.x, i.position.y, i.position.z));
	}
	bShape = trimesh;
}

btCollisionShape* Physics::MeshShape::GetBulletShape()
{
	return bShape;
}