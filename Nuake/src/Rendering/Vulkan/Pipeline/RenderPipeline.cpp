#include "RenderPipeline.h"

using namespace Nuake;

RenderPass::RenderPass(const std::string& name) :
	Name(name)
{
}

PassAttachment& RenderPass::AddAttachment(const std::string& name, ImageFormat format)
{
	auto newAttachment = PassAttachment(name, format);
	Attachments.push_back(std::move(newAttachment));
}

void RenderPass::AddInput(const std::string& name)
{
	Inputs.push_back(name);
}

RenderPipeline::RenderPipeline(const Vector2& size) :
	Size(size)
{
}

RenderPass& RenderPipeline::AddPass(const std::string& name)
{
	auto newPass = RenderPass(name);
	RenderPasses.push_back(std::move(newPass));
}

PassAttachment::PassAttachment(const std::string& name, ImageFormat format) :
	Name(name),
	Format(format)
{
}

