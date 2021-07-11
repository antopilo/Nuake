#pragma once
#include "btBulletCollisionCommon.h"

namespace Nuake
{
	class BulletDebugDrawer : public btIDebugDraw
	{
		void drawLine(const btVector3& from, const btVector3& to, const btVector3& color) override;

		int getDebugMode() const override { return 1; }

		void draw3dText(const btVector3& location, const char* textString) override {}

		void reportErrorWarning(const char* warningString) override {  };

		void setDebugMode(int debugMode) {};

		void drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime, const btVector3& color) override;
	};
}
