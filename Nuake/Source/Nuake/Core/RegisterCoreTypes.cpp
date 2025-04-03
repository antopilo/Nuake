#include "RegisterCoreTypes.h"

#include "Nuake/Scene/Components/BoneComponent.h"
#include "Nuake/Scene/Components/BoxCollider.h"
#include "Nuake/Scene/Components/CameraComponent.h"
#include "Nuake/Scene/Components/CapsuleColliderComponent.h"
#include "Nuake/Scene/Components/CharacterControllerComponent.h"
#include "Nuake/Scene/Components/CylinderColliderComponent.h"
#include "Nuake/Scene/Components/MeshCollider.h"
#include "Nuake/Scene/Components/ModelComponent.h"
#include "Nuake/Scene/Components/NavMeshVolumeComponent.h"
#include "Nuake/Scene/Components/NetScriptComponent.h"
#include "Nuake/Scene/Components/ParticleEmitterComponent.h"
#include "Nuake/Scene/Components/QuakeMap.h"
#include "Nuake/Scene/Components/RigidbodyComponent.h"
#include "Nuake/Scene/Components/SkinnedModelComponent.h"
#include "Nuake/Scene/Components/SphereCollider.h"
#include "Nuake/Scene/Components/SpriteComponent.h"
#include "Nuake/Scene/Components/UIComponent.h"
#include "Nuake/Scene/Components/SkyComponent.h"
#include "Nuake/Scene/Components/EnvironmentComponent.h"


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
        SkyComponent::InternalInitializeClass();
        EnvironmentComponent::InternalInitializeClass();
    }
}

