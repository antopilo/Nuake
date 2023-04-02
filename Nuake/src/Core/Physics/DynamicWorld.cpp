#include "DynamicWorld.h"
#include "Rigibody.h"
#include "../Core/Core.h"
#include <src/Vendors/glm/ext/quaternion_common.hpp>
#include <src/Core/Logger.h>

#include <src/Core/Physics/PhysicsShapes.h>

#include "src/Vendors/glm/gtx/matrix_decompose.hpp"

#include <Jolt/Jolt.h>
#include <Jolt/RegisterTypes.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/Collision/Shape/CylinderShape.h>
#include <Jolt/Physics/Collision/Shape/MeshShape.h>
#include <Jolt/Physics/Collision/Shape/ConvexHullShape.h>

#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Body/BodyActivationListener.h>
#include <Jolt/Physics/Character/Character.h>

namespace Nuake
{
	// Callback for traces, connect this to your own trace function if you have one
	static void TraceImpl(const char* inFMT, ...)
	{
		// Format the message
		va_list list;
		va_start(list, inFMT);
		char buffer[1024];
		vsnprintf(buffer, sizeof(buffer), inFMT, list);

		// Print to the TTY
		std::cout << buffer << std::endl;
	}

#ifdef JPH_ENABLE_ASSERTS

	// Callback for asserts, connect this to your own assert handler if you have one
	static bool AssertFailedImpl(const char* inExpression, const char* inMessage, const char* inFile, uint32_t inLine)
	{
		// Print to the TTY
		std::cout << inFile << ":" << inLine << ": (" << inExpression << ") " << (inMessage != nullptr ? inMessage : "") << std::endl;

		// Breakpoint
		return true;
	};

#endif // JPH_ENABLE_ASSERTS

	// Layer that objects can be in, determines which other objects it can collide with
	// Typically you at least want to have 1 layer for moving bodies and 1 layer for static bodies, but you can have more
	// layers if you want. E.g. you could have a layer for high detail collision (which is not used by the physics simulation
	// but only if you do collision testing).
	namespace Layers
	{
		static constexpr uint8_t NON_MOVING = 0;
		static constexpr uint8_t MOVING = 1;
		static constexpr uint8_t NUM_LAYERS = 2;
	};

	// Function that determines if two object layers can collide
	static bool MyObjectCanCollide(JPH::ObjectLayer inObject1, JPH::ObjectLayer inObject2)
	{
		switch (inObject1)
		{
		case Layers::NON_MOVING:
			return inObject2 == Layers::MOVING; // Non moving only collides with moving
		case Layers::MOVING:
			return true; // Moving collides with everything
		default:
			//JPH_ASSERT(false);
			return false;
		}
	};

	// Each broadphase layer results in a separate bounding volume tree in the broad phase. You at least want to have
	// a layer for non-moving and moving objects to avoid having to update a tree full of static objects every frame.
	// You can have a 1-on-1 mapping between object layers and broadphase layers (like in this case) but if you have
	// many object layers you'll be creating many broad phase trees, which is not efficient. If you want to fine tune
	// your broadphase layers define JPH_TRACK_BROADPHASE_STATS and look at the stats reported on the TTY.
	namespace BroadPhaseLayers
	{
		static constexpr JPH::BroadPhaseLayer NON_MOVING(0);
		static constexpr JPH::BroadPhaseLayer MOVING(1);
		static constexpr uint32_t NUM_LAYERS(2);
	};

	// BroadPhaseLayerInterface implementation
	// This defines a mapping between object and broadphase layers.
	class BPLayerInterfaceImpl final : public JPH::BroadPhaseLayerInterface
	{
	public:
		BPLayerInterfaceImpl()
		{
			// Create a mapping table from object to broad phase layer
			mObjectToBroadPhase[Layers::NON_MOVING] = BroadPhaseLayers::NON_MOVING;
			mObjectToBroadPhase[Layers::MOVING] = BroadPhaseLayers::MOVING;
		}

		virtual JPH::uint GetNumBroadPhaseLayers() const override
		{
			return BroadPhaseLayers::NUM_LAYERS;
		}

		virtual JPH::BroadPhaseLayer GetBroadPhaseLayer(JPH::ObjectLayer inLayer) const override
		{
			using namespace JPH;
			JPH_ASSERT(inLayer < Layers::NUM_LAYERS);
			return mObjectToBroadPhase[inLayer];
		}

#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
		virtual const char* GetBroadPhaseLayerName(BroadPhaseLayer inLayer) const override
		{
			switch ((BroadPhaseLayer::Type)inLayer)
			{
			case (BroadPhaseLayer::Type)BroadPhaseLayers::NON_MOVING:	return "NON_MOVING";
			case (BroadPhaseLayer::Type)BroadPhaseLayers::MOVING:		return "MOVING";
			default:													JPH_ASSERT(false); return "INVALID";
			}
		}
#endif // JPH_EXTERNAL_PROFILE || JPH_PROFILE_ENABLED

