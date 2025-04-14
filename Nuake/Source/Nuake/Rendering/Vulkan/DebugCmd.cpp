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

void DebugCmd::DrawTexturedQuad(const Matrix4& transform, Ref<VulkanImage> texture, const Color& color, int32_t entityId)
{
	debugConstant.Transform = transform;
	debugConstant.TextureID = GPUResources::Get().GetBindlessTextureID(texture->GetID());
	debugConstant.Color = color;
	debugConstant.EntityID = entityId; 

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
	cmd.SetLineRasterizationMode(VK_LINE_RASTERIZATION_MODE_RECTANGULAR_SMOOTH);
	cmd.SetPrimitiveTopology(VK_PRIMITIVE_TOPOLOGY_LINE_LIST);
	cmd.SetLineStipple(2, 0b1111111100000000);
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

void DebugLineCmd::DrawBox(const Matrix4& transform, const Color& inColor, float lineWidth, bool stippled)
{
	cmd.SetPolygonMode(VK_POLYGON_MODE_LINE);
	cmd.SetLineRasterizationMode(VK_LINE_RASTERIZATION_MODE_RECTANGULAR_SMOOTH);
	cmd.SetPrimitiveTopology(VK_PRIMITIVE_TOPOLOGY_LINE_LIST);

	cmd.SetLineStippleEnabled(stippled);
	if (stippled)
	{
		cmd.SetLineStipple(2, 0b1111111100000000);
	}

	cmd.SetLineWidth(lineWidth);

	lineConstant.LineColor = inColor;
	lineConstant.Transform = transform;
	cmd.PushConstants(ctx.renderPass->PipelineLayout, sizeof(LineConstant), &lineConstant);

	auto& boxMesh = VkSceneRenderer::BoxMesh;
	cmd.BindDescriptorSet(ctx.renderPass->PipelineLayout, boxMesh->GetDescriptorSet(), 1);
	cmd.BindIndexBuffer(boxMesh->GetIndexBuffer()->GetBuffer());
	cmd.DrawIndexed(24);
}

void DebugLineCmd::DrawCapsule(const Matrix4& transform, const Color& inColor, float lineWidth, bool stippled)
{
	cmd.SetPolygonMode(VK_POLYGON_MODE_LINE);
	cmd.SetLineRasterizationMode(VK_LINE_RASTERIZATION_MODE_RECTANGULAR_SMOOTH);
	cmd.SetPrimitiveTopology(VK_PRIMITIVE_TOPOLOGY_LINE_LIST);

	cmd.SetLineStippleEnabled(stippled);
	if (stippled)
	{
		cmd.SetLineStipple(2, 0b1111111100000000);
	}

	cmd.SetLineWidth(lineWidth);

	lineConstant.LineColor = inColor;
	lineConstant.Transform = transform;
	cmd.PushConstants(ctx.renderPass->PipelineLayout, sizeof(LineConstant), &lineConstant);

	auto& capsuleMesh = VkSceneRenderer::CapsuleMesh;
	cmd.BindDescriptorSet(ctx.renderPass->PipelineLayout, capsuleMesh->GetDescriptorSet(), 1);
	cmd.BindIndexBuffer(capsuleMesh->GetIndexBuffer()->GetBuffer());
	cmd.DrawIndexed(capsuleMesh->GetIndexBuffer()->GetSize() / sizeof(uint32_t));
}

void DebugLineCmd::DrawSphere(const Matrix4& transform, const Color& inColor, float lineWidth, bool stippled)
{
	cmd.SetPolygonMode(VK_POLYGON_MODE_LINE);
	cmd.SetLineRasterizationMode(VK_LINE_RASTERIZATION_MODE_RECTANGULAR_SMOOTH);
	cmd.SetPrimitiveTopology(VK_PRIMITIVE_TOPOLOGY_LINE_LIST);

	cmd.SetLineStippleEnabled(stippled);
	if (stippled)
	{
		cmd.SetLineStipple(2, 0b1111111100000000);
	}

	cmd.SetLineWidth(lineWidth);

	lineConstant.LineColor = inColor;
	lineConstant.Transform = transform;
	cmd.PushConstants(ctx.renderPass->PipelineLayout, sizeof(LineConstant), &lineConstant);

	auto& sphereMesh = VkSceneRenderer::SphereMesh;
	cmd.BindDescriptorSet(ctx.renderPass->PipelineLayout, sphereMesh->GetDescriptorSet(), 1);
	cmd.BindIndexBuffer(sphereMesh->GetIndexBuffer()->GetBuffer());
	cmd.DrawIndexed(sphereMesh->GetIndexBuffer()->GetSize() / sizeof(uint32_t));
}

void DebugLineCmd::DrawCylinder(const Matrix4& transform, const Color& inColor, float lineWidth, bool stippled)
{
	cmd.SetPolygonMode(VK_POLYGON_MODE_LINE);
	cmd.SetLineRasterizationMode(VK_LINE_RASTERIZATION_MODE_RECTANGULAR_SMOOTH);
	cmd.SetPrimitiveTopology(VK_PRIMITIVE_TOPOLOGY_LINE_LIST);

	cmd.SetLineStippleEnabled(stippled);
	if (stippled)
	{
		cmd.SetLineStipple(2, 0b1111111100000000);
	}

	cmd.SetLineWidth(lineWidth);

	lineConstant.LineColor = inColor;
	lineConstant.Transform = transform;
	cmd.PushConstants(ctx.renderPass->PipelineLayout, sizeof(LineConstant), &lineConstant);

	auto& sphereMesh = VkSceneRenderer::CylinderMesh;
	cmd.BindDescriptorSet(ctx.renderPass->PipelineLayout, sphereMesh->GetDescriptorSet(), 1);
	cmd.BindIndexBuffer(sphereMesh->GetIndexBuffer()->GetBuffer());
	cmd.DrawIndexed(sphereMesh->GetIndexBuffer()->GetSize() / sizeof(uint32_t));
}

void DebugLineCmd::DrawArrow(const Vector3& from, const Vector3& to, const Matrix4& view, const Matrix4& proj, const Color& inColor, float lineWidth, bool stippled)
{
	cmd.SetPolygonMode(VK_POLYGON_MODE_LINE);
	cmd.SetLineRasterizationMode(VK_LINE_RASTERIZATION_MODE_RECTANGULAR_SMOOTH);
	cmd.SetPrimitiveTopology(VK_PRIMITIVE_TOPOLOGY_LINE_LIST);

	cmd.SetLineStippleEnabled(stippled);
	if (stippled)
	{
		cmd.SetLineStipple(2, 0b1111111100000000);
	}

	cmd.SetLineWidth(lineWidth);

	lineConstant.Transform = proj * view;

	lineConstant.LineColor = inColor;
	cmd.PushConstants(ctx.renderPass->PipelineLayout, sizeof(LineConstant), &lineConstant);

	auto& sphereMesh = VkSceneRenderer::ArrowMesh;
	cmd.BindDescriptorSet(ctx.renderPass->PipelineLayout, sphereMesh->GetDescriptorSet(), 1);
	cmd.BindIndexBuffer(sphereMesh->GetIndexBuffer()->GetBuffer());
	cmd.DrawIndexed(sphereMesh->GetIndexBuffer()->GetSize() / sizeof(uint32_t));
}

void DebugLineCmd::DrawCone(const Matrix4& transform, const Color& inColor, float lineWidth, bool stippled)
{
	cmd.SetPolygonMode(VK_POLYGON_MODE_LINE);
	cmd.SetLineRasterizationMode(VK_LINE_RASTERIZATION_MODE_RECTANGULAR_SMOOTH);
	cmd.SetPrimitiveTopology(VK_PRIMITIVE_TOPOLOGY_LINE_LIST);

	cmd.SetLineStippleEnabled(stippled);
	if (stippled)
	{
		cmd.SetLineStipple(2, 0b1111111100000000);
	}

	cmd.SetLineWidth(lineWidth);

	lineConstant.LineColor = inColor;
	lineConstant.Transform = transform;
	cmd.PushConstants(ctx.renderPass->PipelineLayout, sizeof(LineConstant), &lineConstant);

	auto& coneMesh = VkSceneRenderer::ConeMesh;
	cmd.BindDescriptorSet(ctx.renderPass->PipelineLayout, coneMesh->GetDescriptorSet(), 1);
	cmd.BindIndexBuffer(coneMesh->GetIndexBuffer()->GetBuffer());
	cmd.DrawIndexed(coneMesh->GetIndexBuffer()->GetSize() / sizeof(uint32_t));
}