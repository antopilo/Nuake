#include "DebugCmd.h"

#include "VulkanRenderer.h"
#include "Nuake/Rendering/Vulkan/VkResources.h"

using namespace Nuake;

Ref<Scene> DebugCmd::GetScene() const
{
	return ctx.scene;
}

DebugCmd::DebugCmd(Cmd& inCmd, PassRenderContext& inCtx) : 
	cmd(inCmd), ctx(inCtx), debugConstant({})
{
}

void DebugCmd::DrawQuad(const Matrix4& transform)
{
	debugConstant.Transform = transform;
	debugConstant.TextureID = -1;
	cmd.PushConstants(ctx.renderPass->PipelineLayout, sizeof(DebugConstant), &debugConstant);

	auto& quadMesh = VkSceneRenderer::QuadMesh;
	cmd.BindDescriptorSet(ctx.renderPass->PipelineLayout, quadMesh->GetDescriptorSet(), 1);
	cmd.BindIndexBuffer(quadMesh->GetIndexBuffer()->GetBuffer());
	cmd.DrawIndexed(6);
}

void DebugCmd::DrawTexturedQuad(const Matrix4& transform, Ref<VulkanImage> texture)
{
	debugConstant.Transform = transform;
	debugConstant.TextureID = GPUResources::Get().GetBindlessTextureID(texture->GetID());

	cmd.PushConstants(ctx.renderPass->PipelineLayout, sizeof(DebugConstant), &debugConstant);

	auto& quadMesh = VkSceneRenderer::QuadMesh;
	cmd.BindDescriptorSet(ctx.renderPass->PipelineLayout, quadMesh->GetDescriptorSet(), 1);
	cmd.BindIndexBuffer(quadMesh->GetIndexBuffer()->GetBuffer());
	cmd.DrawIndexed(6);
}

void DebugCmd::DrawLine(const Vector3& start, const Vector3& end, const Color& color) const
{
	
}

void DebugCmd::DrawSphere(const Vector2& position, float radius, const Color& color) const
{
}

void DebugCmd::DrawCube(const Vector3& position, const Vector3& size, const Color& color) const
{
}

void DebugCmd::DrawAABB(const Vector3& min, const Vector3& max, const Color& color) const
{
}