	private:
		JPH::BroadPhaseLayer					mObjectToBroadPhase[Layers::NUM_LAYERS];
	};

	// Function that determines if two broadphase layers can collide
	static bool MyBroadPhaseCanCollide(JPH::ObjectLayer inLayer1, JPH::BroadPhaseLayer inLayer2)
	{
		using namespace JPH;
		switch (inLayer1)
		{
		case Layers::NON_MOVING:
			return inLayer2 == BroadPhaseLayers::MOVING;
		case Layers::MOVING:
			return true;
		default:
			JPH_ASSERT(false);
			return false;
		}
	}

	// An example contact listener
	class MyContactListener : public JPH::ContactListener
	{
	public:
		// See: ContactListener
		virtual JPH::ValidateResult	OnContactValidate(const JPH::Body& inBody1, const JPH::Body& inBody2, const JPH::CollideShapeResult& inCollisionResult) override
		{
			std::cout << "Contact validate callback" << std::endl;

			// Allows you to ignore a contact before it is created (using layers to not make objects collide is cheaper!)
			return JPH::ValidateResult::AcceptAllContactsForThisBodyPair;
		}

		virtual void OnContactAdded(const JPH::Body& inBody1, const JPH::Body& inBody2, const JPH::ContactManifold& inManifold, JPH::ContactSettings& ioSettings) override
		{
			std::cout << "A contact was added" << std::endl;
		}

		virtual void OnContactPersisted(const JPH::Body& inBody1, const JPH::Body& inBody2, const JPH::ContactManifold& inManifold, JPH::ContactSettings& ioSettings) override
		{
			std::cout << "A contact was persisted" << std::endl;
		}

		virtual void OnContactRemoved(const JPH::SubShapeIDPair& inSubShapePair) override
		{
			std::cout << "A contact was removed" << std::endl;
		}
	};

	// An example activation listener
	class MyBodyActivationListener : public JPH::BodyActivationListener
	{
	public:
		virtual void OnBodyActivated(const JPH::BodyID& inBodyID, JPH::uint64 inBodyUserData) override
		{
			std::cout << "A body got activated" << std::endl;
		}

		virtual void OnBodyDeactivated(const JPH::BodyID& inBodyID, JPH::uint64 inBodyUserData) override
		{
			std::cout << "A body went to sleep" << std::endl;
		}
	};

	BPLayerInterfaceImpl JoltBroadphaseLayerInterface = BPLayerInterfaceImpl();
	namespace Physics
	{
		DynamicWorld::DynamicWorld() : _stepCount(0)
		{
			const uint32_t MaxBodies = 1024;
			const uint32_t NumBodyMutexes = 0;
			const uint32_t MaxBodyPairs = 1024;
			const uint32_t MaxContactConstraints = 1024;

			_JoltPhysicsSystem = CreateRef<JPH::PhysicsSystem>();
			_JoltPhysicsSystem->Init(MaxBodies, NumBodyMutexes, MaxBodyPairs, MaxContactConstraints, JoltBroadphaseLayerInterface, MyBroadPhaseCanCollide, MyObjectCanCollide);

			// A body activation listener gets notified when bodies activate and go to sleep
			// Note that this is called from a job so whatever you do here needs to be thread safe.
			// Registering one is entirely optional.
			_bodyActivationListener = CreateScope<MyBodyActivationListener>();
			_JoltPhysicsSystem->SetBodyActivationListener(_bodyActivationListener.get());

			// A contact listener gets notified when bodies (are about to) collide, and when they separate again.
			// Note that this is called from a job so whatever you do here needs to be thread safe.
			// Registering one is entirely optional.
			_contactListener = CreateScope<MyContactListener>();
			_JoltPhysicsSystem->SetContactListener(_contactListener.get());

			// The main way to interact with the bodies in the physics system is through the body interface. There is a locking and a non-locking
			// variant of this. We're going to use the locking version (even though we're not planning to access bodies from multiple threads)
			_JoltBodyInterface = &_JoltPhysicsSystem->GetBodyInterface();

			// Optional step: Before starting the physics simulation you can optimize the broad phase. This improves collision detection performance (it's pointless here because we only have 2 bodies).
			// You should definitely not call this every frame or when e.g. streaming in a new level section as it is an expensive operation.
			// Instead insert all new objects in batches instead of 1 at a time to keep the broad phase efficient.
			//_JoltPhysicsSystem->OptimizeBroadPhase();
			_JoltJobSystem = new JPH::JobSystemThreadPool(JPH::cMaxPhysicsJobs, JPH::cMaxPhysicsBarriers, std::thread::hardware_concurrency() - 1);
		}

