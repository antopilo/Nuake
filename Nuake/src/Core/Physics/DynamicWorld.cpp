#include "DynamicWorld.h"
#include "Rigibody.h"
#include "../Core/Core.h"
#include <src/Vendors/glm/ext/quaternion_common.hpp>
#include <src/Core/Logger.h>

#include <src/Core/Physics/PhysicsShapes.h>

#include <Jolt/Jolt.h>
#include <Jolt/RegisterTypes.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Body/BodyActivationListener.h>

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
	JPH::BodyID sphere_id;
	namespace Physics
	{
		DynamicWorld::DynamicWorld() : _stepCount(0)
		{
			const uint32_t MaxBodies = 1024;
			const uint32_t NumBodyMutexes = 0;
			const uint32_t MaxBodyPairs = 1024;
			const uint32_t MaxContactConstraints = 1024;

			BPLayerInterfaceImpl broad_phase_layer_interface;

			_JoltPhysicsSystem = CreateRef<JPH::PhysicsSystem>();
			_JoltPhysicsSystem->Init(MaxBodies, NumBodyMutexes, MaxBodyPairs, MaxContactConstraints, broad_phase_layer_interface, MyBroadPhaseCanCollide, MyObjectCanCollide);

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
			JPH::BodyInterface& bodyInterface = _JoltPhysicsSystem->GetBodyInterface();

			// Next we can create a rigid body to serve as the floor, we make a large box
			// Create the settings for the collision volume (the shape). 
			// Note that for simple shapes (like boxes) you can also directly construct a BoxShape.
			JPH::BoxShapeSettings floor_shape_settings(JPH::Vec3(100.0f, 1.0f, 100.0f));

			// Create the shape
			JPH::ShapeSettings::ShapeResult floor_shape_result = floor_shape_settings.Create();
			JPH::ShapeRefC floor_shape = floor_shape_result.Get(); // We don't expect an error here, but you can check floor_shape_result for HasError() / GetError()

			// Create the settings for the body itself. Note that here you can also set other properties like the restitution / friction.
			JPH::BodyCreationSettings floor_settings(floor_shape, JPH::Vec3(0.0f, -1.0f, 0.0f), JPH::Quat::sIdentity(), JPH::EMotionType::Static, Layers::NON_MOVING);

			// Create the actual rigid body
			JPH::Body* floor = bodyInterface.CreateBody(floor_settings); // Note that if we run out of bodies this can return nullptr

			bodyInterface.AddBody(floor->GetID(), JPH::EActivation::DontActivate);

			JPH::BodyCreationSettings sphere_settings(new JPH::SphereShape(0.5f), JPH::Vec3(0.0, 2.0, 0.0), JPH::Quat::sIdentity(), JPH::EMotionType::Dynamic, Layers::MOVING);
			sphere_id = bodyInterface.CreateAndAddBody(sphere_settings, JPH::EActivation::Activate);

			// Now you can interact with the dynamic body, in this case we're going to give it a velocity.
			// (note that if we had used CreateBody then we could have set the velocity straight on the body before adding it to the physics system)
			bodyInterface.SetLinearVelocity(sphere_id, JPH::Vec3(0.0f, -5.0f, 0.0f));

			// We simulate the physics world in discrete time steps. 60 Hz is a good rate to update the physics system.
			const float cDeltaTime = 1.0f / 60.0f;

			// Optional step: Before starting the physics simulation you can optimize the broad phase. This improves collision detection performance (it's pointless here because we only have 2 bodies).
			// You should definitely not call this every frame or when e.g. streaming in a new level section as it is an expensive operation.
			// Instead insert all new objects in batches instead of 1 at a time to keep the broad phase efficient.
			_JoltPhysicsSystem->OptimizeBroadPhase();
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

			auto rbShape = rb->GetShape();
			switch (rbShape->GetType())
			{
			case RigidbodyShapes::BOX:
			{
				Box* box = (Box*)rbShape.get();
				const Vector3& boxSize = box->GetSize();
				JPH::BoxShapeSettings boxShapeSettings(JPH::Vec3(boxSize.x, boxSize.y, boxSize.z));

				// Create the shape
				JPH::ShapeSettings::ShapeResult floor_shape_result = boxShapeSettings.Create();
				JPH::ShapeRefC floor_shape = floor_shape_result.Get(); // We don't expect an error here, but you can check floor_shape_result for HasError() / GetError()

				// Create the settings for the body itself. Note that here you can also set other properties like the restitution / friction.
				JPH::BodyCreationSettings boxSettings(floor_shape, JPH::Vec3(0.0f, -1.0f, 0.0f), JPH::Quat::sIdentity(), JPH::EMotionType::Static, Layers::NON_MOVING);

				// Create the actual rigid body
				JPH::Body* floor = bodyInterface.CreateBody(floor_settings); // Note that if we run out of bodies this can return nullptr

				bodyInterface.AddBody(floor->GetID(), JPH::EActivation::DontActivate);
			}
					break;
			case RigidbodyShapes::SPHERE:

				break;
			case RigidbodyShapes::CAPSULE:

				break;
			case RigidbodyShapes::MESH:

				break;
			}
		}

		void DynamicWorld::AddGhostbody(Ref<GhostObject> gb)
		{
		}

		void DynamicWorld::AddCharacterController(Ref<CharacterController> cc)
		{
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

			// Output current position and velocity of the sphere
			JPH::Vec3 position = bodyInterface.GetCenterOfMassPosition(sphere_id);
			JPH::Vec3 velocity = bodyInterface.GetLinearVelocity(sphere_id);

			std::cout << "Step " << _stepCount << ": Position = (" << position.GetX() << ", " << position.GetY() << ", " << position.GetZ() << "), Velocity = (" << velocity.GetX() << ", " << velocity.GetY() << ", " << velocity.GetZ() << ")" << std::endl;

			// If you take larger steps than 1 / 60th of a second you need to do multiple collision steps in order to keep the simulation stable. Do 1 collision step per 1 / 60th of a second (round up).
			const int cCollisionSteps = 1;

			// If you want more accurate step results you can do multiple sub steps within a collision step. Usually you would set this to 1.
			const int cIntegrationSubSteps = 1;

			// Step the world
			_JoltPhysicsSystem->Update(ts, cCollisionSteps, cIntegrationSubSteps, new JPH::TempAllocatorMalloc(), _JoltJobSystem);
		}

		void DynamicWorld::Clear()
		{
			
		}
	}
}
