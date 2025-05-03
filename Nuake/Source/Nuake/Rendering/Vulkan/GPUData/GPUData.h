#pragma once
#include "Nuake/Core/Maths.h"

#include <array>

namespace Nuake
{
	struct TransformData
	{
		std::array<Matrix4, 3000> Data;
	};

	struct MaterialBufferStruct
	{
		int HasAlbedo;
		Vector3 AlbedoColor;
		int HasNormal;
		int HasMetalness;
		int HasRoughness;
		int HasAO;
		float MetalnessValue;
		float RoughnessValue;
		float AoValue;
		uint32_t AlbedoTextureId;
		uint32_t NormalTextureId;
		uint32_t MetalnessTextureId;
		uint32_t RoughnessTextureId;
		uint32_t AoTextureId;
		int SamplerType;
		int ReceiveShadow;
		int CastShadow;
		int Unlit;
		int AlphaScissor;
		int pad[3];
	};

	struct LightData
	{
		Vector3 Position;
		int Type;
		Vector4 Color;
		Vector3 Direction;
		float OuterConeAngle;
		float InnerConeAngle;
		int CastShadow;
		int ShadowMapTextureId[4];
		int TransformId[4];
		float pad[2];
	};

	struct CameraView
	{
		Matrix4 View;
		Matrix4 Projection;
		Matrix4 ViewProjection;
		Matrix4 InverseView;
		Matrix4 InverseProjection;
		Vector3 Position;
		float Near;
		float Far;
		float pad;
		float pad2;
		float pad3;
		//char padding[64];            // 124 bytes to reach 128 bytes
	};
}