		void DynamicWorld::DrawDebug()
		{

		}

		void DynamicWorld::SetGravity(glm::vec3 g)
		{
		}

		void DynamicWorld::AddRigidbody(Ref<RigidBody> rb)
		{
			JPH::BodyInterface& bodyInterface = _JoltPhysicsSystem->GetBodyInterface();

			JPH::ShapeSettings::ShapeResult shapeResult;
			auto rbShape = rb->GetShape();
			switch (rbShape->GetType())
			{
			case RigidbodyShapes::BOX:
			{
				Box* box = (Box*)rbShape.get();
				const Vector3& boxSize = box->GetSize();
				JPH::BoxShapeSettings shapeSettings(JPH::Vec3(boxSize.x, boxSize.y, boxSize.z));
				shapeResult = shapeSettings.Create();
			}
					break;
			case RigidbodyShapes::SPHERE:
			{
				Sphere* sphere = (Sphere*)rbShape.get();
				const float sphereRadius = sphere->GetRadius();
				JPH::SphereShapeSettings shapeSettings(sphereRadius);
				shapeResult = shapeSettings.Create();
			}
				break;
			case RigidbodyShapes::CAPSULE:
			{
				Capsule* capsule = (Capsule*)rbShape.get();
				const float radius = capsule->GetRadius();
				const float height = capsule->GetHeight();
				JPH::CapsuleShapeSettings shapeSettings(height / 2.0f, radius);
				shapeResult = shapeSettings.Create();
			}
				break;
			case RigidbodyShapes::CYLINDER:
			{
				Cylinder* capsule = (Cylinder*)rbShape.get();
				const float radius = capsule->GetRadius();
				const float height = capsule->GetHeight();
				JPH::CylinderShapeSettings shapeSettings(height / 2.0f, radius);
				shapeResult = shapeSettings.Create();
			}
			break;
			case RigidbodyShapes::MESH:
			{
				MeshShape* meshShape = (MeshShape*)rbShape.get();
				const auto& mesh = meshShape->GetMesh();
				const auto& vertices = mesh->GetVertices();
				const auto& indices = mesh->GetIndices();
				
				JPH::TriangleList triangles;
				triangles.reserve(indices.size());
				Matrix4 transform = rb->_transform;
				transform[3] = Vector4(0, 0, 0, 1.0f);
				for (int i = 0; i < indices.size() - 3; i += 3)
				{
					const Vector3& p1 = vertices[indices[i]].position;
					const Vector3& p2 = vertices[indices[i + 1]].position;
					const Vector3& p3 = vertices[indices[i + 2]].position;

					const Vector4& tp1 = transform * Vector4(p1, 1.0f);
					const Vector4& tp2 = transform * Vector4(p2, 1.0f);
					const Vector4& tp3 = transform * Vector4(p3, 1.0f);
					triangles.push_back(JPH::Triangle(JPH::Float3(tp1.x, tp1.y, tp1.z), JPH::Float3(tp2.x, tp2.y, tp2.z), JPH::Float3(tp3.x, tp3.y, tp3.z)));
				}

				JPH::MeshShapeSettings shapeSettings(std::move(triangles));
				
				shapeResult = shapeSettings.Create();
			}
			break;
			case CONVEX_HULL:
			{
				ConvexHullShape* shape = (ConvexHullShape*)rbShape.get();

				const auto& hullPoints = shape->GetPoints();
				JPH::Array<JPH::Vec3> points;
				points.reserve(std::size(hullPoints));
				for (const auto& p : hullPoints)
				{
					points.push_back(JPH::Vec3(p.x, p.y, p.z));
				}

				JPH::ConvexHullShapeSettings shapeSettings(points);
				shapeResult = shapeSettings.Create();
			}
			break;
			}

			JPH::EMotionType motionType = JPH::EMotionType::Static;

			float mass = rb->_mass;
			// According to jolt documentation, Mesh shapes should only be static.
			if (mass > 0.0f && rb->GetShape()->GetType() != MESH)
			{
				motionType = JPH::EMotionType::Dynamic;
			}

			const auto& startPos = rb->GetPosition();
			const auto& joltPos = JPH::Vec3(startPos.x, startPos.y, startPos.z);
			JPH::BodyCreationSettings bodySettings(shapeResult.Get(), joltPos, JPH::Quat::sIdentity(), motionType, Layers::MOVING);

			if (mass > 0.0f)
			{
				bodySettings.mOverrideMassProperties = JPH::EOverrideMassProperties::CalculateInertia;
				bodySettings.mMassPropertiesOverride.mMass = mass;
			}
			
			bodySettings.mUserData = rb->GetEntity().GetID();
			// Create the actual rigid body
			JPH::BodyID floor = _JoltBodyInterface->CreateAndAddBody(bodySettings, JPH::EActivation::Activate); // Note that if we run out of bodies this can return nullptr
			_registeredBodies.push_back((uint32_t)floor.GetIndexAndSequenceNumber());
		}

