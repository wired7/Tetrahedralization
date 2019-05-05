#pragma once
#include "ImplicitGeometry.h"
#include "GraphicsObject.h"

static class DiscreteGeometryUtils
{
public:
	static std::vector<ImplicitGeo::Triangle> getTrianglesFromMesh(Graphics::DecoratedGraphicsObject* object);
	static std::pair<glm::vec3, glm::vec3> getBoundingBox(Graphics::DecoratedGraphicsObject* object);
	static bool isPointInsideMesh(glm::vec3 point, const std::vector<ImplicitGeo::Triangle>& triangles);
};

