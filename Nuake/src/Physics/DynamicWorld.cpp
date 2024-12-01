#include "DynamicWorld.h"
#include "Rigibody.h"

#include "src/Core/Core.h"
#include "src/Core/Logger.h"
#include "src/Core/Maths.h"
#include "src/Resource/Project.h"
#include "src/Physics/PhysicsShapes.h"
#include "src/Scene/Components/TransformComponent.h"
#include "src/Scene/Components/CharacterControllerComponent.h"

#include <src/Vendors/glm/ext/quaternion_common.hpp>
#include "src/Vendors/glm/gtx/matrix_decompose.hpp"

#include <Jolt/Jolt.h>
#include <Jolt/RegisterTypes.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Body/BodyActivationListener.h>
#include <Jolt/Physics/Character/CharacterVirtual.h>
#include <Jolt/Physics/Collision/RayCast.h>
#include <Jolt/Physics/Collision/ShapeCast.h>
#include <Jolt/Physics/Collision/CastResult.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/Collision/Shape/CylinderShape.h>
#include <Jolt/Physics/Collision/Shape/MeshShape.h>
#include <Jolt/Physics/Collision/Shape/ConvexHullShape.h>

#include <Jolt/Core/TempAllocator.h>

#include <dependencies/JoltPhysics/Jolt/Physics/Collision/CollisionCollectorImpl.h>
#include <src/Scene/Components/ParentComponent.h>
#include <cstdarg>

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
		static constexpr uint8_t KINEMATIC = 2;
		static constexpr uint8_t CHARACTER_GHOST = 3;
		static constexpr uint8_t CHARACTER = 4;
		static constexpr uint8_t SENSORS = 5;
		static constexpr uint8_t NUM_LAYERS = 6;
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
			mObjectToBroadPhase[Layers::CHARACTER] = BroadPhaseLayers::MOVING;
			mObjectToBroadPhase[Layers::CHARACTER_GHOST] = BroadPhaseLayers::MOVING;
			mObjectToBroadPhase[Layers::SENSORS] = BroadPhaseLayers::MOVING;
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

	private:
		JPH::BroadPhaseLayer					mObjectToBroadPhase[Layers::NUM_LAYERS];
	};

	// An example contact listener
	class MyContactListener : public JPH::ContactListener
	{
	private:
		Physics::DynamicWorld* _World;

	public:
		MyContactListener(Physics::DynamicWorld* world)
			: _World(world)
		{
		}

		// See: ContactListener
		virtual JPH::ValidateResult	OnContactValidate(const JPH::Body& inBody1, const JPH::Body& inBody2, JPH::RVec3Arg inBaseOffset, const JPH::CollideShapeResult& inCollisionResult) override
		{
			//std::cout << "Contact validate callback" << std::endl;

			// Allows you to ignore a contact before it is created (using layers to not make objects collide is cheaper!)
  			return JPH::ValidateResult::AcceptAllContactsForThisBodyPair;
		}

		virtual void OnContactAdded(const JPH::Body& inBody1, const JPH::Body& inBody2, const JPH::ContactManifold& inManifold, JPH::ContactSettings& ioSettings) override
		{
			uint32_t entity1 = static_cast<uint32_t>(inBody1.GetUserData());
			uint32_t entity2 = static_cast<uint32_t>(inBody2.GetUserData());

			JPH::Vec3 joltNormal = inManifold.mWorldSpaceNormal;
			Vector3 normal = Vector3(joltNormal.GetX(), joltNormal.GetY(), joltNormal.GetZ());

			JPH::Vec3 joltPos = inManifold.GetWorldSpaceContactPointOn1(0);
			Vector3 position = Vector3(joltPos.GetX(), joltPos.GetY(), joltPos.GetZ());

			Physics::CollisionData data
			{
				entity1,
				entity2,
				normal,
				position
			};
			_World->RegisterCollisionCallback(std::move(data));
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
			//std::cout << "A body got activated" << std::endl;
		}

		virtual void OnBodyDeactivated(const JPH::BodyID& inBodyID, JPH::uint64 inBodyUserData) override
		{
			//std::cout << "A body went to sleep" << std::endl;
		}
	};

	class ObjectVsBroadPhaseLayerFilterImpl : public JPH::ObjectVsBroadPhaseLayerFilter
	{
	public:
		virtual bool				ShouldCollide(JPH::ObjectLayer inLayer1, JPH::BroadPhaseLayer inLayer2) const override
		{
			switch (inLayer1)
			{
			case Layers::NON_MOVING:
				return inLayer2 == BroadPhaseLayers::MOVING;
			case Layers::MOVING:
				return true;
			case Layers::SENSORS:
				return inLayer2 == BroadPhaseLayers::MOVING;
			default:
				return false;
			}
		}
	};

	class ObjectLayerPairFilterImpl : public JPH::ObjectLayerPairFilter
	{
	public:
		virtual bool					ShouldCollide(JPH::ObjectLayer inObject1, JPH::ObjectLayer inObject2) const override
		{
			switch (inObject1)
			{
			case Layers::NON_MOVING:
				return inObject2 == Layers::MOVING || inObject2 == Layers::CHARACTER_GHOST || inObject2 == Layers::CHARACTER; // Non moving only collides with moving
			case Layers::MOVING:
				return true; // Moving collides with everything
			case Layers::CHARACTER_GHOST:
				return inObject2 != Layers::CHARACTER;
			case Layers::CHARACTER:
				return inObject2 != Layers::CHARACTER_GHOST;
			case Layers::SENSORS:
				return inObject2 == Layers::MOVING || inObject2 == Layers::CHARACTER_GHOST;
			default:
				
				return false;
			}
		}

	};

	BPLayerInterfaceImpl JoltBroadphaseLayerInterface = BPLayerInterfaceImpl();
	ObjectVsBroadPhaseLayerFilterImpl JoltObjectVSBroadphaseLayerFilter = ObjectVsBroadPhaseLayerFilterImpl();
	ObjectLayerPairFilterImpl JoltObjectVSObjectLayerFilter;

	namespace Physics
	{
		DynamicWorld::DynamicWorld() : _stepCount(0)
		{
			_registeredCharacters = std::map<uint32_t, CharacterGhostPair>();

			// Initialize Jolt Physics
			ProjectSettings settings;
			if (Engine::GetProject())
			{
				settings = Engine::GetProject()->Settings;
			}

			const uint32_t MaxBodies = settings.MaxPhysicsBodies;
			const uint32_t NumBodyMutexes = 0;
			const uint32_t MaxBodyPairs = settings.MaxPhysicsBodyPair;
			const uint32_t MaxContactConstraints = settings.MaxPhysicsContactConstraints;

			_JoltPhysicsSystem = CreateRef<JPH::PhysicsSystem>();
			_JoltPhysicsSystem->Init(MaxBodies, NumBodyMutexes, MaxBodyPairs, MaxContactConstraints, JoltBroadphaseLayerInterface, JoltObjectVSBroadphaseLayerFilter, JoltObjectVSObjectLayerFilter);
			// A body activation listener gets notified when bodies activate and go to sleep
			// Note that this is called from a job so whatever you do here needs to be thread safe.
			// Registering one is entirely optional.
			_bodyActivationListener = CreateScope<MyBodyActivationListener>();
			_JoltPhysicsSystem->SetBodyActivationListener(_bodyActivationListener.get());

			// A contact listener gets notified when bodies (are about to) collide, and when they separate again.
			// Note that this is called from a job so whatever you do here needs to be thread safe.
			// Registering one is entirely optional.
			_contactListener = CreateScope<MyContactListener>(this);
			_JoltPhysicsSystem->SetContactListener(_contactListener.get());

			// The main way to interact with the bodies in the physics system is through the body interface. There is a locking and a non-locking
			// variant of this. We're going to use the locking version (even though we're not planning to access bodies from multiple threads)
			_JoltBodyInterface = &_JoltPhysicsSystem->GetBodyInterface();

			// Optional step: Before starting the physics simulation you can optimize the broad phase. This improves collision detection performance (it's pointless here because we only have 2 bodies).
			// You should definitely not call this every frame or when e.g. streaming in a new level section as it is an expensive operation.
			// Instead insert all new objects in batches instead of 1 at a time to keep the broad phase efficient.
			_JoltPhysicsSystem->OptimizeBroadPhase();
			const uint32_t availableThreads = std::thread::hardware_concurrency() - 1;
			_JoltJobSystem = new JPH::JobSystemThreadPool(JPH::cMaxPhysicsJobs, JPH::cMaxPhysicsBarriers, availableThreads);
		}

		void DynamicWorld::ReInit()
		{
			_registeredCharacters = std::map<uint32_t, CharacterGhostPair>();

			// Initialize Jolt Physics
			ProjectSettings settings;
			if (Engine::GetProject())
			{
				settings = Engine::GetProject()->Settings;
			}

			const uint32_t MaxBodies = settings.MaxPhysicsBodies;
			const uint32_t NumBodyMutexes = 0;
			const uint32_t MaxBodyPairs = settings.MaxPhysicsBodyPair;
			const uint32_t MaxContactConstraints = settings.MaxPhysicsContactConstraints;

			_JoltPhysicsSystem = CreateRef<JPH::PhysicsSystem>();
			_JoltPhysicsSystem->Init(MaxBodies, NumBodyMutexes, MaxBodyPairs, MaxContactConstraints, JoltBroadphaseLayerInterface, JoltObjectVSBroadphaseLayerFilter, JoltObjectVSObjectLayerFilter);
			// A body activation listener gets notified when bodies activate and go to sleep
			// Note that this is called from a job so whatever you do here needs to be thread safe.
			// Registering one is entirely optional.
			_bodyActivationListener = CreateScope<MyBodyActivationListener>();
			_JoltPhysicsSystem->SetBodyActivationListener(_bodyActivationListener.get());

			// A contact listener gets notified when bodies (are about to) collide, and when they separate again.
			// Note that this is called from a job so whatever you do here needs to be thread safe.
			// Registering one is entirely optional.
			_contactListener = CreateScope<MyContactListener>(this);
			_JoltPhysicsSystem->SetContactListener(_contactListener.get());

			// The main way to interact with the bodies in the physics system is through the body interface. There is a locking and a non-locking
			// variant of this. We're going to use the locking version (even though we're not planning to access bodies from multiple threads)
			_JoltBodyInterface = &_JoltPhysicsSystem->GetBodyInterface();

			// Optional step: Before starting the physics simulation you can optimize the broad phase. This improves collision detection performance (it's pointless here because we only have 2 bodies).
			// You should definitely not call this every frame or when e.g. streaming in a new level section as it is an expensive operation.
			// Instead insert all new objects in batches instead of 1 at a time to keep the broad phase efficient.
			_JoltPhysicsSystem->OptimizeBroadPhase();
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
			JPH::ObjectLayer layer = Layers::MOVING;

			// According to jolt documentation, Mesh shapes should only be static.
			const bool isMeshShape = rb->GetShape()->GetType() == MESH;
			if (mass > 0.0f && !isMeshShape)
			{
				motionType = JPH::EMotionType::Dynamic;
				layer = Layers::MOVING;
			}

			if (rb->GetForceKinematic())
			{
				motionType = JPH::EMotionType::Kinematic;
				layer = Layers::MOVING;
			}

			const std::string name = rb->GetEntity().GetComponent<NameComponent>().Name;
			if (rb->IsTrigger())
			{
				layer = Layers::SENSORS;
				motionType = JPH::EMotionType::Kinematic;
			}

			const auto& startPos = rb->GetPosition();
			const Quat& bodyRotation = rb->GetRotation();
			const auto& joltRotation = JPH::Quat(bodyRotation.x, bodyRotation.y, bodyRotation.z, bodyRotation.w);
			const auto& joltPos = JPH::Vec3(startPos.x, startPos.y, startPos.z);
			JPH::Ref<JPH::Shape> joltShape = GetJoltShape(rb->GetShape());

			if (!joltShape)
			{
				return;
			}

			JPH::BodyCreationSettings bodySettings(joltShape, joltPos, joltRotation, motionType, layer);
			bodySettings.mIsSensor = rb->IsTrigger();
			if (bodySettings.mIsSensor)
			{
				bodySettings.mCollideKinematicVsNonDynamic = true;
			}

			if (rb->GetForceKinematic())
			{
				bodySettings.mCollideKinematicVsNonDynamic = true;
			}

			bodySettings.mAllowedDOFs = (JPH::EAllowedDOFs::All);

			if (rb->GetLockXAxis())
			{
				bodySettings.mAllowedDOFs ^= JPH::EAllowedDOFs::RotationX;
			}

			if (rb->GetLockYAxis())
			{
				bodySettings.mAllowedDOFs ^= JPH::EAllowedDOFs::RotationY;
			}

			if (rb->GetLockZAxis())
			{
				bodySettings.mAllowedDOFs ^= JPH::EAllowedDOFs::RotationZ;
			}

			if (mass > 0.0f)
			{
				bodySettings.mOverrideMassProperties = JPH::EOverrideMassProperties::CalculateInertia;
				bodySettings.mMassPropertiesOverride.mMass = mass;
			}
			
			if (int entityId = rb->GetEntity().GetHandle(); rb->GetEntity().IsValid())
			{
				bodySettings.mUserData = rb->GetEntity().GetHandle();

				// Create the actual rigid body
				JPH::BodyID body = _JoltBodyInterface->CreateAndAddBody(bodySettings, JPH::EActivation::Activate); // Note that if we run out of bodies this can return nullptr
				uint32_t bodyIndex = (uint32_t)body.GetIndexAndSequenceNumber();
				_registeredBodies.push_back(bodyIndex);
			}
		}

		void DynamicWorld::AddCharacterController(Ref<CharacterController> cc)
		{
			JPH::Ref<JPH::CharacterVirtualSettings> settings = new JPH::CharacterVirtualSettings();
			settings->mMaxSlopeAngle = JPH::DegreesToRadians(cc->MaxSlopeAngle);
			settings->mMaxStrength = 1.0f;
			settings->mCharacterPadding = 0.05f;
			settings->mPenetrationRecoverySpeed = 1.0f;
			settings->mPredictiveContactDistance = 0.01f;
			settings->mShape = GetJoltShape(cc->Shape);
			auto joltPosition = JPH::Vec3(cc->Position.x, cc->Position.y, cc->Position.z);

			const Quat& bodyRotation = cc->Rotation;
			const auto& joltRotation = JPH::Quat(bodyRotation.x, bodyRotation.y, bodyRotation.z, bodyRotation.w);
			auto character = CreateRef<JPH::CharacterVirtual>(settings, std::move(joltPosition), joltRotation, _JoltPhysicsSystem.get());

			// add ghost kinematic body to respond to hit test as the virtual char are not present in the world.
			JPH::BodyInterface& bodyInterface = _JoltPhysicsSystem->GetBodyInterface();

			const float mass = 0.0f;
			JPH::EMotionType motionType = JPH::EMotionType::Dynamic;
			JPH::ObjectLayer layer = Layers::CHARACTER_GHOST;

			const auto& startPos = joltPosition;
			auto joltShape = GetJoltShape(cc->Shape);
			JPH::BodyCreationSettings bodySettings(joltShape, startPos, joltRotation, motionType, layer);

			int entityId = cc->GetEntity().GetID();
			if (entityId == 0)
			{
				Logger::Log("ERROR");
			}

			bodySettings.mUserData = cc->Owner.GetHandle();
			bodySettings.mCollideKinematicVsNonDynamic = true;
			// Create the actual rigid body
			JPH::BodyID body = _JoltBodyInterface->CreateAndAddBody(bodySettings, JPH::EActivation::Activate); // Note that if we run out of bodies this can return nullptr
			uint32_t bodyIndex = body.GetIndexAndSequenceNumber();
			//_registeredBodies.push_back(bodyIndex);

			// To get the jolt character control from a scene entity.
			_registeredCharacters[cc->Owner.GetHandle()] = CharacterGhostPair{ character, bodyIndex };
		}

		bool DynamicWorld::IsCharacterGrounded(const Entity& entity)
		{
			const uint32_t entityHandle = entity.GetHandle();
			if (_registeredCharacters.find(entityHandle) != _registeredCharacters.end())
			{
				auto& characterController = _registeredCharacters[entityHandle].Character;
				const auto groundState = characterController->GetGroundState();

				return groundState == JPH::CharacterBase::EGroundState::OnGround;
			}

			assert("Entity doesn't have a character controller component.");
			return false;
		}

		Vector3 DynamicWorld::GetCharacterGroundVelocity(const Entity& entity)
		{
			const uint32_t entityHandle = entity.GetHandle();
			if (_registeredCharacters.find(entityHandle) != _registeredCharacters.end())
			{
				auto& characterController = _registeredCharacters[entityHandle].Character;
				characterController->UpdateGroundVelocity();
				const auto groundVelocity = characterController->GetGroundVelocity();

				return Vector3(groundVelocity.GetX(), groundVelocity.GetY(), groundVelocity.GetZ());
			}

			return { 0, 0, 0 };
		}

		Vector3 DynamicWorld::GetCharacterGroundNormal(const Entity& entity)
		{
			const uint32_t entityHandle = entity.GetHandle();
			if (_registeredCharacters.find(entityHandle) != _registeredCharacters.end())
			{
				auto& characterController = _registeredCharacters[entityHandle].Character;
				const auto groundNormal = characterController->GetGroundNormal();

				return Vector3(groundNormal.GetX(), groundNormal.GetY(), groundNormal.GetZ());
			}

			return { 0, 0, 0 };
		}

		void DynamicWorld::SetBodyPosition(const Entity& entity, const Vector3& position, const Quat& rotation)
		{
			const auto& bodyInterface = _JoltPhysicsSystem->GetBodyInterface();
			for (const auto& body : _registeredBodies)
			{
				auto bodyId = static_cast<JPH::BodyID>(body);
				auto bodyEntityId = bodyInterface.GetUserData(bodyId);
				if (bodyEntityId == entity.GetHandle())
				{
					JPH::Vec3 currentPosition;
					JPH::Quat currentRotation;
					_JoltBodyInterface->GetPositionAndRotation(bodyId, currentPosition, currentRotation);

					JPH::Vec3 newPosition = { position.x, position.y, position.z };
					JPH::Quat newRotation = { rotation.x, rotation.y, rotation.z, rotation.w };

					if (newPosition != currentPosition || currentRotation != newRotation)
					{
						std::string name = entity.GetComponent<NameComponent>().Name;

						JPH::EMotionType bodyType = _JoltBodyInterface->GetMotionType(bodyId);
						switch (bodyType)
						{
							case JPH::EMotionType::Kinematic:
							{
								_JoltBodyInterface->MoveKinematic(bodyId, newPosition, newRotation, Engine::GetFixedTimeStep());
								break;
							}
							case JPH::EMotionType::Dynamic:
							case JPH::EMotionType::Static:
							{
								_JoltBodyInterface->SetPositionAndRotation(bodyId, newPosition, newRotation, JPH::EActivation::DontActivate);

								break;
							}
						}
					}
				}
			}
		}

		void DynamicWorld::SetCharacterControllerPosition(const Entity& entity, const Vector3 & position)
		{
			const uint32_t entityHandle = entity.GetHandle();
			if (_registeredCharacters.find(entityHandle) != _registeredCharacters.end())
			{
				auto& characterController = _registeredCharacters[entityHandle].Character;
  				characterController->SetPosition({ position.x, position.y, position.z });
			}
		}

		std::vector<ShapeCastResult> DynamicWorld::Raycast(const Vector3& from, const Vector3& to)
		{
			// Create jolt ray
			const auto& fromJolt = JPH::Vec3(from.x, from.y, from.z);
			const auto& toDirectionJolt = JPH::Vec3(to.x - from.x, to.y - from.y, to.z - from.z);
			JPH::RRayCast ray { fromJolt, toDirectionJolt };
			JPH::AllHitCollisionCollector<JPH::CastRayCollector> collector;
			
			JPH::RayCastResult result;
			_JoltPhysicsSystem->GetNarrowPhaseQuery().CastRay(ray, JPH::RayCastSettings(), collector);

			// Fetch results
			std::vector<ShapeCastResult> raycastResults;
			if (collector.HadHit())
			{
				int num_hits = (int)collector.mHits.size();
				JPH::BroadPhaseCastResult* results = collector.mHits.data();
				// Format result
				for (int i = 0; i < num_hits; ++i)
				{
					const float hitFraction = collector.mHits[i].mFraction;
					const JPH::Vec3& hitPosition = ray.GetPointOnRay(collector.mHits[i].mFraction);
					auto bodyId = static_cast<JPH::BodyID>(collector.mHits[i].mBodyID);
					auto layer = _JoltBodyInterface->GetObjectLayer(bodyId);
					int userData = static_cast<int>(_JoltBodyInterface->GetUserData(bodyId));
					ShapeCastResult result
					{
						Vector3(hitPosition.GetX(), hitPosition.GetY(), hitPosition.GetZ()),
						hitFraction,
						Vector3(0, 0, 0),
						layer,
						userData
					};

					raycastResults.push_back(std::move(result));
				}
			}
			
			return raycastResults;
		}

		std::vector<ShapeCastResult> DynamicWorld::CastShape(const Vector3& from, const Vector3& to, const Ref<PhysicShape>& shape)
		{
			auto joltShape = GetJoltShape(shape);
			const auto& fromJolt = JPH::Vec3(from.x, from.y, from.z);
			const auto& toDirectionJolt = JPH::Vec3(to.x - from.x, to.y - from.y, to.z - from.z);
			const auto& scale = JPH::Vec3(1, 1, 1);
			JPH::Mat44 centerOfMass = JPH::Mat44::sIdentity();
			centerOfMass.SetTranslation(JPH::Vec3(from.x, from.y, from.z));
			const JPH::AABox worldBound = JPH::AABox(JPH::Vec3{-1000, -1000, -1000}, JPH::Vec3{1000, 1000, 1000});

			JPH::RShapeCast ray = JPH::RShapeCast(joltShape, scale, centerOfMass, toDirectionJolt, worldBound);
			JPH::AllHitCollisionCollector<JPH::CastShapeCollector> collector;

			JPH::RayCastResult result;

			JPH::ShapeCastSettings shapeCastSetting;
			shapeCastSetting.mCollectFacesMode = JPH::ECollectFacesMode::CollectFaces;
			shapeCastSetting.mUseShrunkenShapeAndConvexRadius = true;

			_JoltPhysicsSystem->GetNarrowPhaseQuery().CastShape(ray, shapeCastSetting, JPH::Vec3(0, 0, 0), collector);

			std::vector<ShapeCastResult> shapecastResults;
			if (collector.HadHit())
			{
				int num_hits = (int)collector.mHits.size();
				auto results = collector.mHits.data();

				// Format result
				for (int i = 0; i < num_hits; ++i)
				{
					const float hitFraction = results[i].mFraction;
					const JPH::Vec3& hitPosition = ray.GetPointOnRay(results[i].mFraction);
					
					auto bodyId = static_cast<JPH::BodyID>(results[i].mBodyID2);
					JPH::TransformedShape ts = _JoltPhysicsSystem->GetBodyInterface().GetTransformedShape(results[i].mBodyID2);
					JPH::Vec3 surfaceNormal = ts.GetWorldSpaceSurfaceNormal(results[i].mSubShapeID2, results[i].mContactPointOn2);

					auto layer = _JoltBodyInterface->GetObjectLayer(bodyId);

					int userData = static_cast<int>(_JoltBodyInterface->GetUserData(bodyId));
					ShapeCastResult result
					{
						Vector3(hitPosition.GetX(), hitPosition.GetY(), hitPosition.GetZ()),
						hitFraction,
						Vector3(surfaceNormal.GetX(), surfaceNormal.GetY(), surfaceNormal.GetZ()),
						static_cast<float>(layer),
						userData
					};

					shapecastResults.push_back(std::move(result));
				}
			}

			return shapecastResults;
		}

		void DynamicWorld::SyncEntitiesTranforms()
		{
			const auto& bodyInterface = _JoltPhysicsSystem->GetBodyInterface();
			for (const auto& body : _registeredBodies)
			{
				auto bodyId = static_cast<JPH::BodyID>(body);
				if (auto entId = static_cast<int>(bodyInterface.GetUserData(bodyId)); entId != 0)
				{
					if (bodyInterface.GetObjectLayer(bodyId) == Layers::SENSORS)
					{
						continue;
					}

					JPH::Vec3 position = bodyInterface.GetPosition(bodyId);
					JPH::Vec3 velocity = bodyInterface.GetLinearVelocity(bodyId);
					JPH::Mat44 joltTransform = bodyInterface.GetWorldTransform(bodyId);
					const auto bodyRotation = bodyInterface.GetRotation(bodyId);

					Matrix4 transform = glm::mat4(
						joltTransform(0, 0), joltTransform(1, 0), joltTransform(2, 0), joltTransform(3, 0),
						joltTransform(0, 1), joltTransform(1, 1), joltTransform(2, 1), joltTransform(3, 1),
						joltTransform(0, 2), joltTransform(1, 2), joltTransform(2, 2), joltTransform(3, 2),
						joltTransform(0, 3), joltTransform(1, 3), joltTransform(2, 3), joltTransform(3, 3)
					);

					Entity entity = { (entt::entity)entId, Engine::GetCurrentScene().get() };
					if (entity.GetComponent<ParentComponent>().HasParent)
					{
						auto& parent = entity.GetComponent<ParentComponent>().Parent;
						auto& parentTransformComponent = parent.GetComponent<TransformComponent>();
						const Matrix4& parentTransform = parentTransformComponent.GetGlobalTransform();

						transform = glm::inverse(parentTransform) * transform;
					}

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
		}
		
		void DynamicWorld::SyncCharactersTransforms()
		{
			for (const auto& e : _registeredCharacters)
			{
				Entity entity { (entt::entity)e.first, Engine::GetCurrentScene().get()};

				Ref<JPH::CharacterVirtual> characterController = e.second.Character;
				JPH::Mat44 joltTransform = characterController->GetWorldTransform();
				const auto bodyRotation = characterController->GetRotation();

				Matrix4 transform = glm::mat4(
					joltTransform(0, 0), joltTransform(1, 0), joltTransform(2, 0), joltTransform(3, 0),
					joltTransform(0, 1), joltTransform(1, 1), joltTransform(2, 1), joltTransform(3, 1),
					joltTransform(0, 2), joltTransform(1, 2), joltTransform(2, 2), joltTransform(3, 2),
					joltTransform(0, 3), joltTransform(1, 3), joltTransform(2, 3), joltTransform(3, 3)
				);

				if(entity.GetComponent<ParentComponent>().HasParent)
				{
					auto& parent = entity.GetComponent<ParentComponent>().Parent;
					auto& parentTransformComponent = parent.GetComponent<TransformComponent>();
					const Matrix4& parentTransform = parentTransformComponent.GetGlobalTransform();

					transform = glm::inverse(parentTransform) * transform;
				}

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
			// Clear collisions, before very step
			{
				std::scoped_lock<std::mutex> lock(_CollisionCallbackMutex);
				_CollisionCallbacks.clear();
			}

			if (ts > 0.1f)
			{
				ts = 0.08f;
			}

			// Next step
			++_stepCount;

			// If you take larger steps than 1 / 90th of a second you need to do multiple collision steps in order to keep the simulation stable.
			// Do 1 collision step per 1 / 60th of a second (round up).
			int collisionSteps = 1;
			const float minStepDuration = 1.0f / static_cast<float>(Engine::GetProject()->Settings.PhysicsStep);
			const int maxStepCount = Engine::GetProject()->Settings.MaxPhysicsSubStep;

			if(ts > minStepDuration)
			{
#ifdef NK_DEBUG
				//Logger::Log("Large step detected: " + std::to_string(ts), "physics", WARNING);
#endif
				collisionSteps = static_cast<int>(static_cast<float>(ts) / minStepDuration);
			}

#ifdef NK_DEBUG
			if (collisionSteps >= maxStepCount)
			{
				Logger::Log("Very large step detected: " + std::to_string(ts), "physics", WARNING);
			}
#endif

			// Prevents having too many steps and running out of jobs
			collisionSteps = std::min(collisionSteps, maxStepCount);

			// Step the world
			try
			{
				auto joltTempAllocator = CreateRef<JPH::TempAllocatorMalloc>();

				JPH::CharacterVirtual::ExtendedUpdateSettings joltUpdateSettings;

				for (auto& c : _registeredCharacters)
				{
					//c.second->PostSimulation(0.05f);
					Entity entity{ (entt::entity)c.first, Engine::GetCurrentScene().get() };

					if (entity.HasComponent<CharacterControllerComponent>())
					{
						auto& characterControllerComponent = entity.GetComponent<CharacterControllerComponent>();
						auto characterController = characterControllerComponent.GetCharacterController();

						const auto& broadPhaseLayerFilter = _JoltPhysicsSystem->GetDefaultBroadPhaseLayerFilter(Layers::NON_MOVING);
						const auto& LayerFilter = _JoltPhysicsSystem->GetDefaultLayerFilter(Layers::CHARACTER);
						const auto& joltGravity = _JoltPhysicsSystem->GetGravity();
						auto& tempAllocatorPtr = *(joltTempAllocator);

						c.second.Character->UpdateGroundVelocity();
						if (characterController->AutoStepping)
						{
							// Create update settings from character controller
							joltUpdateSettings.mStickToFloorStepDown = CreateJoltVec3(characterController->StepDown);
							joltUpdateSettings.mWalkStairsStepDownExtra = CreateJoltVec3(characterController->StepDownExtra);
							joltUpdateSettings.mWalkStairsStepUp = CreateJoltVec3(characterController->StepUp);
							joltUpdateSettings.mWalkStairsStepForwardTest = characterController->StepDistance;
							joltUpdateSettings.mWalkStairsMinStepForward = characterController->StepMinDistance;
							
							c.second.Character->ExtendedUpdate(ts, joltGravity, joltUpdateSettings, broadPhaseLayerFilter, LayerFilter, { }, { }, tempAllocatorPtr);
						}
						else
						{
							c.second.Character->Update(ts, joltGravity, broadPhaseLayerFilter, LayerFilter, {}, {}, tempAllocatorPtr);
						}

						uint32_t ghostId = c.second.Ghost;

						JPH::Mat44 joltTransform = c.second.Character->GetWorldTransform();
						const auto bodyRotation = c.second.Character->GetRotation();
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

						//auto& bodyInterface = _JoltPhysicsSystem->GetBodyInterfaceNoLock();
						_JoltBodyInterface->MoveKinematic(static_cast<JPH::BodyID>(ghostId), JPH::Vec3{ pos.x, pos.y, pos.z }, { rotation.x, rotation.y, rotation.z, rotation.w }, ts);
					}
				}

				auto error = _JoltPhysicsSystem->Update(ts, collisionSteps, joltTempAllocator.get(), _JoltJobSystem);
				if (error != JPH::EPhysicsUpdateError::None)
				{
					std::string errMsg = "";
					switch (error)
					{
					case JPH::EPhysicsUpdateError::ManifoldCacheFull:
						errMsg = "Manifold cache full";
						break;
					case JPH::EPhysicsUpdateError::BodyPairCacheFull:
						errMsg = "Body pair cache full";
						break;
					case JPH::EPhysicsUpdateError::ContactConstraintsFull:
						errMsg = "contact constraints full";
						break;
					}
					Logger::Log("Jolt physics encountered an error: " + errMsg, "jolt", CRITICAL);
				}
			}
			catch (...)
			{
				Logger::Log("Failed to run simulation update", "physics", CRITICAL);
			}

			for (auto& c : _registeredCharacters)
			{
				
			}

			SyncEntitiesTranforms();
			SyncCharactersTransforms();
		}

		void DynamicWorld::Clear()
		{
			_stepCount = 0;

			if (!_registeredBodies.empty())
			{
				for (auto& body : _registeredBodies)
				{
					_JoltBodyInterface->RemoveBody(static_cast<JPH::BodyID>(body));
				}

				_registeredBodies.clear();
			}

			if (!_registeredCharacters.empty())
			{
				
				_registeredCharacters.clear();
			}
		}

		void DynamicWorld::ClearCollisionData()
		{
			std::scoped_lock<std::mutex> lock(_CollisionCallbackMutex);
			_CollisionCallbacks.clear();;
		}

		void DynamicWorld::RegisterCollisionCallback(const CollisionData& data)
		{
			// This will be called from multiple threads
			std::scoped_lock<std::mutex> lock(_CollisionCallbackMutex); 

			_CollisionCallbacks.push_back(std::move(data));
		}

		const std::vector<CollisionData> DynamicWorld::GetCollisionsData()
		{
			std::scoped_lock<std::mutex> lock(_CollisionCallbackMutex);
			return _CollisionCallbacks;
		}

		void DynamicWorld::MoveAndSlideCharacterController(const Entity& entity, const Vector3& velocity)
		{
			const uint32_t entityHandle = entity.GetHandle();
			if (_registeredCharacters.find(entityHandle) != _registeredCharacters.end())
			{
				auto& characterController = _registeredCharacters[entityHandle].Character;
				const auto& joltVelocity = JPH::Vec3(velocity.x, velocity.y, velocity.z);
				characterController->SetLinearVelocity(joltVelocity);

				auto& ghost = _registeredCharacters[entityHandle].Ghost;
				auto ghostPos = _JoltBodyInterface->GetPosition(static_cast<JPH::BodyID>(ghost));
				//std::cout << "Ghost pos: " << ghostPos.GetX() << ", " << ghostPos.GetY() << ", " << ghostPos.GetZ() << std::endl;

				auto charPos = characterController->GetPosition();
				//std::cout << "Char pos: " << charPos.GetX() << ", " << charPos.GetY() << ", " << charPos.GetZ() << std::endl;
				//_JoltBodyInterface->SetLinearVelocity(static_cast<JPH::BodyID>(ghost), joltVelocity);
			}
		}

		void DynamicWorld::AddForceToRigidBody(Entity& entity, const Vector3& force)
		{
			auto& bodyInterface = _JoltPhysicsSystem->GetBodyInterface();
			for (const auto& body : _registeredBodies)
			{
				auto bodyId = static_cast<JPH::BodyID>(body);
				auto entityId = bodyInterface.GetUserData(bodyId);
				if (entityId == entity.GetHandle())
				{
					bodyInterface.AddForce(bodyId, JPH::Vec3(force.x, force.y, force.z));
					return;
				}
			}

			//Logger::Log("Failed to add force to rigidbody. Body not found with id: " + std::to_string(entity.GetHandle()), "physics", WARNING);
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
				JPH::BoxShapeSettings shapeSettings(JPH::Vec3(boxSize.x, boxSize.y, boxSize.z), 0.01f);
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
				JPH::CapsuleShapeSettings shapeSettings(height / 4.0f, radius);
				result = shapeSettings.Create();
			}
			break;
			case RigidbodyShapes::CYLINDER:
			{
				Cylinder* capsule = (Cylinder*)shape.get();
				const float radius = capsule->GetRadius();
				const float height = capsule->GetHeight();
				JPH::CylinderShapeSettings shapeSettings(height / 4.0f, radius);
				result = shapeSettings.Create();
			}
			break;
			case RigidbodyShapes::MESH:
			{
				assert(true);
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

			if (!result.IsValid())
			{
				const std::string errorMessage = std::string("Failed to create physics shape: ") + result.GetError().c_str();
				Logger::Log(errorMessage, "physics", WARNING);

				return nullptr;
			}

			return result.Get();
		}
	}
}