		void DynamicWorld::AddGhostbody(Ref<GhostObject> gb)
		{
		}

		void DynamicWorld::AddCharacterController(Ref<CharacterController> cc)
		{
			auto settings = new JPH::CharacterSettings();
			settings->mMaxSlopeAngle = JPH::DegreesToRadians(45.0f);
			settings->mLayer = Layers::MOVING;
			settings->mFriction = 0.5f;

			// Shape here
		}

		RaycastResult DynamicWorld::Raycast(glm::vec3 from, glm::vec3 to)
		{
			Vector3 localNorm = glm::vec3(0,0,0);

			//Logger::Log("normal: x:" + std::to_string(localNorm.x) + " y:" + std::to_string(localNorm.y )+ "z: " + std::to_string(localNorm.z));
			
			// Map bullet result to dto.
			RaycastResult result{
				glm::vec3(0,0,0),
				glm::vec3(0,0,0),
				localNorm
			};

			return result;
		}

		void DynamicWorld::StepSimulation(Timestep ts)
		{
			// Next step
			++_stepCount;
			const auto& bodyInterface = _JoltPhysicsSystem->GetBodyInterface();

			for (const auto& body : _registeredBodies)
			{
				JPH::BodyID bodyId = static_cast<JPH::BodyID>(body);
				JPH::Vec3 position = bodyInterface.GetCenterOfMassPosition(bodyId);
				JPH::Vec3 velocity = bodyInterface.GetLinearVelocity(bodyId);
				JPH::Mat44 joltTransform = bodyInterface.GetWorldTransform(bodyId);

				Matrix4 transform = glm::mat4(
					joltTransform(0, 0), joltTransform(1, 0), joltTransform(2, 0), joltTransform(3, 0),
					joltTransform(0, 1), joltTransform(1, 1), joltTransform(2, 1), joltTransform(3, 1),
					joltTransform(0, 2), joltTransform(1, 2), joltTransform(2, 2), joltTransform(3, 2),
					joltTransform(0, 3), joltTransform(1, 3), joltTransform(2, 3), joltTransform(3, 3)
				);

				Vector3 scale = Vector3();
				Quat rotation = Quat();
				Vector3 pos = Vector3();
				Vector3 skew = Vector3();
				Vector4 pesp = Vector4();
				glm::decompose(transform, scale, rotation, pos, skew, pesp);

				uint32_t entId = bodyInterface.GetUserData(bodyId);
				Entity entity = Engine::GetCurrentScene()->GetEntityByID(entId);

				const std::string& name = entity.GetComponent<NameComponent>().Name;
				TransformComponent& transformComponent = entity.GetComponent<TransformComponent>();
				//transformComponent.GlobalTransform = transform;
				transformComponent.SetLocalPosition(pos);
				transformComponent.Dirty = false;
				transformComponent.SetLocalRotation(rotation);
				//transformComponent.SetLocalScale(scale);

				Matrix4 newTransform = Matrix4(1.0f);
				newTransform = glm::translate(newTransform, pos);
				newTransform = newTransform * glm::toMat4(rotation);
				newTransform = glm::scale(newTransform, transformComponent.GetGlobalScale());

				transformComponent.SetLocalTransform(transform);

				const std::string& posStr = "(" + std::to_string(pos.x) + ", " + std::to_string(pos.y) + ", " + std::to_string(pos.z) + ")";
				const std::string& logMsg = "Physics pos: " + name + " at " + posStr;
				Logger::Log(logMsg);
			}
			// If you take larger steps than 1 / 60th of a second you need to do multiple collision steps in order to keep the simulation stable. Do 1 collision step per 1 / 60th of a second (round up).
			int collisionSteps = 1;
			const float minStepDuration = 1.0f / 60.0f;
			if(ts > minStepDuration)
			{
				collisionSteps = ts / minStepDuration;
			}

			// If you want more accurate step results you can do multiple sub steps within a collision step. Usually you would set this to 1.
			const int cIntegrationSubSteps = 1;

			// Step the world
			_JoltPhysicsSystem->Update(ts, collisionSteps, cIntegrationSubSteps, new JPH::TempAllocatorMalloc(), _JoltJobSystem);
		}

		void DynamicWorld::Clear()
		{
			_stepCount = 0;

			if (_registeredBodies.size() > 0)
			{
				_JoltBodyInterface->RemoveBodies((JPH::BodyID*)_registeredBodies.data(), _registeredBodies.size());
				_registeredBodies.clear();
			}
		}
	}
}
