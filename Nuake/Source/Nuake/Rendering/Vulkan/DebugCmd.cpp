#include "DebugCmd.h"

#include "VulkanRenderer.h"
#include "Nuake/Rendering/Vulkan/VkResources.h"

using namespace Nuake;


DebugCmd::DebugCmd(Cmd& inCmd, PassRenderContext& inCtx) : 
	cmd(inCmd), ctx(inCtx), debugConstant({})
{
}

Ref<Scene> DebugCmd::GetScene() const
{
	return ctx.scene;
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

DebugLineCmd::DebugLineCmd(Cmd& inCmd, PassRenderContext& inCtx) :
	cmd(inCmd), ctx(inCtx), lineConstant({})
{
}

Ref<Scene> DebugLineCmd::GetScene() const
{
	return ctx.scene;
}

void DebugLineCmd::DrawLine(const Matrix4& transform, const Color& inColor, float lineWidth)
{
	cmd.SetPolygonMode(VK_POLYGON_MODE_LINE);
	cmd.SetPrimitiveTopology(VK_PRIMITIVE_TOPOLOGY_LINE_LIST);
	cmd.SetLineStipple(2, 0b1110110100000000);
	cmd.SetLineStippleEnabled(true);
	lineConstant.LineColor = inColor;
	lineConstant.Transform = transform;
	cmd.PushConstants(ctx.renderPass->PipelineLayout, sizeof(LineConstant), &lineConstant);

	cmd.SetLineWidth(lineWidth);

	auto& quadMesh = VkSceneRenderer::QuadMesh;
	cmd.BindDescriptorSet(ctx.renderPass->PipelineLayout, quadMesh->GetDescriptorSet(), 1);
	cmd.BindIndexBuffer(quadMesh->GetIndexBuffer()->GetBuffer());
	cmd.DrawIndexed(6);
}
