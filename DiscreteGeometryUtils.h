#pragma once
#include "ImplicitGeometry.h"
#include "GraphicsObject.h"

namespace DiscreteGeometryUtils
{
	std::vector<ImplicitGeo::Triangle> getTrianglesFromMesh(Graphics::DecoratedGraphicsObject* object);
	std::pair<glm::vec3, glm::vec3> getBoundingBox(Graphics::DecoratedGraphicsObject* object);
	bool isPointInsideMesh(glm::vec3 point, const std::vector<ImplicitGeo::Triangle>& triangles);
};

