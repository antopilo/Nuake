#include "BulletDebugDrawer.h"
#include "../../Rendering/Renderer.h"
#include <GL/glew.h>

void BulletDebugDrawer::drawLine(const btVector3& from, const btVector3& to, const btVector3& color)
{
	TransformComponent tc = TransformComponent();

	tc.Translation = glm::vec3(from.x(), from.y(), from.z());
	tc.Scale = glm::vec3(1.0f);
	tc.Rotation = glm::vec3(0.0f);

	glBegin(GL_LINES);
	glColor3f((float)(color.x()), (float)(color.y()), (float)(color.z()));
	glVertex3f((float)(from.x()), (float)(from.y()), (float)(from.z()));
	glVertex3f((float)(to.x()), (float)(to.y()), (float)(to.z()));
	glEnd();

	//Renderer::DrawCube(tc, glm::vec4(color.x(), color.y(), color.z(), 1.0f));
}

void BulletDebugDrawer::drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime, const btVector3& color)
{
	if (lifeTime > 3.0f)
		return;
	glBegin(GL_POINTS);
	glColor3f(color.x(), color.y(), color.z());
	glVertex3f(PointOnB.x(), PointOnB.y(), PointOnB.z());
	glEnd();
}

