#pragma once
#include "ParametricShapes.h"
#include <vector>
#include <utility>
#include <glm.hpp>

namespace Graphics
{
	class DecoratedGraphicsObject;
}

namespace DiscreteGeometryUtils
{
	std::vector<Parametric::Triangle> getTrianglesFromMesh(Graphics::DecoratedGraphicsObject* object);
	std::pair<glm::vec3, glm::vec3> getBoundingBox(Graphics::DecoratedGraphicsObject* object);
	bool isPointInsideMesh(glm::vec3 point, const std::vector<Parametric::Triangle>& triangles);
};

