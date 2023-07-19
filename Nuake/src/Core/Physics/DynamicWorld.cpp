#include "DynamicWorld.h"
#include "Rigibody.h"

#include "src/Core/Core.h"
#include "src/Core/Logger.h"
#include "src/Core/Maths.h"
#include <src/Core/Physics/PhysicsShapes.h>
#include "src/Scene/Components/TransformComponent.h"
#include "src/Scene/Components/CharacterControllerComponent.h"

#include <src/Vendors/glm/ext/quaternion_common.hpp>
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
			//std::cout << "Contact validate callback" << std::endl;

			// Allows you to ignore a contact before it is created (using layers to not make objects collide is cheaper!)
			return JPH::ValidateResult::AcceptAllContactsForThisBodyPair;
		}

		virtual void OnContactAdded(const JPH::Body& inBody1, const JPH::Body& inBody2, const JPH::ContactManifold& inManifold, JPH::ContactSettings& ioSettings) override
		{
			//std::cout << "A contact was added" << std::endl;
		}

		virtual void OnContactPersisted(const JPH::Body& inBody1, const JPH::Body& inBody2, const JPH::ContactManifold& inManifold, JPH::ContactSettings& ioSettings) override
		{
			//std::cout << "A contact was persisted" << std::endl;
		}

		virtual void OnContactRemoved(const JPH::SubShapeIDPair& inSubShapePair) override
		{
			//std::cout << "A contact was removed" << std::endl;
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
			_registeredCharacters = std::map<uint32_t, JPH::Character*>();

			// Initialize Jolt Physics
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
			const uint32_t availableThreads = std::thread::hardware_concurrency() - 1;
			_JoltJobSystem = new JPH::JobSystemThreadPool(JPH::cMaxPhysicsJobs, JPH::cMaxPhysicsBarriers, availableThreads);
		}

		void DynamicWorld::DrawDebug()
		{

		}

		void DynamicWorld::SetGravity(const Vector3& gravity)
		{
		}

		void DynamicWorld::AddRigidbody(Ref<RigidBody> rb)
		{
			JPH::BodyInterface& bodyInterface = _JoltPhysicsSystem->GetBodyInterface();
			
			const float mass = rb->_mass;
			JPH::EMotionType motionType = JPH::EMotionType::Static;
			
			// According to jolt documentation, Mesh shapes should only be static.
			const bool isMeshShape = rb->GetShape()->GetType() == MESH;
			if (mass > 0.0f && !isMeshShape)
			{
				motionType = JPH::EMotionType::Dynamic;
			}

			const auto& startPos = rb->GetPosition();
			const Quat& bodyRotation = rb->GetRotation();
			const auto& joltRotation = JPH::Quat(bodyRotation.x, bodyRotation.y, bodyRotation.z, bodyRotation.w);
			const auto& joltPos = JPH::Vec3(startPos.x, startPos.y, startPos.z);
			auto joltShape = GetJoltShape(rb->GetShape());
			JPH::BodyCreationSettings bodySettings(joltShape, joltPos, joltRotation, motionType, Layers::MOVING);

			if (mass > 0.0f)
			{
				bodySettings.mOverrideMassProperties = JPH::EOverrideMassProperties::CalculateInertia;
				bodySettings.mMassPropertiesOverride.mMass = mass;
			}
			
			bodySettings.mUserData = rb->GetEntity().GetID();
			// Create the actual rigid body
			JPH::BodyID body = _JoltBodyInterface->CreateAndAddBody(bodySettings, JPH::EActivation::Activate); // Note that if we run out of bodies this can return nullptr
			_registeredBodies.push_back((uint32_t)body.GetIndexAndSequenceNumber());
		}

		void DynamicWorld::AddGhostbody(Ref<GhostObject> gb)
		{

		}

		void DynamicWorld::AddCharacterController(Ref<CharacterController> cc)
		{
			JPH::Ref<JPH::CharacterSettings> settings = new JPH::CharacterSettings();
			settings->mMaxSlopeAngle = JPH::DegreesToRadians(cc->MaxSlopeAngle);
			settings->mLayer = Layers::MOVING;
			settings->mFriction = cc->Friction;
			settings->mShape = GetJoltShape(cc->Shape);
			settings->mGravityFactor = 0.0f;

			auto& joltPosition = JPH::Vec3(cc->Position.x, cc->Position.y, cc->Position.z);

			Quat& bodyRotation = cc->Rotation;

			// We need to add 180 degrees because our forward is -Z.
			const auto& yOffset = Vector3(0.0f, Rad(180.0), 0.0f);
			bodyRotation = glm::normalize(bodyRotation * Quat(yOffset));

			const auto& joltRotation = JPH::Quat(bodyRotation.x, bodyRotation.y, bodyRotation.z, bodyRotation.w);
			JPH::Character* character = new JPH::Character(settings, joltPosition, joltRotation, cc->GetEntity().GetID() , _JoltPhysicsSystem.get());

			character->AddToPhysicsSystem(JPH::EActivation::Activate);
			
			// To get the jolt character control from a scene entity.
			_registeredCharacters[cc->Owner.GetHandle()] = character;
		}

		bool DynamicWorld::IsCharacterGrounded(const Entity& entity)
		{
			const uint32_t entityHandle = entity.GetHandle();
			if (_registeredCharacters.find(entityHandle) != _registeredCharacters.end())
			{
				auto& characterController = _registeredCharacters[entityHandle];
				const auto groundState = characterController->GetGroundState();
				return groundState == JPH::CharacterBase::EGroundState::OnGround;
			}

			assert("Entity doesn't have a character controller component.");
			return false;
		}

		RaycastResult DynamicWorld::Raycast(const Vector3& from, const Vector3& to)
		{
			Vector3 localNorm = Vector3(0,0,0);

			//Logger::Log("normal: x:" + std::to_string(localNorm.x) + " y:" + std::to_string(localNorm.y )+ "z: " + std::to_string(localNorm.z));
			
			// Map bullet result to dto.
			RaycastResult result{
				Vector3(0,0,0),
				Vector3(0,0,0),
				localNorm
			};

			return result;
		}

		void DynamicWorld::SyncEntitiesTranforms()
		{
			const auto& bodyInterface = _JoltPhysicsSystem->GetBodyInterface();
			for (const auto& body : _registeredBodies)
			{
				auto bodyId = static_cast<JPH::BodyID>(body);
				JPH::Vec3 position = bodyInterface.GetCenterOfMassPosition(bodyId);
				JPH::Vec3 velocity = bodyInterface.GetLinearVelocity(bodyId);
				JPH::Mat44 joltTransform = bodyInterface.GetWorldTransform(bodyId);
				const auto bodyRotation = bodyInterface.GetRotation(bodyId);

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

				auto entId = static_cast<int>(bodyInterface.GetUserData(bodyId));
				Entity entity = Engine::GetCurrentScene()->GetEntityByID(entId);
				auto& transformComponent = entity.GetComponent<TransformComponent>();
				transformComponent.SetLocalPosition(pos);
				transformComponent.SetLocalRotation(Quat(bodyRotation.GetW(), bodyRotation.GetX(), bodyRotation.GetY(), bodyRotation.GetZ()));
				transformComponent.SetLocalTransform(transform);
				transformComponent.Dirty = true;
			}
		}
		
		void DynamicWorld::SyncCharactersTransforms()
		{
			// TODO(ANTO): Finish this to connect updated jolt transforms back to the entity.
			// The problem was that I dont know yet how to go from jolt body ptr to the entity
			// Combinations of find and iterators etc. I do not have the brain power rn zzz.
			// const auto& bodyInterface = _JoltPhysicsSystem->GetBodyInterface();
			for (const auto& e : _registeredCharacters)
			{
				Entity entity { (entt::entity)e.first, Engine::GetCurrentScene().get()};

				JPH::Character* characterController = e.second;
				JPH::Mat44 joltTransform = characterController->GetWorldTransform();
				const auto bodyRotation = characterController->GetRotation();

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

				auto& transformComponent = entity.GetComponent<TransformComponent>();
				transformComponent.SetLocalPosition(pos);
				transformComponent.SetLocalRotation(Quat(bodyRotation.GetW(), bodyRotation.GetX(), bodyRotation.GetY(), bodyRotation.GetZ()));
				transformComponent.SetLocalTransform(transform);
				transformComponent.Dirty = true;
			}
		}

		void DynamicWorld::StepSimulation(Timestep ts)
		{
			// Next step
			++_stepCount;


			// If you take larger steps than 1 / 60th of a second you need to do multiple collision steps in order to keep the simulation stable.
			// Do 1 collision step per 1 / 60th of a second (round up).
			int collisionSteps = 1;
			constexpr float minStepDuration = 1.0f / 90.0f;
			constexpr int maxStepCount = 32;
			if(ts > minStepDuration)
			{
				collisionSteps = static_cast<float>(ts) / minStepDuration;
			}

			// Prevents having too many steps and running out of jobs
			collisionSteps = std::min(collisionSteps, maxStepCount);

			// If you want more accurate step results you can do multiple sub steps within a collision step. Usually you would set this to 1.
			constexpr int subSteps = 1;

			// Step the world
			_JoltPhysicsSystem->Update(ts, collisionSteps, subSteps, new JPH::TempAllocatorMalloc(), _JoltJobSystem);

			for (auto& c : _registeredCharacters)
			{
				c.second->PostSimulation(0.001);
			}

			SyncEntitiesTranforms();
			SyncCharactersTransforms();
		}

		void DynamicWorld::Clear()
		{
			_stepCount = 0;

			if (!_registeredBodies.empty())
			{
				_JoltBodyInterface->RemoveBodies(reinterpret_cast<JPH::BodyID*>(_registeredBodies.data()), _registeredBodies.size());
				_registeredBodies.clear();
			}
			
			if (!_registeredCharacters.empty())
			{
				for (auto& character : _registeredCharacters)
				{
					character.second->RemoveFromPhysicsSystem();
				}
				_registeredCharacters.clear();
			}
		}

		void DynamicWorld::MoveAndSlideCharacterController(const Entity& entity, const Vector3 velocity)
		{
			const uint32_t entityHandle = entity.GetHandle();
			if (_registeredCharacters.find(entityHandle) != _registeredCharacters.end())
			{
				auto& characterController = _registeredCharacters[entityHandle];
				characterController->SetLinearVelocity(JPH::Vec3(velocity.x, velocity.y, velocity.z));
			}
		}

		JPH::Ref<JPH::Shape> DynamicWorld::GetJoltShape(const Ref<PhysicShape> shape)
		{
			JPH::ShapeSettings::ShapeResult result;

			switch (shape->GetType())
			{
			case RigidbodyShapes::BOX:
			{
				Box* box = (Box*)shape.get();
				const Vector3& boxSize = box->GetSize();
				JPH::BoxShapeSettings shapeSettings(JPH::Vec3(boxSize.x, boxSize.y, boxSize.z));
				result = shapeSettings.Create();
			}
			break;
			case RigidbodyShapes::SPHERE:
			{
				Sphere* sphere = (Sphere*)shape.get();
				const float sphereRadius = sphere->GetRadius();
				JPH::SphereShapeSettings shapeSettings(sphereRadius);
				result = shapeSettings.Create();
			}
			break;
			case RigidbodyShapes::CAPSULE:
			{
				Capsule* capsule = (Capsule*)shape.get();
				const float radius = capsule->GetRadius();
				const float height = capsule->GetHeight();
				JPH::CapsuleShapeSettings shapeSettings(height / 2.0f, radius);
				result = shapeSettings.Create();
			}
			break;
			case RigidbodyShapes::CYLINDER:
			{
				Cylinder* capsule = (Cylinder*)shape.get();
				const float radius = capsule->GetRadius();
				const float height = capsule->GetHeight();
				JPH::CylinderShapeSettings shapeSettings(height / 2.0f, radius);
				result = shapeSettings.Create();
			}
			break;
			case RigidbodyShapes::MESH:
			{
				MeshShape* meshShape = (MeshShape*)shape.get();
				const auto& mesh = meshShape->GetMesh();
				const auto& vertices = mesh->GetVertices();
				const auto& indices = mesh->GetIndices();

				JPH::TriangleList triangles;
				triangles.reserve(indices.size());
				auto transform = Matrix4(1.0f);
				transform[3] = Vector4(0.0f, 0.0f, 0.0f, 1.0f);
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

				result = shapeSettings.Create();
			}
			break;
			case CONVEX_HULL:
			{
				auto* convexHullShape = (Physics::ConvexHullShape*)shape.get();
				const auto& hullPoints = convexHullShape->GetPoints();
				JPH::Array<JPH::Vec3> points;
				points.reserve(std::size(hullPoints));
				for (const auto& p : hullPoints)
				{
					points.push_back(JPH::Vec3(p.x, p.y, p.z));
				}

				JPH::ConvexHullShapeSettings shapeSettings(points);
				result = shapeSettings.Create();
			}
			break;
			}

			return result.Get();
		}
	}
}
