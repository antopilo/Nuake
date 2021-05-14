#include "PhysicsShapes.h"
#include "Rigibody.h"
#include <btBulletDynamicsCommon.h>
#include "../Core.h"
#include <glm/trigonometric.hpp>
namespace Physics
{
	RigidBody::RigidBody()
	{
		m_Transform = new btTransform();
		m_Transform->setIdentity();
		//m_Transform->setOrigin(btVector3(position.x, position.y, position.z));

		
	}
	RigidBody::RigidBody(glm::vec3 position)
	{
		Ref<Box> shape = CreateRef<Box>();
		m_CollisionShape = shape;

		m_Transform = new btTransform();
		m_Transform->setIdentity();
		m_Transform->setOrigin(btVector3(position.x, position.y, position.z));

		m_Mass = 0.0f;

		//rigidbody is dynamic if and only if mass is non zero, otherwise static
		m_IsDynamic = (m_Mass != 0.0f);

		btVector3 localInertia(0, 0, 0);
		if (m_IsDynamic)
			m_CollisionShape->GetBulletShape()->calculateLocalInertia(m_Mass, localInertia);

		m_InitialVel = glm::vec3(0, 0, 0);

		//using motionstate is optional, it provides interpolation capabilities, and only synchronizes 'active' objects
		btDefaultMotionState* myMotionState = new btDefaultMotionState(*m_Transform);
		btRigidBody::btRigidBodyConstructionInfo rbInfo(m_Mass, myMotionState, m_CollisionShape->GetBulletShape(), localInertia);

		m_Rigidbody = new btRigidBody(rbInfo);
	}

	RigidBody::RigidBody(float mass, glm::vec3 position, Ref<PhysicShape> shape, glm::vec3 initialVel)
	{
		m_CollisionShape = shape;

		m_Transform = new btTransform();
		m_Transform->setIdentity();
		m_Transform->setOrigin(btVector3(position.x, position.y, position.z));

		m_Mass = mass;

		//rigidbody is dynamic if and only if mass is non zero, otherwise static
		m_IsDynamic = (m_Mass != 0.0f);

		btVector3 localInertia(initialVel.x, initialVel.y, initialVel.z);
		m_InitialVel = initialVel;
		if (m_IsDynamic)
			m_CollisionShape->GetBulletShape()->calculateLocalInertia(m_Mass, localInertia);

		//using motionstate is optional, it provides interpolation capabilities, and only synchronizes 'active' objects
		btDefaultMotionState* myMotionState = new btDefaultMotionState(*m_Transform);
		btRigidBody::btRigidBodyConstructionInfo rbInfo(m_Mass, myMotionState, m_CollisionShape->GetBulletShape(), localInertia);

		m_Rigidbody = new btRigidBody(rbInfo);
		
	}

	void RigidBody::SetShape(Ref<PhysicShape> shape)
	{
		m_Rigidbody->setCollisionShape(shape->GetBulletShape());
		m_CollisionShape = shape;
	}

	void RigidBody::UpdateTransform(btTransform t)
	{
		m_Transform->setOrigin(t.getOrigin());
		m_Transform->setRotation(t.getRotation());
		m_Rigidbody->setWorldTransform(t);
	}

	glm::vec3 RigidBody::GetRotation() const {
		auto q = m_Transform->getRotation();
		btScalar x = 0, y = 0, z = 0;
		q.getEulerZYX(z, y, x);
		return glm::vec3(glm::degrees(x), glm::degrees(y), glm::degrees(z));
	}

	void RigidBody::SetKinematic(bool value)
	{
		if (value) // Kinematic bodies dont deactivate.
		{
			m_Rigidbody->setCollisionFlags(m_Rigidbody->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
			m_Rigidbody->setActivationState(DISABLE_DEACTIVATION);
		}
		else // Reenable deactivation.
		{
			m_Rigidbody->setCollisionFlags(m_Rigidbody->getCollisionFlags() ^ ~btCollisionObject::CF_KINEMATIC_OBJECT);
			m_Rigidbody->setActivationState(WANTS_DEACTIVATION);
		}
	}

	glm::vec3 RigidBody::GetPosition() const {
		btVector3 btPos = m_Transform->getOrigin();

		return glm::vec3(btPos.x(), btPos.y(), btPos.z());
	}

	void RigidBody::SetMass(float m) { m_Rigidbody->setMassProps(m, btVector3(m_InitialVel.x, m_InitialVel.y, m_InitialVel.y)); m_Mass = m; }
	void RigidBody::MoveAndSlide(glm::vec3 velocity)
	{

	}
}