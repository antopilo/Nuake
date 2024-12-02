#include "RegisterCoreTypes.h"

#include "src/Scene/Components/AudioEmitterComponent.h"
#include "src/Scene/Components/BoneComponent.h"
#include "src/Scene/Components/BoxCollider.h"
#include "src/Scene/Components/CameraComponent.h"
#include "src/Scene/Components/CapsuleColliderComponent.h"
#include "src/Scene/Components/CharacterControllerComponent.h"
#include "src/Scene/Components/CylinderColliderComponent.h"
#include "src/Scene/Components/MeshCollider.h"
#include "src/Scene/Components/ModelComponent.h"
#include "src/Scene/Components/NavMeshVolumeComponent.h"
#include "src/Scene/Components/NetScriptComponent.h"
#include "src/Scene/Components/ParticleEmitterComponent.h"
#include "src/Scene/Components/QuakeMap.h"
#include "src/Scene/Components/RigidbodyComponent.h"
#include "src/Scene/Components/SkinnedModelComponent.h"
#include "src/Scene/Components/SphereCollider.h"
#include "src/Scene/Components/SpriteComponent.h"
#include "src/Scene/Components/UIComponent.h"
#include "src/Scene/Components/SkyComponent.h"
#include "src/Scene/Components/EnvironmentComponent.h"


namespace Nuake
{
    void RegisterCoreTypes::RegisterCoreComponents()
    {
        UIComponent::InternalInitializeClass();
        SpriteComponent::InternalInitializeClass();
        SphereColliderComponent::InternalInitializeClass();
        SkinnedModelComponent::InternalInitializeClass();
        RigidBodyComponent::InternalInitializeClass();
        QuakeMapComponent::InternalInitializeClass();
        ParticleEmitterComponent::InternalInitializeClass();
        NetScriptComponent::InternalInitializeClass();
        NavMeshVolumeComponent::InternalInitializeClass();
        ModelComponent::InternalInitializeClass();
        MeshColliderComponent::InternalInitializeClass();
        LightComponent::InternalInitializeClass();
        CylinderColliderComponent::InternalInitializeClass();
        CharacterControllerComponent::InternalInitializeClass();
        CapsuleColliderComponent::InternalInitializeClass();
        CameraComponent::InternalInitializeClass();
        BoxColliderComponent::InternalInitializeClass();
        BoneComponent::InternalInitializeClass();
        AudioEmitterComponent::InternalInitializeClass();
        SkyComponent::InternalInitializeClass();
        EnvironmentComponent::InternalInitializeClass();
    }
}

